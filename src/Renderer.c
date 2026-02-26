#include "Renderer.h"

RenderMesh renderer_upload_mesh(Mesh *mesh) {
  RenderMesh rm = {0};
  (void)mesh;
  return rm;
}

void renderer_draw(RenderMesh *rm, GLuint shader) {
  (void)rm;
  (void)shader;
}

void renderer_free(RenderMesh *rm) {
  (void)rm;
}
