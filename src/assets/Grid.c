#include "Grid.h"
#include "app/App.h"
#include "maths/Maths3D.h"
#include <stdlib.h>

void init_grid(Grid *grid, int size, int spacing) {
  int lines = (size * 2) / spacing + 1;
  int total_vertices = lines * 2 * 2 * 3;
  float *vertices = malloc(total_vertices * sizeof(float));
  int idx = 0;

  for (int i = -size; i < size; i += spacing) {
    vertices[idx++] = (float)i;
    vertices[idx++] = 0.0f;
    vertices[idx++] = -(float)size;
    vertices[idx++] = (float)i;
    vertices[idx++] = 0.0f;
    vertices[idx++] = (float)size;

    vertices[idx++] = -(float)size;
    vertices[idx++] = 0.0f;
    vertices[idx++] = (float)i;
    vertices[idx++] = (float)size;
    vertices[idx++] = 0.0f;
    vertices[idx++] = (float)i;
  }

  grid->vertex_count = idx / 3;
  grid->visible = 1;

  glGenVertexArrays(1, &grid->vao);
  glGenBuffers(1, &grid->vbo);

  glBindVertexArray(grid->vao);
  glBindBuffer(GL_ARRAY_BUFFER, grid->vbo);
  glBufferData(GL_ARRAY_BUFFER, idx * sizeof(float), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  free(vertices);
}

void grid_render(Grid *grid, GLuint shader, float *proj, float *view) {
  if (!grid->visible) return;

  glUseProgram(shader);

  Mat4 proj_mat = *(Mat4*)proj;
  Mat4 view_mat = *(Mat4*)view;
  Mat4 model    = mat4_identity();
  Mat4 mvp      = mat4_mul(proj_mat, mat4_mul(view_mat, model));

  glUniformMatrix4fv(glGetUniformLocation(shader, "u_mvp"), 1, GL_TRUE, &mvp.m[0][0]);
  glUniform3f(glGetUniformLocation(shader, "u_color"), 0.5f, 0.5f, 0.5f);

  glBindVertexArray(grid->vao);
  glDrawArrays(GL_LINES, 0, grid->vertex_count);
  glBindVertexArray(0);
}

void grid_destroy(Grid *grid) {
  glDeleteVertexArrays(1, &grid->vao);
  glDeleteBuffers(1, &grid->vbo);
}

