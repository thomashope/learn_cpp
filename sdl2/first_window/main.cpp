#include <iostream>
#include <SDL2/SDL.h>

// COMPILING
// Grab the frameworks from the libsdl.org and drop them in `/Library/Frameworks/`
// For some reason on my macbook pro I have to use the -F flag to tell clang where the framework is installed
// Compile with `clang++ -F /Library/Frameworks/ -framework SDL2 main.cpp`

SDL_Window* win;
SDL_Renderer* ren;

int main()
{
	if( SDL_Init(SDL_INIT_EVERYTHING) ) {
		std::cout << "ERROR: could not initialise SDL2" << std::endl;
		return 1;
	}
	else std::cout << "Hello World!" << std::endl; 

	win = SDL_CreateWindow( "My First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN );
	ren = SDL_CreateRenderer( win, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

	bool done = false;
	SDL_Event e;
	while( !done )
	{
		while( SDL_PollEvent(&e) )
		{
			if( e.type == SDL_QUIT ) done = true;
		}

		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderClear(ren);
		SDL_RenderPresent(ren);
	}

	SDL_Quit();
	return 0;
}
