#include "runtime/app.hpp"
#include "runtime/world.hpp"
#include "runtime/commands.hpp"
#include "runtime/events.hpp"
#include "runtime/subsystem.hpp"
#include "runtime/window.hpp"
#include "runtime/emitter.hpp"
#include "runtime/file_watcher.hpp"
#include "runtime/input.hpp"
#include "runtime/time.hpp"

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <algorithm>

void App::init_subsystem(Subsystem *subsystem, StringID sid) {
  // should I actually be doing this? maybe initialize it right away after the constructor call
  //commands_->enqueue([this, subsystem, sid] { subsystem->init(*this); subsystems_.emplace(std::pair{ sid, subsystem }); });
  commands_->enqueue([this, subsystem, sid] { subsystem->init(); subsystems_.emplace_back(subsystem); });
}

App::App() {
  commands_ = Unique<Commands>(new Commands);
  events_   = Unique<Events>(new Events(res_, *world_));
  world_    = Unique<World>(new World(*this));
  states_   = Unique<States>(new States(events()));
  subsystem_manager_ = Unique<SubsystemManager>(new SubsystemManager(*this, *commands_));

  fixed_framerate_ = 60;
  frame_rate_ = 30;

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    log_fatal("Error: SDL_Init(): {}", SDL_GetError());
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
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
#endif

  // Create window_.handle with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  [[maybe_unused]] float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  
  window_ = std::make_unique<Window>(Window {
    .width = 1280,
    .height = 720,
    .handle = SDL_CreateWindow("App", 1280, 720, window_flags),
    .title = "MyGame"
  });
  SDL_GLContext gl_context = SDL_GL_CreateContext(window_->handle);

  if (gl_context == nullptr)
  {
    log_fatal("SDL_GL_CreateContext(): {}", SDL_GetError());
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
  {
    log_fatal("Couldn't load OpenGL");
  }

  SDL_GL_MakeCurrent(window_->handle, gl_context);
  SDL_SetWindowPosition(window_->handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window_->handle);

  events()
    .add_event<SDL_KeyboardEvent>()
    .add_event<SDL_MouseMotionEvent>()
    .add_event<SDL_MouseButtonEvent>()
    .add_event<SDL_QuitEvent>()
    .add_event<SDL_TextInputEvent>();
  
  FileWatcher::run(*this);
  Input::init(*this);
}

App::~App() {

}

App::App([[maybe_unused]] CmdlineArgs args)
 : App()
{ }

void App::run() {
  bool quit = false;
  auto last_time = Time::now();
  float time_elapsed = 0.f;

  while(!quit)
  { 
    commands_->execute();
    
    auto now = Time::now();
    Duration delta = now - last_time;
    last_time = now;
    
    float dt = float(delta.sec());
    float target_frametime = (1.f / frame_rate_);
    float frametime = (frame_rate_ == 0) ? dt : target_frametime;

    float fixed_timestep = 1.f / fixed_framerate_;

    for (auto iter = schedule_.begin(); iter != schedule_.end();)
    {
      if (iter->first <= now)
      {
        iter->second();
        auto next = ++iter;
        schedule_.erase(iter);
        iter = next;
      }
      else
      {
        break;
      }
    }

    /*for (auto& [timepoint, fn] : schedule_)
    {
      if (now >= timepoint)
      {
        break;
      }
      fn();
    }

    std::erase_if(schedule_, [now](const auto& node) {
      return now >= node.first;
    });*/

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT: {
        events().trigger(event.quit);
        quit = true;
      } break;
      case SDL_EVENT_MOUSE_MOTION: {
        events().trigger<SDL_MouseMotionEvent>(event.motion);
      }break;
      case SDL_EVENT_MOUSE_WHEEL: {
        //events().batch<SDL_MouseWheelEvent>().push_back(event.wheel);
      }break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        events().trigger<SDL_MouseButtonEvent>(event.button);
      }break;
      case SDL_EVENT_TEXT_INPUT: {
        events().trigger<SDL_TextInputEvent>(event.text);
      } break;
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
        events().trigger<SDL_KeyboardEvent>(event.key);
      } break;
      case SDL_EVENT_WINDOW_RESIZED: {
        window_->width = event.window.data1;
        window_->height = event.window.data2;
        
        // remove
        glViewport(0, 0, window_->width, window_->height);
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
     
    //for (auto& [key, subsystem] : subsystems_)
    
    
    /*i32 fixed_step_updates = 0;
    time_elapsed += dt;
    while ((time_elapsed - fixed_timestep) >= fixed_timestep)
    {
      fixed_step_updates++;
      time_elapsed -= fixed_timestep;
    }

    for (auto& subsystem : subsystems_)
    {
      if (subsystem->run_condition())
      {
        for (i32 i = 0; i < fixed_step_updates; i++)
        {
          subsystem->fixed_update(fixed_timestep);
        }
        subsystem->update(dt);
      }
    }*/
    
    subsystem_manager_->update(dt);

    world_->get().progress(dt);

    events_->run();
    
    SDL_GL_SwapWindow(window_->handle);
    
    // doesn't work btw
    if (frame_rate_ != 0) // fps is limited
    {
      if (dt < target_frametime)
      {
        auto sleep_time = std::chrono::duration<float>(target_frametime - dt);
        //std::this_thread::sleep_for(sleep_time);
      }
    }
  }
}

void App::SubsystemDeleter::operator()(Subsystem *subsystem) const {
  delete subsystem;
}