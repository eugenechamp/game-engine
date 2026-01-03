#include "mesh.hpp"
#include <glad/glad.h>

#include <numbers>

void Mesh::draw()
{
  glBindVertexArray(rcontext_.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rcontext_.ebo);
  glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(indices_.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::draw(Shader& shader)
{
  for (auto& texture : textures_)
  {
    glBindTextureUnit((int)texture.type, texture.id);

    if (texture.type == TextureType::albedo)
    {
      shader.set_int("albedo", (int)texture.type);
    }
    else if (texture.type == TextureType::normal)
    {
      shader.set_int("normal", (int)texture.type);
    }
    else if (texture.type == TextureType::specular)
    {
      shader.set_int("specular", (int)texture.type);
    }
  }
  glBindVertexArray(rcontext_.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rcontext_.ebo);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices_.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::_setup()
{
  glGenVertexArrays(1, &rcontext_.vao);
  glGenBuffers(1, &rcontext_.vbo);
  glGenBuffers(1, &rcontext_.ebo);

  glBindVertexArray(rcontext_.vao);

  glBindBuffer(GL_ARRAY_BUFFER, rcontext_.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rcontext_.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(uint32_t), indices_.data(), GL_STATIC_DRAW);

  // Vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);
  // Vertex normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  // Vertex texture coords
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(2);
}

Mesh make_sphere(uint32_t sector, uint32_t stack)
{
  using namespace std::numbers;

  std::vector<Vertex> vertices;
  vertices.reserve((sector + 1ULL) * (stack + 1ULL));

  std::vector<uint32_t> indices;
  indices.reserve((sector - 1ULL) * stack * 6);

  for (uint32_t y = 0; y <= stack; ++y)
  {
    float v = y / (float)stack;
    float phi = v * pi_v<float>;

    for (uint32_t x = 0; x <= sector; ++x)
    {
      float u = x / (float)sector;
      float theta = u * 2.0f * pi_v<float>;

      float x_pos = std::cos(theta) * std::sin(phi);
      float y_pos = std::cos(phi);
      float z_pos = std::sin(theta) * std::sin(phi);

      vertices.emplace_back() = {
        .position = glm::vec3(x_pos, y_pos, z_pos),
        .normal   = glm::normalize(glm::vec3(x_pos, y_pos, z_pos)),
        .uv       = glm::vec2(u, v)
      };
    }
  }

  for (uint32_t i = 0; i < stack; ++i)
  {
    uint32_t k1 = i * (sector + 1);
    uint32_t k2 = k1 + sector + 1;
    for (uint32_t  j = 0; j < sector; ++j, ++k1, ++k2)
    {
      if (i != 0)
      {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }
      
      if (i != (stack - 1))
      {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }
  
  return Mesh(std::move(vertices), {}, std::move(indices));
}

Mesh make_surface(uint32_t vwidth, uint32_t vheight)
{
  std::vector<Vertex> vertices;

  for (uint32_t j = 0u; j < vheight; j++)
  {
    for (uint32_t i = 0u; i < vwidth; i++)
    {
      vertices.emplace_back() = Vertex{
        .position = {i, j, 0.f},
        .normal = {},
        .uv = {(float)i / vwidth, (float)j / vheight}
      };
    }
  }


  return Mesh();
}




