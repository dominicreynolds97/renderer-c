#include "SDL_events.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH 1080
#define HEIGHT 720

typedef uint32_t Pixel;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

typedef struct {
  int x;
  int y;
} Vec2i;

typedef struct {
  float x;
  float y;
} Vec2f;

typedef struct {
  float x;
  float y;
  float z;
} Vec3f;

Vec3f vec3f_sub(Vec3f a, Vec3f b) {
  return (Vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3f vec3f_cross(Vec3f a, Vec3f b) {
  return (Vec3f) {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

float vec3f_dot(Vec3f a, Vec3f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3f_length(Vec3f v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3f vec3f_normalize(Vec3f v) {
  float len = vec3f_length(v);
  return (Vec3f){v.x / len, v.y / len, v.z / len};
}

float compute_light(Vec3f normal, Vec3f light_dir) {
  float intensity = vec3f_dot(normal, light_dir);
  if (intensity < 0) intensity = 0;
  return intensity;
}

typedef struct {
  float wa;
  float wb;
  float wc;
} Barycentric;

Barycentric barycentric(Vec2i a, Vec2i b, Vec2i c, Vec2i p) {
  float wa = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) /
    (float)((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));

  float wb = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) /
    (float)((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));

  float wc = 1.0f - wa - wb;

  return (Barycentric){wa, wb, wc};
}

int barycentric_inside(Barycentric b) {
  return b.wa >= 0 && b.wb >= 0 && b.wc >= 0;
}

Pixel rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

Pixel color_to_pixel(Color c) {
  return rgb(c.r, c.g, c.b);
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

typedef struct {
  Vec2i max;
  Vec2i min;
} Bounds;

Bounds get_triangle_bounds(Vec2i a, Vec2i b, Vec2i c) {
  int min_x = a.x < b.x
    ? (a.x < c.x ? a.x : c.x)
    : (b.x < c.x ? b.x : c.x );
  int max_x = a.x > b.x
    ? (a.x > c.x ? a.x : c.x)
    : (b.x > c.x ? b.x : c.x );
  int min_y = a.y < b.y
    ? (a.y < c.y ? a.y : c.y)
    : (b.y < c.y ? b.y : c.y );
  int max_y = a.y > b.y
    ? (a.y > c.y ? a.y : c.y)
    : (b.y > c.y ? b.y : c.y );

  return (Bounds){(Vec2i){max_x, max_y},(Vec2i){min_x, min_y}};
}

void draw_triangle_colored(Pixel *fb, Vec2i a, Vec2i b, Vec2i c, Color ca, Color cb, Color cc) {
  Bounds bounds = get_triangle_bounds(a, b, c);

  for (int y = bounds.min.y; y <= bounds.max.y; y++) {
    for (int x = bounds.min.x; x <= bounds.max.x; x++) {
      Barycentric bary = barycentric(a, b, c, (Vec2i){x,y});
      if (!barycentric_inside(bary)) continue;

      uint8_t r = (uint8_t)(bary.wa * ca.r + bary.wb * cb.r + bary.wc * cc.r);
      uint8_t g = (uint8_t)(bary.wa * ca.g + bary.wb * cb.g + bary.wc * cc.g);
      uint8_t b = (uint8_t)(bary.wa * ca.b + bary.wb * cb.b + bary.wc * cc.b);

      put_pixel(fb, x, y, rgb(r, g, b));
    }
  }
}

typedef struct {
  uint8_t *data;
  int width;
  int height;
  int channels;
} Texture;

Texture load_texture(const char *path) {
  Texture t;
  t.data = stbi_load(path, &t.width, &t.height, &t.channels, 4);
  if (!t.data) {
    printf("Failed to load texture: %s\n", path);
  }
  return t;
}

void free_texture(Texture *t) {
  stbi_image_free(t->data);
}

Pixel sample_texture(Texture *t, float u, float v) {
  if (v < 0) u = 0; if (u > 1) u = 1;
  if (v < 0) v = 0; if (v > 1) v = 1;

  int x = (int)(u * (t->width - 1));
  int y = (int)(v * (t->height - 1));

  int idx = (y * t->width + x) * 4;
  return rgb(t->data[idx], t->data[idx +1], t->data[idx + 2]);
}

void draw_triangle_shaded(Pixel *fb, Vec2i a, Vec2i b, Vec2i c,
    Color base_color, Vec3f light_dir) {
  Vec3f normal = {0.0f, 0.0f, 1.0f};
  float intensity = compute_light(normal, light_dir);

  Color shaded = {
    (uint8_t)(base_color.r * intensity),
    (uint8_t)(base_color.g * intensity),
    (uint8_t)(base_color.b * intensity)
  };

  draw_triangle_colored(fb, a, b, c, shaded, shaded, shaded);
}

void draw_triangle_textured(Pixel *fb, Vec2i a, Vec2i b, Vec2i c,
    Vec2f uva, Vec2f uvb, Vec2f uvc,
    Texture *tex) {
  Bounds bounds = get_triangle_bounds(a, b, c);

  for (int y = bounds.min.y; y <= bounds.max.y; y++) {
    for (int x = bounds.min.x; x <= bounds.max.x; x++) {
      Barycentric bary = barycentric(a, b, c, (Vec2i){x,y});
      if (!barycentric_inside(bary)) continue;

      float u = bary.wa * uva.x + bary.wb * uvb.x + bary.wc * uvc.x;
      float v = bary.wa * uva.y + bary.wb * uvb.y + bary.wc * uvc.y;

      put_pixel(fb, x, y, sample_texture(tex, u, v));
    }
  }
}

void draw_triangle_textured_perspective(Pixel *fb, Vec2i a, Vec2i b, Vec2i c,
    Vec2f uva, Vec2f uvb, Vec2f uvc,
    float wa, float wb, float wc,
    Texture *tex) {
  Bounds bounds = get_triangle_bounds(a, b, c);
  for (int y = bounds.min.y; y <= bounds.max.y; y++) {
    for (int x = bounds.min.x; x <= bounds.max.x; x++) {
      Barycentric bary = barycentric(a, b, c, (Vec2i){x,y});
      if (!barycentric_inside(bary)) continue;

      float one_over_w = bary.wa / wa + bary.wb / wb + bary.wc / wc;


      float u = (bary.wa * uva.x + bary.wb * uvb.x + bary.wc * uvc.x) / one_over_w;
      float v = (bary.wa * uva.y + bary.wb * uvb.y + bary.wc * uvc.y) / one_over_w;

      put_pixel(fb, x, y, sample_texture(tex, u, v));
    }
  }
}

void draw_quad_textured(Pixel *fb,
    Vec2i a, Vec2i b, Vec2i c, Vec2i d,
    Vec2f uva, Vec2f uvb, Vec2f uvc, Vec2f uvd,
    Texture *tex) {
  draw_triangle_textured(fb, a, b, c, uva, uvb, uvc, tex);
  draw_triangle_textured(fb, b, d, c, uvb, uvd, uvc, tex);
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

  SDL_RaiseWindow(window);

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
  Texture tex = load_texture("Sprite-0001.png");

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
    draw_triangle_colored(framebuffer,
      (Vec2i){640, 100},
      (Vec2i){900, 600},
      (Vec2i){380, 300},
      (Color){255, 0,   0},
      (Color){0,   255, 0},
      (Color){0,   0,   255}
    );


    draw_triangle_textured(framebuffer,
      (Vec2i){640, 100},
      (Vec2i){1100, 600},
      (Vec2i){200, 600},
      (Vec2f){0.5f, 0.0f},
      (Vec2f){1.0f, 1.0f},
      (Vec2f){0.0f, 1.0f},
      &tex
    );

    draw_quad_textured(framebuffer,
      (Vec2i){200, 100}, (Vec2i){800, 100},
      (Vec2i){100, 400}, (Vec2i){800, 500},
      (Vec2f){0.0f, 0.0f}, (Vec2f){1.0f, 0.0f},
      (Vec2f){0.0f, 1.0f}, (Vec2f){1.0f, 1.0f},
      &tex
    );

    Vec3f light = vec3f_normalize((Vec3f){-1.0f, 1.0f, 1.0f}); // light from top-right
    draw_triangle_shaded(framebuffer,
      (Vec2i){640, 100}, (Vec2i){900, 600}, (Vec2i){380, 600},
      (Color){255, 100, 50},
      light
    );

    draw_triangle_textured_perspective(framebuffer,
      (Vec2i){200, 100}, (Vec2i){800, 100}, (Vec2i){500, 600},
      (Vec2f){0.0f, 0.0f}, (Vec2f){1.0f, 0.0f}, (Vec2f){0.5f, 1.0f},
      1.0f, 1.0f, 1.0f,
      &tex
    );

    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(Pixel));
    SDL_RenderCopy(sdl_renderer, texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
  }

  free_texture(&tex);

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
