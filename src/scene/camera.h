#ifndef CAMERA_H
#define CAMERA_H

#include "maths/Maths3D.h"

extern const Vec3f UP;

typedef struct {
  Vec3f pos;
  float pitch;
  float yaw;
  float last_mx;
  float last_my;
  int   initialized;
} Camera;

Camera init_camera();
Vec3f get_h_forward(Camera *camera);
Vec3f get_forward(Camera *camera);
Vec3f get_right(Camera *camera);
Mat4 get_camera_view(Camera *camera);

#endif
