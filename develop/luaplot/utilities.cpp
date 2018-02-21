#include <iostream>
#include <sstream>
#include <cassert>
#include <lua.hpp>
#include "utilities.h"

// --

static std::string valueToString(lua_State* L, int idx)
{
    std::stringstream r;
    r << luaL_typename(L, idx) << ": ";

    switch (lua_type(L, idx)) {
    case LUA_TNONE:
        r << "none";
        break;

    case LUA_TNIL:
        r << "nil";
        break;

    case LUA_TBOOLEAN:
        r << std::boolalpha << lua_toboolean(L, idx);
        break;

    case LUA_TNUMBER:
        r << lua_tonumber(L, idx);
        break;

    case LUA_TSTRING:
        r << lua_tostring(L, idx);
        break;

    case LUA_TFUNCTION:
        r << lua_topointer(L, idx);
        break;

    case LUA_TUSERDATA:
        r << lua_touserdata(L, idx);
        break;

    case LUA_TLIGHTUSERDATA:
        r << lua_topointer(L, idx);
        break;

    case LUA_TTABLE:
        r << lua_topointer(L, idx);
        break;

    case LUA_TTHREAD:
        r << lua_topointer(L, idx);
        break;

    default:
        assert(false);
    }

    return r.str();
}

void stackindex_stdcout(lua_State *ls, int idx)
{
    std::cout << "[" << idx << "] = " << valueToString(ls, idx)
              << std::endl;
}

void dumpStack(const char* where,
               lua_State* L,
               void(*f)(lua_State*, int))
{
    if (where) {
        std::cout << where << std::endl;
    }
    std::cout << "----- stack dump -----" << std::endl;
    int top = lua_gettop(L);
    for (int i = top; i >= 1; --i) {
        f(L, i);
    }
    std::cout << "--------- end --------" << std::endl;
}

// --
