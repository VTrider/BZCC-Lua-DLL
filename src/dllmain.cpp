// dllmain.cpp : Defines the entry point for the DLL application.

#include <lua.hpp>
#include <ScriptUtils.h> // ScriptUtils.h isn't necessary for a pure lua dll but it's useful to have the native api at your disposal as well
#include <windows.h>

#include <filesystem>
#include <memory>

// It can be useful to know where exactly the file is from C++, depending on the structure of your mod you can use this
// to relative path to things like config files or anything else you need to access
const std::filesystem::path dllPath = []()
    {
        char path[MAX_PATH];
        GetModuleFileName(GetModuleHandle("library.dll"), path, MAX_PATH); // just make sure this matches the name of your final .dll file
        return std::filesystem::path(path);
    }();

// Mydocs is also a nice folder to have
const std::filesystem::path myDocs = []()
    {
        size_t bufSize = 0;
        GetOutputPath(bufSize, nullptr);
        std::unique_ptr<wchar_t[]> path = std::make_unique<wchar_t[]>(bufSize);
        GetOutputPath(bufSize, path.get());
        return std::filesystem::path(path.get());
    }();

// Define a Lua C function that will be exported, use the Lua 5.2 reference manual to learn about the api
int Hello(lua_State* L)
{
    lua_pushstring(L, "Hello from DLL!");
    return 1; // functions returning a value should return the number of values
}

int Add(lua_State* L)
{
    // The index of parameters is the order in which they are passed
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);

    // To return a result it must be pushed to the stack
    lua_pushinteger(L, x + y);

    return 1; // functions that do not return a value should return 0
}
 
// Define the export table for the lua library, in order for lua to properly load it,
// you must name this function luaopen_[name_of_your_library] and this must match the name of
// the .dll file. So this is library.dll and luaopen_library.
extern "C" int __declspec(dllexport) luaopen_library(lua_State* L)
{
    // If you want to reduce the repetitive typing you could use a macro like this
    // #define LUA_EXPORT(name) { #name, name },
    constexpr luaL_Reg EXPORT_TABLE[] = {
        { "Hello", Hello },
        { "Add", Add },
        { 0, 0 } // the last entry of the export table should be two zeros
    };
    lua_newtable(L);
    luaL_setfuncs(L, EXPORT_TABLE, 0);
    return 1;
}

// As an aside, it's also possible to make a cross platform c++/lua dll that could serve as both a dll mission script as well as a lua library,
// you just need to declare the GetMisnApi properly as well as the lua library export function, they won't run at the same time of course
// but it's a cool idea. You could also make a c++ library that is usable in mission dlls as well as lua missions.

// Regular dllmain do as you please here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
