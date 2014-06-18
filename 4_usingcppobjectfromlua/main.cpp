#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <assert.h>
/*****
 * Tutorial concept taken from 
 * http://rubenlaguna.com/wp/2012/12/09/accessing-cpp-objects-from-lua/
 * Modified to be more readable/understandable for me.
 */

/**
 * The class that we are interested in 
 */
class Character
{
public:
    Character(const std::string& n, const int& d = 1, const int& h = 20)
        : name(n), damage(d), health(h)
    {
    }
    
    int damage;
    std::string name;
    int health;

    void dealtDamage(const int& damage)
    {
        health -= damage;
        health = health < 0 ? 0 : health;
    }

    int getDamage() 
    {
        return damage;
    }

    std::string toString()
    {
        return name + ": [Damage : " + std::to_string(damage) + "][Health : " + std::to_string(health) + "]";
    }
};

void putCharacter(lua_State* L, Character& character)
{
    Character** userdata = static_cast<Character**>(lua_newuserdata(L, sizeof(Character*)));
    *userdata = &character;
    // set the attacker metatable
    luaL_setmetatable(L, "CharacterMT");
}

/**
 * Calling a function in Lua from CPP.
 * see part 3 of the examples if this is weird.
 */ 
class ApplyDamageFunction
{
public:
    ApplyDamageFunction(lua_State* state, const std::string& functionname)
        : L(state), name(functionname)
    {
    }
    lua_State* L;
    std::string name;

    /**
     * This method mirrors the function in the lua script.
     */
    void applyDamage(Character& attacker, Character& defender)
    {
        // put the function on the stack
        lua_getglobal(L, name.c_str());
        int type = lua_type(L, -1);
        if(type == LUA_TFUNCTION)
        {
            // create a new user data on the stack, and assign the attacker pointer to it
            putCharacter(L, attacker);
            // create a new user data on the stack, and assign the defender pointer to it
            putCharacter(L, defender);
            // call the function
            lua_call(L, 2, 0);
            // shouldn't have anything to pop
            assert(lua_gettop(L) == 0);
        }
        else
        {
            std::cout << "Cannot find " << name << "function" << std::endl;
        }
    }
};

extern "C" 
{
    //// wrapper method for character /////
    static int function_character_getDamage(lua_State* L)
    {
        // get the user data , pointer to the pointer of character object.
        Character ** character = static_cast<Character**>(luaL_checkudata(L, 1, "CharacterMT"));
        // put the damage value onto the stack.
        int damage = (**character).getDamage();
        lua_pushnumber(L, damage);
        return 1; // the number of values we put into the lua stack. Not the return value
    }

    static int function_character_dealtDamage(lua_State* L)
    {
        // get the user data , pointer to the pointer of character object.
        Character ** character = static_cast<Character**>(luaL_checkudata(L, 1, "CharacterMT"));
        // get the damage to be dealt to this char.
        int damage = luaL_checkint(L, 2);
        // deals the damage
        (**character).dealtDamage(damage);
        return 0; // the number of values we put into the lua stack. Not the return value
    }

    static int function_character_health(lua_State* L)
    {
        int args = lua_gettop(L);
        if(args == 1) // if there are no argument other than self, we will just return the health value
        {
            Character ** character = static_cast<Character**>(luaL_checkudata(L, 1, "CharacterMT"));
            int health = (**character).health;
            lua_pushnumber(L, health);
            return 1;
        }
        else // else, we will set the value to the first argument after "self"
        {
            Character ** character = static_cast<Character**>(luaL_checkudata(L, 1, "CharacterMT"));
            int health = luaL_checkint(L, 2);
            (**character).health = health;
            lua_pushnumber(L, health);
            return 1;
        }
    }
}

void doThings(lua_State* L)
{
    ///////// create the meta table for character class /////////
    // create new meta table for Character
    luaL_newmetatable(L, "CharacterMT");
    // set the meta table of the character meta table to be itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    // set the get damage function
    lua_pushcfunction(L, function_character_getDamage);
    lua_setfield(L, -2, "getDamage");
    // set the dealt damage function
    lua_pushcfunction(L, function_character_dealtDamage);
    lua_setfield(L, -2, "dealtDamage");

    lua_pushcfunction(L, function_character_health);
    lua_setfield(L, -2, "health");

    // reset the stack (meta table is still in the stack)
    lua_settop(L, 0);

    // create the 2 character
    Character attacker("Attacker", 3, 10);
    Character defender("Defender", 1, 20);

    // create the wrapper for the function
    ApplyDamageFunction applyDamageFunction(L, "applyDamage");

    // print the state before the call
    std::cout << "[C++] [Before damage] Attacker Hp : " << attacker.health << " Defender Hp : " << defender.health << std::endl;
    std::cout << "[C++] Calling damage function from lua" << std::endl;

    applyDamageFunction.applyDamage(attacker, defender);

    // print the state after the call 
    std::cout << "[C++] [After damage]  Attacker Hp : " << attacker.health << " Defender Hp : " << defender.health << std::endl;
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
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
        return 1 ;
    }

    doThings(L);

    lua_getglobal(L, "testcharacter");
    Character character("name", 3, 20);
    putCharacter(L, character);
    lua_call(L, 1, 0);
    std::cout << character.toString() << std::endl;

    lua_close(L);
    return 0;
}
