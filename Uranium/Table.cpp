#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lobject.h"
#include "Table.h"
int isreadonly(lua_State* lua_state_ptr)
{
    lua_checkstack(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
    LuaTable* table_ptr = hvalue(luaA_toobject(lua_state_ptr, 1));
    lua_pushboolean(lua_state_ptr, table_ptr->readonly);
    //lua_pushboolean(lua_state_ptr, )
    return 1;
}
int setreadonly(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
    luaL_checktype(lua_state_ptr, 2, LUA_TBOOLEAN);
    lua_setreadonly(lua_state_ptr, 1, lua_toboolean(lua_state_ptr, 2));
    return 0;
}