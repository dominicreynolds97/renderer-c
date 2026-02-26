#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <math.h>
#include <stdio.h>

#include "src/Maths3D.h"
#include "src/Mesh.h"
#include "src/Renderer.h"
#include "src/Shader.h"

#define WIDTH 1080
#define HEIGHT 720

#define CUBE 0
#define SPHERE 1
#define CYLINDER 2

const float SPEED = 5.0f;
const float SENSITIVIY = 0.01f;

const Vec3f UP = {0.0f, 1.0f, 0.0f};

typedef struct {
  int mesh_id;
  //Vec3f color;
  Mat4 transform;
} SceneObject;

typedef struct {
  Vec3f pos;
  float pitch;
  float yaw;
  float last_mx;
  float last_my;
  int   initialized;
} Camera;

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

void move_forward(Camera *camera, float dt) {
  float distance = SPEED * dt;
  camera->pos = vec3f_add(
    camera->pos,
    vec3f_scale(get_h_forward(camera), distance)
  );
}

void move_back(Camera *camera, float dt) {
  move_forward(camera, -dt);
}

void move_right(Camera *camera, float dt) {
  float distance = SPEED * dt;
  camera->pos = vec3f_add(
    camera->pos,
    vec3f_scale(get_right(camera), distance)
  );
}
 void move_left(Camera *camera, float dt) {
   move_right(camera, -dt);
 }

void handle_input(GLFWwindow *window, Camera *camera, float dt) {
  glfwPollEvents();

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    move_forward(camera, dt);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    move_back(camera, dt);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    move_left(camera, dt);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    move_right(camera, dt);
}

void render(GLFWwindow *window, GLuint shader, Camera *camera, RenderMesh meshes[], SceneObject scene_objects[], int object_count) {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Mat4 model = mat4_scale(0.3f, 0.3f, 0.3f);

  Vec3f target = vec3f_add(camera->pos, get_forward(camera));

  Mat4 view = mat4_look_at(camera->pos, target, UP);
  Mat4 proj = mat4_perspective(1.0f, (float)WIDTH / HEIGHT, 0.1f, 100.0f);

  shader_use(shader);
  shader_set_float(shader, "u_light_dir", 0.0f);

  glUniform3f(glGetUniformLocation(shader, "u_light_dir"), 0.3f, 1.0f, 0.7f);
  glUniform3f(glGetUniformLocation(shader, "u_color"), 0.8f, 0.8f, 0.8f);

  for (int i = 0; i < object_count; i++) {
    SceneObject obj = scene_objects[i];
    RenderMesh mesh = meshes[obj.mesh_id];

    Mat4 mvp = mat4_mul(proj, mat4_mul(view, obj.transform));
    shader_set_mat4(shader, "u_mvp", &mvp.m[0][0]);
    shader_set_mat4(shader, "u_model", &model.m[0][0]);

    renderer_draw(&mesh, shader);
  }

  glfwSwapBuffers(window);
}

float clampf(float value, float max, float min) {
  if (value > max) return max;
  if (value < min) return min;
  return value;
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

void game_loop(GLFWwindow *window, GLuint shader) {
  Mesh cube = load_obj("meshes/cube.obj");
  Mesh sphere = load_obj("meshes/sphere.obj");
  Mesh cylinder = load_obj("meshes/cylinder.obj");

  RenderMesh meshes[] = {
    renderer_upload_mesh(&cube),
    renderer_upload_mesh(&sphere),
    renderer_upload_mesh(&cylinder)
  };

  free_mesh(&cube);
  free_mesh(&sphere);
  free_mesh(&cylinder);

  SceneObject scene_objects[] = {
    (SceneObject){CUBE, mat4_translation(-2.0f, -1.0f, 0.0f)},
    (SceneObject){CYLINDER, mat4_translation(0.0f, -1.0f, -2.0f)},
  };

  int object_count = (int)(sizeof(scene_objects) / sizeof(scene_objects[0]));

  Camera camera = init_camera();
  glfwSetWindowUserPointer(window, &camera);
  glfwSetCursorPosCallback(window, mouse_callback);

  float last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    float now = glfwGetTime();
    float delta_time = now - last_frame_time;
    last_frame_time = now;

    handle_input(window, &camera, delta_time);

    render(window, shader, &camera, meshes, scene_objects, object_count);
  }

  for (unsigned long i = 0; i < sizeof(meshes) / sizeof(RenderMesh); i++) {
    renderer_free(&meshes[i]);
  }
}

int main(void) {
  if (!glfwInit()) {
    printf("GLFW init failed\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Renderer", NULL, NULL);
  if (!window) {
    printf("Window creation failed\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwFocusWindow(window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    printf("GLEW init failed\n");
    glfwTerminate();
    return 1;
  }

  printf("OpenGL: %s\n", glGetString(GL_VERSION));

  GLuint shader = shader_load("shaders/mesh.vert", "shaders/mesh.frag");
  glEnable(GL_DEPTH_TEST);
  glPolygonOffset(1.0f, 1.0f);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  game_loop(window, shader);

  shader_free(shader);
  glfwTerminate();

  return 0;
}
