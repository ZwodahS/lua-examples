#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>
/**
 * Parts of the tutorial are taken and modified from 
 * http://www.acamara.es/blog/2012/08/running-a-lua-5-2-script-from-c/
 */
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
        luaL_requiref(L, it.name, it.func, 1);
        // clear the stack in case there is some remaining stuffs there.
        lua_settop(L, 0);
    }

    // load the script
    int status = luaL_loadfile(L, "test.lua");
    if(status == 0) // okay 
    {
        std::cout << "[C++] script loaded" << std::endl;
    }
    else
    {
        std::cout << "[C++] error loading script" << std::endl;
        return 1;
    }

    int result = 0;
    if(status == LUA_OK)
    {
        result = lua_pcall(L, 0, LUA_MULTRET, 0);
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
    }

    lua_close(L);
    return 0;
}
