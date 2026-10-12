#include "runtime/subsystem.hpp"
#include "runtime/app.hpp"

Subsystem::Subsystem(App &app)
  : app_{ app },
  world_{ app.world() }, 
  events_{ app.events() }
{
  //assert(SubsystemInstanceRegistry::contains(this) && "system wasn't properly created\n");

  // verify that Subsystem is created via App::add_subsystem() function
  // by registering an address Subsystem is supposed to live at and checking if this instance address
  // was added to a registry 
  // if not, it means that the instance was created the wrong way and that's absolutely not alright, 
  // log fatal error and terminate the program right away
  // allocate memory -> register the address -> check the address here by comparing to "this" pointer
  // if valid -> subsystem was registered via App::add_subsystem() 
  // else -> somewhere else (error)
}

auto Subsystem::fixed_timestep() -> Duration {
  return Duration::seconds(1.0 / 60.0);
}

void SubsystemSet::add_subsystem_impl(Fn<void()> fn) {
  app.commands().enqueue(std::move(fn));
}
