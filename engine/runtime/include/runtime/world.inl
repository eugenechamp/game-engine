#pragma once
#include "world.hpp"
#include "entity.hpp"

template<ComponentType T>
struct OnAdd : Event {
  Entity entity;
};

template<ComponentType T>
struct OnRemove : Event {
  Entity entity;
};

struct OnEntityCreated: Event {
  Entity entity;
};

struct OnEntityDestroyed: Event {
  // this entity is not alive at this point in code
  // writting to or reading its components will crash your program
  Entity entity;
};

template<typename... Comps>
inline auto World::create(Comps&&... components) -> Entity {
  auto entity = ecs_.entity();
  ((set_component(entity, CORE_FWD(components))),...);
  return make_entity_impl(EntityID(entity.id()));
}

template<typename... Comps>
inline auto World::create_named(std::string_view name, Comps&&... components) -> Entity {
  auto entity = ecs_.entity(name.data());
  ((set_component(entity, CORE_FWD(components))), ...);
  return make_entity_impl(EntityID(entity.id()));
}

template<typename... Comps>
inline auto World::create() -> Entity {
  auto entity = ecs_.entity();
  ((entity.add<Comps>()), ...);
  return make_entity_impl(EntityID(entity.id()));

}

template<typename... Comps>
inline auto World::create_named(std::string_view name) -> Entity {
  auto entity = ecs_.entity(name.data());
  ((entity.add<Comps>()), ...);
  return make_entity_impl(EntityID(entity.id()));
}

template<typename... Comps>
inline auto World::prefab() -> Entity {
  auto prefab = ecs_.prefab();
  ((prefab.add<Comps>()), ...);

  return make_entity_impl(EntityID(prefab.id()));
}

template<typename... Comps>
auto World::prefab(Comps&&... components) -> Entity {
  auto prefab = ecs_.prefab();
  ((set_component(prefab, CORE_FWD(components))),...);
  return make_entity_impl(EntityID(prefab.id()));
}

template<typename T>
auto World::get_res() {
  if constexpr (is_query<T>::value)
  {
    return make_query<T>();
  }
}

template<ComponentType C>
inline void World::add_component(EntityID entity) {
  register_ecs_events<C>();
  auto ent = ecs_.get_alive(static_cast<flecs::entity_t>(entity));
  ent.add<C>();
  
  app_.events().emit<OnAdd<C>>().emplace_back(OnAdd<C>{
    .entity = Entity(entity, this)
  });
}

template<typename T>
inline void World::register_ecs_events() {
  [[maybe_unused]] static bool _ = [this] {
    app_.events().add_event<OnAdd<T>>();
    app_.events().add_event<OnRemove<T>>();
    return bool();
  } ();
}

template<ComponentType C>
inline void World::add_component(EntityID entity, C&& component) {
  register_ecs_events<C>();

  flecs::entity ent = ecs_.get_alive(static_cast<flecs::entity_t>(entity));
  if constexpr (std::is_empty_v<C>)
  {
    ent.add<C>();
  }
  else
  {
    ent.set<C>(std::forward<C>(component));
  }

  app_.events().emit<OnAdd<C>>().emplace_back(OnAdd<C>{
    .entity = Entity(entity, this)
  });
}

template<ComponentType C>
inline auto World::get_component(EntityID entity) -> const C* {
  return ecs_.get_alive(static_cast<flecs::entity_t>(entity)).get<C>();
}

template<ComponentType C>
inline auto World::get_mut_component(EntityID entity) -> C* {
  return ecs_.get_alive(static_cast<flecs::entity_t>(entity)).get_mut<C>();
}

template<ComponentType C>
inline void World::remove_component(EntityID entity) {
  register_ecs_events<C>();
  ecs_.get_alive(static_cast<flecs::entity_t>(entity)).remove<C>();
}

inline void World::destroy(EntityID entity) {
  ecs_.get_alive(static_cast<flecs::entity_t>(entity)).destruct();
}

template<ComponentType C>
inline bool World::has_component(EntityID entity) {
  return ecs_.get_alive(static_cast<flecs::entity_t>(entity)).has<C>();
}