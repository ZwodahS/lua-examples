#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>

///////////// Unit Class ////////////
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

void putUnit(lua_State* L, Unit& unit)
{
    /**
     * Create a userdata on the lua stack.
     * This store the pointer to the unit object.
     * The method returns a pointer to the pointer.
     */
    Unit** userdata = static_cast<Unit**>(lua_newuserdata(L, sizeof(Unit*)));
    /**
     * Assign the pointer value to point to our unit object.
     */
    *userdata = &unit;
    /**
     * Set the metatable of that user data.
     */
    luaL_setmetatable(L, "UnitMT");
}

////////////////////////////////
///////// Apply damage function wrapper //////////
class ApplyDamageFunction
{
public:
    ApplyDamageFunction(const std::string& functionname) 
        : function(functionname)
    {
    }

    std::string function;

    /**
     * This method mirrors the function in the lua script.
     */
    void applyDamage(lua_State* L, Unit& attacker, Unit& defender)
    {
        // put the function on the stack
        lua_getglobal(L, function.c_str());
        int type = lua_type(L, -1);
        if(type == LUA_TFUNCTION)
        {
            // create a new user data on the stack, and assign the attacker pointer to it
            putUnit(L, attacker);
            // create a new user data on the stack, and assign the defender pointer to it
            putUnit(L, defender);
            // call the function
            lua_call(L, 2, 0);
            // shouldn't have anything to pop
        }
        else
        {
            std::cout << "Cannot find " << function << "function" << std::endl;
        }
    }
};

////////////////////////////////

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
    //// wrapper method for character /////
    static int function_unit_getDamage(lua_State* L)
    {
        /**
         * luaL_testudata will return a nullptr/0 if the item pass in do not have a metatable of type "UnitMT"
         * else it will return the pointer to the pointer.
         */
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        if(unit)
        {
            int damage = (**unit).getDamage();
            lua_pushnumber(L, damage);
        }
        else
        {
            lua_pushnil(L);
        }
        return 1; 
    }

    static int function_unit_dealtDamage(lua_State* L)
    {
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        int damage = luaL_checkint(L, 2);
        if(unit)
        {
            (**unit).dealtDamage(damage);
        }
        return 0; 
    }

    static int function_unit_health(lua_State* L)
    {
        int args = lua_gettop(L);
        Unit** unit = static_cast<Unit**>(luaL_testudata(L, 1, "UnitMT"));
        std::cout << "[C++]" << "calling method \"health\"" << std::endl;
        if(args == 1) // if there are no argument other than self, we will just return the health value
        {
            if(unit)
            {
                lua_pushnumber(L, (**unit).health);
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
            else
            {
                lua_pushnil(L);
            }
            return 1;
        }
    }
}

void loadWrapper(lua_State* L)
{
    /**
     * This creates the meta table and put it on the stack and give it a name so you can use it later.
     */
    luaL_newmetatable(L, "UnitMT");
    /**
     * __index is where Lua will search when it can't find the methods in the current table, kind of like the parent class
     * So in this case, we will set the it to itself. 
     */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    /**
     * Each of the follow pairs push the cfunction onto the stack and add the function to the metatable.
     * The metatable will still be on the stack after each of the call.
     */
    // set the getDamage method
    lua_pushcfunction(L, function_unit_getDamage);
    lua_setfield(L, -2, "getDamage");
    // set the dealt damage method
    lua_pushcfunction(L, function_unit_dealtDamage);
    lua_setfield(L, -2, "dealtDamage");
    // set the health method
    lua_pushcfunction(L, function_unit_health);
    lua_setfield(L, -2, "health");
    // pop the meta table from the stack
    lua_pop(L, 1);
}

void doThings(lua_State* L)
{
    // load the function.
    load(L, "functions.lua");
    // load the Unit Wrapper.
    loadWrapper(L);

    ApplyDamageFunction damageFunction("applyDamage");

    Unit unit1(12, 30);
    Unit unit2(7, 40);
    
    std::cout << "Health of unit1 and unit2 before unit1 deals damage to unit2 : [Unit1 : " << unit1.health << "] [Unit2 : " << unit2.health << "]" << std::endl;
    damageFunction.applyDamage(L, unit1, unit2);
    std::cout << "Health of unit1 and unit2 after unit1 deals damage to unit2 : [Unit1 : " << unit1.health << "] [Unit2 : " << unit2.health << "]" << std::endl;
    damageFunction.applyDamage(L, unit2, unit1);
    std::cout << "Health of unit1 and unit2 after unit2 deals damage to unit1 : [Unit1 : " << unit1.health << "] [Unit2 : " << unit2.health << "]" << std::endl;
    
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
