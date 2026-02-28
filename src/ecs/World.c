#include "World.h"
#include "maths/Maths3D.h"

void world_init(World *world) {
  world->next_id    = 0;
  world->positions  = NULL;
  world->rotations  = NULL;
  world->scales     = NULL;
  world->meshes     = NULL;
  world->materials  = NULL;
  world->velocities = NULL;

  mesh_reg_init(&world->mesh_registry);
  mat_reg_init(&world->material_registry);
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

  world->next_id = 0;

  mesh_reg_destroy(&world->mesh_registry);
  mat_reg_destroy(&world->material_registry);
}
