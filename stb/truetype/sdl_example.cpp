#include <SDL2/SDL.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// COMPILING
//
// Compile with `clang++ -lsdl2 main.cpp`

const int BITMAP_SIZE = 512;

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* font_texture;

bool init();
void shutdown();
unsigned char* readEntireFile(const char* filename);

int main()
{
	if( !init() )
	{
		return -1;
	}

	//
	// We will create our bitmap buffer with space for 4 channels of 8bpp, even
	// though stb_truetype only renders 1 greyscale channel, you'll see why in a second
	//

	{
		unsigned char font_bitmap[512*512*4];

		const int FIRST_CHAR = 32;
		const int NUM_CHARS = 95;
		stbtt_bakedchar font_atlas[NUM_CHARS];

		const char* font_filename = "/Library/Fonts/Arial.ttf";
		unsigned char* font_buffer = readEntireFile(font_filename);

		if( font_buffer )
		{
			int result = stbtt_BakeFontBitmap(font_buffer, 0,
				64.0f,
				font_bitmap, BITMAP_SIZE, BITMAP_SIZE,
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

		//
		// Now stb_truetype has rendered a greyscale font into memory, we will convert
		// it to a format SDL can easily accept
		//

		for( int grey_index = BITMAP_SIZE * BITMAP_SIZE - 1; grey_index >= 0; grey_index-- )
		{
			int rgba_index = grey_index * 4 - 1;
			int colour = font_bitmap[grey_index];

			font_bitmap[rgba_index  ] = colour;
			font_bitmap[rgba_index-1] = 255;
			font_bitmap[rgba_index-2] = 255;
			font_bitmap[rgba_index-3] = 255;
		}

		//
		// Now create the texture and upload the font data
		//

		font_texture = SDL_CreateTexture( ren,
			SDL_PIXELFORMAT_RGBA32,
			SDL_TEXTUREACCESS_STATIC,
			BITMAP_SIZE, BITMAP_SIZE );

		if( SDL_UpdateTexture( font_texture, NULL, font_bitmap, BITMAP_SIZE * 4 ) )
		{
			SDL_Log("Error creating texture from font '%s'", font_filename);
			return -1;
		}

		SDL_SetTextureBlendMode( font_texture, SDL_BLENDMODE_BLEND );
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

		SDL_RenderCopy( ren, font_texture, NULL, NULL );

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