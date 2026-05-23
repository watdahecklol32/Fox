#pragma once
#include "lua.h"
namespace Uranium
{
    int iscclosure(lua_State* lua_state_ptr);
    int islclosure(lua_State* lua_state_ptr);
    int clonefunction(lua_State* lua_state_ptr);
    int newcclosure(lua_State* lua_state_ptr);
    int newlclosure(lua_State* L);
    int hookfunction(lua_State* lua_state_ptr);
    int isfunctionhooked(lua_State* lua_state_ptr);
    int restorefunction(lua_State* lua_state_ptr);
    int hookmetamethod(lua_State* lua_state_ptr);
    int ismetamethodhooked(lua_State* lua_state_ptr);
    int restoremetamethod(lua_State* lua_state_ptr);
}