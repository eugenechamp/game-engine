#pragma once
#include "entity.hpp"
#include "world.hpp"

// #ifndef _ENTITY_HPP_
//   #error "implementation file doesn't allow include"
// #endif

template<typename T>
inline void Entity::add() {
  world_->add_component<T>(entity_);
}

template<typename T>
inline void Entity::add(T&& arg) {
  world_->add_component<T>(entity_, CORE_FWD(arg));
}

template<typename T>
inline void Entity::remove() {
  world_->remove_component<T>(entity_);
}

template<ComponentType T>
inline auto Entity::get() const -> const T& {
  return *world_->get_component<T>(entity_);
}

template<ComponentType T>
inline auto Entity::get_mut() -> T& {
  return *world_->get_mut_component<T>(entity_);
}

template<ComponentType T>
inline auto Entity::try_get() const -> const T* {
  return nullptr;
}

template<ComponentType T>
inline auto Entity::try_get_mut() -> T* {
  return nullptr;
}

inline void Entity::destruct() {
  world_->destroy(entity_);
}

inline auto Entity::id() const -> EntityID {
  return entity_;
}

template<ComponentType C>
inline bool Entity::has() const {
  return world_->has_component<C>(entity_);
}

inline bool Entity::active() const {

}

inline void Entity::active(bool value) {

}

inline void Entity::each(Invocable<void(const ComponentID&)> auto callable) {
  
}

inline void Entity::each(Invocable<void(Entity, const ComponentID&)> auto callable) {

}

template<ComponentType C>
inline void Entity::enable() {

}

template<ComponentType C>
inline void Entity::disable() {

}

template<ComponentType C>
inline bool Entity::enabled() {
  return false;
}