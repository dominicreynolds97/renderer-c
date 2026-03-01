#include "Scene.h"

#include "../assets/Mesh.h"
#include "../rendering/Shader.h"
#include "Registry.h"
#include "app/App.h"
#include "assets/Grid.h"
#include "assets/Texture.h"
#include "camera.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "rendering/Renderer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LINE_MAX_LENGTH 128
#define IDLE 0
#define IN_MATERIAL 1
#define IN_OBJECT 2
#define IN_SKYBOX 3
#define IN_PATH 4
#define IN_COLLIDER 5

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

  Material  current_material  = {0};
  Path      current_path      = {0};

  current_path.waypoint_count = 0;
  current_path.starting_pos = vec3f_identity();
  current_path.is_loop = 0;
  current_path.current_waypoint = 0;
  current_path.initialized = 0;

  Vec3f current_half_extents  = vec3f_identity();
  int   current_is_static     = 1;

  Entity current_entity;

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
          current_entity = world_create_entity(&scene->world);
        }
        if (strncmp(line, "skybox", 6) == 0) {
          state = IN_SKYBOX;
        }
        break;
      case(IN_MATERIAL):
        if (strncmp(line, "color", 5) == 0) {
          Vec3f color;
          sscanf(line, "color %f %f %f", &color.x, &color.y, &color.z);
          current_material.color = color;
        }
        if (strncmp(line, "texture", 7) == 0) {
          char path[256];
          sscanf(line, "texture %s", path);
          current_material.texture_id = texture_load(path);
        }
        if (strncmp(line, "end", 3) == 0) {
          mat_reg_add(&scene->world.material_registry, current_material);
          current_material = (Material){0};

          state = IDLE;
        }
        break;
      case(IN_OBJECT):
        if (strncmp(line, "position", 8) == 0) {
          Vec3f position;
          sscanf(line, "position %f %f %f", &position.x, &position.y, &position.z);

          world_add_position(&scene->world, current_entity, position);
        }
        if (strncmp(line, "rotation", 8) == 0) {
          Vec3f rotation;
          sscanf(line, "rotation %f %f %f", &rotation.x, &rotation.y, &rotation.z);

          world_add_rotation(&scene->world, current_entity, rotation);
        }
        if (strncmp(line, "scale", 5) == 0) {
          Vec3f scale;
          sscanf(line, "scale %f %f %f", &scale.x, &scale.y, &scale.z);

          world_add_scale(&scene->world, current_entity, scale);
        }
        if (strncmp(line, "velocity", 8) == 0) {
          Vec3f v;
          sscanf(line, "velocity %f %f %f", &v.x, &v.y, &v.z);
          world_add_velocity(&scene->world, current_entity, v);
        }
        if (strncmp(line, "speed", 5) == 0) {
          float s;
          sscanf(line, "speed %f", &s);
          world_add_speed(&scene->world, current_entity, s);
        }
        if (strncmp(line, "path", 4) == 0) {
          state = IN_PATH;

          current_path.waypoint_count = 0;
          current_path.starting_pos = vec3f_identity();
          current_path.current_waypoint = 0;
          current_path.initialized = 0;

          if (strncmp(line, "path loop", 9) == 0) {
            current_path.is_loop = 1;
          } else {
            current_path.is_loop = 0;
          }
        }
        if (strncmp(line, "collider", 8) == 0) {
          state = IN_COLLIDER;
          current_is_static = strncmp(line, "collider dynamic", 16) == 0 ? 0 : 1;
        }
        if (strncmp(line, "end", 3) == 0) {
          int mesh_id = find_id(mesh_names, scene->world.mesh_registry.count, current_mesh_name);
          int mat_id = find_id(material_names, scene->world.material_registry.count, current_mat_name);

          if (mesh_id >= 0) world_add_mesh(&scene->world, current_entity, mesh_id);
          if (mat_id >= 0) world_add_material(&scene->world, current_entity, mat_id);

          state = IDLE;
        }
        break;
      case(IN_SKYBOX):
        if (strncmp(line, "color", 5) == 0) {
          Vec3f color;
          sscanf(line, "color %f %f %f", &color.x, &color.y, &color.z);
          scene->skybox.color = color;
        }
        if (strncmp(line, "end", 3) == 0) {
          state = IDLE;
        }
        break;
      case(IN_PATH):
        if (strncmp(line, "waypoint", 8) == 0) {
          Vec3f wp;
          sscanf(line, "waypoint %f %f %f", &wp.x, &wp.y, &wp.z);
          current_path.waypoints[current_path.waypoint_count++] = wp;
        }
        if (strncmp(line, "end", 3) == 0) {
          PositionComponent *pc = world_get_position(&scene->world, current_entity);
          if (pc) current_path.starting_pos = pc->position;
          world_add_path(&scene->world, current_entity, current_path);

          state = IN_OBJECT;
        }
        break;
      case(IN_COLLIDER):
        if (strncmp(line, "extents", 7) == 0) {
          Vec3f half_extents;
          sscanf(line, "extents %f %f %f", &half_extents.x, &half_extents.y, &half_extents.z);
          current_half_extents = half_extents;
        }
        if (strncmp(line, "end", 3) == 0) {
          world_add_collider(&scene->world, current_entity, current_half_extents, current_is_static);
          current_half_extents = vec3f_identity();
          current_is_static = 1;

          state = IN_OBJECT;
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
  glClearColor(scene->skybox.color.x, scene->skybox.color.y, scene->skybox.color.z, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Mat4 view = get_camera_view(&scene->world.camera);
  Mat4 proj = mat4_perspective(1.0f, (float)app->width / app->height, 0.1f, 100.0f);

  shader_use(app->shader);

  glUniform3f(glGetUniformLocation(app->shader, "u_light_dir"), 0.3f, 1.0f, 0.7f);


  MeshComponent *mesh_c, *tmp;
  HASH_ITER(hh, scene->world.meshes, mesh_c, tmp) {
    Entity e = mesh_c->entity;

    MaterialComponent *mat_c = world_get_material(&scene->world, e);
    int mat_id = mat_c != NULL ? mat_c->mat_id : -1;

    RenderMesh* mesh = mesh_reg_get(&scene->world.mesh_registry, mesh_c->mesh_id);
    Material* mat = mat_reg_get(&scene->world.material_registry, mat_id);

    glUniform3f(glGetUniformLocation(app->shader, "u_color"), mat->color.x, mat->color.y, mat->color.z);

    if (!mesh) continue;

    Mat4 transform = world_get_transform(&scene->world, e);

    Mat4 mvp = mat4_mul(proj, mat4_mul(view, transform));
    shader_set_mat4(app->shader, "u_mvp", &mvp.m[0][0]);
    shader_set_mat4(app->shader, "u_model", &transform.m[0][0]);

    if (mat->texture_id) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, mat->texture_id);
      glUniform1i(glGetUniformLocation(app->shader, "u_has_texture"), 1);
      glUniform1i(glGetUniformLocation(app->shader, "u_texture"), 0);
    } else {
      glUniform1i(glGetUniformLocation(app->shader, "u_has_texture"), 0);
      glUniform1i(glGetUniformLocation(app->shader, "u_texture"), 0);
    }

    renderer_draw(mesh, app->shader);
  }

  grid_render(&scene->grid, app->flat_shader, &proj.m[0][0], &view.m[0][0]);

  glfwSwapBuffers(app->window);
}

void scene_destroy(Scene *scene) {
  world_destroy(&scene->world);
}
