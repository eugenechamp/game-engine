#pragma once
#ifndef FLECS_CPP
  #define FLECS_CPP
#endif

#include "def.hpp"

class World;

#include <flecs.h>

using native_entity_t = u64;
static_assert(std::is_same_v<native_entity_t, u64>);

enum struct EntityID: native_entity_t {
  Invalid = ~native_entity_t{}
};

struct Entity {
public:
  Entity() = default;
  Entity(EntityID ent, World* world)
    : entity_{ ent },
      world_{ world }
  {
  }

  Entity(const Entity&) = default;
  Entity(Entity&&) = default;

  auto operator=(const Entity&) -> Entity & = default;
  auto operator=(Entity&&) -> Entity & = default;

  template<typename T>
  void add(); 

  template<typename T>
  void add(T&& arg);

  template<typename T>
  void remove();

  template<ComponentType T>
  auto get() const -> const T&;

  template<ComponentType T>
  auto get_mut() -> T&;

  template<ComponentType T>
  auto try_get() const -> const T*;

  template<ComponentType T>
  auto try_get_mut() -> T*;

  void destruct();

  template<ComponentType C>
  bool has() const;

  auto id() const -> EntityID;
  
  bool active() const;

  void active(bool value);

  template<ComponentType C>
  void enable();

  template<ComponentType C>
  void disable();

  template<ComponentType C>
  bool enabled();

  void each(Invocable<void(const ComponentID&)> auto callable);
  void each(Invocable<void(Entity, const ComponentID&)> auto callable);

private:
  EntityID entity_ = { EntityID::Invalid };
  World* world_    = { nullptr };
};

template<typename... Components>
struct OnEntitySpawnWith: Event {
  Entity entity;
};

#include "entity_impl.inl"