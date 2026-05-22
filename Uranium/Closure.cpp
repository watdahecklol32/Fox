#include "Closure.hpp"

#include "lua.h"
#include "luacode.h"
#include "lualib.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <regex>
#include <string>

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
const constexpr int TYPE_L = 0, TYPE_C = 1, TYPE_NC = 2;
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
        luaL_checktype(lua_state_ptr, 2, LUA_TSTRING);
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
int newlclosure(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_newtable(L);
    lua_getfenv(L, 1); // TODO: this is bad!!! create a new enviorment instend!
    lua_setmetatable(L, -2);
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "yeswhatever");
    const int enviorment_index = lua_gettop(L);
    const char* source_str = "return yeswhatever(...)";
    size_t bc_size = 0;
    char* bytecode_str = luau_compile(source_str, strlen(source_str), nullptr, &bc_size);
    if (!bytecode_str || luau_load(L, "wrapper", bytecode_str, bc_size, enviorment_index) != LUA_OK)
    {
        free(bytecode_str);
        lua_error(L);
        return 0;
    }
    free(bytecode_str);
    lua_remove(L, enviorment_index);
    lua_ref(L, -1);
    return 1;
}
int hookfunction(lua_State* lua_state_ptr) // pain n sufferin'
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TFUNCTION);
    Closure* target_function = clvalue(luaA_toobject(lua_state_ptr, 1));
    Closure* replacement_function = clvalue(luaA_toobject(lua_state_ptr, 2));
    if (!target_function->isC && !replacement_function->isC) // L-L hook
    {
        std :: cout << "L-L hook" << std :: endl;
        lua_clonefunction(lua_state_ptr, 1);
        clvalue(luaA_toobject(lua_state_ptr, -1))->env = target_function->env; // god, why?
        const int target_num_upvalues = target_function->nupvalues;
        const int hook_num_upvalues = replacement_function->nupvalues;
        if (hook_num_upvalues > target_num_upvalues)
        {
            // directly copying upvalues when the hook has more upvalues results in a out of bounds write
            // the solution is simple: use newlclosure to wrap the hook
            lua_getglobal(lua_state_ptr, "newlclosure");
            lua_pushvalue(lua_state_ptr, 2);
            lua_call(lua_state_ptr, 1, 1); 
            const Closure* wrapper = clvalue(luaA_toobject(lua_state_ptr, -1));
            Proto* wrapper_proto = wrapper->l.p;
            const Proto* target_proto = target_function->l.p;
            target_function->l.p = wrapper_proto;
            luaC_objbarrier(lua_state_ptr, target_function, target_proto);
            target_function->env = wrapper->env;
            luaC_objbarrier(lua_state_ptr, target_function, target_function->env);
            target_function->nupvalues = 0;
            target_function->stacksize = wrapper->stacksize;
            target_function->preload = wrapper->preload;
            //wrapper->l.p->debugname = target_function->debugname;
            wrapper_proto->debugname = target_proto->debugname;
            wrapper_proto->linedefined = target_proto->linedefined;
            wrapper_proto->source = target_proto->source;
            wrapper_proto->is_vararg = target_proto->is_vararg;
            lua_pop(lua_state_ptr, 1); // be gone
        }
        else
        {
            // if the hook doesnt have more upvalues than the target it's safe to directly copy
            for (int i = 0; i < hook_num_upvalues; i++)
            {
                setobj2n(lua_state_ptr, &target_function->l.uprefs[i], &replacement_function->l.uprefs[i])
            }
            const Proto* target_proto = target_function->l.p;
            Proto* replacement_proto = target_function->l.p;
            target_function->nupvalues = hook_num_upvalues;
            target_function->l.p = replacement_function->l.p;
            luaC_objbarrier(lua_state_ptr, target_function, replacement_proto);
            target_function->env = replacement_function->env;
            luaC_objbarrier(lua_state_ptr, target_function, target_function->env);
            target_function->stacksize = replacement_function->stacksize;
            target_function->preload = replacement_function->preload;
            replacement_proto->debugname = target_proto->debugname;
            replacement_proto->linedefined = target_proto->linedefined;
            replacement_proto->source = target_proto->source;
            replacement_proto->is_vararg = target_proto->is_vararg;
        }
        return 1;
    }

    luaL_argerror(lua_state_ptr, 1, "unsupported hook");
    return 0;
}
}