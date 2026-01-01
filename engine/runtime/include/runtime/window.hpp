#pragma once
#include <SDL3/SDL.h>
#include <runtime/def.hpp>
#include <string>

struct Window {
  u32 width;
  u32 height;
  SDL_Window* handle;
  std::string title;
};