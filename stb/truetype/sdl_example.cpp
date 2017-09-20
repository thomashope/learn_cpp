#include <SDL2/SDL.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// COMPILING
//
// Compile with `clang++ -lsdl2 main.cpp`

SDL_Window* win;
SDL_Renderer* ren;

bool init();
void shutdown();
unsigned char* readEntireFile(const char* filename);

int main()
{
	if( !init() )
	{
		return -1;
	}

	const int BITMAP_SIZE = 512;
	unsigned char bitmap[512*512];

	const int FIRST_CHAR = 32;
	const int NUM_CHARS = 95;
	stbtt_bakedchar font_atlas[NUM_CHARS];

	const char* font_filename = "/Library/Fonts/Arial.ttf";
	unsigned char* font_buffer = readEntireFile(font_filename);

	if( font_buffer )
	{
		int result = stbtt_BakeFontBitmap(font_buffer, 0,
			64.0f,
			bitmap, BITMAP_SIZE, BITMAP_SIZE,
			FIRST_CHAR, NUM_CHARS,
			font_atlas);

		if(result > 0)
		{
			SDL_Log("Baked font '%s' OK", font_filename);
		}
		else
		{
			SDL_Log("Error baking font '%s'", font_filename);
		}

		delete[] font_buffer;
	}
	else
	{
		return -1;
	}

	bool done = false;
	SDL_Event e;
	while( !done )
	{
		while( SDL_PollEvent(&e) )
		{
			if( e.type == SDL_QUIT ) done = true;
		}

		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		SDL_Point p;
		unsigned char c = 0;

		for( int y = 0; y < BITMAP_SIZE; y++ )
		{
			for( int x = 0; x < BITMAP_SIZE; x++ )
			{
				p.x = x;
				p.y = y;
				c = bitmap[y * BITMAP_SIZE + x];

				SDL_SetRenderDrawColor( ren, c, c, c, c );
				SDL_RenderDrawPoints( ren, &p, 1 );
			}
		}

		SDL_RenderPresent(ren);
	}

	shutdown();

	SDL_Quit();
	return 0;
}

bool init()
{
	// Initialise SDL and check for errors
	if( SDL_Init(SDL_INIT_EVERYTHING) )
	{
		SDL_Log("ERROR: could not initialise SDL2 '%s'", SDL_GetError());
		return false;
	}

	// Create the window
	// Set the title, position x y, resolution width height, and then additional flags
	win = SDL_CreateWindow( __FILE__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_SHOWN );
	if( !win )
	{
		SDL_Log("ERROR: could not create window '%s'", SDL_GetError());
		return false;
	}

	// Create the renderer that we use for drawing
	// Associate if with a window, the index of the rendering driver, addional flags for the renderer
	ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if( !ren )
	{
		SDL_DestroyWindow( win );
		SDL_Log("ERROR: could not create renderer '%s'", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawBlendMode( ren, SDL_BLENDMODE_BLEND );

	return true;
}

void shutdown()
{
	// Clean up resources and exit
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
}

unsigned char* readEntireFile(const char* filename)
{	
	SDL_RWops *file = SDL_RWFromFile(filename, "r");
	if(!file)
	{
		SDL_Log("Error opening file '%s'", filename);
		return nullptr;
	}

	unsigned char* buffer = new unsigned char[SDL_RWsize(file)];

	if( SDL_RWread(file, buffer, SDL_RWsize(file), 1) == 0 )
	{
		SDL_Log("Error reading file into memory '%s'", filename);
		delete[] buffer;
		return nullptr;
	}

	// Close the file
	SDL_RWclose(file);

	return buffer;
}