#pragma once

#ifndef FLECS_CPP
  #define FLECS_CPP
#endif

#include <flecs.h>

class World {
public:
  auto query() {

  }
  
  auto local_player() {
    return local_player_;
  }

private:
  flecs::world ecs_;
  flecs::entity local_player_;
};