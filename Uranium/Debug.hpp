#pragma once 
#include "lua.h"
namespace Uranium
{
    int debug_getmetatable(lua_State* lua_state_ptr);
    int debug_setmetatable(lua_State* lua_state_ptr);
    int debug_setmetatable(lua_State* lua_state_ptr);
    int debug_getconstants(lua_State* lua_state_ptr);
    int debug_getupvalues(lua_State* lua_state_ptr);
}
