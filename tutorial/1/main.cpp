#include <lua.hpp>
#include <iostream>
#include <vector>
int main(int argc, char* argv[])
{
    // create a new Lua state.
    lua_State* L = luaL_newstate();

    // load Lua libraries
    std::vector<luaL_Reg> lualibs =
        { {"base", luaopen_base} ,
          {"io", luaopen_io} };
    for(auto& it : lualibs)
    {
        // load the required lua libs and store it in the global space.
        luaL_requiref(L, it.name, it.func, true);
        // clear the stack in case there is some remaining stuffs there.
        lua_settop(L, 0);
    }

    // load the script, this do not run the script.
    int status = luaL_loadfile(L, "test.lua");
    if(status == LUA_OK) // okay 
    {
        std::cout << "[C++] script loaded" << std::endl;
    }
    else
    {
        std::cout << "[C++] error loading script" << std::endl;
        return 1;
    }
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if(result == LUA_OK)
    {
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
    }
    //// the above code is the same as luaL_dofile(L, "test.lua")
    lua_close(L);
    return 0;
}
