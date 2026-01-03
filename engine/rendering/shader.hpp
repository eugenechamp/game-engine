#pragma once
#include <string>
#include <filesystem>
#include <glm/fwd.hpp>

class Shader
{
public:
  enum class Type
  {
    undefined,
    vertex,
    geometry,
    fragment,
  };

  ~Shader()
  {
    _delete_program();
  }

  void bind() const;

  Shader() = default;

  Shader(Shader&& Shader) noexcept
      : program_(Shader.program_)
  {
    Shader.program_ = 0;
  }

  Shader& operator=(Shader&& Shader) noexcept
  {
    if (this != &Shader)
    {
      _delete_program();
      this->program_ = Shader.program_;
      Shader.program_ = 0;
    }
    return *this;
  }

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  template<typename Filepath1, typename Filepath2, typename... Files>
  void load(Filepath1&& f0, Filepath2&& f1, Files&&... files)
  {
    _create_program();

    CompileStatus results[2 + sizeof...(Files)] = {_filepath(std::forward<Filepath1>(f0)), _filepath(std::forward<Filepath2>(f1)),
        _filepath(std::forward<Files>(files))... };

    for(auto& stage : results)
      _attach(stage.shader);

    auto linkage_status = _link_program();
    
    for(auto& stage : results)
      _cleanup(stage.shader);
  }

  void set_bool(const std::string& name, bool value) const;
  void set_int(const std::string& name, int value) const;
  void set_float(const std::string& name, float value) const;
  void vec2(const std::string& name, const glm::vec2& value) const;
  void vec2(const std::string& name, float x, float y) const;
  void vec3(const std::string& name, const glm::vec3& value) const;
  void vec3(const std::string& name, float x, float y, float z) const;
  void vec4(const std::string& name, const glm::vec4& value) const;
  void vec4(const std::string& name, float x, float y, float z, float w) const;
  void mat2(const std::string& name, const glm::mat2& mat) const;
  void mat3(const std::string& name, const glm::mat3& mat) const;
  void mat4(const std::string& name, const glm::mat4& mat) const;
  auto id() const { return program_; }

private:
  using ShaderProgram = uint32_t;

  struct CompileStatus
  {
    uint32_t shader;
    int32_t status;
    Type type;
  };

  CompileStatus _filepath(const std::filesystem::path& path);
  //CompileStatus _filepath(const std::string_view path);

  void _attach(uint32_t shader);

  void _cleanup(uint32_t shader);

  int _link_program();

  void _create_program();
  void _delete_program();
private:
  ShaderProgram program_ = 0;
};