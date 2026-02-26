#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <stdio.h>

#include "src/Mesh.h"

#define WIDTH 1080
#define HEIGHT 720

int main(void) {
  if (!glfwInit()) {
    printf("GLFW init failed\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


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

  Mesh mesh = load_obj("house.obj");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, 1);
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(window);
  }

  free_mesh(&mesh);
  glfwTerminate();

  return 0;
}
