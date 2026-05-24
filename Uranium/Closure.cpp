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
#include "Thread.hpp"
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
std::map<Closure*, Closure*> new_cclosure_map;
std::map<Closure*, Closure*> restore_map;
std::map<const Closure*, bool> unhookables;
static int newcclosure_cont(lua_State* lua_state_ptr, const int status)
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
    lua_state_ptr->nCcalls += 1;
    const int result = lua_pcall(lua_state_ptr, num_args_passed, LUA_MULTRET, 0);
    lua_state_ptr->nCcalls -= 1;
    // w stackless pcall
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
    Closure* original_function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    Closure* cloned_function_ptr = clvalue(luaA_toobject(lua_state_ptr, -1));
    cloned_function_ptr->env = original_function_ptr->env;
    luaC_objbarrier(lua_state_ptr, cloned_function_ptr, cloned_function_ptr->env);
    return 1;
}
int newlclosure(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    lua_newtable(lua_state_ptr);
    //lua_getfenv(lua_state_ptr, 1); // TODO: this is bad!!! create a new enviorment instend!
    //lua_setmetatable(lua_state_ptr, -2);
    lua_pushvalue(lua_state_ptr, 1);
    lua_setfield(lua_state_ptr, -2, "yeswhatever");
    const int enviorment_index = lua_gettop(lua_state_ptr);
    const char* source_str = "return yeswhatever(...)";
    size_t bc_size = 0;
    char* bytecode_str = luau_compile(source_str, strlen(source_str), nullptr, &bc_size);
    if (!bytecode_str || luau_load(lua_state_ptr, "wrapper", bytecode_str, bc_size, enviorment_index) != LUA_OK) // TODO: by source would be neat
    {
        free(bytecode_str);
        lua_error(lua_state_ptr);
        return 0;
    }
    free(bytecode_str);
    lua_remove(lua_state_ptr, enviorment_index);
    lua_ref(lua_state_ptr, -1);
    return 1;
}
int hookfunction(lua_State* lua_state_ptr) // pain n sufferin'
{
    // what needs to be suppported:
    // bleh, for hooks involving newccloure we need to 'unwrap' them and get the orginal function so we dont hook the newccloure handler, that'd be really bad.
    // L->NC
    // NC->L
    // NC->NC
    // buggy, still...
    // TODO: fix NC->L crashing after 3 hooks
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TFUNCTION);
    Closure* target_function = clvalue(luaA_toobject(lua_state_ptr, 1));
    Closure* replacement_function = clvalue(luaA_toobject(lua_state_ptr, 2));
    if (unhookables.find(target_function) != unhookables.end())
    {
        luaL_argerror(lua_state_ptr, 1, "function is marked as a unhookable");
        return 0;
    }
    if (target_function == replacement_function) // since apparently hooking functions with them selfs are SOOO useful??
    {
        // std :: cout << "same func hook detected" << std :: endl;
        lua_getglobal(lua_state_ptr, "restorefunction");
        lua_pushvalue(lua_state_ptr, 1);
        lua_remove(lua_state_ptr, 2);
        return 0;
    }
    if (target_function->isC && target_function->c.f == newcclosure_handler_func)
    {
        const auto it = new_cclosure_map.find(target_function);
        if (it != new_cclosure_map.end())
        {
            luaC_threadbarrier(lua_state_ptr);
            setclvalue(lua_state_ptr, lua_state_ptr->base, it->second);
            target_function = it->second;
        }
    }
    const int target_num_upvalues = target_function->nupvalues;
    const int hook_num_upvalues = replacement_function->nupvalues;
    lua_getglobal(lua_state_ptr, "clonefunction");
    lua_pushvalue(lua_state_ptr, 1);
    lua_call(lua_state_ptr, 1, 1);
    if (restore_map.find(target_function) == restore_map.end())
    {
        restore_map[target_function] = clvalue(luaA_toobject(lua_state_ptr, -1));
    }
    if (!target_function->isC && !replacement_function->isC) // L-L hook
    {
        // std :: cout << "L-L hook" << std :: endl;
        if (hook_num_upvalues > target_num_upvalues)
        {
            // directly copying upvalues when the hook has more upvalues results in a out of bounds write
            // the solution is simple: use newlclosure to wrap the hook
            const Proto* target_proto = target_function->l.p;
            clvalue(luaA_toobject(lua_state_ptr, -1))->env = target_function->env; // god, why?
            lua_getglobal(lua_state_ptr, "newlclosure");
            lua_pushvalue(lua_state_ptr, 2);
            lua_call(lua_state_ptr, 1, 1);
            const Closure* wrapper = clvalue(luaA_toobject(lua_state_ptr, -1));
            Proto* wrapper_proto = wrapper->l.p;
            target_function->l.p = wrapper_proto;
            luaC_objbarrier(lua_state_ptr, target_function, target_proto);
            target_function->env = wrapper->env;
            luaC_objbarrier(lua_state_ptr, target_function, target_function->env);
            target_function->stacksize = wrapper->stacksize;
            target_function->preload = wrapper->preload;
            //wrapper->l.p->debugname = target_function->debugname;
            wrapper_proto->debugname = target_proto->debugname;
            wrapper_proto->linedefined = target_proto->linedefined;
            wrapper_proto->source = target_proto->source;
            wrapper_proto->is_vararg = target_proto->is_vararg;
            lua_pop(lua_state_ptr, 1); // be gone
            return 1;
        }
        else
        {
            for (int i = 0; i < hook_num_upvalues; i++)
            {
                setobj2n(lua_state_ptr, &target_function->l.uprefs[i], &replacement_function->l.uprefs[i]);
            }
            const Proto* target_proto = target_function->l.p;
            Proto* replacement_proto = replacement_function->l.p;
            target_function->l.p = replacement_proto;
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
    if (target_function->isC && replacement_function->isC) // C-C hook
    {
        const char* old_debug_name_str = target_function->c.debugname;
        const auto old_map_it = new_cclosure_map.find(target_function);
        if (old_map_it != new_cclosure_map.end())
        {
            new_cclosure_map[clvalue(luaA_toobject(lua_state_ptr, -1))] = old_map_it->second;
        }
        if (hook_num_upvalues > target_num_upvalues)
        {
            // ye kinda the same here 2
            lua_ref(lua_state_ptr, 2);
            new_cclosure_map[target_function] = replacement_function;
            target_function->c.f = newcclosure_handler_func;
            target_function->c.cont = newcclosure_cont;
            target_function->c.debugname = old_debug_name_str;
            return 1;
        }
        else
        {
            for (int i = 0; i < hook_num_upvalues; i++)
            {
                setobj2n(lua_state_ptr, &target_function->c.upvals[i], &replacement_function->c.upvals[i]);
            }
            target_function->c.f = replacement_function->c.f;
            target_function->c.cont = replacement_function->c.cont;
            target_function->c.debugname = old_debug_name_str;
            const auto repl_map_it = new_cclosure_map.find(replacement_function);
            if (repl_map_it != new_cclosure_map.end())
            {
                new_cclosure_map[target_function] = repl_map_it->second;
            }
            else
            {
                new_cclosure_map.erase(target_function);
            }
        }
        return 1;
    }
    if (target_function->isC && !replacement_function->isC) // C-L hook
    {
        // i'm lazy so i'm just gonna wrap the luau closure into newcclosure'd this is probably really slow, but eh whatever lol
        lua_getglobal(lua_state_ptr, "newcclosure");
        lua_pushvalue(lua_state_ptr, 2);
        lua_call(lua_state_ptr, 1, 1);
        lua_remove(lua_state_ptr, 3); 
        lua_getglobal(lua_state_ptr, "hookfunction");
        lua_pushvalue(lua_state_ptr, 1);
        lua_pushvalue(lua_state_ptr, 3);
        lua_call(lua_state_ptr, 2, 1); 
        return 1;
    }
    if (!target_function->isC && replacement_function->isC) // L-C hook
    {
        // same as be 4
        lua_getglobal(lua_state_ptr, "newlclosure");
        lua_pushvalue(lua_state_ptr, 2);
        lua_call(lua_state_ptr, 1, 1);
        lua_remove(lua_state_ptr, 3); 
        lua_getglobal(lua_state_ptr, "hookfunction");
        lua_pushvalue(lua_state_ptr, 1);
        lua_pushvalue(lua_state_ptr, 3);
        lua_call(lua_state_ptr, 2, 1); 
        return 1;
    }
    luaL_argerror(lua_state_ptr, 1, "unsupported hook");
    return 0;
}
int isfunctionhooked(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    lua_pushboolean(lua_state_ptr, restore_map.count(clvalue(luaA_toobject(lua_state_ptr, 1))) > 0);
    return 1;
}
int restorefunction(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const auto it = restore_map.find(function_ptr);
    if (it == restore_map.end())
    { // TODO: not sure if its the standard for this to error, it errors on most executors, so.
        luaL_argerror(lua_state_ptr, 1, "function is not hooked");
        return 0;
    }
    // clangd LSP likes to whine like a bitch holy shit
    const Closure* original_ptr = it->second;
    restore_map.erase(it);
    if (function_ptr->isC)
    {
        new_cclosure_map.erase(function_ptr);
        function_ptr->c.f = original_ptr->c.f;
        function_ptr->c.cont = original_ptr->c.cont;
        function_ptr->c.debugname = original_ptr->c.debugname;
        function_ptr->nupvalues = original_ptr->nupvalues;
        for (int i = 0; i < original_ptr->nupvalues; i++)
        {
            setobj2n(lua_state_ptr, &function_ptr->c.upvals[i], &original_ptr->c.upvals[i]);
        }
    }
    else
    {
        function_ptr->l.p = original_ptr->l.p;
        luaC_objbarrier(lua_state_ptr, function_ptr, original_ptr->l.p);
        function_ptr->env = original_ptr->env;
        luaC_objbarrier(lua_state_ptr, function_ptr, function_ptr->env);
        function_ptr->nupvalues = original_ptr->nupvalues;
        function_ptr->stacksize = original_ptr->stacksize;
        function_ptr->preload = original_ptr->preload;
        for (int i = 0; i < original_ptr->nupvalues; i++)
        {
            setobj2n(lua_state_ptr, &function_ptr->l.uprefs[i], &original_ptr->l.uprefs[i]);    
        }
    }
    return 0;
}
int hookmetamethod(lua_State* lua_state_ptr)
{
    luaL_checkany(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 2, LUA_TSTRING);
    luaL_checktype(lua_state_ptr, 3, LUA_TFUNCTION);
    if (!lua_getmetatable(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Object has no metatable");
        return 0;
    }
    lua_getfield(lua_state_ptr, -1, lua_tostring(lua_state_ptr, 2));
    if (lua_isnil(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Invalid metamethod");
        return 0;
    }
    if (!lua_isfunction(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Metamethod is not a function");
        return 0;
    }
    lua_getglobal(lua_state_ptr, "hookfunction");
    lua_pushvalue(lua_state_ptr, -2);
    lua_pushvalue(lua_state_ptr, 3);
    lua_call(lua_state_ptr, 2, 1);
    return 1;
}
int ismetamethodhooked(lua_State* lua_state_ptr)
{
    luaL_checkany(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 2, LUA_TSTRING);
    if (!lua_getmetatable(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Object has no metatable");
        return 0;
    }
    lua_getfield(lua_state_ptr, -1, lua_tostring(lua_state_ptr, 2));
    if (lua_isnil(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Invalid metamethod");
        return 0;
    }
    if (!lua_isfunction(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Metamethod is not a function");
        return 0;
    }
    lua_getglobal(lua_state_ptr, "isfunctionhooked");
    lua_pushvalue(lua_state_ptr, -2);
    lua_call(lua_state_ptr, 1, 1);
    return 1;
}
int restoremetamethod(lua_State* lua_state_ptr)
{
    luaL_checkany(lua_state_ptr, 1);
    luaL_checktype(lua_state_ptr, 2, LUA_TSTRING);
    if (!lua_getmetatable(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Object has no metatable");
        return 0;
    }
    lua_getfield(lua_state_ptr, -1, lua_tostring(lua_state_ptr, 2));
    if (lua_isnil(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Invalid metamethod");
        return 0;
    }
    if (!lua_isfunction(lua_state_ptr, -1))
    {
        lua_pop(lua_state_ptr, 2);
        luaL_argerrorL(lua_state_ptr, 2, "Metamethod is not a function");
        return 0;
    }
    lua_getglobal(lua_state_ptr, "restorefunction");
    lua_pushvalue(lua_state_ptr, -2);
    lua_call(lua_state_ptr, 1, 0);
    return 0;
}
int setunhookable(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    if (unhookables.find(function_ptr) != unhookables.end())
    {
        luaL_argerrorL(lua_state_ptr, 1, "function is already marked as unhookable");
        return 0;
    }
    unhookables[function_ptr] = true;
    return 0;
}
int gethookedfunctions(lua_State* lua_state_ptr)
{
    lua_newtable(lua_state_ptr);
    int table_index = 1;
    for (const auto hi: restore_map)
    {
        luaC_threadbarrier(lua_state_ptr);
        setclvalue(lua_state_ptr, lua_state_ptr->top, hi.first);
        lua_state_ptr->top += 1;
        lua_rawseti(lua_state_ptr, -2, table_index++);
    }
    return 1;
}
int getfunctionbytecode(lua_State *lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const Proto* proto_ptr = function_ptr->l.p;
    lua_pushlstring(lua_state_ptr, reinterpret_cast<const char*>(proto_ptr->code), proto_ptr->sizecode * sizeof(Instruction));
    return 1;
}
}