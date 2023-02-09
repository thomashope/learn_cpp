#include <iostream>
#include <SDL2/SDL.h>
#include <lua5.2/lua.hpp>

// compile with: clang++ main.cpp -llua -lsdl2

// Global variables

// SDL variables
const int WIDTH = 640;
const int HEIGHT = 480;
bool done = false;
Uint32 old_ticks = 0;
Uint32 current_ticks = 0;
SDL_Renderer* ren = NULL;
SDL_Window* win = NULL;
SDL_Event event;

struct Colour {
    Colour(char r = 0, char g = 0, char b = 0, char a = 255)
    : red(r), green(g), blue(b), alpha(a) {}
    char red, green, blue, alpha;
} clear_colour;

// Lua variables
const char* LUA_SCRIPT_NAME = "myscript.lua";
lua_State* state = NULL;

// Functions
void print_lua_error(lua_State* state)
{
    std::cout << lua_tostring(state, -1) << std::endl;
    lua_pop(state, 1);
}

void set_clear_colour(int r, int g, int b, int a)
{
    clear_colour.red = r;
    clear_colour.green = g;
    clear_colour.blue = b;
    clear_colour.alpha = a;
}

// A wrapper for lua
int l_set_clear_colour(lua_State* state)
{
    // How many arguments did lua give us?
    // This number is pushed to the top of the stack
    int args = lua_gettop(state);

    // Lets accept 3 or 4 arguments
    if( args < 3 ) {
        std::cout << "Not enough arguments!" << std::endl;
        return 0;
    } else if( args > 4 ) {
        std::cout << "Too many arguments!" << std::endl;
        return 0;
    }

    // So now we know lua gave us an ok number of arguments
    // Fingers crossed they were all numbers ( we could also check this using lua_isnumber() )
    if( args == 4 ) {
        set_clear_colour( lua_tonumber(state, -4), lua_tonumber(state, -3), lua_tonumber(state, -2), lua_tonumber(state, -1) );
    } else {
        set_clear_colour( lua_tonumber(state, -3), lua_tonumber(state, -2), lua_tonumber(state, -1), 255 );
    }

    // This function returns 0 arguments to lua
    return 0;
}

int main()
{
    // The same setup stuff you've seen before
    if( SDL_Init(SDL_INIT_EVERYTHING) )
    {
        std::cout << "ERROR: could not init SDL2" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }

    win = SDL_CreateWindow("scripting with lua", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN );
    if( !win ) {
        std::cout << "ERROR: creating window" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }

    ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( !ren ) {
        SDL_DestroyWindow(win);
        std::cout << "ERROR: creating renderer" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }

    // Now to initialise lua!

    // Create the lua state
    state = luaL_newstate();

    // Open lua standard libraries
    luaL_openlibs(state);

    // Register the c function with lua
    lua_register(state, "set_clear_colour", l_set_clear_colour);

    // dofile is a macro that loads and runs the script at once
    // We have to run the script to make it's contents avalible to the interpreter
    if( luaL_dofile(state, LUA_SCRIPT_NAME) != LUA_OK )
    {
        print_lua_error(state);
        done = true;
    }

    // Find the setup function and push it to the stack
    // If nothing is found getgloabl pushes nil
    lua_getglobal(state, "setup");

    // Check setup is actually a funciton and not some other variable...
    if( lua_isfunction(state, -1) )
    {
        lua_pcall(state, 0, 0, 0);
    }
    else
    {
        std::cout << "Did not find setup() in " << LUA_SCRIPT_NAME << std::endl;
        lua_pop(state, 1);
    }


    // Standard SDL main loop
    while( !done )
    {
        while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT ) done = true;
        }

        // Update the time
        old_ticks = current_ticks;
        current_ticks = SDL_GetTicks();

        // Clear the window
        SDL_SetRenderDrawColor(ren, clear_colour.red, clear_colour.green, clear_colour.blue, clear_colour.alpha);
        SDL_RenderClear(ren);

        // Look for the update() function in our lua script
        lua_getglobal(state, "update");

        // getglobal pushes nil if it didn't find anything
        // Check we got a function, if not pop whatever it was off and ignore it
        if( lua_isfunction(state, -1) )
        {
            lua_pcall(state, 0, 0, 0);
        }
        else
        {
            lua_pop(state, 1);
        }

        // Flip buffers
        SDL_RenderPresent(ren);
    }

    // Cleanup
    lua_close(state);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    return 0;
}
