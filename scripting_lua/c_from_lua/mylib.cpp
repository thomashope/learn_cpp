#include <iostream>
#include <lua5.2/lua.hpp>

int an_example(lua_State* state)
{
    // The number of arrguments passed will be on top of the stack
    int args = lua_gettop(state);

    std::cout << "Called with " << args << " arguments." << std::endl;

    for( int i = 0; i < args; ++i )
    {
        std::cout << "\tAgument " << i << ": " << lua_tostring(state, (args-i) * -1 ) << std::endl;
    }

    // If there were 3 arguments passed, they are at position -3, -2, -1 in that order

    lua_pushnumber(state, 117);

    // Tell lua how many values were returned
    return 1;
}

void print_error(lua_State* state)
{
    std::cout << lua_tostring(state, -1) << std::endl;
    lua_pop(state, 1);
}

void execute(const char* filename)
{
    lua_State* state = luaL_newstate();
    std::cout << "Before running " << filename << " the Lua stack has " << lua_gettop(state) << " values." << std::endl;

    luaL_openlibs(state);

    // Make the c function avalibile to Lua and give it a name
    lua_register(state, "a_c_function", an_example);

    if( luaL_loadfile(state, filename) != LUA_OK )
    {
        print_error(state);
        lua_close(state);
        return;
    }

    if( lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK )
    {
        print_error(state);
        lua_close(state);
        return;
    }

    std::cout << "After running " << filename << " the Lua stack has " << lua_gettop(state) << " values." << std::endl;
    lua_close(state);
}

int main(int argc, char* argv[])
{
    if( argc <= 1 )
    {
        std::cout << "pass filenames to execute" << std::endl;
        return 0;
    }

    for( int i = 1; i < argc; ++i )
    {
        execute(argv[i]);
    }

    return 0;
}
