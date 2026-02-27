#include "camera.h"
#include <math.h>

const Vec3f UP = {0.0f, 1.0f, 0.0f};

Camera init_camera() {
  return (Camera) {
    (Vec3f) {0.0f, 1.0f, 0.0f},
    0.0f,
    -0.3f,
    0.0f,
    0.0f,
    0
  };
}

Vec3f get_h_forward(Camera *camera) {
  return vec3f_normalize(
    (Vec3f){cosf(camera->yaw), 0.0f, sinf(camera->yaw)}
  );
}

Vec3f get_forward(Camera *camera) {
  return vec3f_normalize(
    (Vec3f){
      cosf(camera->yaw) * cosf(camera->pitch),
      sinf(camera->pitch),
      cosf(camera->pitch) * sinf(camera->yaw)
      }
  );
}

Vec3f get_right(Camera *camera) {
  return vec3f_cross(get_h_forward(camera), UP);
}

void move_forward(Camera *camera, float dt, float speed) {
  float distance = speed * dt;
  camera->pos = vec3f_add(
    camera->pos,
    vec3f_scale(get_h_forward(camera), distance)
  );
}

void move_back(Camera *camera, float dt, float speed) {
  move_forward(camera, -dt, speed);
}

void move_right(Camera *camera, float dt, float speed) {
  float distance = speed * dt;
  camera->pos = vec3f_add(
    camera->pos,
    vec3f_scale(get_right(camera), distance)
  );
}
 void move_left(Camera *camera, float dt, float speed) {
   move_right(camera, -dt, speed);
 }


Mat4 get_camera_view(Camera *camera) {
  Vec3f target = vec3f_add(camera->pos, get_forward(camera));

  Mat4 view = mat4_look_at(camera->pos, target, UP);

  return view;
}
