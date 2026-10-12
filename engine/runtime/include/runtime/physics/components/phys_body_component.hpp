#pragma once

#include <core/def.hpp>
#include <runtime/physics/physics.hpp>
#include <runtime/component.hpp>

// automatically attached to an entity after adding to the physics system by physics backend.
struct PhysBodyComponent: Component {
  friend PhysicsBackend;

public:
  PhysBodyID body;

private:
  PhysBodyComponent() = default;
  explicit PhysBodyComponent(PhysBodyID body_id) 
    : body{ body_id }
  { }
};

static_assert(std::is_trivially_copyable_v<PhysBodyComponent>);