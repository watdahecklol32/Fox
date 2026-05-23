#include "Console.hpp"
#include "lualib.h"
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <stdlib.h>
static bool has_console_created_already = false;
namespace Uranium
{
    int rconsolecreate(lua_State *lua_State)
    {
        if (has_console_created_already)
        {
            luaL_error(lua_State, "Cannot create beyond 1 more rconsole");
            return 0;
        }
        has_console_created_already = true;
        
        return 0;
    }
}