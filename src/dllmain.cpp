// dllmain.cpp : Defines the entry point for the DLL application.

#include <lua.hpp>
#include <windows.h> // Note this is included BEFORE ScriptUtils cause otherwise it will overwrite some macros like RGB()
#include <ScriptUtils.h> // ScriptUtils.h isn't necessary for a pure lua dll but it's useful to have the native api at your disposal as well

#include <filesystem>
#include <format>
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

// This function emulates how the stock game handles errors in the event callbacks, use it by
// passing in a format string in the modern style using {} in order for lua to fill in the error message
bool LuaCheckStatus(lua_State* L, int statusCode, const char* message)
{
    if (statusCode == LUA_OK)
        return true;

    const char* errorMessage = lua_tolstring(L, -1, nullptr);

    if (errorMessage == nullptr)
        return true;

    std::string formattedMessage = std::vformat(message, std::make_format_args(errorMessage));

    PrintConsoleMessage(formattedMessage.c_str());
    AddToMessagesBox2(formattedMessage.c_str(), RGB(255, 0, 0));

    return false;
}

// This function simulates implementing a callback from C++ into lua, you can 
int SimulateCallback(lua_State* L)
{
    const int specialValue = 5;

    // You should have the callback defined IN your module table, not globally like how the game does it.
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded");
    lua_getfield(L, -1, "library"); // name of your module, in this case "library"
    lua_getfield(L, -1, "MyCallback"); // name of your script-defined callback function
    
    // Check to see if the callback has been defined, if not then don't process it
    if (!lua_isfunction(L, -1))
    {
        return 0;
    }

    lua_pushinteger(L, specialValue); // Push any parameters
    int status = lua_pcall(L, 1, 1, 0); // Read the reference manual for the details on pcall

    // Check the result to make sure the call went through successfully, like the game does
    // You can specifiy the type of error, like how the game might say
    // "Lua script Update Error: [the actual error from lua]
    LuaCheckStatus(L, status, "Runtime Error: {}");

    // Once we're free of errors...
    int result = luaL_checkinteger(L, -1); // You can also get the return value from lua, so users can return values to change the behavior
    
    if (result == 1)
    {
        // Do stuff
    }

    return 0;
}

// Don't do anything stupid like call back from another thread, a method that works well is to
// use callbacks in code injections like this

/*
int __cdecl MyCallback(int param)
{
    // Do stuff...
    return 1;
}

// As part of a larger function...
__asm
{
    push ecx // say an interesting value is in ecx
    call MyCallback // use an explicit calling convention like cdecl and follow it
    add esp, 0x04 // one int parameter is 4 bytes
    mov [retVal], eax // return is in eax, store it in a variable

    // remember to clean up the stack if you allocated space
    // and restore registers
}
*/


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
        { "SimulateCallback", SimulateCallback },
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
