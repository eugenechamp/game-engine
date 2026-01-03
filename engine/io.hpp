#pragma once

#include<string>
#include<filesystem>

namespace io
{
  using openmode = int32_t;
  auto read_file(const std::filesystem::path&, openmode mode) -> std::string;
  
  inline auto read_binary(const std::filesystem::path& path) -> std::string {
    constexpr openmode std_input_binary = 1 | 32; // std::ios::in | std::ios::binary
    return read_file(path, std_input_binary);
  }

}