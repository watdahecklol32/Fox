#pragma once
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include <vector>
struct function_table_struct
{
    lua_CFunction func;
    std::vector<const char*> names;
};
struct gc_search_proto {
    Proto* target;
    std::vector<Closure*> results;
};