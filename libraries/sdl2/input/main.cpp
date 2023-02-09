#include <SDL2/SDL.h>

// NOTE: compiling this on my MBP requires using `clang++ main.cpp -framework SDL2 -F /Library/Frameworks/ -std=c++11`

// Some global variables for somplicity
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// A rectangle which will represent the 'player'
// NOTE: This requires -std=c++11
SDL_Rect player{0, 0, 32, 32};
const int PLAYER_SPEED = 5;

int main(int argc, const char * argv[])
{
    if( SDL_Init(SDL_INIT_EVERYTHING) )
    {
	// NOTE: Prefer SDL_Log() to std::cout or printf() as sometimes these don't work as expected on differnt platforms,
	// while SDL tries to do something sensible
	//
	// There are also other flavours of log such as SDL_LogError() and SDL_LogCritical() if you want to get _really_ correct
        SDL_Log("ERROR: initialising SDL2");
        SDL_Log("%s", SDL_GetError());
        return 1;
    }
    else SDL_Log("Hello SDL2!\n");
    
    // Create the window, renderer, and check for errors as we do so
    SDL_Window* win = SDL_CreateWindow("Getting Input!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if( !win )
    {
        SDL_Log("ERROR: creating window");
        SDL_Log("%s", SDL_GetError());
        return 1;
    }
    
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if( !ren )
    {
        SDL_Log("ERROR: creating renderer");
        SDL_Log("%s", SDL_GetError());
        SDL_DestroyWindow(win);
        return 1;
    }
    
    bool done = false;
    SDL_Event e;
    while( !done )
    {
        while( SDL_PollEvent(&e) )
        {
            if( e.type == SDL_QUIT ) done = true;
            
            // The are two basic ways of getting keyboard input in SDL2
            // 1) intercept the SDL_KEYDOWN and SDL_KEYUP events here in the event loop
            // 2) use the SDL_GetKeyboardState function (see below) to get an array containing the keys state
            if( e.type == SDL_KEYDOWN )
            {
                if( e.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
            }
        }
        
        // Clear the renderer to all white
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);
        
        // Update the keyboard
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        
        // NOTE: A proper game would use delta time here so the player would
	// move at a consistant speed independant of the framerate.
	//
        // Move the player with WASD keys
        // key[SCANCODE] is true when the key is down
        if( keys[SDL_SCANCODE_W] ) player.y -= PLAYER_SPEED;
        if( keys[SDL_SCANCODE_S] ) player.y += PLAYER_SPEED;
        if( keys[SDL_SCANCODE_A] ) player.x -= PLAYER_SPEED;
        if( keys[SDL_SCANCODE_D] ) player.x += PLAYER_SPEED;
        
        // Constrain the player to the window
        if( player.x < 0 ) player.x = 0;
        if( player.x + player.w > WINDOW_WIDTH ) player.x = WINDOW_WIDTH - player.w;
        if( player.y < 0 ) player.y = 0;
        if( player.y + player.h > WINDOW_HEIGHT ) player.y = WINDOW_HEIGHT - player.h;
        
        // draw the player
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderFillRect(ren, &player); // The draws the inside of the player rect
        
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderDrawRect(ren, &player); // This does the outline
        
        // Show what we drew
        SDL_RenderPresent(ren);
    }
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
