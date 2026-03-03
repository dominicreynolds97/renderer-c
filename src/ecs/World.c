#include "World.h"
#include "maths/Maths3D.h"
#include "scene/camera.h"

static void init_player(World *world) {
  Entity e = world_create_entity(world);

  world->player = (PlayerComponent){
    .entity = e,
    .height = 1.8
  };
  world_add_position(world, e, (Vec3f){0.0f, 1.8f, 0.0f});
  world_add_velocity(world, e, vec3f_identity());
  world_add_collider(world, e, (Vec3f){0.3f, 1.6f, 0.3f}, 0, 0.3, 0.7);
  world_add_locomotion(world, e, 2.0, 5.0);
  world_add_jump(world, e, 5.0);
  world_add_mass(world, e, 80.0);
  //world_add_rotation(world, e, vec3f_identity());
  world_add_speed(world, e, 5.0f);
}

void world_init(World *world) {
  world->next_id      = 0;
  world->positions    = NULL;
  world->rotations    = NULL;
  world->scales       = NULL;
  world->meshes       = NULL;
  world->materials    = NULL;
  world->velocities   = NULL;
  world->paths        = NULL;
  world->speeds       = NULL;
  world->colliders    = NULL;
  world->masses       = NULL;
  world->locomotions  = NULL;
  world->jumps        = NULL;

  init_player(world);

  mesh_reg_init(&world->mesh_registry);
  mat_reg_init(&world->material_registry);

  world->camera = init_camera();
}

Entity world_create_entity(World *world) {
  return world->next_id++;
}


void world_add_position(World *world, Entity e, Vec3f position) {
  PositionComponent *c = malloc(sizeof(PositionComponent));
  c->entity = e;
  c->position = position;
  HASH_ADD_INT(world->positions, entity, c);
}

void world_add_rotation(World *world, Entity e, Vec3f rotation) {
  RotationComponent *c = malloc(sizeof(RotationComponent));
  c->entity = e;
  c->rotation = rotation;
  HASH_ADD_INT(world->rotations, entity, c);
}

void world_add_scale(World *world, Entity e, Vec3f scale) {
  ScaleComponent *c = malloc(sizeof(ScaleComponent));
  c->entity = e;
  c->scale = scale;
  HASH_ADD_INT(world->scales, entity, c);
}

void world_add_material(World *world, Entity e, int mat_id) {
  MaterialComponent *c = malloc(sizeof(MaterialComponent));
  c->entity = e;
  c->mat_id = mat_id;
  HASH_ADD_INT(world->materials, entity, c);
}

void world_add_mesh(World *world, Entity e, int mesh_id) {
  MeshComponent *c = malloc(sizeof(MeshComponent));
  c->entity = e;
  c->mesh_id = mesh_id;
  HASH_ADD_INT(world->meshes, entity, c);
}

void world_add_velocity(World *world, Entity e, Vec3f velocity) {
  VelocityComponent *c = malloc(sizeof(VelocityComponent));
  c->entity = e;
  c->velocity = velocity;
  HASH_ADD_INT(world->velocities, entity, c);
}

void world_add_path(World *world, Entity e, Path path) {
  PathComponent *c = malloc(sizeof(PathComponent));
  c->entity = e;
  c->path = path;
  HASH_ADD_INT(world->paths, entity, c);
}

void world_add_speed(World *world, Entity e, float speed) {
  SpeedComponent *c = malloc(sizeof(SpeedComponent));
  c->entity = e;
  c->speed = speed;
  HASH_ADD_INT(world->speeds, entity, c);
}

void world_add_collider(World *world, Entity e, Vec3f half_extents, int is_static, float restitution, float friction) {
  ColliderComponent *c = malloc(sizeof(ColliderComponent));
  c->entity = e;
  c->half_extents = half_extents;
  c->is_static = is_static;
  c->restitution = restitution;
  c->friction = friction;
  HASH_ADD_INT(world->colliders, entity, c);
}

void world_add_mass(World *world, Entity e, float mass) {
  MassComponent *c = malloc(sizeof(MassComponent));
  c->entity = e;
  c->mass = mass;
  c->grounded_entity = -1;
  HASH_ADD_INT(world->masses, entity, c);
}

void world_add_locomotion(World *world, Entity e, float thrust, float max_speed) {
  LocomotionComponent *c = malloc(sizeof(LocomotionComponent));
  c->entity = e;
  c->thrust = thrust;
  c->max_speed = max_speed;
  HASH_ADD_INT(world->locomotions, entity, c);
}

void world_add_jump(World *world, Entity e, float jump_force) {
  JumpComponent *c = malloc(sizeof(JumpComponent));
  c->entity = e;
  c->jump_force = jump_force;
  HASH_ADD_INT(world->jumps, entity, c);
}


PositionComponent* world_get_position(World *world, Entity e) {
  PositionComponent *c;
  HASH_FIND_INT(world->positions, &e, c);
  return c;
}

RotationComponent* world_get_rotation(World *world, Entity e) {
  RotationComponent *c;
  HASH_FIND_INT(world->rotations, &e, c);
  return c;
}

ScaleComponent* world_get_scale(World *world, Entity e) {
  ScaleComponent *c;
  HASH_FIND_INT(world->scales, &e, c);
  return c;
}

MaterialComponent* world_get_material(World *world, Entity e) {
  MaterialComponent *c;
  HASH_FIND_INT(world->materials, &e, c);
  return c;
}

MeshComponent* world_get_mesh(World *world, Entity e) {
  MeshComponent *c;
  HASH_FIND_INT(world->meshes, &e, c);
  return c;
}

VelocityComponent* world_get_velocity(World *world, Entity e) {
  VelocityComponent *c;
  HASH_FIND_INT(world->velocities, &e, c);
  return c;
}

PathComponent* world_get_path(World *world, Entity e) {
  PathComponent *c;
  HASH_FIND_INT(world->paths, &e, c);
  return c;
}

SpeedComponent* world_get_speed(World *world, Entity e) {
  SpeedComponent *c;
  HASH_FIND_INT(world->speeds, &e, c);
  return c;
}

ColliderComponent* world_get_collider(World *world, Entity e) {
  ColliderComponent *c;
  HASH_FIND_INT(world->colliders, &e, c);
  return c;
}

MassComponent* world_get_mass(World *world, Entity e) {
  MassComponent *c;
  HASH_FIND_INT(world->masses, &e, c);
  return c;
}

JumpComponent* world_get_jump(World *world, Entity e) {
  JumpComponent *c;
  HASH_FIND_INT(world->jumps, &e, c);
  return c;
}

LocomotionComponent* world_get_locomotion(World *world, Entity e) {
  LocomotionComponent *c;
  HASH_FIND_INT(world->locomotions, &e, c);
  return c;
}


Mat4 world_get_transform(World *world, Entity e) {
  PositionComponent *pc = world_get_position(world, e);
  RotationComponent *rc = world_get_rotation(world, e);
  ScaleComponent    *sc = world_get_scale(world, e);

  Vec3f position = pc ? pc->position : vec3f_identity();
  Vec3f rotation = rc ? rc->rotation : vec3f_identity();
  Vec3f scale    = sc ? sc->scale    : (Vec3f){1.0f, 1.0f, 1.0f};

  Mat4 transform = mat4_mul(
    mat4_translation(position.x, position.y, position.z),
    mat4_mul(
      mat4_rotation(rotation),
      mat4_scale(scale.x, scale.y, scale.z)
    )
  );

  return transform;
}


static void destroy_position(World *world, PositionComponent *c) {
  HASH_DEL(world->positions, c);
  free(c);
}

static void destroy_rotation(World *world, RotationComponent *c) {
  HASH_DEL(world->rotations, c);
  free(c);
}

static void destroy_scale(World *world, ScaleComponent *c) {
  HASH_DEL(world->scales, c);
  free(c);
}

static void destroy_mesh(World *world, MeshComponent *c) {
  HASH_DEL(world->meshes, c);
  free(c);
}

static void destroy_material(World *world, MaterialComponent *c) {
  HASH_DEL(world->materials, c);
  free(c);
}

static void destroy_velocity(World *world, VelocityComponent *c) {
  HASH_DEL(world->velocities, c);
  free(c);
}

static void destroy_path(World *world, PathComponent *c) {
  HASH_DEL(world->paths, c);
  free(c);
}

static void destroy_speed(World *world, SpeedComponent *c) {
  HASH_DEL(world->speeds, c);
  free(c);
}

static void destroy_collider(World *world, ColliderComponent *c) {
  HASH_DEL(world->colliders, c);
  free(c);
}

static void destroy_mass(World *world, MassComponent *c) {
  HASH_DEL(world->masses, c);
  free(c);
}

static void destroy_jump(World *world, JumpComponent *c) {
  HASH_DEL(world->jumps, c);
  free(c);
}

static void destroy_locomotion(World *world, LocomotionComponent *c) {
  HASH_DEL(world->locomotions, c);
  free(c);
}


void world_destroy_path(World *world, PathComponent *pc) {
  destroy_path(world, pc);
}


void world_destroy_entity(World *world, Entity e) {
  PositionComponent *p = world_get_position(world, e);
  if (p) destroy_position(world, p);

  RotationComponent *r = world_get_rotation(world, e);
  if (r) destroy_rotation(world, r);

  ScaleComponent *s = world_get_scale(world, e);
  if (s) destroy_scale(world, s);

  MeshComponent *mesh = world_get_mesh(world, e);
  if (mesh) destroy_mesh(world, mesh);

  MaterialComponent *mat = world_get_material(world, e);
  if (mat) destroy_material(world, mat);

  VelocityComponent *v = world_get_velocity(world, e);
  if (v) destroy_velocity(world, v);

  PathComponent *path = world_get_path(world, e);
  if (path) destroy_path(world, path);

  SpeedComponent *speed = world_get_speed(world, e);
  if (speed) destroy_speed(world, speed);

  ColliderComponent *collider = world_get_collider(world, e);
  if (collider) destroy_collider(world, collider);

  MassComponent *mass = world_get_mass(world, e);
  if (mass) destroy_mass(world, mass);

  LocomotionComponent *locomotion = world_get_locomotion(world, e);
  if (locomotion) destroy_locomotion(world, locomotion);

  JumpComponent *jump = world_get_jump(world, e);
  if (jump) destroy_jump(world, jump);
}


void world_destroy(World *world) {
  PositionComponent *p, *tmp1;
  HASH_ITER(hh, world->positions, p, tmp1) { destroy_position(world, p); }

  RotationComponent *r, *tmp2;
  HASH_ITER(hh, world->rotations, r, tmp2) { destroy_rotation(world, r); }

  ScaleComponent *s, *tmp3;
  HASH_ITER(hh, world->scales, s, tmp3) { destroy_scale(world, s); }

  MeshComponent *mesh, *tmp4;
  HASH_ITER(hh, world->meshes, mesh, tmp4) { destroy_mesh(world, mesh); }

  MaterialComponent *mat, *tmp5;
  HASH_ITER(hh, world->materials, mat, tmp5) { destroy_material(world, mat); }

  VelocityComponent *v, *tmp6;
  HASH_ITER(hh, world->velocities, v, tmp6) { destroy_velocity(world, v); }

  PathComponent *path, *tmp7;
  HASH_ITER(hh, world->paths, path, tmp7) { destroy_path(world, path); }

  SpeedComponent *speed, *tmp8;
  HASH_ITER(hh, world->speeds, speed, tmp8) { destroy_speed(world, speed); }

  ColliderComponent *collider, *tmp9;
  HASH_ITER(hh, world->colliders, collider, tmp9) { destroy_collider(world, collider); }

  MassComponent *mass, *tmp10;
  HASH_ITER(hh, world->masses, mass, tmp10) { destroy_mass(world, mass); }

  LocomotionComponent *locomotion, *tmp11;
  HASH_ITER(hh, world->locomotions, locomotion, tmp11) { destroy_locomotion(world, locomotion); }

  JumpComponent *jump, *tmp12;
  HASH_ITER(hh, world->jumps, jump, tmp12) { destroy_jump(world, jump); }

  world->next_id = 0;

  mesh_reg_destroy(&world->mesh_registry);
  mat_reg_destroy(&world->material_registry);
}
