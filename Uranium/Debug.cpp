#include "Debug.hpp"
#include "lua.h"
#include "lualib.h"
#include <cstddef>
#include <iostream>
#include <vector>
#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "ldebug.h"
#include "Types.hpp"
// TODO: FIX DEBUG.GETPROTO ITS A BITCH!!!!!
static bool debug_getproto_visitor(void* raw, lua_Page* gc_page, GCObject* gc_object)
{
    if (gc_object->gch.tt != LUA_TFUNCTION)
    {
        return false;
    }
    gc_search_proto* curr_context = static_cast<gc_search_proto*>(raw);
    Closure* closure = gco2cl(gc_object);
    if (!closure->isC && closure->l.p == curr_context->target)
    {
        curr_context->results.push_back(closure);
        return false;
    }
    return false;
}
namespace Uranium
{
int debug_getmetatable(lua_State* lua_state_ptr)
{
    luaL_checkany(lua_state_ptr, 1);
    if (lua_type(lua_state_ptr, 1) == LUA_TTABLE)
    {
        const LuaTable* table = hvalue(luaA_toobject(lua_state_ptr, 1));
        if (is_table_protected(table))
        {
            luaL_argerror(lua_state_ptr, 1, "table is protected");
            return 0;
        }
    }
    // lua_getmetatable(lua_state_ptr, 1);
    if (!lua_getmetatable(lua_state_ptr, 1))
    {
        lua_pushnil(lua_state_ptr);
        return 1;
    }
    return 1;
}
int debug_setmetatable(lua_State* lua_state_ptr)
{
    luaL_checkany(lua_state_ptr, 1);
    luaL_checkany(lua_state_ptr, 2);
    const int type_int = lua_type(lua_state_ptr, 2);
    if (type_int != LUA_TNIL && type_int != LUA_TTABLE)
    {
        luaL_argerror(lua_state_ptr, 2, "table or nil expected");
        return 0;
    }
    if (type_int == LUA_TTABLE)
    {
        const LuaTable* table = hvalue(luaA_toobject(lua_state_ptr, 1));
        if (is_table_protected(table))
        {
            luaL_argerror(lua_state_ptr, 1, "table is protected");
            return 0;
        }
    }
    lua_setmetatable(lua_state_ptr, 1);
    return 1;
}
int debug_getconstants(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    lua_newtable(lua_state_ptr);
    const Proto* proto_ptr = function_ptr->l.p;
    const int constant_size = proto_ptr->sizek;
    const TValue* constant_pool = proto_ptr->k;
    int table_index = 1;
    for (int i = 0; i < constant_size; i++)
    {
        const TValue* value = &constant_pool[i];
        if (!value || value->tt == LUA_TFUNCTION || value->tt == LUA_TNIL || value->tt == LUA_TTABLE)
        {
            continue;
        }
        else
        {
            luaC_threadbarrier(lua_state_ptr);
            luaA_pushobject(lua_state_ptr, value);
        }
        lua_rawseti(lua_state_ptr, -2, table_index++);
    }
    return 1;
}
int debug_getupvalues(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    // const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    // const Proto* proto_ptr = function_ptr->l.p;
    // const size_t upvalue_size = proto_ptr->sizeupvalues;
    // std::cout<< upvalue_size<<std::endl;
    //  GUESS THATS NOT HOW U DO IT!!
    lua_newtable(lua_state_ptr);
    for (int i = 1;; i++)
    {
        const char* up_value = lua_getupvalue(lua_state_ptr, 1, i);
        if (!up_value)
        {
            break;
        }
        lua_rawseti(lua_state_ptr, -2, i);
    }
    return 1;
}
int debug_getconstant(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const int index = lua_tointeger(lua_state_ptr, 2);
    const Closure* function = clvalue(luaA_toobject(lua_state_ptr, 1));
    const Proto* proto_ptr = function->l.p;
    const int constant_size = proto_ptr->sizek;
    if (index <= 0 || index > constant_size)
    {
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    const TValue* constant = &proto_ptr->k[index - 1];
    if (constant->tt == LUA_TNIL || constant->tt == LUA_TFUNCTION || constant->tt == LUA_TTABLE)
    {
        lua_pushnil(lua_state_ptr);
        return 1;
    }
    luaC_threadbarrier(lua_state_ptr);
    luaA_pushobject(lua_state_ptr, constant);
    return 1;
}
int debug_getupvalue(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    int index = lua_tointeger(lua_state_ptr, 2);
    lua_getglobal(lua_state_ptr, "getupvalues");
    lua_pushvalue(lua_state_ptr, 1);
    lua_call(lua_state_ptr, 1, 1);
    if (index > lua_objlen(lua_state_ptr, -1) || index <= 0)
    {
        lua_pop(lua_state_ptr, 1);
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    lua_rawgeti(lua_state_ptr, -1, index);
    return 1;
}
int debug_getprotos(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const Proto* proto_ptr = function_ptr->l.p;
    const int proto_size = proto_ptr->sizep;
    lua_newtable(lua_state_ptr);
    for (int i = 0; i < proto_size; i++)
    {
        Proto* target_proto = proto_ptr->p[i];
        const Closure* proto_func = luaF_newLclosure(lua_state_ptr, target_proto->nups, function_ptr->env, target_proto);
        luaC_threadbarrier(lua_state_ptr);
        setclvalue(lua_state_ptr, lua_state_ptr->top, proto_func);
        lua_state_ptr->top += 1;
        lua_rawseti(lua_state_ptr, -2, i + 1);
    }
    return 1;
}
int debug_getproto(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const Proto* proto_ptr = function_ptr->l.p;
    const int index = lua_tointeger(lua_state_ptr, 2);
    if (index <= 0 || index > proto_ptr->sizep)
    {
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    Proto* target = proto_ptr->p[index - 1];
    bool activated = false;
    if (!lua_isnoneornil(lua_state_ptr, 3))
    {
        luaL_checktype(lua_state_ptr, 3, LUA_TBOOLEAN);
        activated = lua_toboolean(lua_state_ptr, 3);
    }
    if (!activated)
    {
        const Closure* proto_func = luaF_newLclosure(lua_state_ptr, target->nups, function_ptr->env, target);
        luaC_threadbarrier(lua_state_ptr);
        setclvalue(lua_state_ptr, lua_state_ptr->top, proto_func);
        lua_state_ptr->top += 1;
        return 1;
    }
    gc_search_proto context{target, {}};
    luaM_visitgco(lua_state_ptr, &context, debug_getproto_visitor);
    lua_newtable(lua_state_ptr);
    for (size_t i = 0; i < context.results.size(); i++) // clueless!!
    {
        luaC_threadbarrier(lua_state_ptr);
        setclvalue(lua_state_ptr, lua_state_ptr->top, context.results[i]);
        lua_state_ptr->top += 1;
        lua_rawseti(lua_state_ptr, -2, i + 1);
    }
    return 1;
}
int debug_setconstant(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    // luaL_checkany(lua_state_ptr, 2);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    luaL_checkany(lua_state_ptr, 3);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const int target_type = lua_type(lua_state_ptr, 3);
    int index = lua_tointeger(lua_state_ptr, 2);
    // index -= 1; im stupid
    const Proto* proto_ptr = function_ptr->l.p;
    const int constant_size = proto_ptr->sizek;
    // if (index <= 0 or index > constant_size)
    //{
    // luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
    // return 0;
    //}
    TValue* constant_pool = proto_ptr->k;
    // const TValue target_constant = constant_pool[index];
    // if (target_constant.tt != target_type)
    //{
    // luaL_argerrorL(lua_state_ptr, 3, "type mismatch");
    // return 0;
    //}
    int next = 0;
    for (int i = 0; i < constant_size; i++)
    {
        TValue* value = &constant_pool[i];
        if (value->tt == LUA_TNIL || value->tt == LUA_TFUNCTION)
        {
            continue;
        }
        next += 1;
        if (next == index)
        {
            if (value->tt != target_type)
            {
                luaL_argerror(lua_state_ptr, 3, "type mismatch");
                return 0;
            };
            const TValue* replacement_constanbt = luaA_toobject(lua_state_ptr, 3);
            setobj(lua_state_ptr, value, replacement_constanbt);
            luaC_barrier(lua_state_ptr, proto_ptr, replacement_constanbt);
            return 0;
        }
    }
    luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
    return 0;
}
int debug_setupvalue(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    luaL_checkany(lua_state_ptr, 3);
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    const int upvalue_size = function_ptr->nupvalues;
    int index = lua_tointeger(lua_state_ptr, 2);
    if (index < 1 || index > upvalue_size)
    {
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    // const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr,1));
    // if (!lua_getupvalue(lua_state_ptr, 1, index))
    //{
    // luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
    // return 0;
    //}
    // TODO: no type check, i am not sure if this is unsafe...?
    lua_pushvalue(lua_state_ptr, 3);
    lua_setupvalue(lua_state_ptr, 1, index);
    return 0;
}
int debug_setname(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
    luaL_checktype(lua_state_ptr, 2, LUA_TSTRING);
    Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
    if (lua_iscfunction(lua_state_ptr, 1))
    {
        function_ptr->c.debugname = getstr(tsvalue(luaA_toobject(lua_state_ptr, 2)));
    }
    else
    {
        function_ptr->l.p->debugname = tsvalue(luaA_toobject(lua_state_ptr, 2));
    }
    return 0;
}
// the fucking horrors
int debug_getstack(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TNUMBER);
    const int stack_level = lua_tointeger(lua_state_ptr, 1);
    if (stack_level <= 0)
    {
        luaL_argerrorL(lua_state_ptr, 1, "level out of bounds");
        return 0;
    }
    lua_Debug func_res;
    if (!lua_getinfo(lua_state_ptr, stack_level, "f", &func_res))
    {
        luaL_argerrorL(lua_state_ptr, 1, "level out of bounds");
        return 0;
    }
    const bool is_cfunc = lua_iscfunction(lua_state_ptr, -1);
    lua_pop(lua_state_ptr, 1);
    if (is_cfunc)
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const CallInfo* current_ci = lua_state_ptr->ci - stack_level;
    const Proto* proto = clvalue(current_ci->func)->l.p;
    const int program_counter = pcRel(current_ci->savedpc, proto);
    if (lua_isnoneornil(lua_state_ptr, 2))
    {
        lua_newtable(lua_state_ptr);
        for (int i = 1; ; i++)
        {
            // so we dont get fucked by the optimizations
            const LocVar* result = luaF_getlocal(proto, i, program_counter);
            if (!result){
                break;
            }
            luaC_threadbarrier(lua_state_ptr);
            luaA_pushobject(lua_state_ptr, current_ci->base + result->reg);
            lua_rawseti(lua_state_ptr, -2, i);
        }
        return 1;
    }
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    const int index = lua_tointeger(lua_state_ptr, 2);
    if (index <= 0)
    {
        luaL_argerror(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    // so we dont get fucked by the optimizations
    const LocVar* result = luaF_getlocal(proto, index, program_counter);
    if (!result)
    {
        luaL_argerror(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    luaC_threadbarrier(lua_state_ptr);
    luaA_pushobject(lua_state_ptr, current_ci->base + result->reg);
    return 1;
}
int debug_setstack(lua_State* lua_state_ptr)
{
    luaL_checktype(lua_state_ptr, 1, LUA_TNUMBER);
    luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
    luaL_checkany(lua_state_ptr, 3);
    const int stack_level = lua_tointeger(lua_state_ptr, 1);
    if (stack_level <= 0)
    {
        luaL_argerrorL(lua_state_ptr, 1, "level out of bounds");
        return 0;
    }
    lua_Debug func_res;
    if (!lua_getinfo(lua_state_ptr, stack_level, "f", &func_res))
    {
        luaL_argerrorL(lua_state_ptr, 1, "illegal stack level");
        return 0;
    }
    const bool is_cfunc = lua_iscfunction(lua_state_ptr, -1);
    lua_pop(lua_state_ptr, 1);
    if (is_cfunc)
    {
        luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
        return 0;
    }
    const CallInfo* current_ci = lua_state_ptr->ci - stack_level;
    const Proto* proto = clvalue(current_ci->func)->l.p;
    const int program_counter = pcRel(current_ci->savedpc, proto);
    const int index = lua_tointeger(lua_state_ptr, 2);
    if (index <= 0)
    {
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    const LocVar* result = luaF_getlocal(proto, index, program_counter);
    if (!result)
    {
        luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
        return 0;
    }
    const lua_TValue* current_value = current_ci->base + result->reg;
    const lua_TValue* new_value = luaA_toobject(lua_state_ptr, 3);
    if (ttype(current_value) != ttype(new_value))
    {
        luaL_argerror(lua_state_ptr, 3, "type mismatch");
        return 0;
    }
    setobj2s(lua_state_ptr, current_ci->base + result->reg, new_value);
    return 0;
}
}