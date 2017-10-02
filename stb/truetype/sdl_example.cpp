#include <SDL2/SDL.h>
#include <string>

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
// For more of a tutorial on text input with SDL see:
// https://wiki.libsdl.org/Tutorials/TextInput
//

//
// Constants
//

const int WINDOW_WIDTH    =  640;
const int WINDOW_HEIGHT   =  480;

const int BITMAP_SIZE     =  256; // Width and height of the bitmap I want to pack my font into
const int FIRST_CHAR      =   32; // 32 corresponts to a space char in ASCII
const int NUM_CHARS       =   95; // I want to get the 95 charaters following an ASCII space
const int TEXT_MAX_LENGTH = 1024; // The maximum size of our input string
const float CHAR_HEIGHT   = 28.0f; // Height of a character in pixels

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

void drawString( const std::string& str, float x, float y );

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
			int result = stbtt_BakeFontBitmap(
				font_buffer, 0, CHAR_HEIGHT,
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

	//
	// We call this function to let SDL konw want to receive text related events
	//

	SDL_StartTextInput();

	std::string input_text = "Start typing... ";
	bool done = false;
	SDL_Event e;
	while( !done )
	{
		while( SDL_PollEvent(&e) )
		{
			if( e.type == SDL_QUIT ) done = true;
			else if( e.type == SDL_TEXTINPUT )
			{
				if( input_text.length() + strlen(e.text.text) <= TEXT_MAX_LENGTH )
				{
					input_text += std::string(e.text.text);
				}
			}
			else if( e.type == SDL_KEYDOWN )
			{
				if( e.key.keysym.sym == SDLK_BACKSPACE )
				{
					if( input_text.length() > 0 )
					{
						input_text.pop_back();
					}
				}
				else if( e.key.keysym.sym == SDLK_RETURN )
				{
					input_text.push_back('\n');
				}
			}
		}

		SDL_SetRenderDrawColor( ren, 0, 0, 0, 255 );
		SDL_RenderClear( ren );

		// Render the whole font texture so we can see what we got.

		SDL_Rect dest = { 5, 5, BITMAP_SIZE, BITMAP_SIZE };
		SDL_RenderCopy( ren, font_texture, NULL, &dest );
		SDL_SetRenderDrawColor( ren, 255, 255, 255, 255 );
		SDL_RenderDrawRect( ren, &dest );

		// A handy little draw string function

		drawString( "<- This is the font texture\nstb_truetype just made for us", 280.0f, 0.0f );

		drawString( input_text, 5.0f, 280.0f );

		SDL_RenderPresent(ren);
	}

	SDL_StopTextInput();

	shutdown();
	return 0;
}

void drawString( const std::string& str, float x, float y )
{
	const float start_x = x;

	for( const char& c : str )
	{
		if( c == '\n' )
		{
			x = start_x;
			y += CHAR_HEIGHT;
			continue;
		}

		stbtt_aligned_quad char_rect;
		int char_index = c - FIRST_CHAR;

		stbtt_GetBakedQuad(
			char_data,
			BITMAP_SIZE, BITMAP_SIZE,
			char_index,
			&x, &y,
			&char_rect,
			true );

		if( char_rect.x1 > WINDOW_WIDTH )
		{
			//
			// If the right hand edge of our new char will be off screen,
			// reset the x position, increment the y position, and try again.
			//

			x = start_x;
			y += CHAR_HEIGHT;

			stbtt_GetBakedQuad(
				char_data,
				BITMAP_SIZE, BITMAP_SIZE,
				char_index,
				&x, &y,
				&char_rect,
				true );
		}

		// Personally, it makes more sense to me to specify the top left corner
		char_rect.y0 += CHAR_HEIGHT;
		char_rect.y1 += CHAR_HEIGHT;

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
	win = SDL_CreateWindow( __FILE__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
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

	SDL_Quit();
}