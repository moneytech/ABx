#pragma once

#include <memory>
#include <uuid.h>
#include "Protocol.h"
#include "Connection.h"
#include "Dispatcher.h"
#include <AB/ProtocolCodes.h>
#include "Subsystems.h"
#include "InputQueue.h"
#include <AB/Packets/ClientPackets.h>

namespace Game {
class Player;
}

namespace Net {

class ProtocolGame final : public Protocol
{
    friend class Game::Player;
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Game Protocol"; }
    static std::string serverId_;
private:
    std::weak_ptr<Game::Player> player_;
    DH_KEY clientKey_;
    std::shared_ptr<Game::Player> GetPlayer()
    {
        return player_.lock();
    }
    inline void AddPlayerInput(Game::InputType type, Utils::VariantMap&& data);
    inline void AddPlayerInput(Game::InputType type);
    void Login(AB::Packets::Client::GameLogin packet);
    /// The client requests to enter a game. Find/create it, add the player and return success.
    void EnterGame();
public:
    explicit ProtocolGame(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {
        checksumEnabled_ = ProtocolGame::UseChecksum;
        compressionEnabled_ = ENABLE_GAME_COMPRESSION;
        encryptionEnabled_ = ENABLE_GAME_ENCRYTION;
        // TODO:
        SetEncKey(AB::ENC_KEY);
    }

    void Logout();
    /// Tells the client to change the instance. The client will disconnect and reconnect to enter the instance.
    void ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid);
    void ChangeServerInstance(const std::string& serverUuid,
        const std::string& mapUuid, const std::string& instanceUuid);
    void WriteToOutput(const NetworkMessage& message);
private:
    template <typename Callable, typename... Args>
    void AddPlayerTask(Callable&& function, Args&&... args)
    {
        auto player = GetPlayer();
        if (!player)
            return;
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(std::move(function), player, std::forward<Args>(args)...))
        );
    }

    std::shared_ptr<ProtocolGame> GetPtr()
    {
        return std::static_pointer_cast<ProtocolGame>(shared_from_this());
    }
    void ParsePacket(NetworkMessage& message) override;
    void OnRecvFirstMessage(NetworkMessage& msg) override;
    void OnConnect() override;

    void DisconnectClient(uint8_t error);
    void Connect();

    bool acceptPackets_{ false };
};

}
