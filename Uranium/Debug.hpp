#pragma once 
#include "lua.h"
namespace Uranium
{
    int debug_getmetatable(lua_State* lua_state_ptr);
    int debug_setmetatable(lua_State* lua_state_ptr);
    int debug_setmetatable(lua_State* lua_state_ptr);
    int debug_getconstants(lua_State* lua_state_ptr);
    int debug_getupvalues(lua_State* lua_state_ptr);
    int debug_getconstant(lua_State* lua_state_ptr);
    int debug_getupvalue(lua_State* lua_state_ptr);
    int debug_getprotos(lua_State* lua_state_ptr);
    int debug_getproto(lua_State* lua_state_ptr);
    int debug_setconstant(lua_State* lua_state_ptr);
    int debug_setupvalue(lua_State* lua_state_ptr);
    int debug_setname(lua_State* lua_state_ptr);
    int debug_getstack(lua_State* lua_state_ptr);
    int debug_setstack(lua_State* lua_state_ptr);
}
