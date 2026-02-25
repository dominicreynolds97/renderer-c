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
} Vec2i;

Pixel rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void put_pixel(Pixel *fb, int x, int y, Pixel color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  fb[y * WIDTH + x] = color;
}

void draw_line(Pixel *fb, Vec2i v0, Vec2i v1, Pixel color) {
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

void draw_rect(Pixel *fb, Vec2i v, int w, int h, Pixel color) {
  draw_line(fb, (Vec2i){v.x, v.y},     (Vec2i){v.x + w, v.y},     color); // top
  draw_line(fb, (Vec2i){v.x, v.y + h}, (Vec2i){v.x + w, v.y + h}, color); // bottom
  draw_line(fb, (Vec2i){v.x, v.y},     (Vec2i){v.x    , v.y + h}, color); // left
  draw_line(fb, (Vec2i){v.x + w, v.y}, (Vec2i){v.x + w, v.y + h}, color); // right
}

void draw_rect_filled(Pixel *fb, Vec2i v, int w, int h, Pixel color) {
  for (int row = v.y; row <= v.y + h; row++) {
    for (int col = v.x; col <= v.x + w; col++) {
      put_pixel(fb, col, row, color);
    }
  }
}

void draw_circle(Pixel *fb, Vec2i c, int radius, Pixel color) {
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

void draw_circle_filled(Pixel *fb, Vec2i c, int radius, Pixel color) {
  int x = 0;
  int y = radius;
  int d = 1 - radius;

  while (x <= y) {
    draw_line(fb, (Vec2i){c.x - x, c.y + y}, (Vec2i){c.x + x, c.y + y}, color);
    draw_line(fb, (Vec2i){c.x - x, c.y - y}, (Vec2i){c.x + x, c.y - y}, color);
    draw_line(fb, (Vec2i){c.x - y, c.y + x}, (Vec2i){c.x + y, c.y + x}, color);
    draw_line(fb, (Vec2i){c.x - y, c.y - x}, (Vec2i){c.x + y, c.y - x}, color);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
}

void draw_triangle(Pixel *fb, Vec2i p0, Vec2i p1, Vec2i p2, Pixel color) {
  draw_line(fb, p0, p1, color);
  draw_line(fb, p1, p2, color);
  draw_line(fb, p2, p0, color);
}

void sort_vertices(Vec2i *a, Vec2i *b, Vec2i *c) {
  int tx, ty;
  if (b->y < a->y) {
    tx = a->x;
    ty = a->y;
    a->x = b->x;
    a->y = b->y;
    b->x=tx;
    b->y=ty;
  }
  if (c->y < a->y) {
    tx = a->x;
    ty = a->y;
    a->x = c->x;
    a->y = c->y;
    b->x=tx;
    b->y=ty;
  }
  if (c->y < b->y) {
    tx = b->x;
    ty = b->y;
    b->x = c->x;
    b->y = c->y;
    c->x=tx;
    c->y=ty;
  }
}

void fill_flat_bottom(Pixel *fb, Vec2i a, Vec2i b, Vec2i c, Pixel color) {
  float slope_left = (float)(b.x - a.x) / (b.y - a.y);
  float slope_right = (float)(c.x - a.x) / (c.y - a.y);

  float xl = a.x;
  float xr = a.x;

  for (int y = a.y; y <= b.y; y++) {
    draw_line(fb,
      (Vec2i){(int)xl, y},
      (Vec2i){(int)xr, y},
      color
    );

    xl += slope_left;
    xr += slope_right;
  }
}

void fill_flat_top(Pixel *fb, Vec2i a, Vec2i b, Vec2i c, Pixel color) {
  float slope_left = (float)(c.x - a.x) / (c.y - a.y);
  float slope_right = (float)(c.x - b.x) / (c.y - b.y);

  float xl = a.x;
  float xr = b.x;

  for (int y = a.y; y <= b.y; y++) {
    draw_line(fb,
      (Vec2i){(int)xl, y},
      (Vec2i){(int)xr, y},
      color
    );

    xl += slope_left;
    xr += slope_right;
  }
}

void draw_triangle_filled(Pixel *fb, Vec2i a, Vec2i b, Vec2i c, Pixel color) {
  sort_vertices(&a, &b, &c);

  if (b.y == c.y) {
    fill_flat_bottom(fb, a, b, c, color);
  } else if (a.y == c.y) {
    fill_flat_top(fb, a, b, c, color);
  } else {
    Vec2i d;
    d.x = a.x + (int)((float)(b.y - a.y) / (c.y - a.y) * (c.x - a.x));
    d.y = b.y;

    fill_flat_bottom(fb, a, b, d, color);
    fill_flat_top(fb, a, d, c, color);
  }
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

    draw_line(framebuffer,
      (Vec2i){100, 100},
      (Vec2i){600, 400},
      rgb(255, 0, 0)
    );

    draw_rect(framebuffer,
      (Vec2i){100, 50},
      200,
      150,
      rgb(0, 255, 0)
    );

    draw_rect_filled(framebuffer,
      (Vec2i){200, 100},
      200,
      150,
      rgb(0, 0, 255)
    );

    draw_circle(framebuffer,
      (Vec2i){700, 200},
      50,
      rgb(0, 255, 255)
    );

    draw_circle_filled(framebuffer,
      (Vec2i){200, 300},
      150,
      rgb(0, 255, 255)
    );

    draw_triangle(framebuffer,
      (Vec2i){640, 100},
      (Vec2i){900, 600},
      (Vec2i){380, 600},
      rgb(255,255,0)
    );

    draw_triangle_filled(framebuffer,
      (Vec2i){640, 100},
      (Vec2i){900, 600},
      (Vec2i){380, 600},
      rgb(255, 0, 0)
    );

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
