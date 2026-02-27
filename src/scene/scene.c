#include "Scene.h"

#include "../assets/Mesh.h"
#include "../rendering/Shader.h"
#include "Registry.h"

void scene_create(Scene *scene) {
  scene->camera = init_camera();
  mesh_reg_init(&scene->mesh_registry);
  mat_reg_init(&scene->material_registry);

  Mesh cube = load_obj("meshes/cube.obj");
  int cube_id = mesh_reg_add(&scene->mesh_registry, renderer_upload_mesh(&cube));

  Mesh sphere = load_obj("meshes/sphere.obj");
  int sphere_id = mesh_reg_add(&scene->mesh_registry, renderer_upload_mesh(&sphere));

  Mesh cylinder = load_obj("meshes/cylinder.obj");
  int cylinder_id = mesh_reg_add(&scene->mesh_registry, renderer_upload_mesh(&cylinder));

  free_mesh(&cube);
  free_mesh(&sphere);
  free_mesh(&cylinder);

  int mat_id = mat_reg_add(&scene->material_registry, (Material){(Vec3f){0.4f, 0.8f, 0.7f}});

  scene->objects[0] = (SceneObject){cube_id, mat_id, mat4_translation(-2.0f, -1.0f, 0.0f)};
  scene->objects[1] = (SceneObject){cylinder_id, 1, mat4_translation(0.0f, -1.0f, -2.0f)};
  scene->object_count = 2;
}

void scene_render(Scene *scene, App *app) {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Mat4 view = get_camera_view(&scene->camera);
  Mat4 proj = mat4_perspective(1.0f, (float)app->width / app->height, 0.1f, 100.0f);

  shader_use(app->shader);
  shader_set_float(app->shader, "u_light_dir", 0.0f);

  glUniform3f(glGetUniformLocation(app->shader, "u_light_dir"), 0.3f, 1.0f, 0.7f);

  for (int i = 0; i < scene->object_count; i++) {
    SceneObject obj = scene->objects[i];
    RenderMesh* mesh = mesh_reg_get(&scene->mesh_registry, obj.mesh_id);
    Material* mat = mat_reg_get(&scene->material_registry, obj.mat_id);

    glUniform3f(glGetUniformLocation(app->shader, "u_color"), mat->color.x, mat->color.y, mat->color.z);

    if (!mesh) continue;

    Mat4 mvp = mat4_mul(proj, mat4_mul(view, obj.transform));
    shader_set_mat4(app->shader, "u_mvp", &mvp.m[0][0]);
    shader_set_mat4(app->shader, "u_model", &obj.transform.m[0][0]);

    renderer_draw(mesh, app->shader);
  }

  glfwSwapBuffers(app->window);
}

void scene_destroy(Scene *scene) {
  mesh_reg_destroy(&scene->mesh_registry);
  mat_reg_destroy(&scene->material_registry);
}
