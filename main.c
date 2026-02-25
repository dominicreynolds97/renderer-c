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

typedef struct {
  int x;
  int y;
} Vec2;

Vec2 vec2(int x, int y) {
  Vec2 v2 = { x, y };
  return v2;
}

Pixel rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void put_pixel(Pixel *fb, int x, int y, Pixel color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  fb[y * WIDTH + x] = color;
}

void draw_line(Pixel *fb, Vec2 v0, Vec2 v1, Pixel color) {
  int dx = abs(v1.x - v0.x);
  int dy = -abs(v1.y - v0.y);

  int sx = v0.x < v1.x ? 1 : -1;
  int sy = v0.y < v1.y ? 1 : -1;

  int err = dx + dy;

  int x = v0.x;
  int y = v0.y;

  while (1) {
    put_pixel(fb, x, y, color);
    if (x == v1.x && y == v1.y) break;

    int e2 = 2 * err;

    if (e2 >= dy) {
      err += dy;
      x += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y += sy;
    }
  }
}

void draw_rect(Pixel *fb, Vec2 v, int w, int h, Pixel color) {
  draw_line(fb, vec2(v.x, v.y),     vec2(v.x + w, v.y),     color); // top
  draw_line(fb, vec2(v.x, v.y + h), vec2(v.x + w, v.y + h), color); // bottom
  draw_line(fb, vec2(v.x, v.y),     vec2(v.x    , v.y + h), color); // left
  draw_line(fb, vec2(v.x + w, v.y), vec2(v.x + w, v.y + h), color); // right
}

void draw_rect_filled(Pixel *fb, Vec2 v, int w, int h, Pixel color) {
  for (int row = v.y; row <= v.y + h; row++) {
    for (int col = v.x; col <= v.x + w; col++) {
      put_pixel(fb, col, row, color);
    }
  }
}

void draw_circle(Pixel *fb, Vec2 c, int radius, Pixel color) {
  int x = 0;
  int y = radius;
  int d = 1 - radius;

  while (x <= y) {
    put_pixel(fb, c.x + x, c.y + y, color);
    put_pixel(fb, c.x - x, c.y + y, color);
    put_pixel(fb, c.x + x, c.y - y, color);
    put_pixel(fb, c.x - x, c.y - y, color);
    put_pixel(fb, c.x + y, c.y + x, color);
    put_pixel(fb, c.x - y, c.y + x, color);
    put_pixel(fb, c.x + y, c.y - x, color);
    put_pixel(fb, c.x - y, c.y - x, color);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
}

void draw_circle_filled(Pixel *fb, Vec2 c, int radius, Pixel color) {
  int x = 0;
  int y = radius;
  int d = 1 - radius;

  while (x <= y) {
    draw_line(fb, vec2(c.x - x, c.y + y), vec2(c.x + x, c.y + y), color);
    draw_line(fb, vec2(c.x - x, c.y - y), vec2(c.x + x, c.y - y), color);
    draw_line(fb, vec2(c.x - y, c.y + x), vec2(c.x + y, c.y + x), color);
    draw_line(fb, vec2(c.x - y, c.y - x), vec2(c.x + y, c.y - x), color);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
}

void draw_triangle(Pixel *fb, Vec2 p0, Vec2 p1, Vec2 p2, Pixel color) {
  draw_line(fb, p0, p1, color);
  draw_line(fb, p1, p2, color);
  draw_line(fb, p2, p0, color);
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

    draw_line(framebuffer, vec2(100, 100), vec2(600, 400), rgb(255, 0, 0));
    draw_rect(framebuffer, vec2(100, 50), 200, 150, rgb(0, 255, 0));
    draw_rect_filled(framebuffer, vec2(200, 100), 200, 150, rgb(0, 0, 255));

    draw_circle(framebuffer, vec2(700, 200), 50, rgb(0, 255, 255));
    draw_circle_filled(framebuffer, vec2(200, 300), 150, rgb(0, 255, 255));

    draw_triangle(framebuffer, vec2(640, 100), vec2(900, 600), vec2(380, 600), rgb(255,255,0));

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
