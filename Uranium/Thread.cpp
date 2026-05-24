#include "Thread.hpp"
#include "lua.h"
#include "lualib.h"
#include "lgc.h"
#include "lstate.h"
namespace Uranium
{
    int getfunctionfromthread(lua_State* lua_state_ptr)
    {
        // TOOD: bad way?
        luaL_checktype(lua_state_ptr, 1, LUA_TTHREAD);
        const lua_State* thread = lua_tothread(lua_state_ptr, 1);
        for (const CallInfo* i = thread->ci; i > thread->base_ci; i--)
        {
            if (ttisfunction(i->func))
            {
                setobj2s(lua_state_ptr, lua_state_ptr->top, i->func);
                lua_state_ptr->top += 1;
                return 1;
            }
        }
        for (StkId i = thread->stack; i < thread->top; i++)
        {
            if (ttisfunction(i))
            {
                setobj2s(lua_state_ptr, lua_state_ptr->top, i);
                lua_state_ptr->top += 1;
                return 1;
            }
        }
        if (thread->base_ci + 1 <= thread->end_ci)
        {
            const StkId function = (thread->base_ci + 1)->func;
            if (function >= thread->stack && function < thread->stack + thread->stacksize && ttisfunction(function))
            {
                setobj2s(lua_state_ptr, lua_state_ptr->top, function);
                lua_state_ptr->top += 1;
                return 1;
            }
        }
        lua_pushnil(lua_state_ptr);
        return 1;
    }
    int gettenv(lua_State* lua_state_ptr)
    {
        // TODO: segfaults if the thread is dead.. fix tmr.
        // TODO: doesn't get enviorment from task threads?
        luaL_checktype(lua_state_ptr, 1, LUA_TTHREAD);
        lua_getglobal(lua_state_ptr, "getfunctionfromthread");
        lua_pushvalue(lua_state_ptr, 1);
        lua_call(lua_state_ptr, 1, 1);
        //if (thread->gt == nullptr)
        //{
            //lua_pushnil(lua_state_ptr);
            //return 1;
        //}
        if (lua_isnil(lua_state_ptr, -1))
        {
            lua_pop(lua_state_ptr, 1);
            const lua_State* thread = lua_tothread(lua_state_ptr, 1);
            sethvalue(lua_state_ptr, lua_state_ptr->top, thread->gt);
            lua_state_ptr->top += 1;
            return 1;
        }
        lua_getglobal(lua_state_ptr, "getfenv");
        lua_pushvalue(lua_state_ptr, -2);
        lua_call(lua_state_ptr, 1, 1);
        return 1;
    }
}
