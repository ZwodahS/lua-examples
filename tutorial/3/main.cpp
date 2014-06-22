#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>

/**
 * A Simple function to load and run the file. Instead of just using luaL_dofile, I have split them up to print the proper error message.
 */
bool load(lua_State* L, const std::string& filename)
{
    // load the script
    int status = luaL_loadfile(L, filename.c_str());
    if(status != LUA_OK)
    {
        std::cout << "[C++] error loading script" << std::endl;
        return false;
    }
    std::cout << "[C++] script loaded" << std::endl;
    
    // run the script
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if(result != LUA_OK)
    {
        std::cout << "[C++] Could not run the script." << std::endl;
        return false;
    }
    return true;
}

extern "C"
{
    static int compute(lua_State* L)
    {
        int x = luaL_checkint(L, 1);
        int y = luaL_checkint(L, 2);
        lua_pushnumber(L, x - y);
        return 1;
    }
}

void doThings(lua_State* L)
{
    lua_pushcfunction(L, compute);                                  // push the c function to the stack
    lua_setglobal(L, "compute");                                    // add the function to the global 
    
    load(L, "run.lua");
}

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

    doThings(L);

    lua_close(L);
    return 0;
}
