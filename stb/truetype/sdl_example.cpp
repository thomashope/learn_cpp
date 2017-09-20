#include <SDL2/SDL.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

//
// COMPILING
//
// Make sure stb_truetype.h and stb_rect_pack.h are in the same folder as main.cpp
//
// Compile with `clang++ -lsdl2 main.cpp`.
//

//
// Constants
//

const int BITMAP_SIZE = 512;     // Width and height of the bitmap I want to pack my font into
const int FIRST_CHAR  =  32;     // 32 corresponts to a space char in ASCII
const int NUM_CHARS   =  95;     // I want to get the 95 charaters following an ASCII space
const float CHAR_HEIGHT = 32.0f; // Height of a character in pixels

//
// Globals
//

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* font_texture;

stbtt_bakedchar char_data[NUM_CHARS];

//
// Function forward declarations
//

void drawString( const char* str, float x, float y );

unsigned char* readEntireFile( const char* filename );

SDL_Texture* createRGBA32TextureFromGreyscaleData( const unsigned char* greyscale_bitmap );

bool init();

void shutdown();

//

int main()
{
	if( !init() )
	{
		return -1;
	}

	{
		unsigned char font_bitmap[BITMAP_SIZE*BITMAP_SIZE];

		const char* font_filename = "/Library/Fonts/Arial.ttf";
		unsigned char* font_buffer = readEntireFile(font_filename);

		if( font_buffer )
		{
			int result = stbtt_BakeFontBitmap(font_buffer, 0,
				CHAR_HEIGHT,
				font_bitmap, BITMAP_SIZE, BITMAP_SIZE,
				FIRST_CHAR, NUM_CHARS,
				char_data);

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

		font_texture = createRGBA32TextureFromGreyscaleData( font_bitmap );
	}

	bool done = false;
	SDL_Event e;
	while( !done )
	{
		while( SDL_PollEvent(&e) )
		{
			if( e.type == SDL_QUIT ) done = true;
		}

		SDL_SetRenderDrawColor( ren, 0, 0, 0, 255 );
		SDL_RenderClear( ren );

		// Render the whole font texture so we can see what we got.

		SDL_RenderCopy( ren, font_texture, NULL, NULL );

		// A handy little draw string function

		drawString( "It's you Erica,\nit's always been you.", 20.0f, 200.0f );

		SDL_RenderPresent(ren);
	}

	shutdown();

	SDL_Quit();
	return 0;
}

void drawString( const char* str, float x, float y )
{
	const float start_x = x;

	//
	// Loop over the entire string until we hit a NULL char.
	//
	// This is good enough for this example, but in practice is a bad idea.
	// What if our string has an escaped NULL char embedded in it?
	// Or worse, no terminating NULL char at all?
	//

	for( const char* c = str; *c != 0; c++ )
	{
		if( *c == '\n' )
		{
			x = start_x;
			y += CHAR_HEIGHT;
			continue;
		}

		stbtt_aligned_quad char_rect;
		int char_index = (*c) - FIRST_CHAR;

		stbtt_GetBakedQuad(
			char_data,
			BITMAP_SIZE, BITMAP_SIZE,
			char_index,
			&x, &y,
			&char_rect,
			true );

		//
		// The resulting points s0, t0, s1, t0 are in texture coordinates
		// going from [0,1], for SDL we need to convert these to pixel coordinates
		//

		SDL_Rect source_rect {
			(int)(char_rect.s0 * BITMAP_SIZE),
			(int)(char_rect.t0 * BITMAP_SIZE),
			(int)((char_rect.s1 - char_rect.s0) * BITMAP_SIZE),
			(int)((char_rect.t1 - char_rect.t0) * BITMAP_SIZE)
		};

		SDL_Rect dest_rect {
			(int)char_rect.x0,
			(int)char_rect.y0,
			(int)char_rect.x1 - (int)char_rect.x0,
			(int)char_rect.y1 - (int)char_rect.y0
		};


		SDL_RenderCopy( ren, font_texture, &source_rect, &dest_rect );
	}
}

SDL_Texture* createRGBA32TextureFromGreyscaleData( const unsigned char* greyscale_bitmap )
{
	unsigned char rgba32_bitmap[BITMAP_SIZE * BITMAP_SIZE * 4];

	for( int i = 0; i < BITMAP_SIZE * BITMAP_SIZE; i++ )
	{
		rgba32_bitmap[i * 4    ] = 255;
		rgba32_bitmap[i * 4 + 1] = 255;
		rgba32_bitmap[i * 4 + 2] = 255;
		rgba32_bitmap[i * 4 + 3] = greyscale_bitmap[i];
	}

	SDL_Texture* rgba32_texture = SDL_CreateTexture(
		ren,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STATIC,
		BITMAP_SIZE, BITMAP_SIZE );

	if( SDL_UpdateTexture( rgba32_texture, NULL, rgba32_bitmap, BITMAP_SIZE * 4 ) )
	{
		SDL_Log("Error creating texture from data: %s", __func__);
		return NULL;
	}

	SDL_SetTextureBlendMode( rgba32_texture, SDL_BLENDMODE_BLEND );

	return rgba32_texture;
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
	SDL_DestroyTexture( font_texture );
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
}