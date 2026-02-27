#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include "../assets/Mesh.h"

typedef struct {
  int mesh_id;
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  int index_count;
} RenderMesh;

RenderMesh renderer_upload_mesh(Mesh *mesh);
void renderer_draw(RenderMesh *rm, GLuint shader);
void renderer_free(RenderMesh *rm);

#endif
