#include "Shader.h"
#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    printf("Shader: could not open: %s\n", path);
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  char *src = malloc(size + 1);
  fread(src, 1, size, f);
  src[size] = '\0';
  fclose(f);
  return src;
}

static GLuint compile_shader(const char *path, GLenum type) {
  char *src = read_file(path);
  if (!src) return 0;

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, (const char **)&src, NULL);
  glCompileShader(shader);
  free(src);

  int ok;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    printf("Shader compile error (%s):\n%s\n", path, log);
    return 0;
  }
  return shader;
}

GLuint shader_load(const char *vert_path, const char *frag_path) {
  GLuint vert = compile_shader(vert_path, GL_VERTEX_SHADER);
  GLuint frag = compile_shader(frag_path, GL_FRAGMENT_SHADER);
  if (!vert || !frag) return 0;

  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  glDeleteShader(vert);
  glDeleteShader(frag);

  int ok;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    printf("Shader link error:\n%s\n", log);
    return 0;
  }
  return program;
}

void shader_use(GLuint program){
  glUseProgram(program);
}

void shader_free(GLuint program) {
  glDeleteProgram(program);
}

void shader_set_mat4(GLuint program, const char *name, const float *mat) {
  glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_TRUE, mat);
}

void shader_set_vec3(GLuint program, const char *name, float x, float y, float z) {
  glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

void shader_set_int(GLuint program, const char *name, int value) {
  glUniform1i(glGetUniformLocation(program, name) , value);
}

void shader_set_float(GLuint program, const char *name, float value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

