#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>

SDL_Window* win = NULL;
SDL_Renderer* ren = NULL;
const int WIDTH = 800;
const int HEIGHT = 600;
bool redraw = true;

struct Point
{
    Point() {}
    ~Point() {}
    float x, y;
    Uint8 r, g, b;
};

std::vector<Point> points;

void init();
void draw();

int main()
{
    init();
        
    SDL_Event event;
    bool done = false;
    while( !done )
    {
        while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT ) done = true;
            if( event.type == SDL_KEYDOWN )
            {
                if( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE )
                {
                   done = true;
                }
                else
                {
                redraw = true;
                }
            }
        }

        if( redraw )
        {
           draw();
           redraw = false;
        }
    }

    SDL_Quit();
    return 0;
}

void init()
{
    SDL_Init( SDL_INIT_EVERYTHING );
    win = SDL_CreateWindow( "Voronoi Basic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN );
    ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

    SDL_SetRenderDrawColor( ren, 0, 0, 0, 0 );
    SDL_RenderClear( ren );
    SDL_RenderPresent( ren );
}

void draw()
{
    SDL_SetRenderDrawColor( ren, 0, 0, 0, 0 );
    SDL_RenderClear( ren );

    // Ensure the vector is empty
    points.clear();    

    for( int i = 0; i < 100; i++ )
    {
        points.push_back(Point());
        points.back().x = ( rand()/float(RAND_MAX) ) * WIDTH;
        points.back().y = ( rand()/float(RAND_MAX) ) * HEIGHT;
        points.back().r = rand() % 256;
        points.back().g = rand() % 256;
        points.back().b = rand() % 256;
    }

    // For every pixel
    // Find the closest point to each pixel
    for( int y = 0; y < HEIGHT; y++ )
    {
        for( int x = 0; x < WIDTH; x++ )
        {
            
            // This version requires `-std=c++14`
            // It's also really slow unless you enable optimisations with `-O3`
            /*
            const auto p = std::min_element( begin(points), end(points),
                     [x, y](const auto& a, const auto& b)
                     { return pow(a.x - x, 2) + pow(a.y - y, 2) < pow(b.x - x, 2) + pow(b.y - y, 2); } );

            SDL_SetRenderDrawColor( ren, p->r, p->g, p->b, 255 );
            SDL_RenderDrawPoint( ren, x, y );
            //*/

            //*
            float dist_squared = WIDTH * WIDTH + HEIGHT * HEIGHT;
            int closest_point = 0;

            for( int i = 0; i < points.size(); i++ )
            {
                float x_dist = points[i].x - x;
                float y_dist = points[i].y - y;

                if( x_dist * x_dist + y_dist * y_dist < dist_squared )
                {
                    closest_point = i;
                    dist_squared = x_dist * x_dist + y_dist * y_dist;
                }
            }

            SDL_SetRenderDrawColor( ren, points[closest_point].r, points[closest_point].g, points[closest_point].b, 255 );
            SDL_RenderDrawPoint( ren, x, y );
            //*/

        }
    }

    // Draw all the points
    for( int i = 0; i < points.size(); i++ )
    {
        SDL_SetRenderDrawColor( ren, 255, 255, 255, 255 );
        SDL_RenderDrawPoint( ren, points[i].x, points[i].y );
    }

    SDL_RenderPresent( ren );
}
