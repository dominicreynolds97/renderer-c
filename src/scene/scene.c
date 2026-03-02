#include "Scene.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "../rendering/Shader.h"
#include "Registry.h"
#include "app/App.h"
#include "assets/Grid.h"
#include "camera.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "rendering/Renderer.h"
#include "SceneParser.h"

#define LINE_MAX_LENGTH 128

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
