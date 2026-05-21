#include "Table.hpp"
#include "Luau/Common.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>
#include <iostream>
#include "lapi.h"
#include "lobject.h"
namespace Uranium
{
	int isreadonly(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		LuaTable* table_ptr = hvalue(luaA_toobject(lua_state_ptr, 1));
		lua_pushboolean(lua_state_ptr, table_ptr->readonly);
		// lua_pushboolean(lua_state_ptr, )
		return 1;
	}
	int setreadonly(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		luaL_checktype(lua_state_ptr, 2, LUA_TBOOLEAN);
		lua_setreadonly(lua_state_ptr, 1, lua_toboolean(lua_state_ptr, 2));
		return 0;
	}
	int makewriteable(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		lua_setreadonly(lua_state_ptr, 1, 0);
		return 0;
	}
	int makereadonly(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		lua_setreadonly(lua_state_ptr, 1, 1);
		return 0;
	}
	int getnamecallmethod(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		const char* method_buffer = lua_namecallatom(lua_state_ptr, NULL);
		lua_pushstring(lua_state_ptr, method_buffer);
		return 1;
	}
	int setnamecallmethod(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TSTRING);
		// const char* provided_method = lua_tostring(lua_state_ptr, 1); // apparenlty this is unsafe, i'll have to look into it later
		// std :: cout << provided_method << std :: endl;
		lua_state_ptr->namecall = tsvalue(luaA_toobject(lua_state_ptr, 1));
		return 0;
	}
	int iswriteable(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		LuaTable* table_ptr = hvalue(luaA_toobject(lua_state_ptr, 1));
		uint8_t readonly = table_ptr->readonly;
		lua_pushboolean(lua_state_ptr, readonly == 0);
		return 1;
	}
	int getfflag(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TSTRING);
		const char* name = luaL_checkstring(lua_state_ptr, 1);
		for (Luau::FValue<bool>* flag = Luau::FValue<bool>::list; flag; flag = flag->next)
			{
				if (strcmp(flag->name, name) == 0)
					{
						lua_pushboolean(lua_state_ptr, flag->value);
						return 1;
					}
			}
				for (Luau::FValue<int>* flag = Luau::FValue<int>::list; flag; flag = flag->next)
					{
						// std :: cout << "bro can we like work ok??" << std :: endl;
						if (strcmp(flag->name, name) == 0)
							{
								lua_pushinteger(lua_state_ptr, flag->value);
								return 1;
							}
					}
					luaL_argerrorL(lua_state_ptr, 1, "Illegal Flag Name");
					return 0;
	}
	int getgenv(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		lua_pushvalue(lua_state_ptr, LUA_GLOBALSINDEX);
		return 1;
	}
	int getrenv(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		lua_pushvalue(lua_state_ptr, LUA_GLOBALSINDEX); // idrk what u expect lol
		return 1;
	}

}