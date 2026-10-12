#pragma once
#include "physics_fwd.hpp"
#include <core/def.hpp>

enum struct PhysBodyID: u32 {
  Invalid = u32(-1)
};

enum struct PhysMotionType: u8 {
  Static,
  Kinematic,
  Dynamic,
};

enum struct PhysLayer: u16;