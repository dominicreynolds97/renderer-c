#include "SDL_events.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "src/Maths3D.h"

#define WIDTH 1080
#define HEIGHT 720

typedef uint32_t Pixel;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

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

void put_pixel(Pixel *fb, int x, int y, Pixel color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  fb[y * WIDTH + x] = color;
}

void put_pizel_z(Pixel *fb, float *zb, int x, int y, float z, Pixel color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  int idx = y * WIDTH + x;
  if (z >= zb[idx]) return;
  zb[idx] = z;
  fb[idx] = color;
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
  if (u < 0) u = 0; if (u > 1) u = 1;
  if (v < 0) v = 0; if (v > 1) v = 1;

  int x = (int)(u * (t->width - 1));
  int y = (int)(v * (t->height - 1));

  int idx = (y * t->width + x) * 4;
  return rgb(t->data[idx], t->data[idx +1], t->data[idx + 2]);
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


      float u = (bary.wa * uva.x / wa + bary.wb * uvb.x / wb + bary.wc * uvc.x / wc) / one_over_w;
      float v = (bary.wa * uva.y / wa + bary.wb * uvb.y / wb + bary.wc * uvc.y / wc) / one_over_w;

      put_pixel(fb, x, y, sample_texture(tex, u, v));
    }
  }
}

void clear(Pixel *fb, Pixel color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    fb[i] = color;
  }
}

typedef struct {
  Vec2i screen;
  float depth;
} ProjectedVertex;

ProjectedVertex project(Vec3f vertex, Mat4 mvp) {
  Vec4f v = mat4_mul_vec4(mvp, (Vec4f){vertex.x, vertex.y, vertex.z, 1.0f});

  float inv_w = 1.0f / v.w;
  float nx = v.x * inv_w;
  float ny = v.y * inv_w;
  float nz = v.z * inv_w;

  int sx = (int)((nx + 1.0f) * 0.5f * WIDTH);
  int sy = (int)((1.0f - ny) * 0.5f * HEIGHT);

  return (ProjectedVertex){(Vec2i){sx, sy}, nz};
}

void draw_triangle_3d(Pixel *fb, float *zb,
    ProjectedVertex a, ProjectedVertex b, ProjectedVertex c,
    Color ca, Color cb, Color cc) {
  Bounds bounds = get_triangle_bounds(a.screen, b.screen, c.screen);

  for (int y = bounds.min.y; y <= bounds.max.y; y++) {
    for (int x = bounds.min.x; x <= bounds.max.x; x++) {
      Barycentric bary = barycentric(a.screen, b.screen, c.screen, (Vec2i){x,y});
      if (!barycentric_inside(bary)) continue;

      float z = bary.wa * a.depth + bary.wb * b.depth + bary.wc * c.depth;

      uint8_t r = (uint8_t)(bary.wa * ca.r + bary.wb * cb.r + bary.wc * cc.r);
      uint8_t g = (uint8_t)(bary.wa * ca.g + bary.wb * cb.g + bary.wc * cc.g);
      uint8_t b = (uint8_t)(bary.wa * ca.b + bary.wb * cb.b + bary.wc * cc.b);

      put_pizel_z(fb, zb, x, y, z, rgb(r, g, b));
    }
  }
}


Vec3f cube_vertices[8] = {
  {-1, -1, -1}, { 1, -1, -1},
  { 1,  1, -1}, {-1,  1, -1},
  {-1, -1,  1}, { 1, -1,  1},
  { 1,  1,  1}, {-1,  1,  1}
};

int cube_faces[12][3] = {
  {0,1,2}, {0,2,3}, // back
  {4,6,5}, {4,7,6}, // front
  {0,4,5}, {0,5,1}, // bottom
  {2,6,7}, {2,7,3}, // top
  {0,3,7}, {0,7,4}, // left
  {1,5,6}, {1,6,2}, // right
};

Color face_colors[6] = {
    {255, 0,   0  },
    {0,   255, 0  },
    {0,   0,   255},
    {255, 255, 0  },
    {0,   255, 255},
    {255, 0,   255},
};

void draw_cube(Pixel *fb, float *zb) {
  float angle = SDL_GetTicks() / 1000.f;

  Mat4 model = mat4_mul(
    mat4_rotation_y(angle),
    mat4_rotation_x(angle * 0.5f)
  );
  Mat4 view = mat4_translation(0, 0, -5.0f);
  Mat4 proj = mat4_perspective(1.0f, (float)WIDTH / HEIGHT, 0.1f, 100.0f);
  Mat4 mvp  = mat4_mul(proj, mat4_mul(view, model));

  for (int i = 0; i < 12; i++) {
    Vec3f v0 = cube_vertices[cube_faces[i][0]];
    Vec3f v1 = cube_vertices[cube_faces[i][1]];
    Vec3f v2 = cube_vertices[cube_faces[i][2]];

    ProjectedVertex p0 = project(v0, mvp);
    ProjectedVertex p1 = project(v1, mvp);
    ProjectedVertex p2 = project(v2, mvp);

    Color c = face_colors[i / 2];

    draw_triangle_3d(fb, zb, p0, p1, p2, c, c, c);
  }
}

void clear_zbuffer(float *zb) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    zb[i] = 1.0f;
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
  float zbuffer[WIDTH * HEIGHT];

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
    clear_zbuffer(zbuffer);

    draw_cube(framebuffer, zbuffer);

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
