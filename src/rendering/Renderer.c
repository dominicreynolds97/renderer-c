#include "Renderer.h"
#include <stdio.h>
#include <stdlib.h>

float* get_vertex_data(Mesh *mesh) {
  float *vertex_data = malloc(mesh->vertex_count * 8 * sizeof(float));
  for (int i = 0; i < mesh->vertex_count; i++) {
    vertex_data[i * 8 + 0] = mesh->positions[i].x;
    vertex_data[i * 8 + 1] = mesh->positions[i].y;
    vertex_data[i * 8 + 2] = mesh->positions[i].z;
    vertex_data[i * 8 + 3] = mesh->normals[i].x;
    vertex_data[i * 8 + 4] = mesh->normals[i].y;
    vertex_data[i * 8 + 5] = mesh->normals[i].z;
    vertex_data[i * 8 + 6] = mesh->uvs[i].x;
    vertex_data[i * 8 + 7] = mesh->uvs[i].y;
  }
  return vertex_data;
}

RenderMesh renderer_upload_mesh(Mesh *mesh) {
  RenderMesh rm = {0};

  float* vertex_data = get_vertex_data(mesh);

  glGenVertexArrays(1, &rm.vao);
  glGenBuffers(1, &rm.vbo);
  glGenBuffers(1, &rm.ebo);

  glBindVertexArray(rm.vao);

  glBindBuffer(GL_ARRAY_BUFFER, rm.vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * 8 * sizeof(float),
      vertex_data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rm.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(int),
      mesh->indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  rm.index_count = mesh->index_count;

  free(vertex_data);

  return rm;
}

void renderer_draw(RenderMesh *rm, GLuint shader) {
  glUseProgram(shader);
  glBindVertexArray(rm->vao);
  glDrawElements(GL_TRIANGLES, rm->index_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void renderer_free(RenderMesh *rm) {
  glDeleteVertexArrays(1, &rm->vao);
  glDeleteBuffers(1, &rm->vbo);
  glDeleteBuffers(1, &rm->ebo);
}
