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

// a multiple return.
std::vector<int> multi(lua_State* L, int x, int y, int z)
{
    std::vector<int> ints;
    lua_getglobal(L, "functions");
    lua_getfield(L, -1, "computation");
    lua_remove(L, -2);
    lua_getfield(L, -1, "multi_compute");
    lua_remove(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
    {   
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_pushnumber(L, z);
        lua_call(L, 3, 3);
        int xy = (int) luaL_checkint(L, -3);
        int yz = (int) luaL_checkint(L, -2);
        int xz = (int) luaL_checkint(L, -1);
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

void doThings(lua_State* L)
{

    load(L, "function.lua");

    //////////////////// Single return value function -- "compute" ////////////////////////

    lua_getglobal(L, "functions");                                  // get the functions tables that we defined
    lua_getfield(L, -1, "computation");                             // get the inner table, we can nest it any how we want
    lua_remove(L, -2);                                              // remove the "functions" table from the stack.
    lua_getfield(L, -1, "compute");                                 // get the compute function from the computation table
    lua_remove(L, -2);                                              // remove the "computations" table from the stack.
    int type = lua_type(L, -1);                                     // get the type of the item that we just put on the stack
    if(type == LUA_TFUNCTION)                                       // check that it is a function
    {
        lua_pushnumber(L, 3);                                       // x
        lua_pushnumber(L, 4);                                       // y
        lua_call(L, 2, 1);                                          // call the function, 2 (argument count), 1 (number of return value)
        int sum = (int) luaL_checkint(L, -1);                       // get the return value from the stack. 
        lua_pop(L, 1);                                              // pop the value from the stack 
        std::cout << "[C++] Return value : " << sum<< std::endl;    // output the value
    }
    else
    {
        std::cout << "[C++] No such function !" << std::endl;
        lua_pop(L, 1);                                              // remove the item from the stack. We don't really know what it is.
    }

    
    ////////////////////// multiple return value function -- "multi_compute" //////////////

    auto ints = multi(L, 1, 3, 5);
    if(ints.size() == 3)
    {
        std::cout << "[C++] X + Y : " << ints[0] << std::endl;
        std::cout << "[C++] Y + Z : " << ints[1] << std::endl;
        std::cout << "[C++] X + Z : " << ints[2] << std::endl;
    }

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
