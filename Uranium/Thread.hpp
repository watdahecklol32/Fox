#pragma once
#include "lua.h"
namespace Uranium
{
    int getfunctionfromthread(lua_State* lua_state_ptr);
    int gettenv(lua_State* lua_state_ptr);
}