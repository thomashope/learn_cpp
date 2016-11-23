#include <SDL2/SDL.h>
#include <Box2D/Box2D.h>
#include <iostream>

// $ c++ main.cpp -l SDL2 -l Box2D
//
// download the zip from github, compile the project using xcode
// Drop the libBox2d.a into /usr/local/lib/
// Drop headers in the folder Box2D into /usr/local/include/

const int WIDTH = 800;
const int HEIGHT = 600;
SDL_Window* win;
SDL_Renderer* ren;

const float M2P = 20.0f; // Meters to pixels conversion
const float P2M = 1.0f / M2P; // Pixels to meters conversion
const float TIME_STEP = 1.0f / 60.0f;
const int MAX_STEPS = 6;
b2World* world;

b2Body* add_rect( int x, int y, int w, int h, bool dynamic = true )
{
	// Create the body
	b2BodyDef body_def;
	body_def.position.Set( x * P2M, y * P2M );
	if( dynamic ) {
		body_def.type = b2_dynamicBody;
	}
	b2Body* body = world->CreateBody( &body_def );

	// TODO: what is a shape exactly?
	b2PolygonShape shape;
	shape.SetAsBox( P2M * w / 2, P2M * h / 2 );

	// TODO: what is a fixture exactly?
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;
	fixture_def.density = 1.0f;
	body->CreateFixture( &fixture_def );

	return body;
}

void draw_rect( b2Vec2* points )
{
	SDL_SetRenderDrawColor( ren, 255, 255, 255, 255 );
	SDL_RenderDrawLine( ren, points[0].x * M2P, points[0].y * M2P, points[1].x * M2P, points[1].y * M2P );
	SDL_RenderDrawLine( ren, points[1].x * M2P, points[1].y * M2P, points[2].x * M2P, points[2].y * M2P );
	SDL_RenderDrawLine( ren, points[2].x * M2P, points[2].y * M2P, points[3].x * M2P, points[3].y * M2P );
	SDL_RenderDrawLine( ren, points[3].x * M2P, points[3].y * M2P, points[0].x * M2P, points[0].y * M2P );
}

// Apply rotation and translation
b2Vec2 transform( const b2Vec2& point, const b2Vec2& centre, float angle )
{
	b2Vec2 result;
	result.x = point.x * cos(angle) - point.y * sin(angle);
	result.y = point.x * sin(angle) + point.y * cos(angle);
	result += centre;
	return result;
}

/* Initialise the application */
bool init()
{
    SDL_Init( SDL_INIT_EVERYTHING );

    // Create the window
	// Set the title, position x y, resolution width height, and then additional flags
	win = SDL_CreateWindow( "My First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN );
	if( !win )
	{
		std::cout << "ERROR: could not create window" << std::endl;
		std::cout << SDL_GetError() << std::endl;
		return false;
	}

	// Create the renderer that we use for drawing
	// Associate if with a window, the index of the rendering driver, addional flags for the renderer
	ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if( !ren )
	{
		SDL_DestroyWindow( win );
		std::cout << "ERROR: could not create renderer" << std::endl;
		std::cout << SDL_GetError() << std::endl;
		return false;
	}

	// Create the Physics world!

	world = new b2World( b2Vec2(0.0f, 9.81f) );
	add_rect( WIDTH / 2, HEIGHT - 50, WIDTH, 30, false );

	return true;
}

// Application main function
int main()
{
    std::cout << "Hello Box2D Physics" << std::endl;

    // Initalise the application
    if( !init() )
    {
        SDL_Quit();
        return 1;
    }

    add_rect( WIDTH / 2, HEIGHT / 2, 30, 30, true );
    add_rect( WIDTH / 2 + 10, HEIGHT / 2 + 40, 30, 30, true );


    SDL_Event e;
    bool done = false;
    Uint32 old_ticks;
    Uint32 ticks;
    float accumulated_time;
    old_ticks = ticks = SDL_GetTicks();

    // Main game loop
    while( !done )
    {
    	// Poll for events
    	while( SDL_PollEvent( &e ) )
    	{
    		if( e.type == SDL_QUIT ) done = true;
    		if( e.type == SDL_KEYDOWN )
    		{
    			switch( e.key.keysym.scancode )
    			{
    				case SDL_SCANCODE_ESCAPE:
    					done = true;
    				break;
    				default:
    				break;
    			}
    		}
    		if( e.type == SDL_MOUSEBUTTONDOWN )
    		{
    			int x, y;
    			SDL_GetMouseState( &x, &y );
    			add_rect( x, y, 30, 30, true );
    		}
    	}

    	// Calculate frame time and add to the accumulated time 
    	ticks = SDL_GetTicks();
    	float dt = (ticks - old_ticks) / 1000.0f;
    	old_ticks = ticks;
    	accumulated_time += dt;

    	// Clear the screen
    	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		// Asssume all the bodies are boxes and draw them
		b2Body* body = world->GetBodyList();
		while( body )
		{
			b2Vec2 points[4];

			for( int i = 0; i < 4; ++i )
			{
				points[i] = ((b2PolygonShape*)body->GetFixtureList()->GetShape())->GetVertex(i);
				points[i] = transform( points[i], body->GetWorldCenter(), body->GetAngle() );
			}

			draw_rect( points );
			body = body->GetNext();
		}

		// Keep a fixed timestep even while the framerate varies
		int steps = 0;
		while( accumulated_time > TIME_STEP )
		{
			world->Step( TIME_STEP, 8, 3 );

			accumulated_time -= TIME_STEP;
			steps++;

			// Prevent the 'spiral of death' due the framerate being unable to catch up with complex physics
			if( steps > MAX_STEPS ) break;
		}

		// Display the screen
		SDL_RenderPresent(ren);
    }

    delete world;
    SDL_DestroyRenderer( ren );
    SDL_DestroyWindow( win );
    SDL_Quit();
    return 0;
}