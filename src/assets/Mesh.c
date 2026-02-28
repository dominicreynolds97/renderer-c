#include "Mesh.h"
#include "maths/Maths3D.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RAW 10000
#define MAX_EXPANDED 30000
#define MAX_INDICES 60000

typedef struct { int vi, vti, vni; } VertexKey;

static int find_or_add_vertex(
    VertexKey *keys,
    int *count,
    Vec3f *positions, Vec3f *raw_normals, Vec2f *raw_uvs,
    Vec3f *out_positions, Vec3f *out_normals, Vec2f *out_uvs,
    int vi, int vti, int vni
) {
  for (int i = 0; i < *count; i++) {
    if (keys[i].vi == vi && keys[i].vti == vti && keys[i].vni == vni) {
      return i;
    }
  }

  int idx = *count;
  keys[idx] = (VertexKey){vi, vti, vni};
  out_positions[idx]  = positions[vi];
  out_normals[idx]    = vni >= 0 ? raw_normals[vni] : (Vec3f){0.0f, 1.0f, 0.0f};
  out_uvs[idx]        = vti >= 0 ? raw_uvs[vti] : (Vec2f){0.0f, 0.0f};

  (*count)++;
  return idx;
}

Mesh load_obj(const char *path) {
  Mesh mesh = {0};

  FILE *f = fopen(path, "r");
  if (!f) {
    printf("Failed to open mesh: %s\n", path);
    return mesh;
  }

  Vec3f *raw_pos = malloc(MAX_RAW * sizeof(Vec3f));
  Vec3f *raw_nrm = malloc(MAX_RAW * sizeof(Vec3f));
  Vec2f *raw_uv = malloc(MAX_RAW * sizeof(Vec2f));

  int raw_pos_count = 0, raw_nrm_count = 0, raw_uv_count = 0;

  VertexKey *keys   = malloc(MAX_EXPANDED * sizeof(VertexKey));
  Vec3f *out_pos    = malloc(MAX_EXPANDED * sizeof(Vec3f));
  Vec3f *out_nrm    = malloc(MAX_EXPANDED * sizeof(Vec3f));
  Vec2f *out_uv     = malloc(MAX_EXPANDED * sizeof(Vec2f));
  int *indices      = malloc(MAX_INDICES  * sizeof(int));
  int vertex_count = 0, index_count = 0;
  int has_normals = 0, has_uvs = 0;

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == 'v' && line[1] == ' ') {
      Vec3f v;
      sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
      raw_pos[raw_pos_count++] = v;
    } else if (line[0] == 'v' && line[1] == 't') {
      Vec2f vt;
      sscanf(line, "vt %f %f", &vt.x, &vt.y);
      raw_uv[raw_uv_count++] = vt;
      has_uvs = 1;

    } else if (line[0] == 'v' && line[1] == 'n') {
      Vec3f vn;
      sscanf(line, "vn %f %f %f", &vn.x, &vn.y, &vn.z);
      raw_nrm[raw_nrm_count++] = vn;
      has_normals = 1;

    } else if (line[0] == 'f' && line[1] == ' ') {
      int face_indices[16];
      int face_count = 0;

      char *token = strtok(line + 2, " \n");
      while (token && face_count < 16) {
        int vi = 0, vti = 0, vni = 0;
        int has_vt = 0, has_vn = 0;

        if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3) {
          has_vt = 1; has_vn = 1;
        } else if (sscanf(token, "%d//%d", &vi, &vni) == 2) {
          has_vn = 1;
        } else {
          sscanf(token, "%d", &vi);
        }

        int idx = find_or_add_vertex(
          keys, &vertex_count,
          raw_pos, raw_nrm, raw_uv,
          out_pos, out_nrm, out_uv,
          vi - 1,
          has_vt ? vti - 1 : -1,
          has_vn ? vni - 1 : -1
        );

        face_indices[face_count++] = idx;
        token = strtok(NULL, " \n");
      }

      for (int i = 1; i < face_count - 1; i++) {
        if (index_count + 3 >= MAX_INDICES) break;
        indices[index_count++] = face_indices[0];
        indices[index_count++] = face_indices[i];
        indices[index_count++] = face_indices[i + 1];
      }
    }
  }

  fclose(f);

  if (!has_normals) {
    memset(out_nrm, 0, vertex_count * sizeof(Vec3f));
    for (int i = 0; i < index_count; i += 3) {
      Vec3f v0 = out_pos[indices[i]];
      Vec3f v1 = out_pos[indices[i + 1]];
      Vec3f v2 = out_pos[indices[i + 2]];
      Vec3f n  = compute_face_normal(v0, v1, v2);
      for (int j = 0; j < 3; j++) {
        out_nrm[indices[i + j]].x += n.x;
        out_nrm[indices[i + j]].y += n.y;
        out_nrm[indices[i + j]].z += n.x;
      }
    }
    for (int i = 0; i < vertex_count; i++) {
      out_nrm[i] = vec3f_normalize(out_nrm[i]);
    }
  }

  mesh.positions    = out_pos;
  mesh.normals      = out_nrm;
  mesh.uvs          = out_uv;
  mesh.indices      = indices;
  mesh.vertex_count = vertex_count;
  mesh.index_count  = index_count;

  free(raw_pos);
  free(raw_nrm);
  free(raw_uv);
  free(keys);

  printf("Loaded %d vertices, %d triangles \n", mesh.vertex_count, index_count / 3);
  return mesh;
}

void free_mesh(Mesh *mesh) {
  free(mesh->positions);
  free(mesh->normals);
  free(mesh->uvs);
  free(mesh->indices);
}

