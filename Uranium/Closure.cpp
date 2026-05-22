#include "Closure.hpp"
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
namespace Uranium
{
    int iscclosure(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        lua_pushboolean(lua_state_ptr, lua_iscfunction(lua_state_ptr, 1));
        return 1;
    }
    int islclosure(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        lua_pushboolean(lua_state_ptr, lua_isLfunction(lua_state_ptr, 1));
        return 1;
    }
    int clonefunction(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        if (lua_iscfunction(lua_state_ptr, 1)) // yes of course things can't be this simple
        {
            const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
            const int upvalue_count = function_ptr->nupvalues;
            Closure* clone_ptr = luaF_newCclosure(lua_state_ptr, upvalue_count, function_ptr->env);
            clone_ptr->c.f = function_ptr->c.f;
            clone_ptr->c.cont = function_ptr->c.cont;
            clone_ptr->c.debugname = function_ptr->c.debugname;
            for (int i = 0; i < upvalue_count; i++)
            {
               TValue old_upvalue = function_ptr->c.upvals[i];
               setobj2n(lua_state_ptr, &clone_ptr->c.upvals[i], &old_upvalue);
            }
            luaC_threadbarrier(lua_state_ptr);
            setclvalue(lua_state_ptr, lua_state_ptr->top, clone_ptr);
            lua_state_ptr->top += 1;
            return 1;
        }
        lua_clonefunction(lua_state_ptr, 1);
        return 1;
    }
}


