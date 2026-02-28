#ifndef WORLD_H
#define WORLD_H

#include "maths/Maths3D.h"
#include "scene/Registry.h"
#include "vendor/uthash.h"

typedef int Entity;

typedef struct {
  Entity  entity;
  Mat4    transform;
  UT_hash_handle hh;
} TransformComponent;

typedef struct {
  Entity  entity;
  int     mesh_id;
  UT_hash_handle hh;
} MeshComponent;

typedef struct {
  Entity  entity;
  int     mat_id;
  UT_hash_handle hh;
} MaterialComponent;

typedef struct {
  int next_id;

  TransformComponent  *transforms;
  MeshComponent       *meshes;
  MaterialComponent   *materials;

  MeshRegistry        mesh_registry;
  MaterialRegistry    material_registry;
} World;

void world_init(World *world);
Entity world_create_entity(World *world);

void world_add_transform(World *world, Entity e, Mat4 transform);
void world_add_material(World *world, Entity e, int mesh_id);
void world_add_mesh(World *world, Entity e, int mesh_id);

TransformComponent* world_get_transform(World *world, Entity e);
MaterialComponent* world_get_material(World *world, Entity e);
MeshComponent* world_get_mesh(World *world, Entity e);

void world_destroy_entity(World *world, Entity e);
void world_destroy(World *world);

#endif
