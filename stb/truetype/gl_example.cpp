#include "SDL2/sdl.h"
#include <GL/glew.h>
#include <string>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

//
// Compile with `clang++ -framework opengl -lsdl2 -lglew -std=c++11 gl_example.cpp`
//

//
// Constants
//

const int WINDOW_WIDTH    =   640;
const int WINDOW_HEIGHT   =   480;

const int BITMAP_SIZE     =   512; // Width and height of the bitmap I want to pack my font into
const int FIRST_CHAR      =    32; // 32 corresponts to a space char in ASCII
const int NUM_CHARS       =    95; // I want to get the 95 charaters following an ASCII space
const int TEXT_MAX_LENGTH =  1024; // The maximum size of our input string
const float CHAR_HEIGHT   = 80.0f; // Height of a character in pixels

//
// Globals
//

SDL_Window*   sdl_window     = NULL;
SDL_GLContext sdl_gl_context = NULL;

GLuint simple_texture_shader = 0;
GLuint font_texture          = 0;
GLuint vao                   = 0;
GLuint vbo                   = 0;
                              // x, y, z,  r, g, b, a,  s, t
GLfloat verts[9*6]           = { -1,  1, 0,  1, 1, 1, 1,  0, 0,
                                  1,  1, 0,  1, 1, 1, 1,  1, 0,
                                 -1, -1, 0,  1, 1, 1, 1,  0, 1,
                                  1,  1, 0,  1, 1, 1, 1,  1, 0,
                                  1, -1, 0,  1, 1, 1, 1,  1, 1,
                                 -1, -1, 0,  1, 1, 1, 1,  0, 1 };

stbtt_bakedchar char_data[NUM_CHARS];

//
// Function forward declarations
//

unsigned char* createBitmapFromFont( const unsigned char* font_buffer );
unsigned char* readEntireFile( const char* filename );
GLuint create_shader( GLenum type, const char* source );
GLuint create_program( GLuint vertex_shader, GLuint fragment_shader );

bool init();
void shutdown();

int main()
{
    if( !init() )
    {
        return -1;
    }

    const char*    font_filename        = "/Library/Fonts/Arial.ttf";
    unsigned char* font_file            = nullptr;
    unsigned char* font_bitmap          = nullptr;
    bool           createdFontTexture   = false;

    font_file = readEntireFile( font_filename );
    if( font_file )
    {
        font_bitmap = createBitmapFromFont( font_file );
        if( font_bitmap )
        {
            SDL_Log("Baked font bitmap ok");

            const char* vertex_src =
                R"(#version 150 core
                in vec3 vPos;
                in vec4 vCol;
                in vec2 vTex;
                out vec4 fCol;
                out vec2 fTex;
                void main()
                {
                   fCol = vCol;
                   fTex = vTex;
                   gl_Position = vec4(vPos, 1.0);
                })";
            const char* fragment_src =
                R"(#version 150 core
                uniform sampler2D tex;
                in vec4 fCol;
                in vec2 fTex;
                out vec4 outColour;
                void main()
                {
                    float c = texture(tex, fTex).r;
                    outColour = fCol * vec4(c,c,c,c);
                })";

            // Create the shader
            GLuint vertex_shader = create_shader( GL_VERTEX_SHADER, vertex_src );
            GLuint fragment_shader = create_shader( GL_FRAGMENT_SHADER, fragment_src );
            simple_texture_shader = create_program( vertex_shader, fragment_shader );

            glGenVertexArrays( 1, &vao );
            glBindVertexArray( vao );
            glGenBuffers( 1, &vbo );
            glBindBuffer( GL_ARRAY_BUFFER, vbo );

            // Setup attribute arrays
            GLint posAtrib = glGetAttribLocation( simple_texture_shader, "vPos" );
            if( posAtrib == -1 ) { SDL_Log("ERROR: Position attribute not found in shader\n"); }
            glEnableVertexAttribArray( posAtrib );
            glVertexAttribPointer( posAtrib, 3, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), 0 );

            GLint colAtrib = glGetAttribLocation( simple_texture_shader, "vCol" );
            if( colAtrib == -1 ) { SDL_Log("ERROR: Colour attribute not found in shader\n"); }
            glEnableVertexAttribArray( colAtrib );
            glVertexAttribPointer( colAtrib, 4, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), (void*)(3*sizeof(float)) );

            GLint texAtrib = glGetAttribLocation( simple_texture_shader, "vTex" );
            if( texAtrib == -1 ) { SDL_Log("ERROR: Texture attribute not found in shader\n"); }
            glEnableVertexAttribArray( texAtrib );
            glVertexAttribPointer( texAtrib, 2, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), (void*)(7*sizeof(float)) );

            GLint texUniform = glGetUniformLocation(simple_texture_shader, "tex");
            if( texUniform == -1 ) { SDL_Log("ERROR: Texture sampler not found in shader\n"); }
            glUniform1i( texUniform, 0 );

            // Setup the texture
            glGenTextures( 1, &font_texture );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, font_texture );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, BITMAP_SIZE, BITMAP_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, font_bitmap ); 

            // Set the wraping mode for the texture
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Send the vertex data
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);

            createdFontTexture = true;
        }
    }

    delete[] font_file;
    delete[] font_bitmap;

    if( !createdFontTexture )
    {
        SDL_Log("Something went wrong while creating the font texture...");
        return -1;
    }

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
                if (e.key.keysym.sym == SDLK_ESCAPE ) done = true;
                else if( e.key.keysym.sym == SDLK_BACKSPACE )
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
        
        // Clear the screen
        glClearColor( 0, 0.2, 0, 0 );
        glClear( GL_COLOR_BUFFER_BIT );

        // Draw buffers
        glUseProgram( simple_texture_shader );
        
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, font_texture );
        
        glBindVertexArray( vao );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );

        glDrawArrays( GL_TRIANGLES, 0, sizeof(verts)/9 );

        // Show graphics
        SDL_GL_SwapWindow( sdl_window );
    }

    shutdown();
    return 0;
}

unsigned char* createBitmapFromFont( const unsigned char* font_buffer )
{
    unsigned char* font_bitmap = new unsigned char[BITMAP_SIZE * BITMAP_SIZE];

    int result = stbtt_BakeFontBitmap(
        font_buffer, 0, CHAR_HEIGHT,
        font_bitmap, BITMAP_SIZE, BITMAP_SIZE,
        FIRST_CHAR, NUM_CHARS,
        char_data);

    if(result > 0)
    {
        return font_bitmap;
    }
    else
    {
        SDL_Log("ERROR: creating bitmap from font");
        return nullptr;
    }
}

unsigned char* readEntireFile(const char* filename)
{   
    SDL_RWops *file = SDL_RWFromFile(filename, "r");
    if(!file)
    {
        SDL_Log("ERROR: opening file '%s'", filename);
        return nullptr;
    }

    unsigned char* buffer = new unsigned char[SDL_RWsize(file)];

    if( SDL_RWread(file, buffer, SDL_RWsize(file), 1) == 0 )
    {
        SDL_Log("ERROR: reading file into memory '%s'", filename);
        delete[] buffer;
        return nullptr;
    }

    // Close the file
    SDL_RWclose(file);

    return buffer;
}

GLuint create_shader( GLenum type, const char* source )
{
    GLuint shader = glCreateShader( type );
    if( shader == 0 )
    {
        SDL_Log("ERROR: could not create shaders!\n"); 
        return shader;
    }

    // Compile and check the vertex shader
    glShaderSource( shader, 1, &source, NULL );
    glCompileShader( shader );
    GLint status = GL_TRUE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE )
    {
        SDL_Log( "ERROR: draw could not compile %d shader\n", type );
        // Get the length of the error log
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        // Now get the error log itself
        GLchar buffer[log_length];
        glGetShaderInfoLog( shader, log_length, NULL, buffer );
        SDL_Log("%s\n", buffer);

        // Clean up the resources, not that it really matters at this point
        // we should really just crash
        glDeleteShader( shader );
        shader = 0;
    }

    return shader;
}

GLuint create_program( GLuint vertex_shader, GLuint fragment_shader )
{
    GLuint program = glCreateProgram();
    glAttachShader( program, vertex_shader );
    glAttachShader( program, fragment_shader );
    glBindFragDataLocation( program, 0, "outColour" );
    glLinkProgram( program );
    glUseProgram( program );
    glDeleteShader( vertex_shader );
    glDeleteShader( fragment_shader );
    return program;
}

bool init()
{   
    // Initialise SDL and check for errors
    if( SDL_Init(SDL_INIT_EVERYTHING) )
    {
        SDL_Log("ERROR: could not initialise SDL2 '%s'", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    sdl_window = SDL_CreateWindow(
        __FILE__,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL );
    if( sdl_window == NULL )
    {
        SDL_Log("ERROR: creating window %s\n", SDL_GetError());
        return false;
    }

    sdl_gl_context = SDL_GL_CreateContext( sdl_window );
    if( sdl_gl_context == NULL )
    {
        SDL_Log("ERROR: creating opengl context %s\n", SDL_GetError());
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if( error != GLEW_OK )
    {  
        SDL_Log("ERROR: starting glew %d", error);
        return false;
    }

    glEnable( GL_MULTISAMPLE );     // Turn on multisampling
    SDL_GL_SetSwapInterval( 1 );    // Turn on vsync

    SDL_StartTextInput();           // Start captuing text input

    return true;
}

void shutdown()
{
    glDeleteBuffers( 1, &vbo );
    glDeleteVertexArrays( 1, &vao );
    glDeleteTextures( 1, &font_texture );
    glDeleteProgram( simple_texture_shader );

    SDL_GL_DeleteContext( sdl_gl_context );
    SDL_DestroyWindow( sdl_window );
    SDL_StopTextInput();            // Stop captuing text input
    SDL_Quit();                     // Shutdown SDL
}