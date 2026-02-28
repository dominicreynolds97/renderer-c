#include "App.h"
#include <stdio.h>

#include "rendering/Shader.h"
#include "scene/Scene.h"
#include "Input.h"

const char* APP_NAME = "Renderer";

void setup_cursor_callback(App *app, Scene *scene) {
  glfwSetWindowUserPointer(app->window, &scene->camera);
  glfwSetCursorPosCallback(app->window, mouse_callback);
}

void setup_window_hints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
}

void setup_depth() {
  glEnable(GL_DEPTH_TEST);
  glPolygonOffset(1.0f, 1.0f);
}

int app_create(App *app, int width, int height) {
  app->width = width;
  app->height = height;

  if (!glfwInit()) {
    printf("GLFW init failed\n");
    return 1;
  }

  setup_window_hints();

  GLFWwindow *window = glfwCreateWindow(width, height, APP_NAME, NULL, NULL);
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

  setup_depth();

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  app->window = window;
  app->shader = shader;

  return 0;
}

void app_run(App *app) {
  Scene scene;
  scene_create(&scene);

  setup_cursor_callback(app, &scene);

  float last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(app->window)) {
    float now = glfwGetTime();
    float delta_time = now - last_frame_time;
    last_frame_time = now;

    handle_input(app, &scene, delta_time);

    scene_render(&scene, app);
  }

  scene_destroy(&scene);
}

void app_destroy(App *app) {
  shader_free(app->shader);
  glfwTerminate();
}
