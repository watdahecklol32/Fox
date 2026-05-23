--!nocheck
--!nolint
--!optimize 0
local passed_tests_uint8: number = 0;
local total_tests_uint8: number = 0;
const test_func = function(test_name_str: string, func: (...any) -> (...any)): ()
    total_tests_uint8 += 1;
    const succ: boolean, response: any = pcall(func);
    if (not succ) then
        print(`[❌] Failed test {test_name_str}: {response}`);
        return;
    end;
    print("[✅] Passed Test", test_name_str);
    passed_tests_uint8 += 1;
    return;
end;
test_func("debug.getmetatable", function(...: any): ...any
    assert(debug.getmetatable, "debug.getmetatable was not pushed to the debug lib");
    const metatable: {[string]: any} = {
        __index = test_func;
        __metatable = false;
    };
    const object: setmetatable<T, MT> = setmetatable({}, metatable);
    assert(debug.getmetatable(object) == metatable, "debug.getmetatable did not return the correct metatable");
    assert(debug.getmetatable({}) == nil, "debug.getmetatable should return nil on objects that have no metatables");
    return;
end);
test_func("debug.setmetatable", function(...: any): ...any
    assert(debug.setmetatable, "debug.setmetatable was not pushed to debug lib");
    const metatable: {[string]: any} = {
        __index = function(...) return; end;
        __metatable = false;
    };
    const object: setmetatable<T, MT> = setmetatable({}, metatable);
    debug.setmetatable(object, {});
    assert(getmetatable(object) ~= metatable, "The metatable did not change.");
    return;
end);
test_func("debug.getconstants", function(...: any): ()
    assert(debug.getconstants, "debug.getconstants was not pushed to the debug lib");
    const func = function(): never
        game:GetService("Workspace")
        game:GetService("Players").LocalPlayer:Kick();
        print("hi");
        return;
    end;
    const constants: {[number]: any} = debug.getconstants(func);
    assert(constants[1] == "game", "First constant should be game");
    assert(constants[2] == "Workspace", "Second constant should be workspace");
    return;
end);
test_func("debug.getupvalues", function(...: any): ...any
    assert(debug.getupvalues, "debug.getupvalues was not pushed to the debug lib");
    const up_value: { } = {};
    const hi_how_is_ur_day = function()
        const x = up_value;
        print(x);
        return;
    end;
    const upvalues: {[number]: any} = debug.getupvalues(hi_how_is_ur_day);
    assert(#upvalues == 1, "Function should have only 1 upvalue");
    assert(upvalues[1] == up_value, "debug.getupvalues does not return upvalues correctly, or at all");
    return;
end);
test_func("debug.getprotos", function(...: any): ...any
    assert(debug.getprotos, "debug.getprotos was not pushed to the debug lib");
    local function test_ok(): ()
        local function meow(): number
            return 1;
        end;
        return;
    end;
    const proto: (...any) -> (...any) = debug.getprotos(test_ok)[1];
    assert(proto, "Did not return any protos");
    assert(debug.info(proto, "s") ~= "[C]" or debug.info(proto, "n") == "meow", "Incorrectly returned a proto")
    assert(proto() == 1, "Proto should return 1");
    return;
end);
test_func("debug.getproto", function(...: any): ...any
    assert(debug.getproto, "debug.getproto was not pushed to the debug lib");
    local function test_ok(): ()
        local function meow(): number
            return 1;
        end;
        return;
    end;
    const proto_proxy: any = debug.getproto(test_ok, 1, false);
    assert(type(proto_proxy) == "userdata", "nuh uh");
    const proto: (...any) -> (...any) = debug.getproto(test_ok, 1, true)[1];
    assert(proto() == 1, "Didnt return one");
    return;
end);
test_func("debug.getconstant", function(...: any): ...any
    assert(debug.getconstant, "debug.getconstant was not pushed to the debug lib");
    const func = function(...: any): ()
        print("");
        local a = "gay";
        print(a);
        return;
    end;
    assert(debug.getconstant(func, 1) == "print", "First constant should be print");
    assert(debug.getconstant(func, 3) == "gay", "Fourth constant should be gay");
    return;
end);
test_func("debug.getupvalue", function(...: any): ()
    assert(debug.getupvalue, "debug.getupvalue was not pushed to the debug lib");
    const upvalue: { } = {};
    const func = function(): ()
        print(upvalue);
        return;
    end;
    assert(debug.getupvalue(func, 1) == upvalue, "First upvalue should be the table thingy");
    return;
end);
test_func("debug.getstack", function(...: any): ...any
   	assert(debug.getstack, "debug.getstack was not pushed to the debug lib");
    local a: string = "a" .. "b";
	assert(debug.getstack(1, 1) == "ab", "The first item in the stack should be 'ab'");
	assert(debug.getstack(1)[1] == "ab", "The first item in the stack table should be 'ab'");    
    return;
end);
test_func("debug.setstack", function(...: any): ...any
    assert(debug.setstack, "debug.setstack was not pushed to the debug lib");
    local a: string = "a".."b";
    debug.setstack(1, 1, "gay");
    assert(debug.getstack(1, 1) == "gay", "debug.setstack does not change items in the stack");
    return;
end);
test_func("debug.setconstant", function(...: any): ...any
    assert(debug.setconstant, "debug.setconstant was not pushed to the debub lib");
    const func = function(...: any): ()
        print("gay", 1, 2, 3);
        return;
    end;
    debug.setconstant(func, 1, "bye");
    assert(debug.getconstant(func, 1) == "bye", "debug.setconstant did not set constant");
    return;
end);
test_func("debug.setupvalue", function(...: any): ...any
    assert(debug.setupvalue, "debug.setupvalue was not pushed to the debug.lib");
    const old_upvalue: { } = {};
    const hi = function(): ()
        table.foreach(old_upvalue, print);
        return;
    end;
    debug.setupvalue(hi, 1, {"hi"});
    hi();
    assert(debug.getupvalue(hi, 1) ~= old_upvalue, "debug.setupvalue did not change the upvalue");
    return;
end);
test_func("debug.setname", function(...: any): ...any
    assert(debug.setname, "debug.setname was not pushed to the debug lib");
    local function old_debug_name(): ()
        return;
    end;
    debug.setname(old_debug_name, 'mreaow :3');
    assert(debug.info(old_debug_name, "n") == "mreaow :3", "debug.setname did not do its job gg");
    return;
end);
test_func("isreadonly", function(...: any): ...any
    assert(isreadonly, "isreadonly was not pushed");
    const thing: {} = {};
    table.freeze(thing);
    assert(isreadonly(thing), "wow you failed at the most basic thing, congrats");
    return;
end);
test_func("setreadonly", function(...: any): ...any
    assert(setreadonly, "setreadonly was not pushed");
    const thing = {};
    setreadonly(thing, true);
    assert(isreadonly(thing), "it didnt make the fuckin thing readonly");
    setreadonly(thing, false);
    assert(not isreadonly(thing), "setreadonly does not make tables write-able");
    return;
end);
test_func("getnamecallmethod", function(...): ...any
    assert(getnamecallmethod, "getnamecallmethod was not pushed");
    const proxy: any = newproxy(true);
    const expected_method_str: string = "hi";
    local method_str: string;
    getmetatable(proxy).__namecall = function(...: any): ()
        method_str = getnamecallmethod();
        return;
    end;
    proxy:hi();
    assert(method_str == expected_method_str, "getnamecallmethod did not return the correct method");
    return;
end);
test_func("setnamecallmethod", function(...: any): ...any
    assert(setnamecallmethod, "setnamecallmethod was not pushed");
    const proxy: any = newproxy(true);
    const expected_new_method_str: string = "bye";
    local method_str: string;
    getmetatable(proxy).__namecall = function(...: any): ()
        setnamecallmethod("bye");
        method_str = getnamecallmethod();
        return;
    end;
    proxy:hi();
    assert(method_str == expected_new_method_str, "setnamecallmethod did not set the method of namecall");
    return;
end)
test_func("iswriteable", function(...): ...any
    assert(iswriteable, "iswriteable was not pushed");
    assert(iswriteable({}), "should be true");
    assert(not iswriteable(table.freeze({})), "should be false");
    return;
end);
test_func("makewriteable", function(...): ...any
    assert(makewriteable, "makewriteable did not get pushed");
    const tbl: { } = {};
    setreadonly(tbl, true);
    makewriteable(tbl);
    tbl.__ = false;
    return;
end);
test_func("makereadonly", function(...: any): ...any
    assert(makereadonly, "makereadonly did not get pushed");
    const tbl: { } = {};
    makereadonly(tbl);
    assert(isreadonly(tbl), "makereadonly did not set readonly");
    return;
end);
test_func("getfflag", function(...: any): ...any
    assert(getfflag, "getfflag was not pushed");
    assert(getfflag("LuauStacklessPcall"), "getfflag did not return true for an enabled FFlag");
    return;
end);
test_func("getgc", function(...: any): ...any
    assert(getgc, "getgc was not pushed");
    const _table: {hi: boolean} = {["hi"] = true};
    const _buffer: buffer = buffer.create(0);
    local function UwU(): ()
        return;
    end;
    const getgc_results: {[number]: (buffer| {[string]: any} | (...any) -> (...any))} = {};
    for _: unknown, v: {[number]: (buffer | {[number]: any} | (...any) -> (...any) | any)} in getgc(true) do
        if (v == _table or v == UwU or v == _buffer) then
            table.insert(getgc_results, v);
        end;
    end;
    assert(#getgc_results == 3, "getgc did not find all the target items");
    return;
end);
test_func("identifyexecutor", function(...: any): ...any
    assert(identifyexecutor, "identifyexecutor was not pushed");
    const str: string, ver: string = identifyexecutor();
    assert(type(str) == "string" and str == "mreower", "wtf 1");
    assert(type(ver) == "string" and ver == "0.0.0", "wtf 2");
    return;
end);
test_func("islclosure", function(...: any): ...any
    assert(islclosure, "was not pushed");
    assert(not islclosure(print), "should be false");
    assert(islclosure(function() end), "should be true");
    return;
end);
test_func("iscclosure", function(...: any): ...any
    assert(iscclosure, "iscclosure was not pushed");
    assert(iscclosure(print), "should be true");
    assert(not iscclosure(function() end), "should be false");
    return;
end);
test_func("clonefunction", function(...: any): ...any
    assert(clonefunction, "clonefunction was not pushed");
    const h = function(): boolean
        return true;
    end;
    const cloned_lclosure: () -> boolean = clonefunction(h);
    assert(cloned_lclosure ~= h, "shouldn't be the same");
    assert(cloned_lclosure(), "should return true");
    const cloned_cclosure: (...number) -> number = clonefunction(bit32.bxor);
    assert(debug.info(cloned_cclosure, "n") == "bxor", "cloned C closures should retain their debug name");
    assert(cloned_cclosure ~= bit32.bxor, "C closure clone shouldnt be the same");
    assert(cloned_cclosure(1, 2) == 3, "should return for XOR of 1 and 2");
    return;
end);
test_func("newcclosure", function(...: any): ...any
    assert(newcclosure, "newcclosure was not pushed");
    const a = function(): ()
        return true;
    end;
    const newcclosured: (...any) -> boolean = newcclosure(a);
    assert(type(newcclosure) == "function", "should've returned a function");
    assert(debug.info(newcclosure, "s") == "[C]" and iscclosure(newcclosured), "should've returned a C closure");
    assert(newcclosured(), "returned incorrect values from the wrapped");
    const newcclosured: (...any) -> boolean = newcclosure(a, "hi");
    assert(debug.info(newcclosured, "n") == "hi", "second parameter should've set the debug name");
    do
        const mewower_func = function(): never
            for i,v in true do end
            return;
        end;
        local passed: boolean = false;
        local reason_str: string = "";
        xpcall(newcclosure(mewower_func), function(error_str: string): ()
            if (error_str ~= "attempt to iterate over a boolean value") then
                reason_str = "error strings dont appear as a C closure error";
                return;
            elseif (debug.info(3, "f") == error or debug.info(3, "f") == pcall) then
                reason_str = "pcall or error was pushed to the stack";
                return;
            elseif (coroutine.isyieldable()) then
                reason_str = "xpcall error handler is yieldable"
                return;
            end;
            for i: number = 0, 50 do
                if (not debug.info(i, "f")) then
                    break;
                end;
                if (debug.info(i, "f") == mewower_func) then
                    reason_str = `Leaked non-wrapped at level {i}`
                    return error();
                end;
            end;
            passed = true;
            return;
        end);
        assert(passed, reason_str);
    end;
    return;
end);
--[[test_func("hookfunction", function(...: any): ...any
    assert(hookfunction, "hookfunction was not pushed");
    -- and here we go!
    do
        -- L->C hook test
        const old_info: {[number]: any} = {debug.info(print, "slnaf")};
        local called: boolean = false;
        local hook: (...any) -> (...any); hook = hookfunction(rawget, function(...: any): ...any
            called = true;
            return hook(...);
        end);
        rawget({}, "");
        assert(called, "C->C hook was not called");
        for i: number, v: any in {debug.info(print, "slnaf")} do
            if (old_info[i] ~= v) then
                error("C->C hook detected", i,v, "expected", old_info[i]);
                break;
            end;
        end;
    end;
    return;
end);]]
test_func("newlclosure", function(...: any): ...any
    assert(newlclosure, "newlclosure didnt get pushed");
    local upvalue_test: boolean = false;
    const wrapped: (...any) -> boolean = newlclosure(function(...: any): boolean
        upvalue_test = true;
        return true;
    end);
    assert(islclosure(wrapped), "islclosure didn't return true");
    assert(wrapped() == true, "didn't return true");
    assert(upvalue_test, "upvalue didn't get set");
    return;
end);
test_func("hookfunction C-L hook", function(...: any): ...any
    assert(hookfunction, "hookfunction was not pushed");
    local called: boolean = false;
    const old_info: {[number]: number} = {debug.info(rawget, "slnaf")};
    local hook: (...any) -> (...any); hook = hookfunction(rawget, function(...: any): ...any
        print("C->L hook called")
        called = true;
        return hook(...);
    end);
    rawget({}, "hi");
    assert(called, "bro lol");
    for i: number, v: any in {debug.info(rawget, "slnaf")} do
        if (old_info[i] ~= v) then
            print("hookdetected", i, old_info[i], v);
        end;
    end;
    return;
end);
test_func("hookfunction C->C hook", function(...): ...any -- shouldnt differ that much
    assert(hookfunction, "hookfunction was not pushed");
    local called: boolean = false;
    const old_info: {[number]: number} = {debug.info(rawlen, "slnaf")};
    const old_func_ptr: (...any) -> (...any) = rawlen;
    print(debug.info(rawlen, "slnaf"));
    local hook: (...any) -> (...any); hook = hookfunction(rawlen, newcclosure(function(...: any): ...any
        print("C->C hook called")
        called = true;
        return hook(...);
    end));
    rawlen({})
    assert(called, "bro lol");
    assert(rawlen == old_func_ptr, "this is just unacceptable");
    print(debug.info(rawlen, "slnaf"));
    return;
end);
test_func("hookfunction L-L", function(...: any): ...any
    assert(hookfunction, "hookfunction was not pushed");
    const hi = function(): boolean
        return true;
    end;
    local called: boolean = false;
    --print(debug.info(hi, "slnaf"));
    hookfunction(hi, function(...: any): boolean
         print("L->L hook called")
        called = true;
        return false;
    end);
    --print(debug.info(hi, "slnaf"));
    hi();
    assert(called, "didnt even call");
    assert(not hi(), "should'dve returned false");
    return;
end);
test_func("hookfunction L->NC", function(...: any): ...any  
    const h: () -> boolean = function()
        return true;
    end;
    local upvalue_test: boolean = false;
    hookfunction(h, newcclosure(function(...)
        upvalue_test = true;
        return false;
    end));
    assert(not h(), "should've returned false");
    assert(upvalue_test, "did not set the upvalue");
    return;
end);
test_func("hookfunction NC->NC", function(...: any): ...any
    const hi = newcclosure(function(): boolean
        return true;
    end);
    local called: boolean = false;
    hookfunction(hi, newcclosure(function(...: any): boolean
        called = true;
        print("NC->NC hook called");
        return false;
    end));
    assert(not hi(), "should've returned false");
    assert(called, "should've set the upvalue");
    return;
end);
test_func("isfunctionhooked", function(...: any): ...any
    assert(isfunctionhooked(rawlen), "should've returned true");
    return;
end);
test_func("restorefunction", function(...: any): ...any
    restorefunction(rawlen);
    assert(not isfunctionhooked(rawlen), "should've unhooked rawlen");
    return;
end);
test_func("hookmetamethod", function(...: any): ...any
    assert(hookmetamethod, "hookmetamethod was not pushed");
    const target_metatable: setmetatable<T, MT> = setmetatable({}, {
        __index = function(...)
            return true;
        end;
        __metatable = "locked";
    });
    local hook_called: boolean = false;
    do
        local hook: (...any) -> (...any); hook = hookmetamethod(target_metatable, "__index", function(...: any): ...any
            hook_called = true;
            if (not hook()) then
                hook_called = false;
                return;
            end;
            return false;
        end);
        pcall(function() const h: boolean = target_metatable.__; return; end);
        assert(hook_called, "hook never got called or 'hook' returned illegal value");
    end;
    hook_called = false;
    do
        const target_proxy: any = newproxy(true);
        const metatable: {[string]: any} = getmetatable(target_proxy);
        metatable.__namecall = function(...: any): ...any
            return true;
        end;
        metatable.__metatable = "locked";
        do
            local hook: (...any) -> (...any); hook = hookmetamethod(target_proxy, "__namecall", function(...: any): ...any
                hook_called = true;
                if (not hook()) then
                    hook_called = false;
                    return;
                end;
                return false;
            end);
            pcall(function(...) target_proxy:__(); end);
            assert(hook_called, "userdata hook never got called or 'hook' returned incorrect value");
        end;
    end;
    return;
end);
test_func("ismetamethodhooked", function(...: any): ...any
    assert(ismetamethodhooked, "ismetamethodhooked was not pushed");
    do
        const metatable: setmetatable<T, MT> = setmetatable({}, {__index = function() end, __metatable = "locked"});
        hookmetamethod(metatable, "__index", function() end);
        assert(ismetamethodhooked(metatable, "__index"), "did not return true for a metamethod hooked with hookmetamethod");
    end;
    return;
end);
test_func("restoremetamethod", function(...: any): ...any
    assert(restoremetamethod, "restoremetamethod was not pushed");
    do
        const metatable: setmetatable<T, MT> = setmetatable({}, {__index = function() end, __metatable = "locked"});
        hookmetamethod(metatable, "__index", function() end);
        restoremetamethod(metatable, "__index");
        assert(not ismetamethodhooked(metatable, "__index"), "did not restore metamethod");
    end;
    return;
end);
test_func("Sanity Checks", function(...: any): ...any
    const runner = function(func: (...any) -> (...any), message_str: string, ...: any): ()
        const succ: boolean, response: any = pcall(func, ...);
        --print(response);
        assert(not succ, message_str);
        return;
    end;
    runner(debug.getconstants, "debug.getconstants should error on C closures", print);
    runner(debug.getmetatable, "debug.getmetatable should error on nil-arugments");
    runner(debug.setmetatable, "debug.setmetatable should error if the 2nd arugment is neither nil or a table", {}, gcinfo);
    runner(debug.getupvalues, "debug.getupvalues should error on C closures", xpcall);
    runner(debug.getupvalues, "debug.getupvalues should error on non-closure targets", newproxy(false));
    runner(isreadonly, "isreadonly should error on non-tables", newproxy(false));
    runner(setreadonly, "setreadonly should error on non-table targets", newproxy(false), true);
    runner(setreadonly, "setreadonly should error on non boolean states", {}, pcall);
    runner(setnamecallmethod, "setnamecallmethod should error on non-strings", newproxy(true));
    runner(iswriteable, "iswriteable should error on non-table objects", print);
    runner(makewriteable, "makewriteable should error on non-table targets", print);
    runner(makereadonly, "makereadonly should error on non-table targets", error);
    runner(getfflag, "getfflag should error on non-string flags", error);
    runner(getfflag, "getfflag should error on invalid flags", "LDKWEQJ;LKDQWJ;DJWQL;KJDWQHFQWEFJHWLFWQHUFIJEW;FOIUWUHFWQHUI;O");
    runner(debug.getconstant, "debug.getconstant should error on C clousres", print, 0);
    runner(debug.getconstant, "debug.getconstant should error on out of bounds index", function() end, 99999);
    runner(debug.getconstant, "debug.getconstant should error on negative index", function() end, -1);
    runner(debug.getconstant, "debug.getconstant should error on non-closure targets", newproxy(false), 1);
    runner(debug.getconstant, "debug.getconstant should error on non-number index", function() end, {});
    --[[do 
        const _buffer: buffer = buffer.create(4); 
        buffer.writeu32(_buffer, 0, 0-1);
        runner(debug.getconstant, "debug.getconstant should error on out of bounds index [1]", function() end, buffer.readu32(_buffer, 0));
    end;]]
    runner(debug.getconstant, "debug.getconstant should error on out of bounds index [2]", function() end, bit32.lshift(0xFFFFFFFF + 5, 0xFFFFFFFF));
    runner(debug.getconstant, "debug.getconstant should error on out of bounds index [3]", function() end, 0);
    runner(debug.getupvalue, "debug.getupvalue should error on C closures", print, 1);
    runner(debug.getupvalue, "debug.getupvalue should error on out of bounds index [1]", function() end, 9);
    runner(debug.getupvalue, "debug.getupvalue should error on non-closure targets [2]", {}, 1);
    runner(debug.getupvalue, "debug.getupvalue should error on non-number index", function() end, 2);
    runner(debug.getprotos, "debug.getprotos should error on C closures", print);
    runner(debug.getprotos, "debug.getprotos should error on non-closure targets", newproxy(false));
    runner(debug.getproto, "debug.getproto should error on C closures", print, 1, true);
    runner(debug.getproto, "debug.getproto should error on non-closure targets", newproxy(false), 1, false);
    runner(debug.getproto, "debug.getproto should error on out of bounds index", function() end, 109000);
    runner(debug.getproto, "debug.getproto should error on out of bounds index", function() end, -1);
    runner(debug.setconstant, "debug.setconstant should error on C closures", print, 1, "");
    runner(debug.setconstant, "debug.setconstant should error on out of bounds index", function() end, 1, "");
    runner(debug.setconstant, "debug.setconstant should error on out of bounds index", function() end, -1, "");
    do
        const hi = function(): ()
            print("hi");
            return;
        end;
        runner(debug.setconstant, "debug.setconstant should error on mismatching constants", hi, 1, newproxy(false));
    end;
    runner(debug.setupvalue, "debug.setupvalue should error on C closures", print, 1, {});
    runner(debug.setupvalue, "debug.setupvalue should error on out of bounds index", function() end, 10000, {});
    runner(debug.setupvalue, "debug.setupvalue should error on out of bounds index", function() end, -1, "");
    runner(debug.setupvalue, "debug.setupvalue should error on non-closures", {}, 1, {});
    runner(debug.setupvalue, "debug.setupvalue should error on non-number indexs", function() end, {}, 4);
    do
        const proxy: any = newproxy(true);
        const metatable: {[string]: any} = getmetatable(proxy);
        metatable.__namecall = function(...: any): ...any
            setnamecallmethod('');
            debug.getstack(3);
            return;
        end;
        const succ: boolean, response: any = pcall(function(...)
            proxy:hi();
        end);
        assert(not succ, "debug.getstack should error in a C closure");
    end;
    runner(debug.getstack, "debug.getstack should error on out of bounds index", 1000);
    runner(debug.getstack, "debug.getstack should error on out of bounds index", -1);
    runner(debug.getstack, "debug.getstack should error on C closures", xpcall);
    runner(debug.getstack, "debug.getstack should error if the second parameter is not a number", 2, newproxy(false));
    runner(debug.setname, "debug.setname should error on non-string name", function() end, newproxy(false));
    runner(debug.setname, "debug.setname should error on a non-closure target", {}, "hi");
    runner(debug.setstack, "debug.setstack should error on C closures", xpcall, 2, {});
   do
        const proxy: any = newproxy(true);
        const metatable: {[string]: any} = getmetatable(proxy);
        metatable.__namecall = function(...: any): ...any
            setnamecallmethod('');
            debug.setstack(3, "", {});
            return;
        end;
        const succ: boolean, response: any = pcall(function(...)
            proxy:hi();
        end);
        assert(not succ, "debug.setstack should error in a C closure");
    end;
    runner(getgc, "getgc should error on non-boolean parameters", {});
    runner(clonefunction, "clonefunction should error on non-closure targets", newproxy(false));
    return;
end);
test_func("hookfunction L-L sanity checks", function(...: any): ...any
    const func = function(): never
        for _: never, _: never in true do end;
        return;
    end;
    local hook_called: boolean = false;
    do
        local hook: (...any) -> (...any);
        const replacement_func = function(...: any): never
            hook_called = true;
            return hook(...);
        end;
        hook = hookfunction(func, replacement_func);
    end;
    local passed: boolean = false;
    xpcall(func, function(...)
        for i: number = 0, 100 do
            if (not debug.info(i, "f")) then
                break;
            end;
            if (debug.info(i, "f") == replacement_func) then
                return;
            end;
        end;
        passed = true;
        return;
    end);
    assert(hook_called, "hook never got called");
    assert(passed, "hook leaked in callstack");
    do
        const func = function(...)
            return;
        end;
        const old_line: number = debug.info(func, "l");
        setfenv(func, {});
        hookfunction(func, function(...: any): ()
            return;
        end);
        const new_line: number = debug.info(func, "l");
        if (getfenv(func) ~= getfenv()) then
            print("enviorment changed - nothing i can do about it");
        end;
        --print(new_line, old_line);
        assert(new_line == old_line, "line changed skull");
    end;
    return;
end);
test_func("hookfunction C->L sanity checks", function(...: any): ...any
    local hook: (...any) -> (...any);
    local called: boolean = false;
    const replacement_func = function(...: any): never
        called = true;
        return hook(...);
    end;
    hook = hookfunction(rawequal, replacement_func);
    do
        local passed: boolean = false;
        xpcall(rawequal, function(...: any): ()
            print("hmm.. i wonder if my error message changed", ...);
            for i: number = 0, 10 do
                if (not debug.info(i, "f")) then
                    break;
                end;
                if (debug.info(i, "f") == replacement_func) then
                    return;
                end;
            end;
            passed = true;
            return;
        end);
        assert(called, "lol");
        assert(passed, "leaked the hook in callstack");
    end;
    return;
end);
test_func("hookfunction same hook-target test", function(...: any): ...any
   local h = function() end;
   hookfunction(h, h);
   assert(not isfunctionhooked(h), "should be false");
   return;
end);

print("Done");
print("Passed:", passed_tests_uint8);
print("Failed:", total_tests_uint8 - passed_tests_uint8);
print("Total:", total_tests_uint8);