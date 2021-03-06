/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"
#include <sa/PragmaWarning.h>
#define USE_STANDALONE_ASIO
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4457 4456 4150)
#include <SimpleWeb/client_https.hpp>
PRAGMA_WARNING_POP
#include <iostream>
#include <fstream>
#include "Random.h"
#include <thread>

namespace Client {

class HttpsClient : public SimpleWeb::Client<SimpleWeb::HTTPS>
{
public:
    HttpsClient(const std::string& server_port_path,
        bool verify_certificate = true,
        const std::string& cert_file = std::string(),
        const std::string& private_key_file = std::string(),
        const std::string& verify_file = std::string()) :
        SimpleWeb::Client<SimpleWeb::HTTPS>::Client(server_port_path, verify_certificate,
            cert_file, private_key_file, verify_file)
    { }
    virtual ~HttpsClient();
};

HttpsClient::~HttpsClient() = default;

Client::Client(Receiver& receiver) :
    receiver_(receiver),
    ioService_(std::make_shared<asio::io_service>()),
    loginHost_("127.0.0.1"),
    loginPort_(2748)
{
    Utils::Random::Instance.Initialize();
    // Always create new keys
    dhKeys_.GenerateKeys();
}

Client::~Client()
{
    if (httpClient_)
        delete httpClient_;
    Terminate();
}

void Client::ResetPoll()
{
    // Blocking!
    // Reset must always be called prior to poll
    ioService_->reset();
    ioService_->poll();
}

void Client::Poll()
{
    ioService_->poll();
}

void Client::Run()
{
#ifdef _WIN32
    ioService_->run();
#else
    if (state_ != State::World)
    {
        // WTF, why is this needed on Linux but not on Windows?
        if (ioService_->stopped())
            ioService_->reset();
    }
    ioService_->poll();
#endif // _WIN32
}

void Client::Terminate()
{
    ioService_->stop();
    Connection::Terminate();
}

void Client::OnLoggedIn(const std::string& accountUuid, const std::string& authToken, AB::Entities::AccountType accType)
{
    accountUuid_ = accountUuid;
    authToken_ = authToken;
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    else
        // If game host is empty use the login host
        gameHost_ = loginHost_;

    filePort_ = protoLogin_->filePort_;
    if (!protoLogin_->fileHost_.empty())
        fileHost_ = protoLogin_->fileHost_;
    else
        // If file host is empty use the login host
        fileHost_ = loginHost_;

    if (!fileHost_.empty() && filePort_ != 0)
    {
        std::stringstream ss;
        ss << fileHost_ << ":" << filePort_;
        httpClient_ = new HttpsClient(ss.str(), false);
    }

    receiver_.OnLoggedIn(accountUuid_, authToken_, accType);
}

void Client::OnGetCharlist(const AB::Entities::CharList& chars)
{
    state_ = State::SelectChar;

    receiver_.OnGetCharlist(chars);

    // Get list of outposts
    GetOutposts();
}

void Client::OnGetOutposts(const std::vector<AB::Entities::Game>& games)
{

    receiver_.OnGetOutposts(games);
}

void Client::OnGetServices(const std::vector<AB::Entities::Service>& services)
{
    receiver_.OnGetServices(services);
}

void Client::OnAccountCreated()
{
    receiver_.OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    receiver_.OnPlayerCreated(uuid, mapUuid);
}

void Client::OnAccountKeyAdded()
{
    receiver_.OnAccountKeyAdded();
}

void Client::OnCharacterDeleted(const std::string& uuid)
{
    receiver_.OnCharacterDeleted(uuid);;
}

void Client::OnLog(const std::string& message)
{
    receiver_.OnLog(message);
}

void Client::OnNetworkError(ConnectionError connectionError, const std::error_code& err)
{
    receiver_.OnNetworkError(connectionError, err);
}

void Client::OnProtocolError(AB::ErrorCodes err)
{
    receiver_.OnProtocolError(err);
}

void Client::OnPong(int lastPing)
{
    gotPong_ = true;
    pings_.Enqueue(lastPing);
}

ProtocolLogin& Client::GetProtoLogin()
{
    if (!protoLogin_)
    {
        protoLogin_ = std::make_shared<ProtocolLogin>(dhKeys_, *ioService_);
        protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1, std::placeholders::_2));
        protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    }
    assert(protoLogin_);
    return *protoLogin_;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == State::Disconnected || state_ == State::CreateAccount))
        return;

    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    GetProtoLogin().Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnLoggedIn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        std::bind(&Client::OnGetCharlist, this, std::placeholders::_1));
}

void Client::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    if (state_ != State::CreateAccount)
        return;

    accountName_ = name;
    password_ = pass;

    GetProtoLogin().CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
}

void Client::CreatePlayer(const std::string& charName, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (state_ != State::SelectChar)
        return;

    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().CreatePlayer(loginHost_, loginPort_, accountUuid_, authToken_,
        charName, profUuid, modelIndex, sex, isPvp,
        std::bind(&Client::OnPlayerCreated, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::DeleteCharacter(const std::string& uuid)
{
    if (accountUuid_.empty() || authToken_.empty())
        return;
    GetProtoLogin().DeleteCharacter(loginHost_, loginPort_, accountUuid_, authToken_,
        uuid,
        std::bind(&Client::OnCharacterDeleted, this, std::placeholders::_1));
}

void Client::AddAccountKey(const std::string& newKey)
{
    if (state_ != State::SelectChar)
        return;

    if (accountUuid_.empty() || authToken_.empty())
        return;
    if (newKey.empty())
        return;
    GetProtoLogin().AddAccountKey(loginHost_, loginPort_, accountUuid_, authToken_,
        newKey,
        std::bind(&Client::OnAccountKeyAdded, this));
}

void Client::Logout()
{
    if (state_ != State::World)
        return;
    if (protoGame_)
    {
        state_ = State::Disconnected;
        protoGame_->Logout();
        Run();
    }
}

void Client::GetOutposts()
{
    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().GetOutposts(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetOutposts, this, std::placeholders::_1));
}

void Client::GetServers()
{
    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().GetServers(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetServices, this, std::placeholders::_1));
}

void Client::EnterWorld(const std::string& charUuid, const std::string& mapUuid,
    const std::string& host /* = "" */, uint16_t port /* = 0 */, const std::string& instanceId /* = "" */)
{
    assert(!accountUuid_.empty());
    assert(!authToken_.empty());
    // Enter or changing the world
    if (state_ != State::SelectChar && state_ != State::World)
        return;

    if (state_ == State::World)
    {
        // We are already logged in to some world so we must logout
        Logout();
        gotPong_ = true;
    }

    // Maybe different server
    if (!host.empty())
        gameHost_ = host;
    if (port != 0)
        gamePort_ = port;

    // 2. Login to game server
    if (!protoGame_)
        protoGame_ = std::make_shared<ProtocolGame>(*this, dhKeys_, *ioService_);

    protoGame_->Login(accountUuid_, authToken_, charUuid, mapUuid, instanceId,
        gameHost_, gamePort_);
}

void Client::Update(int timeElapsed)
{
    if (state_ == State::World)
    {
        if ((lastPing_ >= 1000 && gotPong_) || (lastPing_ > 5000))
        {
            // Send every second a Ping. If we didn't get a pong the last 5 seconds also send a ping.
            if (protoGame_)
            {
                gotPong_ = false;
                protoGame_->Ping();
            }
            lastPing_ = 0;
        }
    }

    lastRun_ += timeElapsed;
    if (lastRun_ >= 16)
    {
        // Don't send more than ~60 updates to the server, it might DC.
        // If running @144Hz every 2nd Update. If running @60Hz every update
        lastRun_ = 0;
        Run();
    }
    if (state_ == State::World)
        lastPing_ += timeElapsed;
}

bool Client::HttpRequest(const std::string& path, std::ostream& out)
{
    if (httpClient_ == nullptr)
        return false;
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Connection", "keep-alive");
    std::stringstream ss;
    ss << accountUuid_ << authToken_;
    header.emplace("Auth", ss.str());

    auto get = [&]() -> bool
    {
        try
        {
            auto r = httpClient_->request("GET", path, "", header);
            if (r->status_code != "200 OK")
                return false;
            out << r->content.rdbuf();
            return true;
        }
        catch (...)
        {
            return false;
        }
    };

    if (get())
        return true;

    // Try once again
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    if (get())
        return true;

    return false;
}

bool Client::HttpDownload(const std::string& path, const std::string& outFile)
{
    std::remove(outFile.c_str());
    std::ofstream f;
    f.open(outFile);
    if (!f.is_open())
        return false;
    bool ret = HttpRequest(path, f);
    f.close();
    return ret;
}

uint32_t Client::GetIp() const
{
    if (protoGame_ && protoGame_->IsConnected())
        return protoGame_->GetIp();
    return 0;
}

int64_t Client::GetClockDiff() const
{
    if (protoGame_)
        return protoGame_->GetClockDiff();
    return 0;
}

void Client::ChangeMap(const std::string& mapUuid)
{
    if (state_ == State::World)
        protoGame_->ChangeMap(mapUuid);
}

void Client::GetMailHeaders()
{
    if (state_ == State::World)
        protoGame_->GetMailHeaders();
}

void Client::GetMail(const std::string& mailUuid)
{
    if (state_ == State::World)
        protoGame_->GetMail(mailUuid);
}

void Client::GetInventory()
{
    if (state_ == State::World)
        protoGame_->GetInventory();
}

void Client::InventoryStoreItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->InventoryStoreItem(pos);
}

void Client::InventoryDestroyItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->InventoryDestroyItem(pos);
}

void Client::InventoryDropItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->InventoryDropItem(pos);
}

void Client::GetChest()
{
    if (state_ == State::World)
        protoGame_->GetChest();
}

void Client::ChestDestroyItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->ChestDestroyItem(pos);
}

void Client::DeleteMail(const std::string& mailUuid)
{
    if (state_ == State::World)
        protoGame_->DeleteMail(mailUuid);
}

void Client::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    if (state_ == State::World)
        protoGame_->SendMail(recipient, subject, body);
}

void Client::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    if (state_ == State::World)
        protoGame_->GetPlayerInfoByName(name, fields);
}

void Client::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    if (state_ == State::World)
        protoGame_->GetPlayerInfoByAccount(accountUuid, fields);
}

void Client::Move(uint8_t direction)
{
    if (state_ == State::World)
        protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    if (state_ == State::World)
        protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    if (state_ == State::World)
        protoGame_->SetDirection(rad);
}

void Client::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->ClickObject(sourceId, targetId);
}

void Client::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->SelectObject(sourceId, targetId);
}

void Client::FollowObject(uint32_t targetId, bool ping)
{
    if (state_ == State::World)
        protoGame_->Follow(targetId, ping);
}

void Client::Command(AB::GameProtocol::CommandType type, const std::string& data)
{
    if (state_ == State::World)
        protoGame_->Command(type, data);
}

void Client::GotoPos(const Vec3& pos)
{
    if (state_ == State::World)
        protoGame_->GotoPos(pos);
}

void Client::PartyInvitePlayer(uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->PartyInvitePlayer(targetId);
}

void Client::PartyKickPlayer(uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->PartyKickPlayer(targetId);
}

void Client::PartyAcceptInvite(uint32_t inviterId)
{
    if (state_ == State::World)
        protoGame_->PartyAcceptInvite(inviterId);
}

void Client::PartyRejectInvite(uint32_t inviterId)
{
    if (state_ == State::World)
        protoGame_->PartyRejectInvite(inviterId);
}

void Client::PartyGetMembers(uint32_t partyId)
{
    if (state_ == State::World)
        protoGame_->PartyGetMembers(partyId);
}

void Client::PartyLeave()
{
    if (state_ == State::World)
        protoGame_->PartyLeave();
}

void Client::UseSkill(uint32_t index, bool ping)
{
    if (state_ == State::World)
        protoGame_->UseSkill(index, ping);
}

void Client::Attack(bool ping)
{
    if (state_ == State::World)
        protoGame_->Attack(ping);
}

void Client::QueueMatch()
{
    if (state_ == State::World)
        protoGame_->QueueMatch();
}

void Client::UnqueueMatch()
{
    if (state_ == State::World)
        protoGame_->UnqueueMatch();
}

void Client::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    if (state_ == State::World)
        protoGame_->AddFriend(name, relation);
}

void Client::RemoveFriend(const std::string& accountUuid)
{
    if (state_ == State::World)
        protoGame_->RemoveFriend(accountUuid);
}

void Client::UpdateFriendList()
{
    if (state_ == State::World)
        protoGame_->UpdateFriendList();
}

void Client::Cancel()
{
    if (state_ == State::World)
        protoGame_->Cancel();
}

void Client::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (state_ == State::World)
        protoGame_->SetPlayerState(newState);
}

void Client::SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status)
{
    if (state_ == State::World)
        protoGame_->SetOnlineStatus(status);
}

void Client::SetSecondaryProfession(uint32_t profIndex)
{
    if (state_ == State::World)
        protoGame_->SetSecondaryProfession(profIndex);
}

void Client::SetAttributeValue(uint32_t attribIndex, uint8_t value)
{
    if (state_ == State::World)
        protoGame_->SetAttributeValue(attribIndex, value);
}

void Client::EquipSkill(uint32_t skillIndex, uint8_t pos)
{
    if (state_ == State::World)
        protoGame_->EquipSkill(skillIndex, pos);
}

void Client::LoadSkillTemplate(const std::string& templ)
{
    if (state_ == State::World)
        protoGame_->LoadSkillTemplate(templ);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet)
{
    state_ = State::World;
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPosUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GameError& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetPosition& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectGroupMaskChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectResourceChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::DialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendList& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GuildInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GuildMemberList& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::SetObjectAttributeValue& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSecProfessionChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetSkill& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::SkillTemplateLoaded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

}
