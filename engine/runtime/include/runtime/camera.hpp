#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

///TODO: rewrite
struct FreeCamera {
public:
  enum class projection_type {
    none = 0,
    perspective,
    orthographic,
  };

public:
  FreeCamera() = default;
  FreeCamera(const glm::vec3& position)
    : position_{position}
  {

  }
  
  void perspective(float fovy, float aspect_ratio, float near, float far)
  {
    type_ = projection_type::perspective;
    fovy_ = fovy;
    aspect_ratio_ = aspect_ratio;
    near_ = near;
    far_ = far;
  }

  void ortho(float fovy, float aspect, float near = 0.1f, float far = 1000.f)
  {
    type_ = projection_type::orthographic;
    fovy_ = fovy;
    aspect_ratio_ = aspect;
    near_ = near;
    far_ = far;
  }

  inline glm::mat4 projection() const
  {
    switch(type_)
    {
    case projection_type::perspective:
      return glm::perspective(fovy_, aspect_ratio_, near_, far_);
    case projection_type::orthographic:
      return glm::ortho(-1.6f, 1.6f, -0.9f, 0.9f, near_, far_); // wtf
    }

    return glm::mat4(1.f);
  }

  inline glm::mat4 view()
  {
    front_.x = cos(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));
    front_.y = sin(glm::radians(rotation_.y));
    front_.z = sin(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));

    right_ = glm::normalize(glm::cross(front_, WORLD_UP));
    auto up = glm::normalize(glm::cross(right_, front_));

    return glm::lookAt(position_, position_ + front_, up);
  }
  
  inline const glm::vec3& position() const
  {
    return position_;
  }

  void position(const glm::vec3& position)
  {
    position_ = position;
  }

  void rotation(const glm::vec3& rotation)
  {
    rotation_ = rotation;
  }

  void rotation(float yaw, float pitch, float roll)
  {
    rotation_ = { yaw, pitch, roll };
  }

  const glm::vec3& rotation() const
  {
    return rotation_;
  }

  const glm::vec3& right() const 
  {
    return right_;
  }

  const glm::vec3& front() const {
    return front_;
  }

private:
  static constexpr glm::vec3 WORLD_UP = { 0.f, 1.f, 0.f };

  glm::vec3 position_{};
  glm::vec3 rotation_{-90.f, 0.f, 0.f};

  float near_ = 0.001f;
  float far_ = 100.f;
  float fovy_ = 45.f;
  
  float aspect_ratio_ = 16 / 9.f;
  
  projection_type type_ = { projection_type::perspective };

  glm::vec3 front_ = {};
  glm::vec3 right_ = {};
};

struct view_info
{
  glm::mat4 projection;
  glm::mat4 view;
};