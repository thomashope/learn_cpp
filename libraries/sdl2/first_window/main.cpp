#include <iostream>
#include <SDL2/SDL.h>

// COMPILING
// Grab the frameworks from the libsdl.org and drop them in `/Library/Frameworks/`
// For some reason on my macbook pro I have to use the -F flag to tell clang where the framework is installed
//
// Compile with `clang++ -F /Library/Frameworks/ -framework SDL2 main.cpp`

SDL_Window* win;
SDL_Renderer* ren;

int main()
{
	// Initialise SDL and check for errors
	if( SDL_Init(SDL_INIT_EVERYTHING) )
	{
		std::cout << "ERROR: could not initialise SDL2" << std::endl;
		std::cout << SDL_GetError() << std::endl;
		return 1;
	}
	else std::cout << "Hello World!" << std::endl;

	// Create the window
	// Set the title, position x y, resolution width height, and then additional flags
	win = SDL_CreateWindow( "My First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN );
	if( !win )
	{
		std::cout << "ERROR: could not create window" << std::endl;
		std::cout << SDL_GetError() << std::endl;
		return 1;
	}

	// Create the renderer that we use for drawing
	// Associate if with a window, the index of the rendering driver, addional flags for the renderer
	ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if( !ren )
	{
		SDL_DestroyWindow( win );
		std::cout << "ERROR: could not create renderer" << std::endl;
		std::cout << SDL_GetError() << std::endl;
		return 1;
	}

	bool done = false;
	SDL_Event e;
	while( !done )
	{
		// At bare minimum our program must process events otherwise the OS will consider it to have stalled
		// the event queue might fill up, the window might be laggy, and more bad things...
		while( SDL_PollEvent(&e) )
		{
			if( e.type == SDL_QUIT ) done = true;
		}

		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderClear(ren);
		SDL_RenderPresent(ren);
	}

	// Clean up resources and exit
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
	SDL_Quit();
	return 0;
}
