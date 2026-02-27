#include "App.h"
#include <stdio.h>

#include "../rendering/Shader.h"
#include "../scene/Scene.h"
#include "Input.h"

App app_create(int width, int height) {
  App app = {0};
  app.width = width;
  app.height = height;

  if (!glfwInit()) {
    printf("GLFW init failed\n");
    return app;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);

  GLFWwindow *window = glfwCreateWindow(width, height, "Renderer", NULL, NULL);
  if (!window) {
    printf("Window creation failed\n");
    glfwTerminate();
    return app;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwFocusWindow(window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    printf("GLEW init failed\n");
    glfwTerminate();
    return app;
  }

  printf("OpenGL: %s\n", glGetString(GL_VERSION));

  GLuint shader = shader_load("shaders/mesh.vert", "shaders/mesh.frag");
  glEnable(GL_DEPTH_TEST);
  glPolygonOffset(1.0f, 1.0f);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  app.window = window;
  app.shader = shader;

  return app;
}

int app_check_init_state(App *app) {
  if (app->shader == 0 || app->window == 0) return 0;
  return 1;
}

void app_run(App *app) {
  Scene scene;
  scene_create(&scene);

  glfwSetWindowUserPointer(app->window, &scene.camera);
  glfwSetCursorPosCallback(app->window, mouse_callback);

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
