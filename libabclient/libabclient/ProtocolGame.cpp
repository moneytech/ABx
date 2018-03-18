#include "stdafx.h"
#include "ProtocolGame.h"
#include "TimeUtils.h"

#include "DebugNew.h"

namespace Client {

ProtocolGame::ProtocolGame() :
    Protocol(),
    pingCallback_(nullptr)
{
    checksumEnabled_ = ProtocolGame::UseChecksum;
    encryptEnabled_ = ENABLE_GAME_ENCRYTION;
}

void ProtocolGame::Login(const std::string& accountName,
    const std::string& accountPass, const std::string& charName,
    const std::string& map,
    const std::string& host, uint16_t port)
{
    accountName_ = accountName;
    accountPass_ = accountPass;
    charName_ = charName;
    map_ = map;

    Connect(host, port);
}

void ProtocolGame::OnConnect()
{
    firstRevc_ = true;
    Protocol::OnConnect();

    SendLoginPacket();

    Receive();
}

void ProtocolGame::OnReceive(const std::shared_ptr<InputMessage>& message)
{
    if (firstRevc_)
    {
        firstRevc_ = false;
    }
    ParseMessage(message);
    Receive();
}

void ProtocolGame::OnError(const asio::error_code& err)
{
    Protocol::OnError(err);
}

void ProtocolGame::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t opCode = 0;
    while (!message->Eof())
    {
        opCode = message->Get<uint8_t>();

        switch (opCode)
        {
        case AB::GameProtocol::Error:
            ParseError(message);
            break;
        case AB::GameProtocol::GameEnter:
            ParseEnterWorld(message);
            break;
        case AB::GameProtocol::GamePong:
            ParsePong(message);
            break;
        case AB::GameProtocol::GameUpdate:
            ParseUpdate(message);
            break;
        case AB::GameProtocol::GameSpawnObjectExisting:
        case AB::GameProtocol::GameSpawnObject:
            ParseSpawnObject(opCode == AB::GameProtocol::GameSpawnObjectExisting, message);
            break;
        case AB::GameProtocol::GameLeaveObject:
            ParseLeaveObject(message);
            break;
        case AB::GameProtocol::GameObjectPositionChange:
            ParseObjectPosUpdate(message);
            break;
        case AB::GameProtocol::GameObjectRotationChange:
            ParseObjectRotUpdate(message);
            break;
        case AB::GameProtocol::GameObjectStateChange:
            ParseObjectStateChange(message);
            break;
        case AB::GameProtocol::GameObjectSelectTarget:
            ParseObjectSelected(message);
            break;
        case AB::GameProtocol::ServerMessage:
            ParseServerMessage(message);
            break;
        case AB::GameProtocol::ChatMessage:
            ParseChatMessage(message);
            break;
        }
    }
}

void ProtocolGame::ParseObjectRotUpdate(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    float rot = message->Get<float>();
    bool manual = message->Get<uint8_t>() != 0;
    if (receiver_)
        receiver_->OnObjectRot(objectId, rot, manual);
}

void ProtocolGame::ParseObjectStateChange(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(message->Get<uint8_t>());
    if (receiver_)
        receiver_->OnObjectStateChange(objectId, state);
}

void ProtocolGame::ParseObjectSelected(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnObjectSelected(sourceId, targetId);
}

void ProtocolGame::ParseServerMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(message->Get<uint8_t>());
    std::string sender = message->GetString();
    std::string data = message->GetString();
    if (receiver_)
        receiver_->OnServerMessage(type, sender, data);
}

void ProtocolGame::ParseChatMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::ChatMessageChannel type =
        static_cast<AB::GameProtocol::ChatMessageChannel>(message->Get<uint8_t>());
    uint32_t senderId = message->Get<uint32_t>();
    std::string sender = message->GetString();
    std::string data = message->GetString();
    if (receiver_)
        receiver_->OnChatMessage(type, senderId, sender, data);
}

void ProtocolGame::ParseObjectPosUpdate(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    Vec3 newPos
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    if (receiver_)
        receiver_->OnObjectPos(objectId, newPos);
}

void ProtocolGame::ParseLeaveObject(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnDespawnObject(objectId);
}

void ProtocolGame::ParseSpawnObject(bool existing, const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();

    Vec3 pos
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    float rot = message->Get<float>();
    Vec3 scale
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    std::string data = message->GetString();
    PropReadStream stream;
    stream.Init(data.c_str(), data.length());
    if (receiver_)
        receiver_->OnSpawnObject(objectId, pos, scale, rot, stream, existing);
}

void ProtocolGame::ParseUpdate(const std::shared_ptr<InputMessage>& message)
{
    /* int64_t tick = */ message->Get<int64_t>();
}

void ProtocolGame::ParsePong(const std::shared_ptr<InputMessage>& message)
{
    AB_UNUSED(message);
    lastPing_ = static_cast<int>(AbTick() - pingTick_);
    if (pingCallback_)
        pingCallback_(lastPing_);
}

void ProtocolGame::ParseError(const std::shared_ptr<InputMessage>& message)
{
    uint8_t error = message->Get<uint8_t>();
    if (error != 0)
        ProtocolError(error);
}

void ProtocolGame::ParseEnterWorld(const std::shared_ptr<InputMessage>& message)
{
    std::string map = message->GetString();
    uint32_t playerId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnEnterWorld(map, playerId);
}

void ProtocolGame::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolGame::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->AddString(accountName_);
    msg->AddString(accountPass_);
    msg->AddString(charName_);
    msg->AddString(map_);
    Send(msg);
}

void ProtocolGame::Logout()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeLogout);
    Send(msg);
    Connection::Poll();
}

void ProtocolGame::Ping(const PingCallback& callback)
{
    pingCallback_ = callback;
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePing);
    pingTick_ = AbTick();
    Send(msg);
}

void ProtocolGame::Move(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeMove);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::Turn(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeTurn);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::SetDirection(float rad)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetDirection);
    msg->Add<float>(rad);
    Send(msg);
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSelect);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCommand);
    msg->Add<uint8_t>(type);
    msg->AddString(data);
    Send(msg);
}

}
