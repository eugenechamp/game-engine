#include <runtime/app.hpp>
#include <runtime/subsystem.hpp>
#include <runtime/events.hpp>
#include <runtime/batcher.hpp>
#include <runtime/window.hpp>
#include <backends/imgui_impl_sdl3.h>
#include <glad/glad.h>
#include <runtime/assets.hpp>

#include <rendering/shader.hpp>
#include <rendering/model.hpp>
#include <runtime/camera.hpp>
#include <runtime/input.hpp>

template<>
struct Loader<Shader> {
  Shader load(const LoadContext& ctx) {
    Shader shader;
    shader.load(ctx.paths[0], ctx.paths[1]);
    return shader;
  }
};

enum class AppState {
  Startup,
  Running,
  Paused,
  LoadingIn,
  Gameplay,
  LoadingOut,
  Shutdown,
};

class StartupSubsystem : public Subsystem {
  using Subsystem::Subsystem;
  
  bool run_condition(App& app) final {
    return true; //!app.in_state(AppState::Paused);
  }
  bool shutdown_condition(App&) final {
    return true;
  }
};

class ShowSplash : public StartupSubsystem {
  using StartupSubsystem::StartupSubsystem;
  
  void init(App& app) override {
    log_info("{} subsystem initialilzation", "ShowSplash");
  }

  void fixed_update(float dt) override {
  
  }
  
  void update(float dt) override {
    log_info("ShowSplash is running");

    // static float time = 0.f;
    // time += dt;
    // std::cout << "time elapsed: " << time << "\n";
    //
    //if (time >= 5.f) {
    //  should_run_ = false;
    //  std::cout << "SplashScreen Closed\n";
    //  return;
    //}

    //std::cout << "SplashScreen\n";
  }

  void shutdown(App& app) override { }

  bool should_run_ = true;
};

class LoadingSystem : public StartupSubsystem {
  using StartupSubsystem::StartupSubsystem;
  
  void init(App& app) override {
    std::cout << "LoadingSystem\n";
    // app.set_state(AppState::LoadingIn);
    app.add_subsystem<ShowSplash>();
  }

  // Inherited via StartupSubsystem
  void update(float dt) override {
    // update and render loading screen
    std::cout << "Loading...\n";
  }
  void shutdown(App&) override { }
};

// Dear ImGui: standalone example application for SDL3 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

struct TestEvent {
  u32 num;
};

struct TestBatchEvent {
  u32 num;
};

void on_test_event(Span<const TestEvent> events, Batcher<TestBatchEvent> batch_events) {
  //log_info("TestEvent called size: {}", events.size());
  /*for(u32 i = 0; auto& event : events)
  {
    batch_events.push_back(TestBatchEvent{ .num = event.num });
  }*/
}

void on_test_batch_event(Span<const TestBatchEvent> events) {
  //log_info("TestBatchEvent called size: {}", events.size());
  //for (u32 i = 0; auto& event : events)
  //{
  //  //log_info("TestBatchEvent N{}: value: {}", ++i, event.num);
  //}
}

class ImguiEditor : public Subsystem {
  using Subsystem::Subsystem;
  friend App;
  
  void init(App& app) override
  {
    app_ = &app;
    window = app.main_window().handle;
    
    app.events()
      .add_event<TestEvent>()
      .listen(on_test_event)
      .add_event<TestBatchEvent>()
      .listen(on_test_batch_event);
    
    app.events()
      .listen(&ImguiEditor::on_keyboard_event, this)
      .listen(&ImguiEditor::on_mouse_button,   this)
      .listen(&ImguiEditor::on_mouse_motion,   this);
    
    const char* glsl_version = "#version 130";
    SDL_GLContext gl_context = SDL_GL_GetCurrentContext();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will call AddFontDefault() to select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
  }

  void update(float dt) override {
    auto batcher = app_->events().batch<TestEvent>();
   /* for (size_t i{}; i < 100'000; i++) {
      batcher.push_back(TestEvent{ .num = u32(i + i) });
    }*/
    
    ImGuiIO& io = ImGui::GetIO();

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
    {
      SDL_Delay(10);
    }

    //SDL_Event event;
    //while (SDL_PollEvent(&event))
    //{
    //  ImGui_ImplSDL3_ProcessEvent(&event);
    //  
    //  //if (event.type == SDL_EVENT_QUIT)
    //  //  done = true;
    //  //if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
    //  //  done = true;
    //}
    // Start the Dear ImGui frame
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);
      
      if (ImGui::Checkbox("VSync", &vsynced))
      {
        SDL_GL_SetSwapInterval(vsynced);
      }

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
      ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  #ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
  #endif
  }
  
  void on_mouse_motion(Viewer<SDL_MouseMotionEvent> events) {
    auto& event = events.front();
    auto& io = ImGui::GetIO();
    if (ImGui::FindViewportByPlatformHandle((void*)(intptr_t)event.windowID) == nullptr)
      return;
    ImVec2 mouse_pos((float)event.x, (float)event.y);
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      int window_x, window_y;
      SDL_GetWindowPosition(SDL_GetWindowFromID(event.windowID), &window_x, &window_y);
      mouse_pos.x += window_x;
      mouse_pos.y += window_y;
    }
    io.AddMouseSourceEvent(event.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
    io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
  }
  
  void on_mouse_button(Viewer<SDL_MouseButtonEvent> events) {
    auto& event = events.front();
    auto& io = ImGui::GetIO();
    auto* bd = ImGui::GetCurrentContext() ? (struct ImGui_ImplSDL3_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;;

    if (ImGui::FindViewportByPlatformHandle((void*)(intptr_t)event.windowID) == nullptr)
      return;

    int mouse_button = -1;
    if (event.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
    if (event.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
    if (event.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
    if (event.button == SDL_BUTTON_X1) { mouse_button = 3; }
    if (event.button == SDL_BUTTON_X2) { mouse_button = 4; }
    if (mouse_button == -1)
      return;
    io.AddMouseSourceEvent(event.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
    io.AddMouseButtonEvent(mouse_button, (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN));
    bd->MouseButtonsDown = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
  }

  void on_keyboard_event(Span<const SDL_KeyboardEvent> events) {
    
    auto& event = events.front();
    auto& io = ImGui::GetIO();
    
    ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)(intptr_t)event.windowID);
    if (viewport == nullptr)
      return;
    //IMGUI_DEBUG_LOG("SDL_EVENT_KEY_%s : key=0x%08X ('%s'), scancode=%d ('%s'), mod=%X, windowID=%d, viewport=%08X\n",
    //    (event->type == SDL_EVENT_KEY_DOWN) ? "DOWN" : "UP  ", event->key.key, SDL_GetKeyName(event->key.key), event->key.scancode, SDL_GetScancodeName(event->key.scancode), event->key.mod, event->key.windowID, viewport ? viewport->ID : 0);
    io.AddKeyEvent(ImGuiMod_Ctrl, ((SDL_Keymod)event.mod & SDL_KMOD_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, ((SDL_Keymod)event.mod & SDL_KMOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, ((SDL_Keymod)event.mod & SDL_KMOD_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, ((SDL_Keymod)event.mod & SDL_KMOD_GUI) != 0);
    
    ImGuiKey key = ImGui_ImplSDL3_KeyEventToImGuiKey(event.key, event.scancode);
    io.AddKeyEvent(key, (event.type == SDL_EVENT_KEY_DOWN));
    io.SetKeyEventNativeData(key, (int)event.key, (int)event.scancode, (int)event.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
  }

  void shutdown(App&) override
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
  


  bool run_condition(App&) override
  {
    return true;
  }
  
  bool shutdown_condition(App&) override
  {
    return false;
  }
private:
  SDL_Window* window{};
  bool show_demo_window = true;
  bool show_another_window = false;
  bool vsynced = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  App* app_;
};

class HelloTriangle : public Subsystem {
  using Subsystem::Subsystem;
  friend App;
public:
  // Inherited via Subsystem
  void init(App& app) override
  {

  const char* vert_src = "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";
  const char* frag_src = "#version 330 core\n"
      "out vec4 FragColor;\n"
      "void main()\n"
      "{\n"
      "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
      "}\n\0";

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vert_src, NULL);
    glCompileShader(vert);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vert, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &frag_src, NULL);
    glCompileShader(frag);
    // check for shader compile errors
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(frag, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    shader = glCreateProgram();

    glAttachShader(shader, vert);
    glAttachShader(shader, frag);
    glLinkProgram(shader);
  }

  void update(float dt) override
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    glUseProgram(shader);
    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // glBindVertexArray(0); // no need to unbind it every time 

    //// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    //// -------------------------------------------------------------------------------
    //glfwSwapBuffers(window);
  }

  void shutdown(App&) override
  {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader);
  }
private:
  static inline float vertices[] = {
    // Positions
     0.0f,  0.5f, 0.0f, // Top
     0.5f, -0.5f, 0.0f, // Bottom Right
    -0.5f, -0.5f, 0.0f  // Bottom Left
  };
  u32 vert, frag{};
  u32 VBO, VAO{};
  u32 shader{};
};



class ModelPreview : public Subsystem {
  Subsystem::Subsystem;
  void init(App& app) override {
    shaders = &app.assets<Shader>();
    
    app.events()
      .listen(&ModelPreview::on_camera_mouse_move, this);
    
    glCreateBuffers(1, &uniform_buffer);
    glNamedBufferData(uniform_buffer, static_cast<uint32_t>(sizeof(view_info)), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0u, uniform_buffer);
    
    glEnable(GL_DEPTH_TEST);
    
    log_info("current working directory: {}", std::filesystem::current_path().string());
    
    //shader.load("../../sandbox/assets/shaders/surface.vert", "../../sandbox/assets/shaders/surface.frag");
    shader_handle = shaders->load("Surface_Shader", "../../sandbox/assets/shaders/surface.vert", "../../sandbox/assets/shaders/surface.frag");

    model.load("../../sandbox/assets/models/ak-47/scene.gltf");
    camera.perspective(45.f, app.main_window().width / (float)app.main_window().height, 0.1f, 1000.f);
  }

  void on_camera_mouse_move(Viewer<SDL_MouseMotionEvent> events) {
    for (auto& event : events)
    {
      static float sensitivity = 0.2f;

      float x = event.x;
      float y = event.y;

      static float last_x = static_cast<float>(x);
      static float last_y = static_cast<float>(y);

      float xoffset = static_cast<float>(x - last_x);
      float yoffset = static_cast<float>(last_y - y);

      last_x = static_cast<float>(x);
      last_y = static_cast<float>(y);

      /*if (!camera_fly_)
        return;*/

      vec3 rotation = camera.rotation();

      rotation.x += xoffset * sensitivity;
      rotation.y += yoffset * sensitivity;

      if (rotation.y > 89.0f)
      {
        rotation.y = 89.0f;
      }
      if (rotation.y < -89.0f)
      {
        rotation.y = -89.0f;
      }

      camera.rotation(rotation);
    }
  }


  void update(float dt) override
  {
    process_input();
    
    static float angle_accum{ 0.f };
    
    camera.position(camera.position() + (camera_velocity * dt));

    view_info vi{ camera.projection(), camera.view() };
    glNamedBufferSubData(uniform_buffer, 0, sizeof(view_info), &vi);
    
    Shader& shader = shaders->get(shader_handle);

    shader.bind();
    mat4 model_mat(1.f);
    
    angle_accum += 1.f * dt;

    model_mat = glm::translate(model_mat, { 0.f, 0.f, -10.f });
    model_mat = glm::rotate(model_mat, angle_accum, { 0.f, 1.f, 0.f });

    shader.mat4("model", model_mat);
    shader.mat3("normal_mat", glm::transpose(glm::inverse(model_mat)));
    shader.vec3("light_dir", vec3(0.f, 5.f, -7.f));
    model.draw(shader);
  }
  
  void process_input() {
    float sprint_speed_multiplier = 1.4f;
    
    if (Input::key_just_pressed(KeyCode::F))
    {
      log_info("F is just pressed...");
    }

    camera_velocity = vec3();
    if (Input::key_pressed(KeyCode::W))
    {
      camera_velocity = camera.front() * camera_speed;
    }
    else if (Input::key_pressed(KeyCode::S))
    {
      camera_velocity = camera.front() * -camera_speed;
    }
    
    if (Input::key_pressed(KeyCode::D))
    {
      camera_velocity = camera.right() * camera_speed;
    }
    else if (Input::key_pressed(KeyCode::A))
    {
      camera_velocity = camera.right() * -camera_speed;
    }
  }

  void shutdown(App&) override {

  }

private:
  Handle<Shader> shader_handle;
  Model model;
  FreeCamera camera;
  vec3 camera_velocity = vec3();
  float camera_speed = 10.f;
  u32 uniform_buffer{};
  AssetCache<Shader>* shaders = nullptr;
};

enum class FramePhase {
  Startup,
  PreUpdate,
  Update,
  PostUpdate
};

auto main() -> int {
  auto app = App();

  app.add_resource<AssetCache<Model>>();
  app.add_resource<AssetCache<Shader>>();
  
  // auto a = new AssetCache<Model>();

  app.add_subsystem<ModelPreview>();
  app.add_subsystem<ImguiEditor>();
  app.run();
}

#if 0
// Main code
int main(int, char**)
{
  // Setup SDL
  // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    printf("Error: SDL_Init(): %s\n", SDL_GetError());
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char* glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char* glsl_version = "#version 300 es";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
  const char* glsl_version = "#version 150";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
  if (window == nullptr)
  {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return 1;
  }
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (gl_context == nullptr)
  {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup scaling
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
  style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If fonts are not explicitly loaded, Dear ImGui will call AddFontDefault() to select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
  //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
  // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
  //style.FontSizeBase = 20.0f;
  //io.Fonts->AddFontDefaultVector();
  //io.Fonts->AddFontDefaultBitmap();
  //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
  //IM_ASSERT(font != nullptr);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool done = false;
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
  // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
  io.IniFilename = nullptr;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!done)
#endif
  {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        done = true;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
    {
      SDL_Delay(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
      ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  // Cleanup
  // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DestroyContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
#endif // 0