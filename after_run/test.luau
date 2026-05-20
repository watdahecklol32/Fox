--!nocheck
--!nolint
local passed_tests_uint8: number = 0;
local total_tests_uint8: number = 0;
const test_func = function(test_name_str: string, func: (...any) -> (...any)): ()
    const succ: boolean, response: any = pcall(func);
    if (not succ) then
        print("[X] Failed test", test_name_str, response);
    end;
    print("[!] Passed Test", test_name_str);
    total_tests_uint8 += 1;
    return;
end;
test_func("debug.getmetatable | getrawmetatable", function(...: any): ...any
    assert(debug.getmetatable, "debug.getmetatable was not pushed to the debug lib");
    const metatable: {[string]: any} = {
        __index = test_func;
        __metatable = false;
    };
    const object: setmetatable<T, MT> = setmetatable({}, metatable);
    assert(debug.getmetatable(object) == metatable, "debug.getmetatable did not return the correct metatable");
    return;
end);
test_func("debug.setmetatable | setrawmetatable", function(...: any): ...any
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
test_func("debug.getconstants | getconstants", function(...: any): ()
    assert(debug.getconstants, "debug.getconstants was not pushed to the debug lib");
    const func = function(): never
        game:GetService("Workspace")
        game:GetService("Players").LocalPlayer:Kick();
        print("hi");
        return;
    end;
    -- mk there was an oob read, im a fuckin genius
    --[[
        1	game
        2	Workspace
        3	GetService
        4	Players
        5	LocalPlayer
        6	Kick
        7	print   
        8	hi
    ]]
    const constants: {[number]: any} = debug.getconstants(func);
    assert(constants[1] == "game", "First constant should be game");
    assert(constants[2] == "Workspace", "Second constant should be Workspace");
    assert(constants[3] == "GetService", "Third constant should be GetService");
    assert(constants[4] == "Players", "Fourth constant should be Players");
    assert(constants[5] == "LocalPlayer", "Fith constant should be LocalPlayer");
    assert(constants[6] == "Kick", "Sixth constant should be Kick");
    assert(constants[7] == "print", "Seventh constant should be print");
    assert(constants[8] == "hi", "Eightieth constant should be hi");
    -- whoever made the english languge is fucking retarded horrible dogshit
    --table.foreach(constants, print);

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
end)
test_func("isreadonly", function(...: any): ...any
    assert(isreadonly, "isreadonly was not pushed");
    const thing: {} = {};
    table.freeze(thing);
    assert(isreadonly(thing), "wow you failed at the most basic thing, congrats");
end)
test_func("setreadonly", function(...: any): ...any
    assert(setreadonly, "setreadonly was not pushed");
    const thing = {};
    setreadonly(thing, true);
    assert(isreadonly(thing), "it didnt make the fuckin thing readonly");
    setreadonly(thing, false);
    assert(not isreadonly(thing), "setreadonly does not make tables write-able");
    return;
end);