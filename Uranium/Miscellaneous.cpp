#include "Miscellaneous.hpp"
#include "lua.h"
#include "lualib.h"
#include <string>
#include <cstring>
#include <cstdio>
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
}