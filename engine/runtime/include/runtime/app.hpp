#pragma once
#include "def.hpp"
#include "fwd.hpp"
#include "meta.hpp"

#include "resource.hpp"
// refactor it out

/// NOTE: you could enclose it in the App class, think about that
struct CmdlineArgs {
  int    argc{};
  char** argv{};
};

class App {
  /// TODO: make this a singleton
public:
  App();
  ~App();
  App(CmdlineArgs);
  App(const App&)            = delete;
  App(App&&)                 = delete;
  auto operator=(const App&) = delete;
  auto operator=(App&&)      = delete;
  
  template<typename Res>
  auto add_resource(Res&& resource) -> App& {
    constexpr auto sid = core::type_sid<Res>();
    res_.emplace(std::pair { sid, std::make_unique<ResourceStorage<Res>>(std::move(resource)) });
    
    return *this;
  }

  template<typename Res>
  auto add_resource() -> App& {
    constexpr auto sid = core::type_sid<Res>();
    res_.emplace(std::pair{ sid, std::make_unique<ResourceStorage<Res>>() });

    return *this;
  }
  
  template<typename /*std::derived_from<Subsystem>*/ SubsystemType>
  auto add_subsystem() -> App& {
    Subsystem* subsystem = new SubsystemType(*this);
    constexpr auto sid = core::type_sid<SubsystemType>();
    subsystems_.emplace(std::pair{sid, subsystem });
    
    //auto devirtualized_update = [this](Subsystem* base, float dt) {
    // //make the interface public and forget 
    // SubsystemType* subsystem = static_cast<SubsystemType*>(base);

    //  if (!subsystem->SubsystemType::run_condition(*this))
    //    return;

    //  subsystem->SubsystemType::update(dt);
    //};

    init_subsystem(subsystem);

    return *this;
  }
  
  template<typename Res>
  auto get() -> Res& {
    constexpr auto sid = core::type_sid<Res>();
    Res* res = &static_cast<ResourceStorage<Res>*>(res_.at(sid).get())->resource;
    return *res;
  }

  auto events() -> Events& {
    return *events_;
  }

  auto world() -> World& {
    return *world_;
  }

  template<typename AssetType>
  auto assets() -> AssetCache<AssetType>& {
    return this->get<AssetCache<AssetType>>();
  }
  
  template<typename State> requires(std::is_enum_v<State>)
  auto set_state(State state) -> App& {
    // states_.get<State>().load(state);
    return *this;
  }
  
  template<typename State> requires(std::is_enum_v<State>)
  bool in_state() {

  }

  template<typename State> requires(std::is_enum_v<State>)
  auto state() -> State {
    //return states_.at(type_name<State>()).load();
  }
  
  auto main_window() -> Window& {
    return *window_;
  }

  void run();
private:
  struct SubsystemDeleter {
    void operator()(Subsystem* subsystem) const;
  };
  void init_subsystem(Subsystem*);

private:
  Unique<World>                          world_;
  Unique<Events>                        events_;
  Unique<Commands>                    commands_;
  Unique<Window>                        window_;
  Resources res_;
  
  // hash map not good
  HashMap<StringID, Unique<Subsystem, SubsystemDeleter>> subsystems_;
};