#include "SDL_events.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>

#define WIDTH 1080
#define HEIGHT 720

typedef uint32_t Pixel;

Pixel rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFF << 24) | (r << 16) | (g << 0) | b;
}

void clear(Pixel *fb, Pixel color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    fb[i] = color;
  }
}

int main(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
    "Renderer",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WIDTH, HEIGHT, 0
  );

  SDL_Renderer *sdl_renderer = SDL_CreateRenderer(
    window, -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );

  SDL_Texture *texture = SDL_CreateTexture(
    sdl_renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    WIDTH, HEIGHT
  );

  Pixel framebuffer[WIDTH * HEIGHT];

  int running = 1;
  while (running) {

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = 0;
      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
          case (SDLK_ESCAPE):
            running = 0;
            break;
        }
      }
    }

    clear(framebuffer, rgb(30, 30, 30));

    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(Pixel));
    SDL_RenderCopy(sdl_renderer, texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
