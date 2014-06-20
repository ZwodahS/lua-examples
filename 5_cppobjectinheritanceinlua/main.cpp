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
 *
 * Suppose all unit have damage health but only character have name
 */
class Unit 
{
public:
    Unit(const int& d = 1, const int& h = 20)
        : damage(d), health(h)
    {
    }
    int damage;
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
};

class Character : public Unit
{
public:
    Character(const std::string& n, const int& d = 1, const int& h = 20)
        : Unit(d, h), name(n)
    {
    }
    
    std::string name;

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

void putUnit(lua_State* L, Unit& unit)
{
    Unit** userdata = static_cast<Unit**>(lua_newuserdata(L, sizeof(Unit*)));
    *userdata = &unit;
    luaL_setmetatable(L, "UnitMT");
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
    static int function_unit_getDamage(lua_State* L)
    {
        // test which class called this method.
        // sadly we have to do double checking here.
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        Character** character = static_cast<Character**>(luaL_testudata(L, 1, "CharacterMT"));
        std::cout << "[C++]" << "calling method \"getDamage\"" << std::endl;
        std::cout << "[C++] Class type is : " << (unit ? "Unit" : "Character") << std::endl;
        if(character)
        {
            int damage = (**character).getDamage();
            lua_pushnumber(L, damage);
        }
        else if(unit)
        {
            int damage = (**unit).getDamage();
            lua_pushnumber(L, damage);
        }
        else
        {
            lua_pushnil(L);
        }
        // put the damage value onto the stack.
        return 1; // the number of values we put into the lua stack. Not the return value
    }

    static int function_unit_dealtDamage(lua_State* L)
    {
        // get the user data , pointer to the pointer of character object.
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        Character** character = static_cast<Character**>(luaL_testudata(L, 1, "CharacterMT"));
        // get the damage to be dealt to this char.
        std::cout << "[C++]" << "calling method \"dealtDamage\"" << std::endl;
        std::cout << "[C++] Class type is : " << (unit ? "Unit" : "Character") << std::endl;
        int damage = luaL_checkint(L, 2);
        if(character)
        {
            (**character).dealtDamage(damage);
        }
        else if(unit)
        {
            (**unit).dealtDamage(damage);
        }
        // deals the damage
        return 0; // the number of values we put into the lua stack. Not the return value
    }

    static int function_unit_health(lua_State* L)
    {
        int args = lua_gettop(L);
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        Character** character = static_cast<Character**>(luaL_testudata(L, 1, "CharacterMT"));
        std::cout << "[C++]" << "calling method \"health\"" << std::endl;
        std::cout << "[C++] Class type is : " << (unit ? "Unit" : "Character") << std::endl;
        if(args == 1) // if there are no argument other than self, we will just return the health value
        {
            if(unit)
            {
                lua_pushnumber(L, (**unit).health);
            }
            else if(character)
            {
                lua_pushnumber(L, (**character).health);
            }
            else
            {
                lua_pushnil(L);
            }
            return 1;
        }
        else // else, we will set the value to the first argument after "self"
        {
            int health = luaL_checkint(L, 2);
            if(unit)
            {
                (**unit).health = health;
                lua_pushnumber(L, health);
            }
            else if(character)
            {
                (**character).health = health;
                lua_pushnumber(L, health);
            }
            else
            {
                lua_pushnil(L);
            }
            return 1;
        }
    }
}

void doThings(lua_State* L)
{
    ///////// create the meta table for character class /////////
    luaL_newmetatable(L, "UnitMT");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, function_unit_getDamage);
    lua_setfield(L, -2, "getDamage");
    lua_pushcfunction(L, function_unit_dealtDamage);
    lua_setfield(L, -2, "dealtDamage");
    lua_pushcfunction(L, function_unit_health);
    lua_setfield(L, -2, "health");
    lua_pop(L, 1);
    //
    //
    //
    //
    // create new meta table for Character
    luaL_newmetatable(L, "CharacterMT");
    // set the meta table of the character meta table to be itself
    // lua_pushvalue(L, -1);
    // instead of referencing itself, we reference the unit mt as "parent"
    luaL_getmetatable(L, "UnitMT");
    lua_setfield(L, -2, "__index");
    // pop the meta table from the stack
    lua_pop(L, 1);

    // create the 2 character
    Character attacker("Attacker", 3, 10);
    Unit defender(1, 20);
    

    // print the state before the call
    std::cout << "[C++] [Before damage] Attacker Hp : " << attacker.health << " Defender Hp : " << defender.health << std::endl;
    std::cout << "[C++] Calling damage function from lua" << std::endl;

    // manually do it here, to test the polymorphism 
    // put the function on the stack
    lua_getglobal(L, "applyDamage");
    // create a new user data on the stack, and assign the attacker pointer to it
    putCharacter(L, attacker);
    // create a new user data on the stack, and assign the defender pointer to it
    putUnit(L, defender);
    // call the function
    lua_call(L, 2, 0);
    // shouldn't have anything to pop
    assert(lua_gettop(L) == 0);


    // print the state after the call 
    std::cout << "[C++] [After damage]  Attacker Hp : " << attacker.health << " Defender Hp : " << defender.health << std::endl;


    lua_getglobal(L, "testcharacter");
    putCharacter(L, attacker);
    lua_call(L, 1, 0);
    lua_getglobal(L, "testcharacter");
    putUnit(L, defender);
    lua_call(L, 1, 0);
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

    int result = result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if(status == LUA_OK)
    {
    }
    else
    {
        std::cout << "[C++] Could not run the script." << std::endl;
        return 1 ;
    }

    doThings(L);


    lua_close(L);
    return 0;
}
