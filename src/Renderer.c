#include "Renderer.h"
#include <stdio.h>
#include <stdlib.h>

RenderMesh renderer_upload_mesh(Mesh *mesh) {
  RenderMesh rm = {0};

  float *vertex_data = malloc(mesh->vertex_count * 6 * sizeof(float));
  for (int i = 0; i < mesh->vertex_count; i++) {
    vertex_data[i * 6 + 0] = mesh->vertices[i].x;
    vertex_data[i * 6 + 1] = mesh->vertices[i].y;
    vertex_data[i * 6 + 2] = mesh->vertices[i].z;
    vertex_data[i * 6 + 3] = mesh->normals[i].x;
    vertex_data[i * 6 + 4] = mesh->normals[i].y;
    vertex_data[i * 6 + 5] = mesh->normals[i].z;
  }

  int *indices = malloc(mesh->face_count * 3 * sizeof(int));
  for (int i = 0; i< mesh->face_count; i++) {
    indices[i * 3 + 0] = (unsigned int)mesh->faces[i][0];
    indices[i * 3 + 1] = (unsigned int)mesh->faces[i][1];
    indices[i * 3 + 2] = (unsigned int)mesh->faces[i][2];
  }

  printf("vertex_count: %d, face_count: %d\n", mesh->vertex_count, mesh->face_count);

  // verify no index is out of bounds
  for (int i = 0; i < mesh->face_count * 3; i++) {
      if (indices[i] < 0 || indices[i] >= mesh->vertex_count) {
          printf("BAD INDEX at %d: %d (max %d)\n", i, indices[i], mesh->vertex_count - 1);
      }
  }

  glGenVertexArrays(1, &rm.vao);
  glGenBuffers(1, &rm.vbo);
  glGenBuffers(1, &rm.ebo);

  glBindVertexArray(rm.vao);

  glBindBuffer(GL_ARRAY_BUFFER, rm.vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * 6 * sizeof(float),
      vertex_data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rm.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->face_count * 3 * sizeof(unsigned int),
      indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
      (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  rm.index_count = mesh->face_count * 3;

  free(vertex_data);
  free(indices);

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
