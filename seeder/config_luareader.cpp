#include "config_luareader.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

vector<MCDnsSeedOpts> ConfigLuaReader::ReadConfig(const char* filename)
{
    vector<MCDnsSeedOpts> vecOpts;
    lua_State* L = lua_open();
    luaL_openlibs(L);

    if (luaL_loadfile(L, filename) == 0)
    {

    }

    lua_close(L);
    return vecOpts;
}
