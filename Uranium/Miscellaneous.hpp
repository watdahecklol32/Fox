#pragma once
#include "lua.h"
namespace Uranium 
{
    int identifyexecutor(lua_State* lua_state_ptr);
    int setclipboard(lua_State* lua_state_ptr);
    int crash(lua_State* lua_state_ptr);
    int cloneref(lua_State* lua_state_ptr);
}