#pragma once
#include <runtime/physics/physics.hpp>
#include <runtime/component.hpp>

struct RigidBodyComponent: Component {
  PhysMotionType motion;
};