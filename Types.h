#pragma once
#include "lua.h"
#include <vector>
struct function_table_struct
{
    lua_CFunction func;
    std::vector<const char*> names;
};
