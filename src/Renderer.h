#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include "Mesh.h"
#include "Maths3D.h"

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint vco;
  int index_count;
} RenderMesh;

RenderMesh renderer_upload_mesh(Mesh *mesh);
void renderer_draw(RenderMesh *rm, GLuint shader);
void renderer_free(RenderMesh *rm);

#endif
