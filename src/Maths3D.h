#ifndef MATHS3D_H
#define MATHS3D_H

typedef struct {
  int x;
  int y;
} Vec2i;

typedef struct {
  float x;
  float y;
} Vec2f;

typedef struct {
  float x;
  float y;
  float z;
} Vec3f;

typedef struct {
  float x;
  float y;
  float z;
  float w;
} Vec4f;

typedef struct {
  float m[4][4];
} Mat4;

Vec3f vec3f_sub(Vec3f a, Vec3f b);

Vec3f vec3f_cross(Vec3f a, Vec3f b);

float vec3f_dot(Vec3f a, Vec3f b);

float vec3f_length(Vec3f v);

Vec3f vec3f_normalize(Vec3f v);

Vec4f mat4_mul_vec4(Mat4 m, Vec4f v);

Mat4 mat4_mul(Mat4 a, Mat4 b);

Mat4 mat4_translation(float x, float y, float z);

Mat4 mat4_rotation_y(float angle);

Mat4 mat4_rotation_x(float angle);

Mat4 mat4_rotation_z(float angle);

Mat4 mat4_perspective(float fov, float aspect, float near, float far);

#endif
