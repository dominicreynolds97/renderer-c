#version 330 core

in vec3 v_normal;
in vec3 v_world_pos;
in vec2 v_uv;

uniform vec3 u_light_dir;
uniform vec3 u_color;
uniform sampler2D u_texture;
uniform int u_has_texture;

out vec4 frag_color;

void main() {
  vec3 base_color = u_has_texture == 1
    ? texture(u_texture, v_uv).rgb
    : u_color;

  vec3 n = normalize(v_normal);
  vec3 l = normalize(u_light_dir);
  float diffuse = max(dot(n, l), 0.0);
  float ambient = 0.3;
  float intensity = ambient + (1.0 - ambient) * diffuse;

  frag_color = vec4(base_color * intensity, 1.0);
}
