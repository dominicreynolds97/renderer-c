#ifndef SCENE_H
#define SCENE_H

#include "app/App.h"
#include "camera.h"
#include "ecs/World.h"

typedef struct {
  World world;
  Camera camera;
} Scene;

void scene_create(Scene *scene);
void scene_render(Scene *scene, App *app);
void scene_destroy(Scene *scene);

#endif
