#ifndef MESH_H
#define MESH_H

#include "../maths/Maths3D.h"

typedef struct {
  Vec3f *vertices;
  Vec3f *normals;
  int vertex_count;
  int (*faces)[3];
  int face_count;
} Mesh;

Mesh load_obj(const char *path);

void free_mesh(Mesh *mesh);

#endif
