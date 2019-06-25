#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "vec2.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

SDL_Window* sdl_window = nullptr;
SDL_GLContext sdl_gl_context;

const float PI = 3.14159f;
const float TAU = PI * 2.0f;

//
// types
//

struct Platform
{
    struct
    {
        float w = 1280;
        float h = 720;
    } window;
    
    struct
    {
        float lr_axis = 0; // strafe left and right
        float fb_axis = 0; // walk forward and back

        float yaw_axis = 0;     // look turn
        float pitch_axis = 0;   //
    } input;

    struct
    {
    	float x = 0.0f;
        float y = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;
    	bool l_pressed = false;
        bool l_down = false;
        bool l_released = false;
    } mouse;

    struct
    {
        Uint64 prev_time = 0;
        Uint64 time = 0;
        Uint64 frequency = 0;
        float dt = 0;
        const float fixed_dt = 1.0f / 60.0f;
        float accumulator = 0;
    } time;

    void clear_frame_state()
    {
        input.lr_axis = 0;
        input.fb_axis = 0;
        input.yaw_axis = 0;
        input.pitch_axis = 0;
    }

    void clear_update_state()
    {
        // clear state that should only take place for 1 update e.g. button presses
        mouse.l_pressed = false;
        mouse.l_released = false;
        mouse.dx = 0.0f;
        mouse.dy = 0.0f;
    }
};

//
// function forwards
//

bool init_sdl();
void init_imgui();
void init_world();

void update(Platform& p);
void render(Platform& p);

vec2 pointOnCurve(const std::vector<vec2>& points, float t);

//
// globals
//

std::vector<vec2> points;
float tStart = 0.0f;
vec2 pStart;

int main(int argc, char* argv[])
{
    if( !init_sdl() )
    {
        SDL_Log("ERROR: failed to init SDL2");
        return -1;
    }

    if (glewInit() != GLEW_OK)
    {
        SDL_Log("ERROR: failed to init glew");
        return -1;
    }

    init_imgui();
    ImGuiIO& io = ImGui::GetIO();

    init_world();

    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

    Platform p;

    p.time.time = SDL_GetPerformanceCounter();
    p.time.frequency = SDL_GetPerformanceFrequency();

    bool done = false;
    while (!done)
    {
        Uint32 ticks_start = SDL_GetTicks();

        p.time.prev_time = p.time.time;
        p.time.time = SDL_GetPerformanceCounter();
        p.time.dt = (p.time.time - p.time.prev_time) / (float)p.time.frequency;
        p.time.accumulator += p.time.dt;

        p.clear_frame_state();
        p.clear_update_state();

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                done = true;
            }
            else if (event.type == SDL_WINDOWEVENT && event.window.windowID == SDL_GetWindowID(sdl_window))
            { 
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    done = true;
                }
                else if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    p.window.w = (float)event.window.data1;
                    p.window.h = (float)event.window.data2;
                }
            }
            
            if( !io.WantCaptureKeyboard )
            {
                /*
                if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && event.key.repeat == 0)
                    {
                        //p.input.fire_pressed = true;
                    }
                }
                //*/
            }

            if( !io.WantCaptureMouse )
            {
                if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        p.mouse.l_pressed = true;
                        p.mouse.l_down = true;
                    }
                }
                else if(event.type == SDL_MOUSEMOTION)
                {
                    p.mouse.x = (float)event.motion.x;
                    p.mouse.y = (float)event.motion.y;
                    p.mouse.dx = (float)event.motion.xrel;
                    p.mouse.dy = (float)event.motion.yrel;
                }
                else if(event.type == SDL_MOUSEBUTTONUP)
                {
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        p.mouse.l_released = true;
                        p.mouse.l_down = false;
                    }
                }
            }
        }

        if( !io.WantCaptureKeyboard )
        {
            const Uint8* keys = SDL_GetKeyboardState(nullptr);

            if(keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D])
            {
                p.input.lr_axis = -1;
            }
            else if(!keys[SDL_SCANCODE_A] && keys[SDL_SCANCODE_D])
            {
                p.input.lr_axis = 1;
            }

            if(keys[SDL_SCANCODE_S] && !keys[SDL_SCANCODE_W])
            {
                p.input.fb_axis = -1;
            }
            else if(!keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_W])
            {
                p.input.fb_axis = 1;
            }

            if( p.input.fb_axis != 0.0f || p.input.lr_axis != 0.0f )
            {
                //glm::vec2 normed = glm::normalize(glm::vec2(p.input.lr_axis, p.input.fb_axis));
                //p.input.lr_axis = normed.x;
                //p.input.fb_axis = normed.y;
            }

            if( keys[SDL_SCANCODE_LEFT] && !keys[SDL_SCANCODE_RIGHT] )
            {
                p.input.yaw_axis = -1;
            }
            else if( !keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT] )
            {
                p.input.yaw_axis = 1;
            }
        }

        if( !io.WantCaptureMouse )
        {

        }

        const int max_updates = 5;
        int num_updates = 0;
        while( p.time.accumulator >= p.time.fixed_dt && num_updates < max_updates )
        {
            update(p);
            p.time.accumulator -= p.time.fixed_dt;
            num_updates++;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(sdl_window);
        ImGui::NewFrame();

        if (show_demo_window)
        {
            p.clear_update_state();
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        {
            ImGui::Begin("Debug");

            ImGui::Separator();
            
            ImGui::Value("Num Points", (int)points.size());
            ImGui::SameLine();
            if( ImGui::Button("Clear") )
            {
                points.clear();
                points.shrink_to_fit();
            }

            ImGui::SliderFloat("start t", &tStart, 0.0f, (float)points.size() - 1 );
            ImGui::SliderFloat("mouse dx", &p.mouse.dx, -10, 10);
            ImGui::SliderFloat("mouse dy", &p.mouse.dy, -10, 10);
            ImGui::Value("l pressed", p.mouse.l_pressed);
            ImGui::Value("l released", p.mouse.l_released);            

            ImGui::Separator();

            if( ImGui::TreeNode("imgui info") )
            {
                ImGui::Checkbox("Show demo Window", &show_demo_window);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                ImGui::Text("Frame rate %.1f FPS, %.3f ms/frame (timer frequency %llu Hz) ",  1.0f / p.time.dt, p.time.dt * 1000.0f, p.time.frequency );

                ImGui::TreePop();
            }

            ImGui::End();
        }

        render(p);

        ImGui::Render();

        // Cap the framerate so we don't eat all the CPU
        Uint32 ticks_end = SDL_GetTicks();
        Uint32 ticks_taken = ticks_end - ticks_start;
        Uint32 ticks_cap = 10;
        if( ticks_taken < ticks_cap )
        {
            SDL_Delay(ticks_cap - ticks_taken);
        }

        SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(sdl_window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(sdl_gl_context);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}

bool init_sdl()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("ERROR: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    sdl_window = SDL_CreateWindow("Splines", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    return true;
}

void init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;

    ImGui_ImplSDL2_InitForOpenGL(sdl_window, sdl_gl_context);
    const char* glsl_version = "#version 150";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
}

void init_world()
{
}

vec2 pointOnCurve(const std::vector<vec2>& points, float t)
{
    t = clamp(t, 0.0f, ((float)points.size()) - 1.0f);

    if(points.empty())
        return vec2();

    if(points.size() == 1)
        return points[0];

    if(points.size() == 2)
    {
        return lerp(points[0], points[1], t);
    }

    int i = std::floor(t);
    i = clamp(i, 0, (int)points.size() - 1);

    if(i + 1 == (int)points.size())
        return points.back();

    return lerp(points[i], points[i+1], t - (float)i);
}

vec2 moveAlongCurve(const std::vector<vec2>& points, float tStart, float pixelsToMove)
{
    vec2 start = pointOnCurve(points, tStart);

    return start;
}

static int hovered_point = -1;
static const float hover_distance = 10.0f;

void update(Platform& p)
{
    if(!p.mouse.l_down)
    {
        hovered_point = -1;
    }

    for(size_t i = 0; i < points.size() && hovered_point == -1; ++i)
    {
        vec2 m(p.mouse.x, p.mouse.y);

        if(distance(m, points[i]) < hover_distance)
        {
            hovered_point = i;
            break;
        }
    }

	if(p.mouse.l_pressed)
    {
        if(hovered_point >= 0)
        {
            points[hovered_point] += vec2(p.mouse.dx, p.mouse.dy);
        }
        else
    	{
    		points.push_back({p.mouse.x, p.mouse.y});
    	}
    }

    pStart = pointOnCurve(points, tStart);
}

void render(Platform& p)
{
    ImDrawList* g = ImGui::GetBackgroundDrawList();

    for( size_t i = 0; i < points.size(); i++)
    {
        g->AddCircleFilled(points[i], 5, 0xffaaaaaa);

        if(hovered_point == (int)i)
        {
            g->AddCircle(points[i], hover_distance, 0xffffffff);
        }
    }

    char buf[32];

    for( size_t i = 0; i + 1 < points.size(); i++)
    {
        const vec2& a = points[i];
        const vec2& b = points[i+1];

    	g->AddLine(a, b, 0xff2222aa);

        float dist = distance(a, b);
        vec2 mid = lerp(a, b, 0.5f);

        snprintf(buf, ARRAY_SIZE(buf), "%.2f", dist);

        g->AddText(mid, 0xffaaaaaa, buf);
    }

    g->AddCircleFilled(pStart, 5, 0xffffffff);

	/*
    glm::vec2 pos = { player.pos.x, player.pos.z };
    glm::vec2 target = pos + player.getDir() * 10.0f;

    glm::mat4 view = glm::lookAt({player.pos.x, player.pos.y, player.pos.z}, glm::vec3(target.x, player.pos.y, target.y), {0, 1, 0});
    glm::mat4 proj = glm::perspective(PI * 0.5f, p.window.w / p.window.h, 10.0f, 1000.0f);
    glm::mat4 camera = proj * view;

    float size = 8;
    g->AddCircleFilled(pos, size, 0xffaaaaaa);
    g->AddLine(pos, pos + player.getDir() * size * 2.0f, 0xff0000ff);

    for( size_t i = 0; i < walls.size(); ++i )
    {
        g->AddLine(walls[i].a, walls[i].b, 0xffdddddd);    

        glm::vec4 v4[4];

        v4[0] = camera * glm::vec4( walls[i].a.x,  0, walls[i].a.y, 1);
        v4[1] = camera * glm::vec4( walls[i].a.x, 50, walls[i].a.y, 1);
        v4[3] = camera * glm::vec4( walls[i].b.x,  0, walls[i].b.y, 1);
        v4[2] = camera * glm::vec4( walls[i].b.x, 50, walls[i].b.y, 1);

        for( int i = 0; i < 4; ++i )
        {
            v4[i] /= v4[i].w;
        }

        glm::vec2 v2[4];

        for( int i = 0; i < 4; ++i )
        {
            v2[i] = translate_to_viewport(v4[i], p.window.w, p.window.h);
        }

        //g->AddLine({ a.x, p.window.h * 0.5f }, { b.x, 0 }, 0xffffffff);

        g->AddQuadFilled(v2[0], v2[1], v2[2], v2[3], 0x88ffffff);
    }

    {
        glm::mat4 inv = glm::inverse( camera );

        glm::vec4 v[4];

        v[0] = inv * glm::vec4( 1, 0,  1, 1);
        v[1] = inv * glm::vec4( 1, 0, -1, 1);
        v[2] = inv * glm::vec4(-1, 0, -1, 1);
        v[3] = inv * glm::vec4(-1, 0,  1, 1);

        for( int i = 0; i < 4; ++i )
        {
            v[i] /= v[i].w;
        }

        g->AddQuad(
            glm::vec2( v[0].x, v[0].z ),
            glm::vec2( v[1].x, v[1].z ),
            glm::vec2( v[2].x, v[2].z ),
            glm::vec2( v[3].x, v[3].z ), 0xffffffff);

        g->AddLine( { v[1].x, v[1].z }, { v[2].x, v[2].z }, 0xff0000ff );
        g->AddLine( { v[0].x, v[0].z }, { v[3].x, v[3].z }, 0xff0000ff );
    }
    //*/
}