#include "Registry.h"

Material DEFAULT_MATERIAL = {
  (Vec3f){1.0f, 1.0f, 1.0f}
};

void mesh_reg_init(MeshRegistry *mesh_registry) {
  mesh_registry->count = 0;
}

int mesh_reg_add(MeshRegistry *mesh_registry, RenderMesh item) {
  if (mesh_registry->count == MAX_MESHES) {
    printf("Failed to add RenderMesh to MeshRegistry - hit max mesh count of: %d\n", mesh_registry->count);
    return -1;
  }
  int new_id = mesh_registry->count;
  mesh_registry->items[new_id] = item;
  mesh_registry->count++;
  return new_id;
}

RenderMesh* mesh_reg_get(MeshRegistry *mesh_registry, int id) {
  if (id >= mesh_registry->count || id < 0) {
    return NULL;
  }
  return &mesh_registry->items[id];
}

void mat_reg_init(MaterialRegistry *material_registry) {
  material_registry->count = 0;
}

int mat_reg_add(MaterialRegistry *material_registry, Material item) {
  if (material_registry->count == MAX_MATERIALS) {
    printf("Failed to add Material to MaterialRegistry - hit max material count of: %d\n", material_registry->count);
    return -1;
  }
  int new_id = material_registry->count;
  material_registry->items[new_id] = item;
  material_registry->count++;
  return new_id;
}

Material* mat_reg_get(MaterialRegistry *material_registry, int id) {
  if (id >= material_registry->count || id < 0) {
    return &DEFAULT_MATERIAL;
  }
  return &material_registry->items[id];
}

void mesh_reg_destroy(MeshRegistry *mesh_registry) {
  for (int i = 0; i < mesh_registry->count; i++) {
    renderer_free(&mesh_registry->items[i]);
  }
  mesh_registry->count = 0;
}

void mat_reg_destroy(MaterialRegistry *material_registry) {
  material_registry->count = 0;
}

