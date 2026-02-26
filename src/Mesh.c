#include "Mesh.h"
#include "Maths3D.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 10000
#define MAX_FACES 20000

Mesh load_obj(const char *path) {
  Mesh mesh = {0};
  mesh.vertices = malloc(MAX_VERTICES * sizeof(Vec3f));
  mesh.faces = malloc(MAX_FACES * 3 * sizeof(int));

  FILE *f = fopen(path, "r");
  if (!f) {
    printf("Failed to open mesh: %s\n", path);
    return mesh;
  }

  Vec3f *raw_verts = malloc(MAX_VERTICES * sizeof(Vec3f));
  int raw_vert_count = 0;

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == 'v' && line[1] == ' ') {
      Vec3f v;
      sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
      raw_verts[raw_vert_count++] = v;
    } else if (line[0] == 'f' && line[1] == ' ') {
      int face_verts[16];
      int face_vert_count = 0;

      char *token = strtok(line + 2, " \n");
      while (token && face_vert_count < 16) {
        int vi = 0, vti = 0, nvi = 0;
        sscanf(token, "%d/%d/%d", &vi, &vti, &nvi);
        if (vi == 0) sscanf(token, "%d//%d", &vi, &nvi);
        if (vi == 0) sscanf(token, "%d", &vi);

        face_verts[face_vert_count++] = vi - 1;
        token = strtok(NULL, " \n");
      }
      for (int i = 1; i < face_vert_count - 1; i++) {
        if (mesh.face_count >= MAX_FACES) break;
        mesh.faces[mesh.face_count][0] = face_verts[0];
        mesh.faces[mesh.face_count][1] = face_verts[i];
        mesh.faces[mesh.face_count][2] = face_verts[i + 1];
        mesh.face_count++;
      }
    }
  }

  fclose(f);

  mesh.vertex_count = raw_vert_count;
  for (int i = 0; i < raw_vert_count; i++) {
    mesh.vertices[i] = raw_verts[i];
  }

  free(raw_verts);

  mesh.normals = calloc(mesh.vertex_count, sizeof(Vec3f));

  for (int i = 0; i< mesh.face_count; i++) {
    Vec3f v0 = mesh.vertices[mesh.faces[i][0]];
    Vec3f v1 = mesh.vertices[mesh.faces[i][1]];
    Vec3f v2 = mesh.vertices[mesh.faces[i][2]];

    Vec3f normal = compute_face_normal(v0, v1, v2);

    mesh.normals[mesh.faces[i][0]].x += normal.x;
    mesh.normals[mesh.faces[i][0]].y += normal.y;
    mesh.normals[mesh.faces[i][0]].z += normal.z;

    mesh.normals[mesh.faces[i][1]].x += normal.x;
    mesh.normals[mesh.faces[i][1]].y += normal.y;
    mesh.normals[mesh.faces[i][1]].z += normal.z;

    mesh.normals[mesh.faces[i][2]].x += normal.x;
    mesh.normals[mesh.faces[i][2]].y += normal.y;
    mesh.normals[mesh.faces[i][2]].z += normal.z;
  }

  for (int i = 0; i < mesh.vertex_count; i++) {
    mesh.normals[i] = vec3f_normalize(mesh.normals[i]);
  }

  printf("Loaded %d vertices, %d triangles \n", mesh.vertex_count, mesh.face_count);
  return mesh;
}

void free_mesh(Mesh *mesh) {
  free(mesh->vertices);
  free(mesh->faces);
  free(mesh->normals);
}

