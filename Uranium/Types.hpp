#pragma once
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include <map>
#include <vector>
struct function_table_struct
{
    lua_CFunction func;
    std::vector<const char*> names;
};
struct gc_search_proto 
{
    Proto* target;
    std::vector<Closure*> results;
};
struct getgc_context 
{
    int total_items_found;
    lua_State* lua_state_ptr;
    bool include_tables;
};
extern std::map<const LuaTable*, bool> protected_map;
extern bool is_table_protected(const LuaTable* table);