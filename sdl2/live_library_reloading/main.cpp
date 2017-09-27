#include <SDL2/sdl.h>
#include <sys/stat.h>

//
// Globals
//

SDL_Window* win;
SDL_Renderer* ren;

bool init();
void shutdown();

int  getTimeModified( const char* filepath );
bool getLibraryFunctions();

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

//
// The path to the library we want to load dynamically
//

const char* LIB_FILENAME = "libgame.dylib";

//
// Handle to the loaded library
//

void* libgame = NULL;

//
// Function pointers to the functions we care about in the library
//

void (*gameUpdate)(float) = NULL;
void (*gameRender)(SDL_Renderer*) = NULL;

int main()
{
	init();

	//
	// Load a handle to the library
	//

	libgame = SDL_LoadObject( LIB_FILENAME );
	if( libgame == NULL )
	{
		SDL_Log("Error loading library %s: %s", LIB_FILENAME, SDL_GetError());
		return -1;
	}

	if( !getLibraryFunctions() )
	{
		return -1;
	}
	
	Uint32 ticks = SDL_GetTicks();
	Uint32 oldTicks = ticks;
	SDL_Event event;
	bool done = false;

	while( !done )
	{
		// Whatever we are doing, we have to poll the event loop

		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT ) done = true;
			else if( event.type == SDL_KEYDOWN )
			{
				if( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
			}
		}

		//
		// Check if the library should be reloaded
		//

		{
			static int lastTimeModified = 0;
			int currentTimeModified = getTimeModified( LIB_FILENAME );

			if( currentTimeModified >= 0 && currentTimeModified != lastTimeModified )
			{
				// The time modified changed, so we should reload the library 

				if( libgame )
				{
					SDL_UnloadObject( libgame );
				}

				libgame = SDL_LoadObject( LIB_FILENAME );
				if( libgame == NULL )
				{
					SDL_Log("Error reloading library %s: %s", LIB_FILENAME, SDL_GetError());
					break;
				}

				// Reload the functions within the library

				if( !getLibraryFunctions() )
				{
					SDL_Log("Error failed to get library functions");
					break;
				}

				lastTimeModified = currentTimeModified;
				SDL_Log("Loaded %s last modified at %i", LIB_FILENAME, currentTimeModified);
			}
		}

		// Calculate delta time
		oldTicks = ticks;
		ticks = SDL_GetTicks();
		float dt = (ticks - oldTicks) / 1000.0f;

		// Call the libray update function
		gameUpdate( dt );

		// Call the library render function
		gameRender( ren );
	}


	// Cleanup
	SDL_UnloadObject( libgame );
	shutdown();
	return 0;
}

bool getLibraryFunctions()
{
	//
	// Now get pointers to the functions we care about. This requires that
	// the library has declared them with 'c linkage', which means prefixing
	// the definiton with `extern "C"`.
	//

	const char* functionName = NULL;

	functionName = "gameUpdate";
	gameUpdate = (void (*)(float)) SDL_LoadFunction(libgame, functionName);
	if( gameUpdate == NULL )
	{
		SDL_Log( "Error finding function: %s", functionName );
		return false;
	}

	functionName = "gameRender";
	gameRender = (void (*)(SDL_Renderer*)) SDL_LoadFunction(libgame, functionName);
	if( gameRender == NULL )
	{
		SDL_Log( "Error finding function: %s", functionName );
		return false;
	}

	return true;
}

int getTimeModified( const char* filepath )
{
	struct stat libraryStatus;

	if( stat( LIB_FILENAME, &libraryStatus ) != 0 )
	{
		SDL_Log("Error getting library status for %s", LIB_FILENAME);
		return -1;
	}

	return libraryStatus.st_mtimespec.tv_sec;
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

	return true;
}

void shutdown()
{
	// Clean up resources and exit
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
}