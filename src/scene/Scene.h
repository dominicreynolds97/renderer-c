#ifndef SCENE_H
#define SCENE_H

#include "../app/App.h"
#include "../rendering/Renderer.h"
#include "camera.h"

#define MAX_MESHES 16
#define MAX_OBJECTS 64

typedef struct {
  int mesh_id;
  //Vec3f color;
  Mat4 transform;
} SceneObject;

typedef struct {
  int mesh_count;
  int object_count;
  RenderMesh meshes[MAX_MESHES];
  SceneObject objects[MAX_OBJECTS];
  Camera camera;
} Scene;

void scene_create(Scene *scene);
void scene_render(Scene *scene, App *app);
void scene_destroy(Scene *scene);

#endif
