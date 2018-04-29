#include "stdafx.h"
#include "IOGame.h"
#include "DataClient.h"
#include <AB/Entities/GameList.h>

#include "DebugNew.h"

namespace IO {

bool IOGame::LoadGame(AB::Entities::Game& game)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Read(game);
}

bool IOGame::LoadGameByName(Game::Game* game, const std::string& name)
{
    game->data_.name = name;
    return LoadGame(game->data_);
}

bool IOGame::LoadGameByUuid(Game::Game* game, const std::string& uuid)
{
    game->data_.uuid = uuid;
    return LoadGame(game->data_);
}

std::string IOGame::GetLandingGame()
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
        return "";
    if (gl.gameUuids.size() == 0)
        return "";

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (client->Read(g) && g.landing)
            return g.name;
    }
    return "";
}

AB::Entities::GameType IOGame::GetGameType(const std::string& mapName)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Game g;
    g.name = mapName;
    if (!client->Read(g))
        return AB::Entities::GameTypeUnknown;
    return g.type;
}

std::vector<AB::Entities::Game> IOGame::GetGameList()
{
    std::vector<AB::Entities::Game> result;

    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
        return result;

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (!client->Read(g))
            continue;
        result.push_back(g);
    }

    if (result.size() == 0)
        LOG_WARNING << "No Games found!" << std::endl;
    return result;
}

}
