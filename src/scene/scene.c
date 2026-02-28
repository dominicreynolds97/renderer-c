#include "Scene.h"

#include "../assets/Mesh.h"
#include "../rendering/Shader.h"
#include "Registry.h"
#include "camera.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LINE_MAX_LENGTH 128
#define IDLE 0
#define IN_MATERIAL 1
#define IN_OBJECT 2

void trimString(char *str) {
  int start = 0, end = strlen(str) -1;
  while (isspace(str[start])) start++;
  while (end > start && isspace(str[end])) end--;

  if (start > 0 || end < ((int)strlen(str) - 1)) {
    memmove(str, str + start, end - start + 1);
    str[end - start + 1] = '\0';
  }
}

int find_id(char names[][64], int count, char *name) {
  for (int i = 0; i < count; i++) {
    if (strncmp(names[i], name, 63) == 0) return i;
  }
  return -1;
}

int parse_scene_file(Scene *scene, char* filepath) {
  scene->camera = init_camera();
  world_init(&scene->world);

  FILE *file = fopen(filepath, "r");
  if (!file) {
    printf("Failed to open scene file: %s\n", filepath);
    return 1;
  }

  char line[128];
  int state = IDLE;
  char mesh_names[MAX_MESHES][64];
  char material_names[MAX_MATERIALS][64];
  char current_mesh_name[64];
  char current_mat_name[64];

  Material current_material = {0};

  Vec3f translate = vec3f_identity();
  Vec3f rotation = vec3f_identity();
  Vec3f scale = {1.0f, 1.0f, 1.0f};

  while (fgets(line, sizeof(line), file)) {
    trimString(line);

    if (line[0] == '\n' || line[0] == '\0') continue;
    if (line[0] == '/' && line[1] == '/') continue;

    switch (state) {
      case(IDLE):
        if (strncmp(line, "mesh", 4) == 0) {
          char name[64], path[256];
          sscanf(line, "mesh %s %s", name, path);

          strncpy(mesh_names[scene->world.mesh_registry.count], name, 63);
          mesh_names[scene->world.mesh_registry.count][63] = '\0';

          Mesh mesh = load_obj(path);
          RenderMesh rm = renderer_upload_mesh(&mesh);
          mesh_reg_add(&scene->world.mesh_registry, rm);

          free_mesh(&mesh);
        }
        if (strncmp(line, "material", 8) == 0) {
          state = IN_MATERIAL;

          char name[64];
          sscanf(line, "material %s", name);

          strncpy(material_names[scene->world.material_registry.count], name, 63);
          material_names[scene->world.material_registry.count][63] = '\0';
        }
        if (strncmp(line, "object", 6) == 0) {
          state = IN_OBJECT;

          sscanf(line, "object %s %s", current_mesh_name, current_mat_name);
        }
        break;
      case(IN_MATERIAL):
        if (strncmp(line, "color", 5) == 0) {
          Vec3f color;
          sscanf(line, "color %f %f %f", &color.x, &color.y, &color.z);
          current_material.color = color;
        }
        if (strncmp(line, "end", 3) == 0) {
          mat_reg_add(&scene->world.material_registry, current_material);
          current_material = (Material){0};

          state = IDLE;
        }
        break;
      case(IN_OBJECT):
        if (strncmp(line, "translate", 9) == 0) {
          Vec3f t;
          sscanf(line, "translate %f %f %f", &t.x, &t.y, &t.z);

          translate = vec3f_add(translate, t);
        }
        if (strncmp(line, "rotation", 8) == 0) {
          Vec3f r;
          sscanf(line, "rotation %f %f %f", &r.x, &r.y, &r.z);
          rotation = vec3f_add(rotation, r);
        }
        if (strncmp(line, "scale", 5) == 0) {
          Vec3f s;
          sscanf(line, "scale %f %f %f", &s.x, &s.y, &s.z);
          scale = vec3f_add(scale, s);
        }
        if (strncmp(line, "end", 3) == 0) {
          int mesh_id = find_id(mesh_names, scene->world.mesh_registry.count, current_mesh_name);
          int mat_id = find_id(material_names, scene->world.material_registry.count, current_mat_name);
          Mat4 transform = mat4_mul(
            mat4_translation(translate.x, translate.y, translate.z),
            mat4_mul(
              mat4_rotation(rotation),
              mat4_scale(scale.x, scale.y, scale.z)
            )
          );

          Entity e = world_create_entity(&scene->world);
          world_add_transform(&scene->world, e, transform);
          if (mesh_id >= 0) world_add_mesh(&scene->world, e, mesh_id);
          if (mat_id >= 0) world_add_material(&scene->world, e, mat_id);

          translate = vec3f_identity();
          rotation = vec3f_identity();
          scale = (Vec3f){1.0f, 1.0f, 1.0f};

          state = IDLE;
        }
        break;
    }
  }

  return 0;
}

void scene_create(Scene *scene) {
  parse_scene_file(scene, "scenes/scene1.scene");
}

void scene_render(Scene *scene, App *app) {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Mat4 view = get_camera_view(&scene->camera);
  Mat4 proj = mat4_perspective(1.0f, (float)app->width / app->height, 0.1f, 100.0f);

  shader_use(app->shader);

  glUniform3f(glGetUniformLocation(app->shader, "u_light_dir"), 0.3f, 1.0f, 0.7f);

  MeshComponent *mesh_c, *tmp;
  HASH_ITER(hh, scene->world.meshes, mesh_c, tmp) {
    Entity e = mesh_c->entity;

    MeshComponent *mesh_c = world_get_mesh(&scene->world, e);
    if (!mesh_c) continue;

    MaterialComponent *mat_c = world_get_material(&scene->world, e);
    int mat_id = mat_c != NULL ? mat_c->mat_id : -1;

    RenderMesh* mesh = mesh_reg_get(&scene->world.mesh_registry, mesh_c->mesh_id);
    Material* mat = mat_reg_get(&scene->world.material_registry, mat_id);

    glUniform3f(glGetUniformLocation(app->shader, "u_color"), mat->color.x, mat->color.y, mat->color.z);

    if (!mesh) continue;

    TransformComponent *tc = world_get_transform(&scene->world, e);
    Mat4 transform = tc != NULL ? tc->transform : mat4_identity();

    Mat4 mvp = mat4_mul(proj, mat4_mul(view, transform));
    shader_set_mat4(app->shader, "u_mvp", &mvp.m[0][0]);
    shader_set_mat4(app->shader, "u_model", &transform.m[0][0]);

    renderer_draw(mesh, app->shader);
  }

  glfwSwapBuffers(app->window);
}

void scene_destroy(Scene *scene) {
  world_destroy(&scene->world);
}
