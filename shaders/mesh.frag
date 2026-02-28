#version 330 core

in vec3 v_normal;
in vec3 v_world_pos;

uniform vec3 u_light_dir;
uniform vec3 u_color;

out vec4 frag_color;

void main() {
  vec3 n = normalize(v_normal);
  vec3 l = normalize(u_light_dir);
  float diffuse = max(dot(n, l), 0.0);
  float ambient = 0.3;
  float intensity = ambient + (1.0 - ambient) * diffuse;

  frag_color = vec4(u_color * intensity, 1.0);
}
