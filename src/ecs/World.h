#ifndef WORLD_H
#define WORLD_H

#include "maths/Maths3D.h"
#include "scene/Registry.h"
#include "scene/camera.h"
#include "vendor/uthash.h"

#define MAX_WAYPOINTS 16

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
  Entity  entity;
  Vec3f   velocity;
  UT_hash_handle hh;
} VelocityComponent;

typedef struct {
  Vec3f   starting_pos;
  Vec3f   waypoints[MAX_WAYPOINTS];
  int     is_loop;
  int     current_waypoint;
  int     waypoint_count;
  int     initialized;
} Path;

typedef struct {
  Entity  entity;
  Path    path;
  UT_hash_handle hh;
} PathComponent;

typedef struct {
  Entity  entity;
  float   speed;
  UT_hash_handle hh;
} SpeedComponent;

typedef struct {
  Entity  entity;
  float   height;
} PlayerComponent;

typedef struct {
  Entity  entity;
  Vec3f   half_extents;
  int     is_static;
  float   restitution;
  float   friction;
  UT_hash_handle hh;
} ColliderComponent;

typedef struct {
  Entity  entity;
  float   mass;
  Entity  grounded_entity;
  UT_hash_handle hh;
} MassComponent;

typedef struct {
  Entity  entity;
  float   thrust;
  float   max_speed;
  UT_hash_handle hh;
} LocomotionComponent;

typedef struct {
  Entity  entity;
  float   jump_force;
  UT_hash_handle hh;
} JumpComponent;

typedef struct {
  int next_id;

  PositionComponent   *positions;
  RotationComponent   *rotations;
  ScaleComponent      *scales;
  MeshComponent       *meshes;
  MaterialComponent   *materials;
  VelocityComponent   *velocities;
  PathComponent       *paths;
  SpeedComponent      *speeds;
  PlayerComponent     player;
  ColliderComponent   *colliders;
  MassComponent       *masses;
  LocomotionComponent *locomotions;
  JumpComponent       *jumps;

  MeshRegistry        mesh_registry;
  MaterialRegistry    material_registry;

  Camera              camera;
} World;

void world_init(World *world);
Entity world_create_entity(World *world);

void world_add_position(World *world, Entity e, Vec3f position);
void world_add_rotation(World *world, Entity e, Vec3f rotation);
void world_add_scale(World *world, Entity e, Vec3f scale);
void world_add_material(World *world, Entity e, int mesh_id);
void world_add_mesh(World *world, Entity e, int mesh_id);
void world_add_velocity(World *world, Entity e, Vec3f velocity);
void world_add_path(World *world, Entity e, Path path);
void world_add_speed(World *world, Entity e, float speed);
void world_add_collider(World *world, Entity e, Vec3f half_extents, int is_static, float restitution, float friction);
void world_add_mass(World *world, Entity e, float mass);
void world_add_locomotion(World *world, Entity e, float thrust, float max_speed);
void world_add_jump(World *world, Entity e, float jump_force);

PositionComponent* world_get_position(World *world, Entity e);
RotationComponent* world_get_rotation(World *world, Entity e);
ScaleComponent* world_get_scale(World *world, Entity e);
MaterialComponent* world_get_material(World *world, Entity e);
MeshComponent* world_get_mesh(World *world, Entity e);
VelocityComponent* world_get_velocity(World *world, Entity e);
PathComponent* world_get_path(World *world, Entity e);
SpeedComponent* world_get_speed(World *world, Entity e);
ColliderComponent* world_get_collider(World *world, Entity e);
MassComponent* world_get_mass(World *world, Entity e);
LocomotionComponent* world_get_locomotion(World *world, Entity e);
JumpComponent* world_get_jump(World *world, Entity e);

void world_destroy_path(World *world, PathComponent *pc);

Mat4 world_get_transform(World *world, Entity e);

void world_destroy_entity(World *world, Entity e);
void world_destroy(World *world);

#endif
