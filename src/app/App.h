#ifndef APP_H
#define APP_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gltypes.h>

typedef struct {
  GLFWwindow  *window;
  GLuint      shader;
  int         width;
  int         height;
} App;

int app_create(App *app, int width, int height);
void app_run(App *app);
void app_destroy(App *app);

#endif
