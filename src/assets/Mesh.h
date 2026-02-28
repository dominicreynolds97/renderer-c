#ifndef MESH_H
#define MESH_H

#include "maths/Maths3D.h"

typedef struct {
  Vec3f *positions;
  Vec3f *normals;
  Vec2f *uvs;
  int   *indices;
  int   vertex_count;
  int   index_count;
} Mesh;

Mesh load_obj(const char *path);

void free_mesh(Mesh *mesh);

#endif
