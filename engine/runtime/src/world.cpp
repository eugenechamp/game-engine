#include "runtime/world.hpp"
#include <runtime/entity.hpp>

auto World::make_entity_impl(EntityID entity) -> Entity {
  return Entity(entity, this);
}

auto World::find_entity(std::string_view name) -> Entity {
  return make_entity_impl(EntityID(ecs_.entity(name.data()).id()));
}