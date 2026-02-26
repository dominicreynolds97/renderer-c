#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

GLuint  shader_load(const char *vert_path, const char *grag_path);
void    shader_use(GLuint program);
void    shader_free(GLuint program);

void shader_set_mat4(GLuint program, const char *name, const float *mat);
void shader_set_int(GLuint program, const char *name, const int value);
void shader_set_float(GLuint program, const char *name, const float value);

#endif
