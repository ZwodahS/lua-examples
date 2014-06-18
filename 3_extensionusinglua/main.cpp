#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>

class DamageFunction
{
public:
    DamageFunction(lua_State* state, const std::string& functionname)
        : L(state), name(functionname)
    {
    }
    lua_State* L;
    std::string name;

    /**
     * Get the damage based on hero's strength.
     */
    int getDamage(const int& str)
    {
        lua_getglobal(L, name.c_str());
        int type = lua_type(L, -1);
        if(type == LUA_TFUNCTION)
        {
            lua_pushnumber(L, str);
            lua_call(L, 1, 1);
            int damageValue = (int) lua_tointeger(L, -1);
            lua_pop(L, 1);
            return damageValue;
        }
        else
        {
            std::cout << "Cannot find " << name << " function" << std::endl;
            return -1;
        }
    }
};

class Monster
{
public:
    Monster(const std::string& n, const int& str, const DamageFunction& func)
        : name(n), strength(str), damageFunction(func)
    {
    }
    std::string name;
    int strength;
    DamageFunction damageFunction;
    
    int getDamage()
    {
        return damageFunction.getDamage(strength);
    } 
};

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

    int result = result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if(status == LUA_OK)
    {
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
        return 1;
    }

    Monster snake1("Snake1", 4, DamageFunction(L, "snake_damage_func"));
    Monster snake2("Snake2", 5, DamageFunction(L, "snake_damage_func"));
    Monster bear1("Bear1", 5, DamageFunction(L, "bear_damage_func"));

    std::cout << snake1.name << " : Str Value [" << snake1.strength << "] Dmg Value [" << snake1.getDamage()<< "]"<< std::endl;
    std::cout << snake2.name << " : Str Value [" << snake2.strength << "] Dmg Value [" << snake2.getDamage()<< "]"<< std::endl;
    std::cout << bear1.name << " : Str Value [" << bear1.strength << "] Dmg Value [" << bear1.getDamage()<< "]"<< std::endl;

    lua_close(L);
    return 0;
}
