#include "stdafx.h"
#include "SimpleConfigManager.h"
#include "Logger.h"

namespace IO {

SimpleConfigManager SimpleConfigManager::Instance;

std::string SimpleConfigManager::GetGlobal(const std::string& ident, const std::string& default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isstring(L, -1))
    {
        lua_pop(L, 1);
        return default;
    }

    int len = (int)luaL_len(L, -1);
    std::string ret(lua_tostring(L, -1), len);
    lua_pop(L, 1);

    return ret;
}

int64_t SimpleConfigManager::GetGlobal(const std::string& ident, int64_t default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isnumber(L, -1))
    {
        lua_pop(L, 1);
        return default;
    }

    int64_t val = (int64_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return val;
}

bool SimpleConfigManager::GetGlobalBool(const std::string& ident, bool default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isboolean(L, -1))
    {
        lua_pop(L, 1);
        return default;
    }

    bool val = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);

    return val;
}

bool SimpleConfigManager::Load(const std::string& file)
{
    if (L)
        lua_close(L);
    L = luaL_newstate();
    if (!L)
        return false;
    luaL_openlibs(L);

    if (luaL_dofile(L, file.c_str()) != 0)
    {
        int len = (int)luaL_len(L, -1);
        std::string err(lua_tostring(L, -1), len);
        LOG_ERROR << err << std::endl;
        lua_close(L);
        L = nullptr;
        return false;
    }

    isLoaded = true;
    return true;
}

}

#pragma comment(lib, "lua53.lib")

