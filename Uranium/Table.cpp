#include "Table.hpp"
#include "Luau/Common.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>
#include <iostream>
#include <map>
#include "lapi.h"
#include "Types.hpp"
#include "lmem.h"
#include "lobject.h"
std::map<const LuaTable*, bool> protected_map;
bool is_table_protected(const LuaTable* table)
{
	return protected_map.find(table) != protected_map.end();
}
static bool getgc_visitor(void* context, lua_Page* gc_page, GCObject* gc_object)
{
	// somewhat ugly
	getgc_context* curr_context = static_cast<getgc_context*>(context);
	lua_State* lua_state_ptr = curr_context->lua_state_ptr;
	const uint8_t type = gc_object->gch.tt;
	if (type != LUA_TTABLE && type != LUA_TFUNCTION && type != LUA_TUSERDATA && type != LUA_TTHREAD && type != LUA_TBUFFER)
	{
		return false;
	}
	if (isdead(lua_state_ptr->global, gc_object) || (type == LUA_TTABLE && !curr_context->include_tables))
	{
		return false;
	}
	lua_checkstack(lua_state_ptr, 1);
	TValue* top = lua_state_ptr->top;
	top->value.gc = gc_object;
	top->tt = type;
	lua_state_ptr->top += 1;
	curr_context->total_items_found += 1;
	lua_rawseti(lua_state_ptr, -2, curr_context->total_items_found);
	return false;
}
namespace Uranium
{
	int isreadonly(lua_State* lua_state_ptr)
	{
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
		const LuaTable* target_table = hvalue(luaA_toobject(lua_state_ptr, 1));
		if (is_table_protected(target_table))
		{
			luaL_argerror(lua_state_ptr, 1, "table is protected");
			return 0;
		}
		lua_setreadonly(lua_state_ptr, 1, lua_toboolean(lua_state_ptr, 2));
		return 0;
	}
	int makewriteable(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		const LuaTable* target_table = hvalue(luaA_toobject(lua_state_ptr, 1));
		if (is_table_protected(target_table))
		{	
			luaL_argerror(lua_state_ptr, 1, "table is protected");
			return 0;
		}
		lua_setreadonly(lua_state_ptr, 1, 0);
		return 0;
	}
	int makereadonly(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		const LuaTable* target_table = hvalue(luaA_toobject(lua_state_ptr, 1));
		if (is_table_protected(target_table))
		{
			luaL_argerror(lua_state_ptr, 1, "table is protected");
			return 0;
		}
		lua_setreadonly(lua_state_ptr, 1, 1);
		return 0;
	}
	int getnamecallmethod(lua_State* lua_state_ptr)
	{
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
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		LuaTable* table_ptr = hvalue(luaA_toobject(lua_state_ptr, 1));
		uint8_t readonly = table_ptr->readonly;
		lua_pushboolean(lua_state_ptr, readonly == 0);
		return 1;
	}
	int getfflag(lua_State* lua_state_ptr)
	{
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
		lua_pushvalue(lua_state_ptr, LUA_GLOBALSINDEX);
		return 1;
	}
	int getrenv(lua_State* lua_state_ptr)
	{
		lua_pushvalue(lua_state_ptr, LUA_GLOBALSINDEX); // idrk what u expect lol
		return 1;
	}
	int getgc(lua_State* lua_state_ptr)
	{
		lua_newtable(lua_state_ptr);
		const int type = lua_type(lua_state_ptr, 1);
		if (type != LUA_TBOOLEAN && type != LUA_TNONE && type != LUA_TNIL)
		{
			luaL_argerrorL(lua_state_ptr, 1, "boolean or nil expected");
			return 0;
		}
		const bool include_tables = luaL_optboolean(lua_state_ptr, 1, false);
		getgc_context context{0, lua_state_ptr, include_tables};
		luaM_visitgco(lua_state_ptr, &context, getgc_visitor);
		return 1;
	}
	int getreg(lua_State* lua_state_ptr)
	{
		lua_pushvalue(lua_state_ptr, LUA_REGISTRYINDEX);
		return 1;
	}
	int setsafeenv(lua_State* lua_state_ptr)
	{
		// https://actualmasteroogway.github.io/synapse-x-documentation/reference/category/table.html?highlight=setunt#setuntouched
		// OK, simple enough!
		luaL_checkany(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 2, LUA_TBOOLEAN);
		const int type = lua_type(lua_state_ptr, 1);
		const bool state = luaL_optboolean(lua_state_ptr, 2, false);
		switch (type)
		{
			case LUA_TTABLE:
			{
				lua_setsafeenv(lua_state_ptr, 1, state);
				break;
			}
			case LUA_TFUNCTION:
			{
				lua_getfenv(lua_state_ptr, 1);
				lua_setsafeenv(lua_state_ptr, -1, state);
				break;
			}
			case LUA_TTHREAD:
			{
				lua_getglobal(lua_state_ptr, "getfunctionfromthread");
				lua_pushvalue(lua_state_ptr, 1);
				lua_call(lua_state_ptr, 1, 1);
				lua_getfenv(lua_state_ptr, -1);
				lua_setsafeenv(lua_state_ptr, -1, state);
				break;
			}
			default:
			{
				luaL_argerrorL(lua_state_ptr, 1, "Unsupported arugment");
				break;
			}
		}
		return 0;
	}
	int isuntouched(lua_State* lua_state_ptr)
	{
		luaL_checkany(lua_state_ptr, 1);
		const int type = lua_type(lua_state_ptr, 1);
		switch (type)
		{
			case LUA_TTABLE:
			{
				const LuaTable* env = hvalue(luaA_toobject(lua_state_ptr, 1));
				lua_pushboolean(lua_state_ptr, env->safeenv);
				break;
			}
			case LUA_TFUNCTION:
			{
				lua_getfenv(lua_state_ptr, 1);
				const LuaTable* env = hvalue(luaA_toobject(lua_state_ptr, -1));
				lua_pushboolean(lua_state_ptr, env->safeenv);
				break;
			}
			case LUA_TTHREAD:
			{
				lua_getglobal(lua_state_ptr, "getfunctionfromthread");
				lua_pushvalue(lua_state_ptr, 1);
				lua_call(lua_state_ptr, 1, 1);
				lua_getfenv(lua_state_ptr, -1);
				const LuaTable* env = hvalue(luaA_toobject(lua_state_ptr, -1));
				lua_pushboolean(lua_state_ptr, env->safeenv);
				break;
			}
			default:
			{
				luaL_argerrorL(lua_state_ptr, 1, "Unsupported arugment");
				break;
			}
		}
		return 1;
	}
	int setprotected(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		const LuaTable* table = hvalue(luaA_toobject(lua_state_ptr, 1));
		if (is_table_protected(table))
		{
			luaL_argerrorL(lua_state_ptr, 1, "table is already protected");
			return 0;
		}
		protected_map[table] = true;
		return 0;
	}
	int isprotected(lua_State* lua_state_ptr)
	{
		luaL_checktype(lua_state_ptr, 1, LUA_TTABLE);
		const LuaTable* table = hvalue(luaA_toobject(lua_state_ptr, 1));
		lua_pushboolean(lua_state_ptr, is_table_protected(table));
		return 1;
	}
}