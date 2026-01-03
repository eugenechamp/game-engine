#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <iostream>


void Model::load(const std::filesystem::path &path)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
  
  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cout << "Couldn\'t load a Model at " << path << " reason: " << importer.GetErrorString() << '\n';
  }
  else
    _process_node(scene->mRootNode, scene, path);
}

void Model::draw(Shader& shader)
{
  for (auto& mesh : meshes_)
  {
    mesh.draw(shader);
  }
}

void Model::draw()
{
  for(auto& mesh : meshes_)
  {
    mesh.draw();
  }
}

void Model::_process_node(aiNode *node, const aiScene *scene, const std::filesystem::path& model_path)
{
  for(int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.emplace_back(_process_mesh(mesh, scene, model_path));
  }

  for(int i = 0; i < node->mNumChildren; i++)
  {
    _process_node(node->mChildren[i], scene, model_path);
  }
}

Mesh Model::_process_mesh(aiMesh *msh, const aiScene *scene, const std::filesystem::path& model_path)
{
  std::vector<Vertex>  vertices;
  std::vector<Texture> textures;
  std::vector<uint32_t> indices;
  
  vertices.reserve(msh->mNumVertices);

  for(int i = 0; i < msh->mNumVertices; i++)
  {
    Vertex& vertex = vertices.emplace_back();
    
    const auto& pos = msh->mVertices[i];
    vertex.position = { pos.x, pos.y, pos.z };
    
    if(msh->HasNormals())
    {
      const auto& normal = msh->mNormals[i];
      vertex.normal = glm::normalize(glm::vec3{ normal.x, normal.y, normal.z });
    }

    if (msh->mTextureCoords[0])
    {
      const auto& uvs = msh->mTextureCoords[0][i];
      vertex.uv = { uvs.x, uvs.y };
    }
  }
  
  size_t size = 0;
  for (uint32_t i = 0; i < msh->mNumFaces; i++)
  {
    aiFace face = msh->mFaces[i];
    size += face.mNumIndices;
  }

  indices.reserve(indices.size() + size);
  for(uint32_t i = 0; i < msh->mNumFaces; i++)
  {
    aiFace face = msh->mFaces[i];

    for(uint32_t j = 0; j < face.mNumIndices; j++)
    {
      indices.push_back(face.mIndices[j]);
    }
  }

  aiMaterial* material = scene->mMaterials[msh->mMaterialIndex];

  auto load_material_textures = [&textures, parent_path = model_path.parent_path()](const aiMaterial* material, const aiTextureType aitype, const TextureType type) mutable {
    
    for(uint32_t i = 0; i < material->GetTextureCount(aitype); i++)
    {
      aiString name;
      material->GetTexture(aitype, i, &name);
      
      auto texture_path = parent_path / name.C_Str();

      textures.emplace_back(load_texture(texture_path, type));
    }
  };
  
  load_material_textures(material, aiTextureType_DIFFUSE, TextureType::albedo);
  load_material_textures(material, aiTextureType_SPECULAR, TextureType::specular);
  load_material_textures(material, aiTextureType_HEIGHT, TextureType::normal);
  load_material_textures(material, aiTextureType_AMBIENT, TextureType::ambient);

  return Mesh(std::move(vertices), std::move(textures), std::move(indices));

}
Model::~Model() = default;