#version 450 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(std140, binding = 0) uniform view_info
{
  mat4 projection;
  mat4 view;
};

out vec3 o_normal;
out vec2 o_uv;
out vec3 frag_pos;

uniform mat4 model;
uniform mat3 normal_mat;

void main()
{
  vec4 vertex_position = vec4(a_position, 1.0);
  gl_Position = projection * view * model * vertex_position;
  
  frag_pos = vec3(model * vertex_position);
  
  o_normal = normalize(normal_mat * a_normal);
  o_uv = a_uv;
}