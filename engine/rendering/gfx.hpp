#pragma once
#include <cstdint>

namespace std::filesystem {
  class path;
}

enum class TextureType {
  albedo,
  normal,
  specular,
  ambient
};

struct Texture {
  uint32_t id;
  TextureType type;
};

struct Image {
  uint8_t* data;
  int width;
  int height;
  int channels;
};

Texture load_texture(const std::filesystem::path& filepath, const TextureType type = TextureType::albedo);

Texture texture_from_image(const Image& image);

Image load_image(const std::filesystem::path& filepath);
