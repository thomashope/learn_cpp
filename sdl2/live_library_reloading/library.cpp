#include <iostream>
#include <SDL2/sdl.h>

//
// Compile this as a library with `clang++ -dynamiclib library.cpp -o libgame.dylib`
//
// If all is working you should be able to recompile this while the game is
// running and see your changes happen live. If you're not sure what to do
// try changing the variables in the player struct or some of the colours
// in the render function.
//

#define LINK extern "C"

struct Player {
	float x = 100.0f;
	float y = 100.0f;
	const float SPEED = 100.0f;
	const float SIZE = 100.0f;
} player;

LINK void gameUpdate( float dt )
{
	const Uint8* keyboard = SDL_GetKeyboardState( NULL );

	if( keyboard[SDL_SCANCODE_LEFT] )
	{
		player.x -= player.SPEED * dt;
	}
	else if( keyboard[SDL_SCANCODE_RIGHT] )
	{
		player.x += player.SPEED * dt;
	}

	if( keyboard[SDL_SCANCODE_UP] )
	{
		player.y -= player.SPEED * dt;
	}
	else if( keyboard[SDL_SCANCODE_DOWN] )
	{
		player.y += player.SPEED * dt;
	}
}

LINK void gameRender( SDL_Renderer* ren )
{
	// Clear the screen
	SDL_SetRenderDrawColor( ren, 10, 50, 0, 255 );
	SDL_RenderClear( ren );

	// Draw a 'player' rectangle
	SDL_Rect rect;
	rect.x = player.x;
	rect.y = player.y;
	rect.w = rect.h = player.SIZE;

	SDL_SetRenderDrawColor( ren, 255, 255, 255, 255 );
	SDL_RenderDrawRect( ren, &rect );

	// Show graphics
	SDL_RenderPresent( ren );
}