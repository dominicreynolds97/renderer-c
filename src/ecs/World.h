#ifndef WORLD_H
#define WORLD_H

#include "maths/Maths3D.h"
#include "scene/Registry.h"
#include "vendor/uthash.h"

typedef int Entity;

typedef struct {
  Entity  entity;
  Vec3f   position;
  UT_hash_handle hh;
} PositionComponent;

typedef struct {
  Entity  entity;
  Vec3f   rotation;
  UT_hash_handle hh;
} RotationComponent;

typedef struct {
  Entity  entity;
  Vec3f   scale;
  UT_hash_handle hh;
} ScaleComponent;

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

  PositionComponent   *positions;
  RotationComponent   *rotations;
  ScaleComponent      *scales;
  MeshComponent       *meshes;
  MaterialComponent   *materials;

  MeshRegistry        mesh_registry;
  MaterialRegistry    material_registry;
} World;

void world_init(World *world);
Entity world_create_entity(World *world);

void world_add_position(World *world, Entity e, Vec3f position);
void world_add_rotation(World *world, Entity e, Vec3f rotation);
void world_add_scale(World *world, Entity e, Vec3f scale);
void world_add_material(World *world, Entity e, int mesh_id);
void world_add_mesh(World *world, Entity e, int mesh_id);

PositionComponent* world_get_position(World *world, Entity e);
RotationComponent* world_get_rotation(World *world, Entity e);
ScaleComponent* world_get_scale(World *world, Entity e);
MaterialComponent* world_get_material(World *world, Entity e);
MeshComponent* world_get_mesh(World *world, Entity e);
Mat4 world_get_transform(World *world, Entity e);

void world_destroy_entity(World *world, Entity e);
void world_destroy(World *world);

#endif
