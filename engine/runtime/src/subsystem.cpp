#include "runtime/subsystem.hpp"
#include "runtime/app.hpp"

Subsystem::Subsystem(App &app)
  : world_{ app.world() }
{ }