#include "gfx.hpp"

#include <glad/glad.h>
#include <stb_image.h>

#include <iostream>
#include <filesystem>

#include <unordered_map>

std::unordered_map<std::filesystem::path, uint32_t> texture_cache;

Texture load_texture(const std::filesystem::path& filepath, const TextureType type)
{
  stbi_set_flip_vertically_on_load(false);
  if (auto it = texture_cache.find(filepath); it != texture_cache.end())
  {
    return Texture{ it->second, type };
  }
  
  int width, height, channels;
  uint8_t* data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
  
  Texture texture {0, type};
  glGenTextures(1, &texture.id);

  if(data)
  {
    GLenum format = GL_RGBA;

    switch(channels)
    {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    }

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else 
  {
    std::cout << "texture failed to load at path: " << filepath << std::endl;
  }

  stbi_image_free(data);
  
  texture_cache.emplace(std::pair{filepath, texture.id});

  return texture;
}

Texture texture_from_image(const Image& image)
{
  // TODO: implement the function
  return Texture();
}

Image load_image(const std::filesystem::path &filepath)
{
  int width, height, channels;
  uint8_t* data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
  
  return { data, width, height, channels };
}