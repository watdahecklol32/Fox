#include "Miscellaneous.hpp"
#include "lua.h"
namespace Uranium
{
   int identifyexecutor(lua_State* lua_state_ptr)
   {
        lua_pushstring(lua_state_ptr, "mreower");
        lua_pushstring(lua_state_ptr, "0.0.0");
        return 2;
   }
   


}