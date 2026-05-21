#include "Debug.hpp"
#include "lua.h"
#include "lualib.h"
#include <cstddef>
#include <vector>
#include "lapi.h"
#include "lfunc.h"
#include <iostream>
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "Types.hpp"
namespace Uranium
{
	int debug_getmetatable(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checkany(lua_state_ptr, 1);
		//lua_getmetatable(lua_state_ptr, 1);
        if (!lua_getmetatable(lua_state_ptr, 1))
        {
            lua_pushnil(lua_state_ptr);
            return 1;
        }
		return 1;
	}
	int debug_setmetatable(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checkany(lua_state_ptr, 1);
		luaL_checkany(lua_state_ptr, 2);
		const int type_int = lua_type(lua_state_ptr, 2);
		if (type_int != LUA_TNIL && type_int != LUA_TTABLE)
			{
				luaL_argerror(lua_state_ptr, 2, "table or nil expected");
				return 0;
			}
			lua_setmetatable(lua_state_ptr, 1);
			return 1;
	}
	int debug_getconstants(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
		if (lua_iscfunction(lua_state_ptr, 1))
			{
				luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
				return 0;
			}
			const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
			lua_newtable(lua_state_ptr);
			const Proto* proto_ptr = function_ptr->l.p;
			const size_t constant_size = proto_ptr->sizek;
			const TValue* constant_pool = proto_ptr->k;
			size_t table_index = 1;
			for (size_t i = 0; i < constant_size; i++)
				{
					const TValue value = constant_pool[i];
					if (value.tt == LUA_TFUNCTION || value.tt == LUA_TNIL)
						{
							// lua_pushnil(lua_state_ptr);
							// lua_rawseti(lua_state_ptr, -2, table_index++);
							continue;
						}
						luaC_threadbarrier(lua_state_ptr);
						luaA_pushobject(lua_state_ptr, &value);
						lua_rawseti(lua_state_ptr, -2, table_index++);
				}
				return 1;
	}
	int debug_getupvalues(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
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
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
		luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
		if (lua_iscfunction(lua_state_ptr, 1))
			{
				luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
				return 0;
			}
			int index = lua_tonumber(lua_state_ptr, 2);
			lua_getglobal(lua_state_ptr, "getconstants");
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
				//const Closure* function = clvalue(luaA_toobject(lua_state_ptr, 1));
				//const Proto* proto_ptr = function->l.p;
				//const int constant_size = proto_ptr->sizek;
				//const TValue* constant_pool = proto_ptr->k;
				//if (index > constant_size)
							//{
								//  luaL_argerrorL(lua_state_ptr, 2, "index is out of bounds");
								// return 0;
								//}
							//const TValue constant = constant_pool[index];
							//if (constant.tt == LUA_TNIL || constant.tt == LUA_TFUNCTION)
								//{
									//lua_pushnil(lua_state_ptr);
									//return 1;
									//}
								//luaC_threadbarrier(lua_state_ptr);
								//luaA_pushobject(lua_state_ptr, &constant);
								//return 1;
	}
	int debug_getupvalue(lua_State* lua_state_ptr)
	{
		lua_checkstack(lua_state_ptr, 1);
		luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
		luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
		int index = lua_tonumber(lua_state_ptr, 2);
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
        lua_checkstack(lua_state_ptr, 1);
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        if (lua_iscfunction(lua_state_ptr, 1))
        {
            luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
            return 0;
        }
        const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
        const Proto* proto_ptr = function_ptr->l.p;
        bool activated = true;
        if (!lua_isnoneornil(lua_state_ptr, 2))
        {
            luaL_checktype(lua_state_ptr, 2, LUA_TBOOLEAN);
            activated = lua_toboolean(lua_state_ptr, 2);
        }
        const int proto_size = proto_ptr->sizep;
        lua_newtable(lua_state_ptr);
        if (activated)
        {
            for (int i = 0; i < proto_size; i++)
            {
                Proto* target = proto_ptr->p[i];
                Closure* proto_func = luaF_newLclosure(lua_state_ptr, target->nups, function_ptr->env, target);
                luaC_threadbarrier(lua_state_ptr);
                setclvalue(lua_state_ptr, lua_state_ptr->top, proto_func);
                lua_state_ptr->top += 1;
                lua_rawseti(lua_state_ptr, -2, i + 1);
            }
        }
        else
        {
            for (int i = 0; i < proto_size; i++)
            {
                const Proto* target = proto_ptr->p[i];
                lua_pushlightuserdata(lua_state_ptr, (void*)target); // TODO: proto proxy if i ever bother to.
                lua_rawseti(lua_state_ptr, -2, i + 1);
            }
        }
        return 1;
    }
    int debug_getproto(lua_State* lua_state_ptr)
    {
        lua_checkstack(lua_state_ptr, 1);
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
        if (lua_iscfunction(lua_state_ptr, 1))
        {
            luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
            return 0;
        }
        const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
        const Proto* proto_ptr = function_ptr->l.p;
        const int index = (int)lua_tonumber(lua_state_ptr, 2);
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
            lua_pushlightuserdata(lua_state_ptr, (void*)target); // TODO: protoproxy, if i even bother to.
            return 1;
        }
        gc_search_proto ctx{target, {}};
        luaM_visitgco(lua_state_ptr, &ctx, [](void* raw, lua_Page* page, GCObject* gco) -> bool {
            if (gco->gch.tt != LUA_TFUNCTION)
            {
                return false;
            }
            gc_search_proto* c = static_cast<gc_search_proto*>(raw);
            Closure* closure = gco2cl(gco);
            if (!closure->isC && closure->l.p == c->target)
            {
                //std :: cout << "WAAAAAAAAAAAAAAAAAAAAA" << std :: endl;
                c->results.push_back(closure);
            }
            return false;
        });
        lua_newtable(lua_state_ptr);
        for (size_t i = 0; i < ctx.results.size(); i++) // clueless!!
        {
            luaC_threadbarrier(lua_state_ptr);
            setclvalue(lua_state_ptr, lua_state_ptr->top, ctx.results[i]);
            lua_state_ptr->top += 1;
            lua_rawseti(lua_state_ptr, -2, i + 1);
        }
        return 1;
    }
    int debug_setconstant(lua_State* lua_state_ptr)
    {
        lua_checkstack(lua_state_ptr, 1);
        luaL_checktype(lua_state_ptr, 1, LUA_TFUNCTION);
        //luaL_checkany(lua_state_ptr, 2);
        luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
        luaL_checkany(lua_state_ptr, 3);
        if (lua_iscfunction(lua_state_ptr, 1))
        {
            luaL_argerrorL(lua_state_ptr, 1, "Luau closure expected");
            return 0;
        }
        const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr, 1));
        const int target_type = lua_type(lua_state_ptr, 3);
        int index = lua_tonumber(lua_state_ptr, 2);
        //index -= 1; im stupid
        const Proto* proto_ptr = function_ptr->l.p;
        const int constant_size = proto_ptr->sizek;
        //if (index <= 0 or index > constant_size)
        //{
           //luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
            //return 0;
        //}
        TValue* constant_pool = proto_ptr->k;
       // const TValue target_constant = constant_pool[index];
        //if (target_constant.tt != target_type)
        //{
            //luaL_argerrorL(lua_state_ptr, 3, "type mismatch");
            //return 0;
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
                    luaL_argerror(lua_state_ptr, 3, "type-mismatch");
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
        lua_checkstack(lua_state_ptr, 1);
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
        int index = lua_tonumber(lua_state_ptr, 2);
        if (index < 1 || index > upvalue_size)
        {
            luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
            return 0;
        }
        //const Closure* function_ptr = clvalue(luaA_toobject(lua_state_ptr,1));
        //if (!lua_getupvalue(lua_state_ptr, 1, index))
        //{
            //luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
            //return 0;
        //}
        // TODO: no type check, i am not sure if this is unsafe...?

        lua_pushvalue(lua_state_ptr, 3);
        lua_setupvalue(lua_state_ptr, 1, index);
        return 0;
    }
    int debug_getstack(lua_State* lua_state_ptr)
    {
        lua_checkstack(lua_state_ptr, 3);
        int stack_level = 0;
        if (lua_isnumber(lua_state_ptr, 1))
        {
            stack_level = (int)lua_tonumber(lua_state_ptr, 1);
            if (stack_level <= 0)
            {
                luaL_argerrorL(lua_state_ptr, 1, "level out of bounds");
                return 0;
            }
        }
        else if (lua_isfunction(lua_state_ptr, 1))
        {
            stack_level = -lua_gettop(lua_state_ptr);
        }
        else
        {
            luaL_argerrorL(lua_state_ptr, 1, "number or function expected");
            return 0;
        }
        lua_Debug func_option;
        if (!lua_getinfo(lua_state_ptr, stack_level, "f", &func_option))
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
        const CallInfo current_instruction = lua_state_ptr->ci[-stack_level];
        const int frame_size = (current_instruction.top - current_instruction.base); // oh ok
        if (!lua_isnoneornil(lua_state_ptr, 2))
        {
            luaL_checktype(lua_state_ptr, 2, LUA_TNUMBER);
            int stack_index = lua_tonumber(lua_state_ptr, 2) - 1;
            if (stack_index < 0 || stack_index >= frame_size)
            {
                luaL_argerrorL(lua_state_ptr, 2, "index out of bounds");
                return 0;
            }
            const TValue* value = current_instruction.base + stack_index;
            luaC_threadbarrier(lua_state_ptr);
            luaA_pushobject(lua_state_ptr, value);
            return 1;
        }
        lua_newtable(lua_state_ptr);
        int index = 0;
        for (StkId i = current_instruction.base; i < current_instruction.top; i++)
        {
            luaC_threadbarrier(lua_state_ptr);
            luaA_pushobject(lua_state_ptr, i);
            lua_rawseti(lua_state_ptr, -2, ++index);
        }
        return 1;
    }

}