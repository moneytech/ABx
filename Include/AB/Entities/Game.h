#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_GAMES = "game_maps";

enum GameType : uint8_t
{
    GameTypeUnknown = 0,
    GameTypeOutpost = 1,
    GameTypeTown,
    GameTypeGuildHall,
    // Bellow this games are exclusive and always created new
    GameTypePvPCombat,
    GameTypeExploreable,
    GameTypeMission,
};

enum GameModeFlags : uint32_t
{
    GameModeFlagNone = 0,
};

enum GameMode : uint32_t
{
    GameModeUnknown = 0,
};

struct Game : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GAMES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_MAP_NAME);
        s.text1b(directory, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(queueMapUuid, Limits::MAX_UUID);
        s.value1b(type);
        s.value4b(mode);
        s.value1b(landing);
        s.value1b(partySize);
        s.value1b(partyCount);
        s.value1b(randomParty);
        s.value4b(mapCoordX);
        s.value4b(mapCoordY);
        s.value1b(defaultLevel);
    }

    /// The name of the game
    std::string name;
    /// Directory with game data
    std::string directory;
    /// Script file
    std::string script;
    std::string queueMapUuid;
    GameType type = GameTypeUnknown;
    GameMode mode = GameModeUnknown;
    bool landing = false;
    uint8_t partySize = 0;
    uint8_t partyCount = 0;
    bool randomParty = false;
    int32_t mapCoordX = 0;
    int32_t mapCoordY = 0;
    int8_t defaultLevel = 1;
};

}
}
