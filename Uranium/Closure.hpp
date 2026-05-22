#pragma once
#include "lua.h"
namespace Uranium
{
    int iscclosure(lua_State* lua_state_ptr);
    int islclosure(lua_State* lua_state_ptr);
    int clonefunction(lua_State* lua_state_ptr);
}