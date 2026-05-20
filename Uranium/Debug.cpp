#include "Debug.h"
#include "lua.h"
#include "lualib.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include "lapi.h"
#include "lobject.h"
#include "lstate.h"
#include "lgc.h"
int debug_getmetatable(lua_State* lua_state_ptr)
{
    lua_checkstack(lua_state_ptr, 1);
    luaL_checkany(lua_state_ptr, 1);
    lua_getmetatable(lua_state_ptr, 1);
    return 1;
}
int debug_setmetatable(lua_State* lua_state_ptr)
{
    lua_checkstack(lua_state_ptr, 1);
    luaL_checkany(lua_state_ptr, 1);
    luaL_checkany(lua_state_ptr, 2);
    const int type_int = lua_type(lua_state_ptr, 2);
    if (type_int != LUA_TNIL && type_int != LUA_TTABLE)
    {
        luaL_argerror(lua_state_ptr, 2, "table or nil expected");
        return 0;
    }
    lua_setmetatable(lua_state_ptr, 1);
    return 1;
}
int debug_getconstants(lua_State* lua_state_ptr)
{
    lua_checkstack(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    lua_newtable(lua_state_ptr);
    const Proto* proto_ptr = function_ptr->l.p;
    const size_t constant_size = proto_ptr->sizek;
    const TValue* constant_pool = proto_ptr->k;
    size_t table_index = 1;
    for (size_t i = 0; i < constant_size; i++)
    {
        const TValue value = constant_pool[i];
        if (value.tt == LUA_TFUNCTION || value.tt == LUA_TNIL){
            //lua_pushnil(lua_state_ptr);
            // lua_rawseti(lua_state_ptr, -2, table_index++);
            continue;
        }
        luaC_threadbarrier(lua_state_ptr);
        luaA_pushobject(lua_state_ptr, &value);
        lua_rawseti(lua_state_ptr, -2, table_index++);
    }
    return 1;
}
int debug_getupvalues(lua_State* lua_state_ptr)
{
    lua_checkstack(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    //const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    //const Proto* proto_ptr = function_ptr->l.p;
    //const size_t upvalue_size = proto_ptr->sizeupvalues;
    //std::cout<< upvalue_size<<std::endl;
    // GUESS THATS NOT HOW U DO IT!!
    lua_newtable(lua_state_ptr);
    for (int i = 1; ;i++)
    {
        const char* up_value = lua_getupvalue(lua_state_ptr, 1, i);
        if (!up_value)
        {
            break;
        }
        lua_rawseti(lua_state_ptr, -2, i);
    }
    return 1;
}