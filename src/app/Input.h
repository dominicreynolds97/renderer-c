#ifndef INPUT_H
#define INPUT_H

#include "scene/Scene.h"
#include "App.h"

void handle_input(App *app, Scene *scene, float dt);
void mouse_callback(GLFWwindow *window, double mx, double my);

#endif
