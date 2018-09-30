#include "stdafx.h"
#include "LuaScript.h"
#include "Logger.h"
#include <lua.hpp>

namespace Game {

template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vectorbuf : public std::basic_streambuf<CharT, TraitsT>
{
public:
    vectorbuf(std::vector<CharT> &vec)
    {
        setg(vec.data(), vec.data(), vec.data() + vec.size());
    }
};

bool LuaScript::Execute(kaguya::State& luaState)
{
    vectorbuf<char> databuf(buffer_);
    std::istream is(&databuf);
    if (!luaState.dostream(is))
    {
        LOG_ERROR << lua_tostring(luaState.state(), -1) << std::endl;
        return false;
    }
    return true;
}

}