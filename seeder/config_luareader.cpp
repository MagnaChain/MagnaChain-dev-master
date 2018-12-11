#include "config_luareader.h"

//yum install lua-devel.x86_64 lua-devel.i686
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <stdio.h>
#include <string.h>
#include <string>

bool ConfigLuaReader::ReadConfig(const char* filename, vector<MCDnsSeedOpts*> &vecOpts)
{
    lua_State* L = lua_open();
    luaL_openlibs(L);

    if (luaL_dofile(L, filename))
    {
        printf("%s not exist!", filename);
        return false;
    }

    lua_getglobal(L, "seederconfig");
    if (!lua_istable(L, -1))
    {
        return false;
    }

    lua_pushnil(L);
    while (lua_next(L, -2))
    {
        /*
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(L, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
        const char *key = lua_tostring(L, -1);
        const char *value = lua_tostring(L, -2);
        printf("%s => %s\n", key, value);
        // pop value + copy of key, leaving original key
        lua_pop(L, 2);
        // stack now contains: -1 => key; -2 => table
        */
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            continue;
        }
        MCDnsSeedOpts* opt = new MCDnsSeedOpts();
        {
            lua_getfield(L, -1, "branchid");
            if (!lua_isstring(L, -1)) { printf("miss branchid\n"); return false; }
            opt->branchid.assign(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "defaultport");
            if (!lua_isnumber(L, -1)) { printf("miss defaultport\n"); return false; }
            opt->defaultport = lua_tonumber(L, -1);
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "nThreads");
            if (lua_isnumber(L, -1)) 
            {
                opt->nThreads = lua_tonumber(L, -1);
            }
            
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "host");
            if (!lua_isstring(L, -1)) { printf("miss host\n"); return false; }
            const char* tempstr = lua_tostring(L, -1);
            size_t len = strlen(tempstr);
            opt->host = (char*)malloc(len + 1);
            memset(opt->host, 0, len + 1);
            memcpy(opt->host, tempstr, len);
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "ns");
            if (!lua_isstring(L, -1)) { printf("miss ns\n"); return false; }
            const char* tempstr = lua_tostring(L, -1);
            size_t len = strlen(tempstr);
            opt->ns = (char*)malloc(len + 1);
            memset(opt->ns, 0, len + 1);
            memcpy(opt->ns, tempstr, len);
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "mbox");
            if (!lua_isstring(L, -1)) { printf("miss mbox\n"); return false; }
            const char* tempstr = lua_tostring(L, -1);
            size_t len = strlen(tempstr);
            opt->mbox = (char*)malloc(len + 1);
            memset(opt->mbox, 0, len + 1);
            memcpy(opt->mbox, tempstr, len);
            lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "seeds");
            if (!lua_istable(L, -1)) { printf("miss seeds\n"); return false; }
            
            lua_pushnil(L);
            while (lua_next(L, -2))
            {
                if (!lua_isstring(L, -1)) { printf("seeds item must string\n"); return false; }
                const char* confseed = lua_tostring(L, -1);
                std::string strSeed(confseed);
                opt->seeds.push_back(strSeed);
                lua_pop(L, 1);
            }

            lua_pop(L, 1);
        }
        vecOpts.push_back(opt);
        lua_pop(L, 1);
    }
    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(L, 1);

    lua_close(L);
    return true;
}
