#include <iostream>
#include <lua5.2/lua.hpp>

// An example of running lua code from c
// info from: https://csl.name/post/lua-and-cpp/

// To get it running I installed lua using homebrew (NOTE: i'm using lua 5.2, change the inlucde for your version!)
// compiled using: `clang++ main.cpp -llua -o runlua`
// Run with: `./runlua filename.lua`

// For more info on the lua c api: http://www.lua.org/manual/5.2/manual.html#4

void print_error(lua_State* state)
{
    // Lua interacts with C using a stack
    // -1 reffers to the thing on top of the stack
    // print it, then pop it
    const char* message = lua_tostring(state, -1);
    std::cout << message << std::endl;
    lua_pop(state, 1);
}

void execute(const char* filename)
{
    // Create a new lua state
    lua_State* state = luaL_newstate();

    // Load the lua standard libraries
    luaL_openlibs(state);

    // lua the program, supports both source and bytecode
    int result = luaL_loadfile(state, filename);

    if( result != LUA_OK )
    {
        print_error(state);
        goto FAIL;
    }

    // If there were no errors, execute the program
    result = lua_pcall(state, 0, LUA_MULTRET, 0);

    if( result != LUA_OK )
    {
        print_error(state);
        goto FAIL;
    }

FAIL:
    // We are done with this state...
    lua_close(state);
}

int main(int argc, char* argv[])
{
    if( argc <= 1 )
    {
        std::cout << "Pass lua files as arguments!" << std::endl;
        return 0;
    }

    for( int i = 1; i < argc; ++i )
    {
        execute(argv[i]);
    }

    return 0;
}
