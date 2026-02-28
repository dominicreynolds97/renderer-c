#include "World.h"
#include "scene/Registry.h"

void world_init(World *world) {
  world->next_id = 0;
  world->transforms = NULL;
  world->meshes = NULL;
  world->materials = NULL;

  mesh_reg_init(&world->mesh_registry);
  mat_reg_init(&world->material_registry);
}

Entity world_create_entity(World *world) {
  return world->next_id++;
}


void world_add_transform(World *world, Entity e, Mat4 transform) {
  TransformComponent *c = malloc(sizeof(TransformComponent));
  c->entity = e;
  c->transform = transform;
  HASH_ADD_INT(world->transforms, entity, c);
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


TransformComponent* world_get_transform(World *world, Entity e) {
  TransformComponent *c;
  HASH_FIND_INT(world->transforms, &e, c);
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


void destroy_transform(World *world, TransformComponent *c) {
  HASH_DEL(world->transforms, c);
  free(c);
}

void destroy_mesh(World *world, MeshComponent *c) {
  HASH_DEL(world->meshes, c);
  free(c);
}

void destroy_material(World *world, MaterialComponent *c) {
  HASH_DEL(world->materials, c);
  free(c);
}


void world_destroy_entity(World *world, Entity e) {
  TransformComponent *t = world_get_transform(world, e);
  if (t) destroy_transform(world, t);

  MeshComponent *mesh = world_get_mesh(world, e);
  if (mesh) destroy_mesh(world, mesh);

  MaterialComponent *mat = world_get_material(world, e);
  if (mat) destroy_material(world, mat);
}

void world_destroy(World *world) {
  TransformComponent *t, *tmp1;
  HASH_ITER(hh, world->transforms, t, tmp1) { destroy_transform(world, t); }

  MeshComponent *mesh, *tmp2;
  HASH_ITER(hh, world->meshes, mesh, tmp2) { destroy_mesh(world, mesh); }

  MaterialComponent *mat, *tmp3;
  HASH_ITER(hh, world->materials, mat, tmp3) { destroy_material(world, mat); }

  world->next_id = 0;

  mesh_reg_destroy(&world->mesh_registry);
  mat_reg_destroy(&world->material_registry);
}
