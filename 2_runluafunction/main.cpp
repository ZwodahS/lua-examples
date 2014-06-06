#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>


// a multiple return.
std::vector<int> multi(lua_State* L, int x, int y, int z)
{
    std::vector<int> ints;
    lua_getglobal(L, "multi"); 
    if(lua_type(L, -1) == LUA_TFUNCTION)
    {   
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_pushnumber(L, z);
        lua_call(L, 3, 3);
        int xy = (int) lua_tointeger(L, -3);
        int yz = (int) lua_tointeger(L, -2);
        int xz = (int) lua_tointeger(L, -1);
        lua_pop(L, 3);
        ints.push_back(xy);
        ints.push_back(yz);
        ints.push_back(xz);
    }
    else
    {
        lua_pop(L, 1);
    }
    return ints;
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

    // load the script
    int status = luaL_loadfile(L, "function.lua");
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
        // load the script 
        result = lua_pcall(L, 0, LUA_MULTRET, 0);

        // get the global function "add" and put it on the stack
        lua_getglobal(L, "add");
        // check if it is a function
        int type = lua_type(L, -1);
        if(type == LUA_TFUNCTION)
        {
            // if function, push the argument in. 
            lua_pushnumber(L, 3); // x
            lua_pushnumber(L, 4); // y
            lua_call(L, 2, 1); // call the function, 2 (argument count), 1 (number of return value)
            int sum = (int) lua_tointeger(L, -1); // get the return value from the stack. 
            lua_pop(L, 1); // pop the value from the stack 
            std::cout << "[C++] Return value : " << sum<< std::endl; // output the value
        }
        else
        {
            std::cout << "[C++] No such function !" << std::endl;
            lua_pop(L, 1);
        }
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
    }

    auto ints = multi(L, 1, 3, 5);
    if(ints.size() == 3)
    {
        std::cout << "[C++] X + Y : " << ints[0] << std::endl;
        std::cout << "[C++] Y + Z : " << ints[1] << std::endl;
        std::cout << "[C++] X + Z : " << ints[2] << std::endl;
    }
    lua_close(L);
    return 0;
}
