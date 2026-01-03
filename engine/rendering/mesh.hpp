#pragma once
#include <glm/glm.hpp>

struct SimpleVertext
{
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 uv;
};

struct Vertex{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

#include "gfx.hpp"

#include <vector>
#include "shader.hpp"

struct RenderContext
{
  uint32_t vao;
  uint32_t vbo;
  uint32_t ebo;
};

class Mesh
{
public:
  Mesh() = default;
  Mesh(const std::vector<Vertex>& v, const std::vector<Texture>& t, const std::vector<uint32_t>& indices)
    : vertices_{v}, textures_{t}, indices_{indices}
  { 
    _setup();
  }

  Mesh(std::vector<Vertex>&& v, std::vector<Texture>&& t, std::vector<uint32_t>&& indices)
    : vertices_{ std::move(v) }, textures_{ std::move(t) }, indices_{std::move(indices)}
  {
    _setup();
  }

  Mesh(const Mesh& Mesh) = default;
  Mesh(Mesh&& Mesh) = default;

  Mesh& operator=(const Mesh& Mesh) = default;
  Mesh& operator=(Mesh&& Mesh) = default;

  void draw();
  void draw(Shader& shader);
private:
  void _setup();

private:
  RenderContext rcontext_ = {};
  
  std::vector<Vertex> vertices_;
  std::vector<Texture> textures_;
  std::vector<uint32_t> indices_;
};

Mesh make_sphere(uint32_t sector, uint32_t stack); 
Mesh make_surface(uint32_t vwidth, uint32_t vheight);