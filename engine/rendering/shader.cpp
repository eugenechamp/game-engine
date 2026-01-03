#include "shader.hpp"

#include "../io.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>

namespace detail
{
  static const std::unordered_map<std::string, Shader::Type> shader_extension_dictionary =
  {
    { ".vert", Shader::Type::vertex },   { ".vs", Shader::Type::vertex },
    { ".frag", Shader::Type::fragment }, { ".fs", Shader::Type::fragment },
  };

  Shader::Type get_type(const std::string& extension)
  {
    auto type = Shader::Type::undefined;
    auto res = shader_extension_dictionary.find(extension);
    
    if (res != shader_extension_dictionary.end())
    {
      type = res->second;
    }

    return type;
  }
  
  GLenum native_shader_type(const Shader::Type type)
  {
    switch(type)
    {
    case Shader::Type::vertex:
      return GL_VERTEX_SHADER;
    case Shader::Type::fragment:
      return GL_FRAGMENT_SHADER;
    case Shader::Type::geometry:
      return GL_GEOMETRY_SHADER;
    }
    return GL_NONE;
  }

  std::string to_string(const Shader::Type type)
  {   
    switch(type)
    {
    case Shader::Type::vertex:
      return "VERTEX";
    case Shader::Type::fragment:
      return "FRAGMENT";
    case Shader::Type::geometry:
      return "GEOMETRY";
    }
    return "UNDEFINED";
  }
}

void Shader::bind() const
{
  glUseProgram(program_);
}

Shader::CompileStatus Shader::_filepath(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path))
  {
    std::cout << "file does not exist\n";
  }
  
  CompileStatus result{};
  
  result.type = detail::get_type(path.extension().string());

  auto source = io::read_binary(path);

  const char* source_cstring = source.c_str();

  result.shader = glCreateShader(detail::native_shader_type(result.type));
  glShaderSource(result.shader, 1, &source_cstring, nullptr);
  glCompileShader(result.shader);
  
  glGetShaderiv(result.shader, GL_COMPILE_STATUS, &result.status);

  char infoLog[512];
  if(!result.status)
  {
    glGetShaderInfoLog(result.shader, 1024, NULL, infoLog);
    std::cout << path.filename() << "COMPILATION FAILED: " << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl; 
  }
  return result;
}

void Shader::_attach(ShaderProgram shader)
{
  glAttachShader(program_, shader);
}

void Shader::_cleanup(ShaderProgram shader)
{
  glDeleteShader(shader);
}

int Shader::_link_program()
{
  int status;
  glLinkProgram(program_);
  glGetProgramiv(program_, GL_LINK_STATUS, &status);
  
  char infoLog[1024]{};
  
  if (!status)
  {
    glGetProgramInfoLog(program_, 1024, NULL, infoLog);
    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << " linking " << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
  }

  return status;
}

void Shader::_create_program()
{
  program_ = glCreateProgram();
}

void Shader::_delete_program()
{
  glDeleteProgram(program_);
}

void Shader::set_bool(const std::string& name, bool value) const
{
  glUniform1i(glGetUniformLocation(program_, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::set_int(const std::string& name, int value) const
{
  glUniform1i(glGetUniformLocation(program_, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::set_float(const std::string& name, float value) const
{
  glUniform1f(glGetUniformLocation(program_, name.c_str()), value);
}
void Shader::vec2(const std::string& name, const glm::vec2& value) const
{
  glUniform2fv(glGetUniformLocation(program_, name.c_str()), 1, &value[0]);
}
void Shader::vec2(const std::string& name, float x, float y) const
{
  glUniform2f(glGetUniformLocation(program_, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::vec3(const std::string& name, const glm::vec3& value) const
{
  glUniform3fv(glGetUniformLocation(program_, name.c_str()), 1, &value[0]);
}
void Shader::vec3(const std::string& name, float x, float y, float z) const
{
  glUniform3f(glGetUniformLocation(program_, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::vec4(const std::string& name, const glm::vec4& value) const
{
  glUniform4fv(glGetUniformLocation(program_, name.c_str()), 1, &value[0]);
}
void Shader::vec4(const std::string& name, float x, float y, float z, float w) const
{
  glUniform4f(glGetUniformLocation(program_, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::mat2(const std::string& name, const glm::mat2& mat) const
{
  glUniformMatrix2fv(glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::mat3(const std::string& name, const glm::mat3& mat) const
{
  glUniformMatrix3fv(glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::mat4(const std::string& name, const glm::mat4& mat) const
{
  glUniformMatrix4fv(glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
