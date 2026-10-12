#pragma once
#include "def.hpp"
#include "fwd.hpp"
#include "meta.hpp"
#include "states.hpp"

#include "resource.hpp"
#include "subsystem_manager.hpp"
#include "time.hpp"
// refactor it out

#include "subsystem.hpp"

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
  
  template<typename T>
  auto add_resource(T&& resource) -> App&;

  template<typename T>
  auto add_resource() -> App&;
  
#if 0
  template<std::derived_from<SubsystemFamily> FamilyType>
  auto add_subsystem_family() -> SubsystemFamilyBuilder {
    auto* update_family = [](App* app, void* subsystem_state, float dt) {
      SubsystemState& state = *(SubsystemState*)(subsystem_state);
      SubsystemType& subsystems = *static_cast<SubsystemType*>(state->subsystem);

      for (auto& subsystem : subsystems)
      {
        subsystem.update(app, subsystem.state, dt);
      }
      };

    return SubsystemFamilyBuilder();
  }
#endif // 0

  template<SubsystemSetConcept T>
  auto add_subsystem_set() -> SubsystemSetBuilder<T>;

  template<SubsystemConcept T, typename... Args>
  auto add_subsystem(Args&&... args) -> decltype(auto);
  
  template<typename SubsystemType>
  auto get_subsystem() -> const SubsystemType* {
  
  }

  template<typename Res>
  auto get() -> Res& {
    constexpr auto sid = ResourceID(core::type_sid<Res>());
    Res* res = &static_cast<ResourceStorage<Res>*>(res_.at(sid).get())->resource;
    return *res;
  }
  
  auto commands() -> Commands& {
    return *commands_;
  }

  auto events() -> Events& {
    return *events_;
  }

  auto world() -> World& {
    return *world_;
  }

  auto main_window() -> Window& {
    return *window_;
  }

  template<typename AssetType>
  auto assets() -> AssetCache<AssetType>& {
    return this->get<AssetCache<AssetType>>();
  }

  template<typename State> requires(std::is_enum_v<State>)
  auto init_state(State state) -> App& {
    states_->init_state(state);
    return *this;
  }

  template<typename State> requires(std::is_enum_v<State>)
  auto set_state(State state) -> App& {
    states_->set_state(state);
    return *this;
  }
  
  template<typename State> requires(std::is_enum_v<State>)
  bool in_state(State state) {
    return states_->in_state(state);
  }
  
  /// NOTE: might be not necessary
  template<typename State> requires(std::is_enum_v<State>)
  auto state() -> State {
    // return states_.at(type_name<State>()).load();
  }

  void schedule(TimePoint tp, auto&& task) {
    schedule_.emplace(tp, std::move(task));
  }

  void run();
private:
  struct SubsystemDeleter {
    void operator()(Subsystem* subsystem) const;
  };

  void init_subsystem(Subsystem*, StringID);

private:
  Unique<World>                          world_;
  Unique<Events>                        events_;
  Unique<Commands>                    commands_;
  Unique<Window>                        window_;
  Unique<States>                        states_;
  Resources res_;
  u16 frame_rate_{0}; // zero means unlimited
  u16 fixed_framerate_{0};
  
  std::multimap<TimePoint, Fn<void()>> schedule_;
  
  Vec<Unique<Subsystem, SubsystemDeleter>> subsystems_;
  Unique<SubsystemManager> subsystem_manager_;
};

template<typename T>
inline auto App::add_resource(T&& resource) -> App& {
  using resource_type = T;

  constexpr auto rid = ResourceID(core::type_sid<resource_type>());
  res_.emplace(std::pair{ rid, std::make_unique<ResourceStorage<resource_type>>(std::move(resource)) });

  return *this;
}

template<typename T>
inline auto App::add_resource() -> App& {
  constexpr auto sid = ResourceID(core::type_sid<T>());
  res_.emplace(std::pair{ sid, std::make_unique<ResourceStorage<T>>() });

  return *this;
}

template<SubsystemSetConcept T>
inline auto App::add_subsystem_set() -> SubsystemSetBuilder<T> {
  return SubsystemSetBuilder<T>(subsystem_manager_.get());
}

template<SubsystemConcept T, typename... Args>
inline auto App::add_subsystem(Args&&... args) -> decltype(auto) {
  
  return SubsystemBuilder<T, Args...>(subsystem_manager_.get(), CORE_FWD(args)...);

  // T* subsystem = (T*)std::malloc(sizeof(T));

  // constexpr auto sid = core::type_sid<T>();
  // SubsystemInstanceRegistry::register_subsystem(sid, subsystem);
  
  // ::new((void*)subsystem) T(*this, std::forward<Args>(args)...);

  // constexpr bool is_subsystem_family = {
  //   std::is_base_of_v<SubsystemFamily, SubsystemType> && 
  //   std::is_abstract_v<SubsystemType> &&
  //   !std::is_final_v<SubsystemType>
  // };

  // constexpr bool is_subsystem_family_member = {
  //   std::void_t<typename T::Family> &&
  //   std::is_base_of_v<SubsystemFamily, typename T::Family> &&
  //   std::is_base_of_v<typename T::Family, T>
  // };

  // constexpr bool is_standalone_subsystem = !is_subsystem_family_member;

#if 0
  if constexpr (IsSubsystemFamilyMember<SubsystemType>)
  {
    return SubsystemFamilyMemberBuilder<typename SubsystemType::Family>();
  }
  else
  {
    return SubsystemBuilder();
  }


  auto* update = [](App* app, void* subsystem_state, float dt) {
    SubsystemState& internal_state = *(SubsystemState*)(subsystem_state);
    SubsystemType& subsystem = *static_cast<SubsystemType*>(state->subsystem);

    if (subsystem.SubsystemType::shutdown_condition(*app))
    {
      // shutdown the subsystem and return
      return;
    }

    if (subsystem.SubsystemType::run_condition(*app))
    {
      if constexpr (&SubsystemType::start == &Subsystem::start)
      {
        if (!internal_state->was_running_last_frame)
        {
          subsystem.SubsystemType::start();
          internal_state->was_running_last_frame = !internal_state->was_running_last_frame;
        }
      }

      /// NOTE: add a warning that fixed_update should not be default if fixed_timestep is overriden

      if constexpr (&SubsystemType::fixed_update != &Subsystem::fixed_update)
      {
        float fixed_timestep = subsystem.SubsystemType::fixed_timestep().sec();
        float& elapsed_sec = internal_state->elapsed_sec;

        elapsed_sec += dt;
        while ((elapsed_sec - fixed_timestep) >= fixed_timestep)
        {
          subsystem.SubsystemType::fixed_update(fixed_timestep);
          time_elapsed -= fixed_timestep;
        }
      }

      subsystem.SubsystemType::update(dt);
    }
    else
    {
      if constexpr (&SubsystemType::finish == &Subsystem::finish)
      {
        if (internal_state->was_running_last_frame)
        {
          subsystem.SubsystemType::finish();
          internal_state->was_running_last_frame = !internal_state->was_running_last_frame;
        }
      }
    }
    };
#endif // 0

  // init_subsystem(subsystem, sid);

  //return SubsystemBuilder{};

  //subsystems_.emplace(std::pair{ sid, subsystem });

  /*commands_->enqueue([] {
  });*/

  //auto devirtualized_update = [this](Subsystem* base, float dt) {
  // //make the interface public and forget 
  // SubsystemType* subsystem = static_cast<SubsystemType*>(base);

  //  if (!subsystem->SubsystemType::run_condition(*this))
  //    return;

  //  subsystem->SubsystemType::update(dt);
  //};
}
