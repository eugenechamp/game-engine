#version 450 core

out vec4 FragColor;

in vec3 o_normal;
in vec2 o_uv;
in vec3 frag_pos;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D specular;

uniform vec3 light_pos;

void main()
{
  float ambient_strength = 0.3;
  
  vec3 ambient = vec3(1.0, 1.0, 1.0) * ambient_strength;

  vec3 light_dir = normalize(light_pos - frag_pos);

  float diffuse = max(dot(light_dir, o_normal), 0.0);

  //TODO: implement specular later on

  vec4 lit = vec4(vec3(ambient + diffuse).rgb, 1.0);
  
  FragColor = lit * texture(albedo, o_uv);
}