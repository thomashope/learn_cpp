#include <SDL2/SDL.h>

// Globals
SDL_Window* win;
SDL_Renderer* ren;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// keys
bool up = false;
bool down = false;
bool left = false;
bool right = false;

// Forward declarations
bool init();
void shutdown();

// Player class
class Player
{
public:
    Player() :
        x_pos(0.0f), y_pos(0.0f),
        x_vel(0.0f), y_vel(0.0f),
        w(32), h(32),
        speed(300.0f),
        on_ground(false) {}
    ~Player() {}

    float x_pos, y_pos;     // Position
    float x_vel, y_vel;     // Velocity
    int w, h;               // Dimensions
    float speed;            // Pixels per second
    bool on_ground;         // True when the player is touching the ground
} player;

int main()
{
    if( !init() ) return 1;

    // Different computers will run our application at different frame rates. This means if
    // objects are updated a set amount per frame they application will behave differently as
    // the framerate changes.
    //
    // To fix this we should base time dependant things such as physics on the amount of time elapsed
    // between each frame, known as delta time. Using delta time will ensure the game runs similarly
    // event as the framerate varies
    //
    Uint32 ticks = SDL_GetTicks();  // SDL_GetTicks() returns the time since SDL was initialised in milliseconds
    Uint32 old_ticks = ticks;
    float delta_time;

    bool done = false;
    SDL_Event e;
    while( !done )
    {
        // Here we calculate the delta time based on the difference between the time now, and the
        // time last frame `ticks` and `old_ticks`. We divide by 1000 because ticks are integers
        // in milliseconds.
        ticks = SDL_GetTicks();
        delta_time = (ticks - old_ticks) / 1000.0f;
        old_ticks = ticks;

        //// Handle keyboard input
        while( SDL_PollEvent( &e ) )
        {
            if( e.type == SDL_QUIT ) done = true;
            if( e.type == SDL_KEYDOWN )
            {
                switch( e.key.keysym.scancode )
                {
                    case SDL_SCANCODE_ESCAPE: done = true; break;
                    case SDL_SCANCODE_UP: up = true; break;
                    case SDL_SCANCODE_DOWN: down = true; break;
                    case SDL_SCANCODE_LEFT: left = true; break;
                    case SDL_SCANCODE_RIGHT: right = true; break;
                    default: break;
                }
            }
            if( e.type == SDL_KEYUP )
            {
                switch( e.key.keysym.scancode )
                {
                    case SDL_SCANCODE_UP: up = false; break;
                    case SDL_SCANCODE_DOWN: down = false; break;
                    case SDL_SCANCODE_LEFT: left = false; break;
                    case SDL_SCANCODE_RIGHT: right = false; break;
                    default: break;
                }
            }
        }

        //// Physics using delta time

        // Apply gravity
        if( !player.on_ground ) {
            player.y_vel += 9.8f;
        }

        // move left and right
        if( left && !right )
            player.x_vel = -player.speed;
        else if( right && !left )
            player.x_vel = player.speed;
        else
            player.x_vel *= 0.8f;
        // jump
        if( up && player.on_ground ) {
            player.y_vel -= 1000.0f;
            player.on_ground = false;
        }
        // TODO: ground slam with down key

        // Collision detection
        if( player.x_pos < 0 ) {
            player.x_pos = 0;
            if( player.x_vel < 0 ) {
                player.x_vel = 0;
            }
        }
        if( player.x_pos > WINDOW_WIDTH - player.w ) {
            player.x_pos = WINDOW_WIDTH - player.w;
            if( player.x_vel > 0 ) {
                player.x_vel = 0;
            }
        }
        if( player.y_pos > WINDOW_HEIGHT - player.h ) {
            player.y_pos = WINDOW_HEIGHT - player.h;
            player.on_ground = true;
            if( player.y_vel > 0 ) {
                player.y_vel = 0;
            }
        }

        // Apply resultant forces, WITH DELTA TIME!
        player.x_pos += player.x_vel * delta_time;
        player.y_pos += player.y_vel * delta_time;

        //// Rendering

        // Clear the renderer
        SDL_SetRenderDrawColor( ren, 255, 255, 255, 255 );
        SDL_RenderClear(ren);

        // Draw the player
        SDL_Rect p;
        p.x = player.x_pos;
        p.y = player.y_pos;
        p.w = player.w;
        p.h = player.h;

        // TODO: Draw the FPS somehow

        SDL_SetRenderDrawColor( ren, 200, 100, 10, 255 );
        SDL_RenderFillRect( ren, &p );

        // Show the screen
        SDL_RenderPresent(ren);
    }

    shutdown();
    return 0;
}

bool init()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    win = SDL_CreateWindow("Delta Time", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if( !win )
    {
        SDL_Log("ERROR: creating window");
        SDL_Log("%s", SDL_GetError());
        return false;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if( !ren )
    {
        SDL_Log("ERROR: creating renderer");
        SDL_Log("%s", SDL_GetError());
        SDL_DestroyWindow(win);
        return false;
    }
    
    return true;
}

void shutdown()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
