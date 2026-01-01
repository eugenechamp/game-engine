#include "runtime/app.hpp"
#include "runtime/world.hpp"
#include "runtime/commands.hpp"
#include "runtime/events.hpp"
#include "runtime/subsystem.hpp"
#include "runtime/window.hpp"
#include "runtime/batcher.hpp"

#include <SDL3/SDL.h>

void App::init_subsystem(Subsystem *subsystem) {
  // should I actually be doing this? maybe initialize it right away after the constructor call
  commands_->enqueue([this, subsystem] { subsystem->init(*this); });
}

App::App() {
  commands_ = Unique<Commands>(new Commands);
  events_   = Unique<Events>(new Events(*this));
  
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    printf("Error: SDL_Init(): %s\n", SDL_GetError());
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

  // Create window_.handle with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

  window_ = std::make_unique<Window>(Window{
    .width = 640,
    .height = 480,
    .handle = SDL_CreateWindow("App", 1280, 720, window_flags),
    .title = "MyGame"
  });

  SDL_GLContext gl_context = SDL_GL_CreateContext(window_->handle);

  if (gl_context == nullptr)
  {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
  }

  SDL_GL_MakeCurrent(window_->handle, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  SDL_SetWindowPosition(window_->handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window_->handle);

  events()
    .add_event<SDL_KeyboardEvent>()
    .add_event<SDL_KeyboardEvent>()
    .add_event<SDL_MouseMotionEvent>()
    .add_event<SDL_MouseButtonEvent>()
    .add_event<SDL_QuitEvent>()
    .add_event<SDL_TextInputEvent>();
  
  /*
  SDL_Init(SDL_INIT_VIDEO);
  
  window_ = Window {
    .width  = 640,
    .height = 480,
    .handle = SDL_CreateWindow("App", 640, 480, {}),
    .title  = "MyGame"
  };*/
}

App::~App() {

}

App::App(CmdlineArgs args)
 : App()
{ }

void App::run() {
  bool quit = false;
  u64 last_time = SDL_GetTicksNS();
  while(!quit)
  {
    commands_->execute();

    u64 now = SDL_GetTicksNS();
    u64 delta = now - last_time;
    last_time = now;
    float dt = delta / 1000.f * 1000.f ;
    
   /* SDL_Event events[255];
    SDL_PeepEvents(events, 255, SDL_EventAction::SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);*/
      
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT: {
        events().batch<SDL_QuitEvent>().push_back(event.quit);
      } break;
      case SDL_EVENT_MOUSE_MOTION: {
        events().batch<SDL_MouseMotionEvent>().push_back(event.motion);
      }break;
                                 //case SDL_EVENT_MOUSE_WHEEL:
                                 //{
                                 //  events().batch<SDL_MouseWheelEvent>().push_back(event.wheel);
                                 //}break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        events().batch<SDL_MouseButtonEvent>().push_back(event.button);
      }break;
      case SDL_EVENT_TEXT_INPUT: {
        events().batch<SDL_TextInputEvent>().push_back(event.text);
      } break;
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
        events().batch<SDL_KeyboardEvent>().push_back(event.key);
      } break;
              
                           //case SDL_EVENT_DISPLAY_ORIENTATION:
                           //case SDL_EVENT_DISPLAY_ADDED:
                           //case SDL_EVENT_DISPLAY_REMOVED:
                           //case SDL_EVENT_DISPLAY_MOVED:
                           //case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
                           //{
                           //  bd->WantUpdateMonitors = true;
                           //  return true;
                           //}
                           //case SDL_EVENT_WINDOW_MOUSE_ENTER:
                           //{
                           //  if (ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                           //    return false;
                           //  bd->MouseWindowID = event->window.windowID;
                           //  bd->MousePendingLeaveFrame = 0;
                           //  return true;
                           //}
                           //// - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
                           ////   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
                           ////   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
                           //// FIXME: Unconfirmed whether this is still needed with SDL3.
                           //case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                           //{
                           //  if (ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                           //    return false;
                           //  bd->MousePendingLeaveFrame = ImGui::GetFrameCount() + 1;
                           //  return true;
                           //}
                           //case SDL_EVENT_WINDOW_FOCUS_GAINED:
                           //case SDL_EVENT_WINDOW_FOCUS_LOST:
                           //{
                           //  ImGuiViewport* viewport = ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID);
                           //  if (viewport == nullptr)
                           //    return false;
                           //  //IMGUI_DEBUG_LOG("%s: windowId %d, viewport: %08X\n", (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED) ? "SDL_EVENT_WINDOW_FOCUS_GAINED" : "SDL_WINDOWEVENT_FOCUS_LOST", event->window.windowID, viewport ? viewport->ID : 0);
                           //  io.AddFocusEvent(event->type == SDL_EVENT_WINDOW_FOCUS_GAINED);
                           //  return true;
                           //}
                           //case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                           //case SDL_EVENT_WINDOW_MOVED:
                           //case SDL_EVENT_WINDOW_RESIZED:
                           //{
                           //  ImGuiViewport* viewport = ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID);
                           //  if (viewport == NULL)
                           //    return false;
                           //  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                           //    viewport->PlatformRequestClose = true;
                           //  if (event->type == SDL_EVENT_WINDOW_MOVED)
                           //    viewport->PlatformRequestMove = true;
                           //  if (event->type == SDL_EVENT_WINDOW_RESIZED)
                           //    viewport->PlatformRequestResize = true;
                           //  return true;
                           //}
                           //case SDL_EVENT_GAMEPAD_ADDED:
                           //case SDL_EVENT_GAMEPAD_REMOVED:
                           //{
                           //  bd->WantUpdateGamepadsList = true;
                           //  return true;
                           //}
                           //default:
                           //  break;
      };
    }

    for (auto& [key, subsystem] : subsystems_)
    {
      if (subsystem->run_condition(*this))
      {
        subsystem->update(dt);
      }
    }

    events_->run();
  }
}

void App::SubsystemDeleter::operator()(Subsystem *subsystem) const
{
  delete subsystem;
}