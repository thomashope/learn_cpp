#include <SDL2/SDL.h>
#include <iostream>

//// Globals
SDL_Window* win;
SDL_Renderer* ren;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// keys
bool up = false;
bool left = false;
bool right = false;

// Forward declarations
bool init();
void shutdown();
// Press space to enable/disable vsync
//
// The method I use here is bad practice as it destroys the renderer
// and creates a new one. This is because you cannot (afaik) toggle vsync
// after the SDL_Renderer has been created. If using an SDL_GL context though its
// as simple as a function call.
void toggle_vsync();    

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

    // DELTA TIME
    //
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

    // BUT WAIT
    //
    // See how I said the game will run 'similarly' not 'exactly'? Yea, getting perfectly consistent
    // physics isn't quite as easy as multiplying everything by delta_time...
    //
    // A somewhat better solution is using more complicated physics equations:
    // http://stackoverflow.com/questions/17263580/how-do-i-keep-the-jump-height-the-same-when-using-delta-time
    // 
    // A much better solution is to used a fixed delta_time and run the physics independantly of the
    // renderer. Then possibly (depending on the type of game and size of the fixed delta_time) do
    // some interpolation to display the physics simulation 'between' steps, to fix the 'temporal aliasing'
    // that can occur.
    //
    // More info is found here:
    // http://gafferongames.com/game-physics/fix-your-timestep/

    bool done = false;
    SDL_Event e;
    while( !done )
    {
        // Here we calculate the delta time based on the difference between the time now, and the
        // time last frame `ticks` and `old_ticks`. We divide by 1000 because ticks are integers
        // in milliseconds.
        ticks = SDL_GetTicks();
        delta_time = (ticks - old_ticks) / 1000.0f;
        std::cout << "\rms/frame: " << ticks - old_ticks << "     " << std::flush;
        old_ticks = ticks;

        //// Handle keyboard input
        while( SDL_PollEvent( &e ) )
        {
            if( e.type == SDL_QUIT ) done = true;
            if( e.type == SDL_KEYDOWN )
            {
                switch( e.key.keysym.scancode ) {
                    case SDL_SCANCODE_ESCAPE: done = true; break;
                    case SDL_SCANCODE_SPACE: toggle_vsync(); break;
                    case SDL_SCANCODE_UP: up = true; break;
                    case SDL_SCANCODE_LEFT: left = true; break;
                    case SDL_SCANCODE_RIGHT: right = true; break;
                    default: break;
                }
            }
            else if( e.type == SDL_KEYUP )
            {
                switch( e.key.keysym.scancode ) {
                    case SDL_SCANCODE_UP: up = false; break;
                    case SDL_SCANCODE_LEFT: left = false; break;
                    case SDL_SCANCODE_RIGHT: right = false; break;
                    default: break;
                }
            }
        }

        //// Physics
        
        // We modify the velocity rather than changing
        // the position directly. 

        // Apply gravity
        if( !player.on_ground ) {
            player.y_vel += 9800.0f * delta_time;
        }

        // move left and right
        if( left && !right )
            player.x_vel = -player.speed;
        else if( right && !left )
            player.x_vel = player.speed;
        else
            player.x_vel = 0.0f;
        // jump
        if( up && player.on_ground ) {
            player.y_vel -= 2000.0f;
            player.on_ground = false;
        }

        // Apply resultant forces, WITH DELTA TIME
        player.x_pos += player.x_vel * delta_time;
        player.y_pos += player.y_vel * delta_time;

        //// Collision detection
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

        SDL_SetRenderDrawColor( ren, 200, 100, 10, 255 );
        SDL_RenderFillRect( ren, &p );

        // Show the screen
        SDL_RenderPresent(ren);
    }

    shutdown();
    return 0;
}

void toggle_vsync()
{
    static bool toggle = false;
    toggle = !toggle;

    // Destory the old renderer
    SDL_DestroyRenderer( ren );
    // Create a new one with vsync enabled/disabled
    ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | (toggle? SDL_RENDERER_PRESENTVSYNC : 0) );
}

bool init()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    win = SDL_CreateWindow("Delta Time",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN);
    if( !win )
    {
        SDL_Log("ERROR: creating window");
        SDL_Log("%s", SDL_GetError());
        return false;
    }

    // Create the unrestricted renderer
    ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED );
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
