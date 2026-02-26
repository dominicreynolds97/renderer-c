#include <math.h>
#include "Maths3D.h"

Vec3f vec3f_sub(Vec3f a, Vec3f b) {
  return (Vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3f vec3f_cross(Vec3f a, Vec3f b) {
  return (Vec3f) {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

float vec3f_dot(Vec3f a, Vec3f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3f_length(Vec3f v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3f vec3f_normalize(Vec3f v) {
  float len = vec3f_length(v);
  return (Vec3f){v.x / len, v.y / len, v.z / len};
}

Vec3f compute_face_normal(Vec3f a, Vec3f b, Vec3f c) {
  Vec3f edge1 = vec3f_sub(b, a);
  Vec3f edge2 = vec3f_sub(c, a);
  return vec3f_normalize(vec3f_cross(edge1, edge2));
}

Mat4 mat4_identity(void) {
  Mat4 m = {0};
  m.m[0][0] = 1.0f;
  m.m[1][1] = 1.0f;
  m.m[2][2] = 1.0f;
  m.m[3][3] = 1.0f;
  return m;
}

Mat4 mat4_mul(Mat4 a, Mat4 b) {
  Mat4 result = {0};
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      for (int k = 0; k < 4; k++) {
        result.m[row][col] += a.m[row][k] * b.m[k][col];
      }
    }
  }
  return result;
}

Vec4f mat4_mul_vec4(Mat4 m, Vec4f v) {
  return (Vec4f){
    m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w,
    m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w,
    m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w,
    m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w,
  };
}

Mat4 mat4_translation(float x, float y, float z) {
  Mat4 m = mat4_identity();
  m.m[0][3] = x;
  m.m[1][3] = y;
  m.m[2][3] = z;
  return m;
}

Mat4 mat4_scale(float x, float y, float z) {
  Mat4 m = mat4_identity();
  m.m[0][0] = x;
  m.m[1][1] = y;
  m.m[2][2] = z;
  return m;
}

Mat4 mat4_rotation_y(float angle) {
  Mat4 m = mat4_identity();
  m.m[0][0] = cosf(angle);
  m.m[0][2] = sinf(angle);
  m.m[2][0] = -sinf(angle);
  m.m[2][2] = cosf(angle);
  return m;
}

Mat4 mat4_rotation_x(float angle) {
  Mat4 m = mat4_identity();
  m.m[1][1] = cosf(angle);
  m.m[1][2] = sinf(angle);
  m.m[2][1] = -sinf(angle);
  m.m[2][2] = cosf(angle);
  return m;
}

Mat4 mat4_rotation_z(float angle) {
  Mat4 m = mat4_identity();
  m.m[0][0] = cosf(angle);
  m.m[0][1] = sinf(angle);
  m.m[1][0] = -sinf(angle);
  m.m[1][1] = cosf(angle);
  return m;
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
  float f = 1.0f / tanf(fov * 0.5f);
  Mat4 m = {0};
  m.m[0][0] = f / aspect;
  m.m[1][1] = f;
  m.m[2][2] = (far + near) / (near - far);
  m.m[2][3] = (2.0f * far * near) / (near - far);
  m.m[3][2] = -1.0f;
  return m;
}

int is_backface(Vec3f normal, Vec3f a, Vec3f camera_pos) {
  Vec3f view_dir = vec3f_sub(a, camera_pos);
  return vec3f_dot(normal, view_dir) >= 0;
}
