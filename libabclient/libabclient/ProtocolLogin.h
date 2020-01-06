#pragma once

#include "Protocol.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/Service.h>
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>
#include "Structs.h"
#include <AB/Packets/LoginPackets.h>

namespace Client {

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& accountUuid, const std::string& authToken)> LoggedInCallback;
    typedef std::function<void(const AB::Entities::CharList& chars)> CharlistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Game>& outposts)> GamelistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Service>& services)> ServerlistCallback;
    typedef std::function<void()> CreateAccountCallback;
    typedef std::function<void(const std::string& uuid, const std::string& mapUuid)> CreatePlayerCallback;
    typedef std::function<void()> AccountKeyAddedCallback;
private:
    std::string host_;
    uint16_t port_;
    std::string accountName_;
    std::string accountUuid_;
    std::string password_;
    std::string authToken_;
    std::string email_;
    std::string accKey_;
    std::string addAccountKey_;
    std::string charName_;
    std::string profUuid_;
    uint32_t itemIndex_;
    AB::Entities::CharacterSex sex_;
    bool isPvp_;
    bool firstRecv_;
    LoggedInCallback loggedInCallback_;
    CharlistCallback charlistCallback_;
    GamelistCallback gamelistCallback_;
    ServerlistCallback serverlistCallback_;
    CreateAccountCallback createAccCallback_;
    CreatePlayerCallback createPlayerCallback_;
    AccountKeyAddedCallback accountKeyAddedCallback_;
    void SendLoginPacket();
    void SendCreateAccountPacket();
    void SendCreatePlayerPacket();
    void SendGetOutpostsPacket();
    void SendGetServersPacket();
    void SendAddAccountKeyPacket();
    void ParseMessage(InputMessage& message);

    void HandleCharList(const AB::Packets::Server::Login::CharacterList& packet);
    void HandleOutpostList(const AB::Packets::Server::Login::OutpostList& packet);
    void HandleServerList(const AB::Packets::Server::Login::ServerList& packet);
    void HandleLoginError(const AB::Packets::Server::Login::Error& packet);
    void HandleCreatePlayerSuccess(const AB::Packets::Server::Login::CreateCharacterSuccess& packet);
protected:
    void OnReceive(InputMessage& message) override;
public:
    ProtocolLogin(Crypto::DHKeys& keys, asio::io_service& ioService);
    ~ProtocolLogin() override {}

    void Login(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const LoggedInCallback& onLoggedIn,
        const CharlistCallback& callback);
    void CreateAccount(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const std::string& email, const std::string& accKey,
        const CreateAccountCallback& callback);
    void CreatePlayer(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const std::string& charName, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp,
        const CreatePlayerCallback& callback);
    void AddAccountKey(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const std::string& newAccountKey,
        const AccountKeyAddedCallback& callback);
    void GetOutposts(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const GamelistCallback& callback);
    void GetServers(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const ServerlistCallback& callback);

    std::string gameHost_;
    uint16_t gamePort_;
    std::string fileHost_;
    uint16_t filePort_;
};

}
