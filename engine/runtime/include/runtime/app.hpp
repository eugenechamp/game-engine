#pragma once
#include "def.hpp"
#include "fwd.hpp"

class App {
  // TODO: make this a singleton
public:
  App()                      = default;
  App(const App&)            = delete;
  App(App&&)                 = delete;
  auto operator=(const App&) = delete;
  auto operator=(App&&)      = delete;
  
  auto add_resource(auto&& resource) -> App& {
    return *this;
  }
  
  auto register_resource(auto& resource) -> App& {
    return *this;
  }

  auto add_system(auto&& system) -> App& {
    return *this;
  }
  
  template<typename Resource>
  auto get() {

  }

  auto events() -> Events& {
    return *events_;
  }

  auto world() -> World& {
    return *world_;
  }

  void run();

private:
  Unique<World>                         world_;
  Unique<Events>                        events_;
  Unique<Commands>                      commands_;
  Unique<Window>                        main_window_;
  HashMap<u32, Unique<ResourceStorage>> res_;
};