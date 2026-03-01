#include "Input.h"

const float SPEED = 5.0f;
const float SENSITIVIY = 0.01f;

static void update_player_pos(World *world, Vec3f new_pos) {
  PositionComponent *player_pos = world_get_position(world, world->player.entity);
  player_pos->position = new_pos;
}

void move_forward(World *world, float dt, float speed) {
  float distance = speed * dt;
  Vec3f new_pos = vec3f_add(
    world->camera.pos,
    vec3f_scale(get_h_forward(&world->camera), distance)
  );
  update_player_pos(world, new_pos);
}

void move_back(World *world, float dt, float speed) {
  move_forward(world, -dt, speed);
}

void move_right(World *world, float dt, float speed) {
  float distance = speed * dt;
  Vec3f new_pos = vec3f_add(
    world->camera.pos,
    vec3f_scale(get_right(&world->camera), distance)
  );
  update_player_pos(world, new_pos);
}

 void move_left(World *world, float dt, float speed) {
   move_right(world, -dt, speed);
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
    move_forward(&scene->world, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_S) == GLFW_PRESS)
    move_back(&scene->world, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_A) == GLFW_PRESS)
    move_left(&scene->world, dt, SPEED);
  if (glfwGetKey(app->window, GLFW_KEY_D) == GLFW_PRESS)
    move_right(&scene->world, dt, SPEED);
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
