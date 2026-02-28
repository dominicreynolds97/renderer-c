#ifndef SCENE_H
#define SCENE_H

#include "app/App.h"
#include "camera.h"
#include "ecs/World.h"

typedef struct {
  Vec3f color;
} Skybox;

typedef struct {
  World world;
  Skybox skybox;
  Camera camera;
} Scene;

void scene_create(Scene *scene);
void scene_render(Scene *scene, App *app);
void scene_destroy(Scene *scene);

#endif
