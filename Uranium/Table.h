#pragma once
#include "lua.h"
#include "lstate.h"
int isreadonly(lua_State* lua_state_ptr);
int setreadonly(lua_State* lua_state_ptr);
int makereadonly(lua_State* lua_state_ptr);
int makewriteable(lua_State* lua_state_ptr);
int setnamecallmethod(lua_State* lua_state_ptr);
int getnamecallmethod(lua_State* lua_state_ptr);
int iswriteable(lua_State* lua_state_ptr);
int getfflag(lua_State* lua_state_ptr);