#include "Scene.h"

#include "../assets/Mesh.h"
#include "../rendering/Shader.h"

#define CUBE 0
#define SPHERE 1
#define CYLINDER 2

void scene_create(Scene *scene) {
  scene->camera = init_camera();

  Mesh cube = load_obj("meshes/cube.obj");
  Mesh sphere = load_obj("meshes/sphere.obj");
  Mesh cylinder = load_obj("meshes/cylinder.obj");

  scene->meshes[0] = renderer_upload_mesh(&cube);
  scene->meshes[1] = renderer_upload_mesh(&sphere);
  scene->meshes[2] = renderer_upload_mesh(&cylinder);
  scene->mesh_count = 3;

  free_mesh(&cube);
  free_mesh(&sphere);
  free_mesh(&cylinder);

  scene->objects[0] = (SceneObject){CUBE, mat4_translation(-2.0f, -1.0f, 0.0f)};
  scene->objects[1] = (SceneObject){CYLINDER, mat4_translation(0.0f, -1.0f, -2.0f)};
  scene->object_count = 2;
}

void scene_render(Scene *scene, App *app) {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Mat4 model = mat4_scale(0.3f, 0.3f, 0.3f);

  Mat4 view = get_camera_view(&scene->camera);
  Mat4 proj = mat4_perspective(1.0f, (float)app->width / app->height, 0.1f, 100.0f);

  shader_use(app->shader);
  shader_set_float(app->shader, "u_light_dir", 0.0f);

  glUniform3f(glGetUniformLocation(app->shader, "u_light_dir"), 0.3f, 1.0f, 0.7f);
  glUniform3f(glGetUniformLocation(app->shader, "u_color"), 0.8f, 0.8f, 0.8f);

  for (int i = 0; i < scene->object_count; i++) {
    SceneObject obj = scene->objects[i];
    RenderMesh mesh = scene->meshes[obj.mesh_id];

    Mat4 mvp = mat4_mul(proj, mat4_mul(view, obj.transform));
    shader_set_mat4(app->shader, "u_mvp", &mvp.m[0][0]);
    shader_set_mat4(app->shader, "u_model", &model.m[0][0]);

    renderer_draw(&mesh, app->shader);
  }

  glfwSwapBuffers(app->window);
}

void scene_destroy(Scene *scene) {
  for (int i = 0; i < scene->mesh_count; i++) {
    renderer_free(&scene->meshes[i]);
  }
}
