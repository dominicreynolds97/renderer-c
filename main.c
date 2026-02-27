#include "src/app/App.h"

#define WIDTH 1080
#define HEIGHT 720

int main(void) {
  App app = app_create(WIDTH, HEIGHT);
  if (app_check_init_state(&app) == 0) return 1;
  app_run(&app);
  app_destroy(&app);
  return 0;
}
