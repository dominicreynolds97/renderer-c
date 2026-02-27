#include "src/app/App.h"

#define WIDTH 1080
#define HEIGHT 720

int main(void) {
  App app;
  int app_status = app_create(&app, WIDTH, HEIGHT);

  if (app_status != 0) return app_status;

  app_run(&app);
  app_destroy(&app);
  return 0;
}
