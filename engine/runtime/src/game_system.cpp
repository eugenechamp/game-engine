#include "runtime/game_system.hpp"
#include "runtime/app.hpp"

GameSystem::GameSystem(App &app)
  : world_{ app.world() }
{ }
