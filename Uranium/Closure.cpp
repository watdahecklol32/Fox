#include "Closure.hpp"
#include "lua.h"
#include "lualib.h"
#include <cstring>
#include <map>
#include <regex>
#include <string>
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
std::map<Closure*, Closure*> new_cclosure_map;
static int newcclosure_cont(lua_State* lua_state_ptr, int status)
{
    if (status != 0)
    {
        lua_error(lua_state_ptr);
        return 0;
    }
    return lua_gettop(lua_state_ptr);
}
static int newcclosure_handler_func(lua_State* lua_state_ptr)
{
    const int num_args_passed = lua_gettop(lua_state_ptr);
    const auto it = new_cclosure_map.find(clvalue(lua_state_ptr->ci->func));
    if (it == new_cclosure_map.end())
    {
        luaL_error(lua_state_ptr, "function not found"); // should never happen
        return 0;
    }
    luaC_threadbarrier(lua_state_ptr);
    setclvalue(lua_state_ptr, lua_state_ptr->top, it->second);
    lua_state_ptr->top += 1;
    lua_insert(lua_state_ptr, 1);
    const int result = lua_pcall(lua_state_ptr, num_args_passed, LUA_MULTRET, 0);
    if (result != 0)
    {
        if (lua_isstring(lua_state_ptr, -1))
        {
            // to spoof the error messages, if the error has the path and line to it, which doesnt happen in C closure errors
            std::string msg = lua_tostring(lua_state_ptr, -1);
            static const std::regex loc_re(R"(^.+:\d+: )", std::regex::optimize);
            msg = std::regex_replace(msg, loc_re, "");
            lua_pop(lua_state_ptr, 1);
            lua_pushstring(lua_state_ptr, msg.c_str());
        }
        lua_error(lua_state_ptr);
        return 0;
    }
    return lua_gettop(lua_state_ptr);
}
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
    int newcclosure(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        lua_ref(lua_state_ptr, 1);
        const char* debug_name_str = "";
        if (!lua_isnone(lua_state_ptr, 2))
        {
            luaL_checktype(lua_state_ptr, LUA_TSTRING);
            debug_name_str = lua_tostring(lua_state_ptr, 2);
        }
        lua_pushcclosurek(lua_state_ptr, newcclosure_handler_func, debug_name_str, 0, newcclosure_cont);
        new_cclosure_map[&luaA_toobject(lua_state_ptr, -1)->value.gc->cl] = &luaA_toobject(lua_state_ptr, 1)->value.gc->cl;
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


