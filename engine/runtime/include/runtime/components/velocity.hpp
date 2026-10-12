#pragma once
#include <glm/glm.hpp>
#include <runtime/component.hpp>

struct Velocity: vec3, public Component {
  using vec3::vec3;
};