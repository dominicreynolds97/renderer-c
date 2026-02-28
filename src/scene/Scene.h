#ifndef SCENE_H
#define SCENE_H

#include "app/App.h"
#include "Registry.h"
#include "camera.h"

#define MAX_OBJECTS 64

typedef struct {
  int mesh_id;
  int mat_id;
  Mat4 transform;
} SceneObject;

typedef struct {
  int object_count;
  MeshRegistry mesh_registry;
  MaterialRegistry material_registry;
  SceneObject objects[MAX_OBJECTS];
  Camera camera;
} Scene;

void scene_create(Scene *scene);
void scene_render(Scene *scene, App *app);
void scene_destroy(Scene *scene);

#endif
