#include "Miscellaneous.hpp"
#include "lua.h"
#include "lualib.h"
#include <cstddef>
#include <exception>
#include <string>
#include <cstring>
#include <cstdio>
#include "iostream"
#include "lobject.h"
#ifdef _WIN32
#include <windows.h>
void set_clipboard(const std::string& text)
{
    if (!OpenClipboard(nullptr))
        return;

    EmptyClipboard();
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hGlob)
    {
        CloseClipboard();
        return;
    }
    void* buffer = GlobalLock(hGlob);
    if (!buffer)
    {
        GlobalFree(hGlob);
        CloseClipboard();
        return;
    }
    memcpy(buffer, text.c_str(), text.size() + 1);
    GlobalUnlock(hGlob);
    SetClipboardData(CF_TEXT, hGlob);
    CloseClipboard();
}
#elif defined(__linux__)
void set_clipboard(const std::string& text)
{
    const char* wayland = getenv("WAYLAND_DISPLAY");
    const char* x11 = getenv("DISPLAY");
    FILE* pipe = nullptr;
    if (wayland)
    {
      pipe = popen("wl-copy", "w");
    }
    else if (x11)
    {
      pipe = popen("xclip -selection clipboard", "w");
    }
    if (pipe)
    {
      fputs(text.c_str(), pipe);
      pclose(pipe);
    }
    return;
}
#endif
namespace Uranium
{
    int identifyexecutor(lua_State* lua_state_ptr)
    {
        lua_pushstring(lua_state_ptr, "mreower");
        lua_pushstring(lua_state_ptr, "0.0.0");
        return 2;
    }
    int setclipboard(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TSTRING);
        const char* provided_str = lua_tostring(lua_state_ptr, 1);
        set_clipboard(provided_str);
        return 0;
    }
    int crash(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TNUMBER);
        const int crash_type = lua_tonumber(lua_state_ptr, 1);
        if (crash_type == 1)
        {
            int* hi = nullptr;
            *hi = 30;
        }else if (crash_type == 2) {
            char* buffer[1];
            buffer[0] = (char*)"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
            strcpy(buffer[0], "h");
         }else if (crash_type == 3)
         {
            __asm__ volatile("hlt");
         }else if (crash_type == 4)
         {
           std :: terminate();
        }else if (crash_type == 5)
        {
            // __asm__ volatile(".intel_syntax noprefix; mov rax, 9999; syscall");
            __asm__ volatile("ud2");
        }
        luaL_argerror(lua_state_ptr, 1, "invalid type");
        return 0;
    }
    int cloneref(lua_State* lua_state_ptr)
    {
        luaL_checktype(lua_state_ptr, 1, LUA_TUSERDATA);
        //const void* userdata = lua_touserdata(lua_state_ptr, 1);
        // taking a look at other implementations they use rbx::pushinstance, and we obviously dont have rbx::pushinstance
        // so i'll just create a new userdata and copy the contents
        const void* userdata = lua_touserdata(lua_state_ptr, 1);
        const int userdata_size = lua_objlen(lua_state_ptr, 1);
        void* new_userdata = lua_newuserdata(lua_state_ptr, userdata_size);
        memcpy(new_userdata, userdata, userdata_size);
        if (lua_getmetatable(lua_state_ptr, 1))
        {
            lua_setmetatable(lua_state_ptr, -2);
        }
        return 1;
    }
}