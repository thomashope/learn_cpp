#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <bullet/btBulletDynamicsCommon.h>

// NOTE: If you are getting strange errors when compiling relating to _SDL_main try uncommenting this line
//#undef main

// This example was created with the tutorials from 'thecplusplusguy' on youtube
// https://www.youtube.com/channel/UC6A2B9G_y-fzAXEu2hHPlMg
// And others:
// - https://gist.github.com/exavolt/2360410

// COMPILING ON OSX
//
// If you don't have it, install homebrew http://brew.sh
// run:
//  $ brew update
//  $ brew install bullet
//  $ brew install SDL2
//
// This should install bullet and SDL2 and link them into /usr/local/lib/ and /usr/local/include/
// The OpenGL frmaework should be installed by default on OSX
//
//  $ clang++ *.cpp -framework OpenGL -l SDL2 -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -I /usr/local/include/bullet/
//
// Should compile all `cpp` files in the current directory into an executable called 'a.out'
//
// NOTE: this command links parts of bullet we aren't actually using but is usefull to have a generic compile command,
// feel free to omit the components you don't use sure as `-lBulletSoftBody`
// NOTE: if you already installed SDL2 as a framework simply replace `-l SDL2` with `-framework SDL2`

const int WIDTH = 640;
const int HEIGHT = 480;
SDL_Window* win;
SDL_GLContext context;

// A vector for our convinience we can iterate over
std::vector<btRigidBody*> bodies;

// TODO: override drawLine in btIDebugDraw and use it for debug drawing

struct PhysicsWorld
{
    // Bullet physics world components
    btDynamicsWorld*            world;
    btDispatcher*               dispatcher;
    btCollisionConfiguration*   collisionConfig;
    btBroadphaseInterface*      broadphase;
    btConstraintSolver*         solver;

    PhysicsWorld()
    : world( NULL )
    , dispatcher( NULL )
    , collisionConfig( NULL )
    , broadphase( NULL )
    , solver( NULL )
    {}
    ~PhysicsWorld() {}

    // Initialise the seperate bullet physics world components 
    void init()
    {
        collisionConfig = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher( collisionConfig );
        broadphase = new btDbvtBroadphase();
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld( dispatcher, broadphase, solver, collisionConfig );
        world->setGravity( btVector3( 0, -10, 0 ) );
    }
    
    void shutdown()
    {
        if( world ) delete world ;
        if( dispatcher ) delete dispatcher;
        if( collisionConfig ) delete collisionConfig;
        if( broadphase ) delete broadphase;
        if( solver ) delete solver;
    }

    // Add a sphere to the world
    btRigidBody* add_sphere( float radius, float x, float y, float z, float mass )
    {
        btTransform t;                          // Stores position and rotation
        t.setIdentity();
        t.setOrigin( btVector3( x, y, z ) );    // Set position
        btSphereShape* sphere = new btSphereShape( radius );
        btVector3 inertia( 0, 0, 0 );           // Inertia is 0 for static object
        if( mass != 0.0f )                      // If the object has mass, let bt figure out the inertia
            sphere->calculateLocalInertia( mass, inertia );

        btMotionState* motion = new btDefaultMotionState( t );
        btRigidBody::btRigidBodyConstructionInfo info( mass, motion, sphere, inertia );
        btRigidBody* body = new btRigidBody( info );    // Create the body
        world->addRigidBody( body );                    // Register it with the world
        bodies.push_back( body );                       // Store it in a vector for our convinience
        return body;
    }
} physics_world;    // Physics world object

struct Camera
{
    float x, y, z;
    float pitch, yaw;
    float move_speed;
    float rotate_speed;

    Camera()
    : x( 0.0f ), y( 2.0f ), z( 6.0f )
    , pitch( 0.0f ), yaw( 0.0f )
    , move_speed( 3.0f )
    , rotate_speed( 50.0f )
    {}
    ~Camera() {}

    void update(float dt)
    {
        const Uint8* keyboard = SDL_GetKeyboardState(NULL);
        
        // Rotation
        if( keyboard[SDL_SCANCODE_UP] )     pitch += rotate_speed * dt;
        if( keyboard[SDL_SCANCODE_DOWN] )   pitch -= rotate_speed * dt;
        if( keyboard[SDL_SCANCODE_LEFT] )   yaw += rotate_speed * dt;
        if( keyboard[SDL_SCANCODE_RIGHT] )  yaw -= rotate_speed * dt;
        // Up and down
        if( keyboard[SDL_SCANCODE_E] )      y += move_speed * dt;
        if( keyboard[SDL_SCANCODE_Q] )      y -= move_speed * dt;
        // Horizontal movement
        if( keyboard[SDL_SCANCODE_W] )
        {
            x -= std::sin( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
            z -= std::cos( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
        }
        if( keyboard[SDL_SCANCODE_S] )
        {
            x += std::sin( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
            z += std::cos( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
        }
        if( keyboard[SDL_SCANCODE_A] )
        {
            x -= std::cos( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
            z += std::sin( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
        }
        if( keyboard[SDL_SCANCODE_D] )
        {
            x += std::cos( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
            z -= std::sin( (yaw / 180.0f) * 3.1415 ) * move_speed * dt;
        }
    }

} camera;       // Camera object

/* Initialise the application */
bool init()
{
    SDL_Init( SDL_INIT_EVERYTHING );

    // Specify the version of OpenGL we want
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create the window ready for OpenGL rendering
    win = SDL_CreateWindow( "Hello Bullet Physics", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL );
    context = SDL_GL_CreateContext(win);
    if( context == NULL ) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
    }

    SDL_Log("OpenGL version %s", glGetString( GL_VERSION) );
    SDL_GL_SetSwapInterval( 1 );

    // Setup OpenGL
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    /* Setup our viewport. */
    float ratio = ( GLfloat )WIDTH / ( GLfloat )HEIGHT;
    glViewport( 0, 0, ( GLsizei )WIDTH, ( GLsizei )HEIGHT );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    return true;
}

// Draw a sphere
void render_sphere( btRigidBody* sphere )
{
    GLUquadricObj* quad = gluNewQuadric();
    glColor3f( 0.8, 0.8, 0.8 );
    float radius = ((btSphereShape*)sphere->getCollisionShape())->getRadius();
    btTransform t;
    sphere->getMotionState()->getWorldTransform( t );
    float matrix[16];
    t.getOpenGLMatrix( matrix );    // Stores the rotation and position
    glPushMatrix();
        glMultMatrixf( matrix );
        gluSphere( quad, radius, 20, 20 );
    glPopMatrix();
    gluDeleteQuadric( quad );
}

// Application main function
int main()
{
    std::cout << "Hello Bullet Physics" << std::endl;

    // Initalise the application
    if( !init() )
    {
        std::cout << "ERROR: could not initialize" << std::endl;
        SDL_Quit();
        return 1;
    }

    // Run init on the world
    physics_world.init();

    // Create the ground plane
    {
        btTransform t;
        t.setIdentity();
        t.setOrigin( btVector3(0,0,0) );
        btStaticPlaneShape* plane = new btStaticPlaneShape( btVector3( 0, 1, 0 ), 0 );
        btMotionState* motion = new btDefaultMotionState( t );
        btRigidBody::btRigidBodyConstructionInfo info( 0.0, motion, plane );
        btRigidBody* ground = new btRigidBody( info );
        physics_world.world->addRigidBody( ground );
        bodies.push_back( ground );
    }

    // Create a few static spheres
    physics_world.add_sphere( 3, 4, 0, 0, 0 );
    physics_world.add_sphere( 2, 1, 0, 1, 0 );
    physics_world.add_sphere( 6, -8, -4, 5, 0 );

    // Main loop
    SDL_Event event;
    bool done = false;
    Uint32 ticks = SDL_GetTicks();
    Uint32 old_ticks = ticks;
    float dt = 1.0f / 60.0f;
    while( !done )
    {
        while( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    if( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
                    if( event.key.keysym.scancode == SDL_SCANCODE_SPACE )
                    {
                        btRigidBody* sphere = physics_world.add_sphere( 1, camera.x, camera.y, camera.z, 0.5 );
                        sphere->setLinearVelocity( btVector3(
                            -std::sin( (camera.yaw / 180.0f) * 3.1415 ) * 20.0f,
                            2.0f,
                            -std::cos( (camera.yaw / 180.0f) * 3.1415 ) * 20.0f
                        ) );

                    }
                break;
                default:
                    break;
            }
        }

        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Position the camera
        camera.update( dt );
        glLoadIdentity();
        glRotatef( -camera.pitch, 1, 0, 0 );
        glRotatef( -camera.yaw, 0, 1, 0 );
        glTranslatef( -camera.x, -camera.y, -camera.z );

        // Draw physics shapes
        for( int i = 0; i < bodies.size(); i++ )
        {
            switch( bodies[i]->getCollisionShape()->getShapeType() )
            {
                case STATIC_PLANE_PROXYTYPE:
                break;
                case SPHERE_SHAPE_PROXYTYPE:
                    render_sphere( bodies[i] );
                break;
                default:
                break;
            }
        }

        // Draw the ground plane
        glBegin( GL_QUADS );
            glColor3f( 0.1, 0.1, 0.1 );
            glVertex3f( -100, 0,  100 );
            glVertex3f( -100, 0, -100 );
            glVertex3f(  100, 0, -100 );
            glVertex3f(  100, 0,  100 );
        glEnd( );

        // Step the physics simulation
        // The btDiscreteDynamicsWorld automatically takes into account variable timestep by performing interpolation
        // instead of simulation for small timesteps. It uses an internal fixed timestep of 60 Hertz.
        physics_world.world->stepSimulation( dt );

        // Present the screen
        SDL_GL_SwapWindow( win );
    }
    
    // Shutdown the application
    physics_world.shutdown();
    SDL_Quit();
    return 0;
}
