--!nocheck
--!nolint
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
    assert(debug.getconstant(func, 4) == "gay", "Fourth constant should be gay");
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
    print("getfflag: LuauStacklessPcall:", getfflag("LuauStacklessPcall"));
    return;
end);
test_func("getgc", function(...: any): ()
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
    return;
end);
print("Done");
print("Passed:", passed_tests_uint8);
print("Failed:", total_tests_uint8 - passed_tests_uint8);
print("Total:", total_tests_uint8);