#include <runtime/input.hpp>
#include <runtime/app.hpp>
#include <runtime/events.hpp>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

namespace {
  bool key_just_pressed_state[512] = {};
  u8* key_state = nullptr;

  void on_key_event(Viewer<SDL_KeyboardEvent> events) {
    for(auto& event : events)
    {
      switch (event.type) {
        case SDL_EVENT_KEY_DOWN: {
          key_just_pressed_state[event.scancode] = true;
        } break;
        case SDL_EVENT_KEY_UP: {
          key_just_pressed_state[event.scancode] = false;
        } break;
      }
    }
  }
}

bool Input::key_pressed(KeyCode keycode) {
  return key_state[(SDL_Scancode)keycode];
}

bool Input::key_just_pressed(KeyCode keycode) {
  if (key_just_pressed_state[i32(keycode)])
  {
    key_just_pressed_state[i32(keycode)] = false;
    return true;
  }

  return false;
}

bool Input::key_just_released(KeyCode keycode) {
  return !key_just_pressed_state[i32(keycode)];
}

bool Input::key_released(KeyCode keycode) {
  u8* key_state = (u8*)SDL_GetKeyboardState(nullptr);
  return !key_state[(SDL_Scancode)keycode];
}

bool Input::any_key_pressed(Span<KeyCode> keycodes) {
  return false;
}

bool Input::mouse_moved() {
  return false;
}

auto Input::mouse_position() -> vec2 {
  return vec2();
}

void Input::mouse_position(vec2) {
}

auto Input::pressed_keycodes() -> Vec<KeyCode> {
  return Vec<KeyCode>();
}

auto Input::pressed_keys() -> Vec<Key> {
  return Vec<Key>();
}

auto Input::just_pressed_keys() -> Vec<Key> {
  return Vec<Key>();
}

auto Input::just_pressed_keycodes() -> Vec<KeyCode> {
  return Vec<KeyCode>();
}

auto Input::released_keys() -> Vec<Key> {
  return Vec<Key>();
}

auto Input::released_keycodes() -> Vec<KeyCode> {
  return Vec<KeyCode>();
}

auto Input::just_released_keys() -> Vec<Key> {
  return Vec<Key>();
}

auto Input::just_released_keycodes() -> Vec<KeyCode> {
  return Vec<KeyCode>();
}

void Input::init(App& app) {
  app.events()
    .listen(on_key_event);
  
  key_state = (u8*)SDL_GetKeyboardState(nullptr);
}
