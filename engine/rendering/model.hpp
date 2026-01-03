#pragma once

#include "mesh.hpp"
#include <runtime/def.hpp>

namespace std::filesystem {
  class path;
}

class Mesh;
class Shader;

class Model
{
public:
  Model() = default;
  ~Model();
  
  Model(const std::filesystem::path& path)
  {
    load(path);
  }

  void load(const std::filesystem::path& path);
  void draw();
  void draw(Shader& shader);
private:
  void _process_node(struct aiNode*, struct aiScene const*, const std::filesystem::path&);
  Mesh _process_mesh(struct aiMesh*, struct aiScene const*, const std::filesystem::path&);
private:
  Vec<Mesh> meshes_;
};