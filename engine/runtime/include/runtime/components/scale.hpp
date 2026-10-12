#pragma once
#include <runtime/component.hpp>
#include <glm/glm.hpp>

struct Scale: vec3, public Component {
  using vec3::vec3;
};