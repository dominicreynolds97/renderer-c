#ifndef REGISTRY_H
#define REGISTRY_H

#include "../maths/Maths3D.h"
#include "../rendering/Renderer.h"
#include <stdio.h>

#define MAX_MESHES 16
#define MAX_MATERIALS 16

typedef struct {
  Vec3f color;
} Material;

typedef struct {
  RenderMesh  items[MAX_MESHES];
  int         count;
} MeshRegistry;

typedef struct {
  Material  items[MAX_MATERIALS];
  int       count;
} MaterialRegistry;

void        mesh_reg_init(MeshRegistry *mesh_registry);
int         mesh_reg_add(MeshRegistry *mesh_registry, RenderMesh item);
RenderMesh* mesh_reg_get(MeshRegistry *mesh_registry, int id);
void        mesh_reg_destroy(MeshRegistry *mesh_registry);
void        mat_reg_init(MaterialRegistry *material_registry);
int         mat_reg_add(MaterialRegistry *material_registry, Material item);
Material*   mat_reg_get(MaterialRegistry *material_registry, int id);
void        mat_reg_destroy(MaterialRegistry *material_registry);

#endif
