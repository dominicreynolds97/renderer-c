#ifndef GRID_H
#define GRID_H

#include "app/App.h"

typedef struct {
  GLuint  vao;
  GLuint  vbo;
  int     vertex_count;
  int     visible;
} Grid;

void init_grid(Grid *grid, int size, int spacing);
void grid_render(Grid *grid, GLuint shader, float *proj, float *view);
void grid_destroy(Grid *grid);

#endif
