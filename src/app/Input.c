#include "Input.h"

const float SPEED = 5.0f;
const float SENSITIVIY = 0.01f;

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
    move_forward(&scene->camera, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_S) == GLFW_PRESS)
    move_back(&scene->camera, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_A) == GLFW_PRESS)
    move_left(&scene->camera, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_D) == GLFW_PRESS)
    move_right(&scene->camera, dt, SPEED);
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
