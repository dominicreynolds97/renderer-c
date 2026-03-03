#include "Input.h"
#include "GLFW/glfw3.h"
#include "ecs/System.h"
#include "scene/camera.h"

const float SPEED = 5.0f;
const float SENSITIVIY = 0.01f;

void move_forward(World *world, float dt) {
  Vec3f dir = get_h_forward(&world->camera);
  apply_thrust(world, world->player.entity, dir, dt);
}

void move_back(World *world, float dt) {
  move_forward(world, -dt);
}

void move_right(World *world, float dt) {
  Vec3f dir = get_right(&world->camera);
  apply_thrust(world, world->player.entity, dir, dt);
}

 void move_left(World *world, float dt) {
   move_right(world, -dt);
 }

float clampf(float value, float max, float min) {
  if (value > max) return max;
  if (value < min) return min;
  return value;
}

void handle_input(App *app, Scene *scene, float dt) {
  glfwPollEvents();

  if (glfwGetKey(app->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(app->window, 1);
  if (glfwGetKey(app->window, GLFW_KEY_W) == GLFW_PRESS)
    move_forward(&scene->world, dt);
  if (glfwGetKey(app->window, GLFW_KEY_S) == GLFW_PRESS)
    move_back(&scene->world, dt);
  if (glfwGetKey(app->window, GLFW_KEY_A) == GLFW_PRESS)
    move_left(&scene->world, dt);
  if (glfwGetKey(app->window, GLFW_KEY_D) == GLFW_PRESS)
    move_right(&scene->world, dt);
  if (glfwGetKey(app->window, GLFW_KEY_SPACE) == GLFW_PRESS)
    jump(&scene->world, scene->world.player.entity);
  if (glfwGetKey(app->window, GLFW_KEY_G) == GLFW_PRESS)
    scene->grid.visible = !scene->grid.visible;
}

void mouse_callback(GLFWwindow *window, double mx, double my) {
  Camera *camera = glfwGetWindowUserPointer(window);

  if (!camera->initialized) {
    camera->last_mx = mx;
    camera->last_my = my;
    camera->initialized = 1;
    return;
  }

  float dx = mx - camera->last_mx;
  float dy = camera->last_my - my;

  camera->yaw += dx * SENSITIVIY;
  camera->pitch += dy * SENSITIVIY;
  camera->pitch = clampf(camera->pitch, 1.5707f, -1.5707f);

  camera->last_mx = mx;
  camera->last_my = my;
}
