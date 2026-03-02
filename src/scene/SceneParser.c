#include "SceneParser.h"
#include "assets/Texture.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "scene/Registry.h"
#include "scene/Scene.h"
#include <ctype.h>
#include <stdio.h>

static void trimString(char *str) {
  int start = 0, end = strlen(str) -1;
  while (isspace(str[start])) start++;
  while (end > start && isspace(str[end])) end--;

  if (start > 0 || end < ((int)strlen(str) - 1)) {
    memmove(str, str + start, end - start + 1);
    str[end - start + 1] = '\0';
  }
}

static int find_id(char names[][64], int count, char *name) {
  for (int i = 0; i < count; i++) {
    if (strncmp(names[i], name, 63) == 0) return i;
  }
  return -1;
}


typedef struct {
  FILE *file;
  char line[128];
  int  in_multiline_comment;
  char mesh_names[MAX_MESHES][64];
  char material_names[MAX_MATERIALS][64];
  char current_mesh_name[64];
  char current_mat_name[64];
} SceneIterator;

static int next_line(SceneIterator *it) {
  while (fgets(it->line, sizeof(it->line), it->file)) {
    trimString(it->line);
    if (it->line[0] == '\n' || it->line[0] == '\0') continue;
    if (it->line[0] == '/' && it->line[1] == '/') continue;
    if (it->line[0] == '/' && it->line[1] == '*') {
      it->in_multiline_comment = 1;
      continue;
    }
    if (it->in_multiline_comment) {
      if (it->line[0] == '*' && it->line[1] == '/') {
        it->in_multiline_comment = 0;
        continue;
      };
    }
    return 1;
  }
  return 0;
}

typedef void (*ComponentParser)(SceneIterator *it, Entity e, World *world);
typedef void (*BlockParser)(SceneIterator *it, Scene *scene);

typedef struct {
  const char      *keyword;
  int             keyword_len;
  ComponentParser parse;
} ParserEntry;


// COMPONENT PARSERS

static void parse_position(SceneIterator *it, Entity e, World *world) {
  Vec3f position;
  sscanf(it->line, "position %f %f %f", &position.x, &position.y, &position.z);
  world_add_position(world, e, position);
}

static void parse_rotation(SceneIterator *it, Entity e, World *world) {
  Vec3f rotation;
  sscanf(it->line, "rotation %f %f %f", &rotation.x, &rotation.y, &rotation.z);
  world_add_rotation(world, e, rotation);
}

static void parse_collider(SceneIterator *it, Entity e, World *world) {
  int is_static = strncmp(it->line, "collider dynamic", 16) == 0 ? 0 : 1;
  Vec3f half_extents = vec3f_identity();
  while (next_line(it)) {
    if (strncmp(it->line, "extents", 7) == 0) {
      sscanf(it->line, "extents %f %f %f", &half_extents.x, &half_extents.y, &half_extents.z);
    }
    if (strncmp(it->line, "end", 3) == 0) {
      world_add_collider(world, e, half_extents, is_static);
      break;
    }
  }
}

static void parse_scale(SceneIterator *it, Entity e, World *world) {
  Vec3f scale;
  sscanf(it->line, "scale %f %f %f", &scale.x, &scale.y, &scale.z);
  world_add_scale(world, e, scale);
}

static void parse_velocity(SceneIterator *it, Entity e, World *world) {
  Vec3f v;
  sscanf(it->line, "velocity %f %f %f", &v.x, &v.y, &v.z);
  world_add_velocity(world, e, v);
}

static void parse_speed(SceneIterator *it, Entity e, World *world) {
  float s;
  sscanf(it->line, "speed %f", &s);
  world_add_speed(world, e, s);
}

static void parse_mass(SceneIterator *it, Entity e, World *world) {
  float mass;
  sscanf(it->line, "mass %f", &mass);
  world_add_mass(world, e, mass);
}

static void parse_locomotion(SceneIterator *it, Entity e, World *world) {
  (void)it;
  (void)e;
  (void)world;
}

static void parse_jump(SceneIterator *it, Entity e, World *world) {
  (void)it;
  (void)e;
  (void)world;
}

static void parse_path(SceneIterator *it, Entity e, World *world) {
  Path path = {0};
  path.waypoint_count = 0;
  path.starting_pos = vec3f_identity();
  path.current_waypoint = 0;
  path.initialized = 0;

  if (strncmp(it->line, "path loop", 9) == 0) {
    path.is_loop = 1;
  } else {
    path.is_loop = 0;
  }

  while (next_line(it)) {
    if (strncmp(it->line, "waypoint", 8) == 0) {
      Vec3f wp;
      sscanf(it->line, "waypoint %f %f %f", &wp.x, &wp.y, &wp.z);
      path.waypoints[path.waypoint_count++] = wp;
    }
    if (strncmp(it->line, "end", 3) == 0) {
      PositionComponent *pc = world_get_position(world, e);
      if (pc) path.starting_pos = pc->position;
      world_add_path(world, e, path);
      break;
    }
  }
}


static const ParserEntry component_parsers[] = {
  { "position",   8,   parse_position    },
  { "rotation",   8,   parse_rotation    },
  { "collider",   8,   parse_collider    },
  { "scale",      5,   parse_scale       },
  { "mass",       4,   parse_mass        },
  { "locomotion", 10,  parse_locomotion  },
  { "velocity",   8,   parse_velocity    },
  { "speed",      5,   parse_speed       },
  { "path",       4,   parse_path        },
  { "jump",       4,   parse_jump        },
};

typedef struct {
  const char      *keyword;
  int             keyword_len;
  BlockParser     parse;
} BlockParserEntry;

// BLOCK PARSERS

static void parse_object(SceneIterator *it, Scene *scene) {
  sscanf(it->line, "object %s %s", it->current_mesh_name, it->current_mat_name);
  Entity e = world_create_entity(&scene->world);
  while (next_line(it)) {

    if (strncmp(it->line, "end", 3) == 0) {
      int mesh_id = find_id(it->mesh_names, scene->world.mesh_registry.count, it->current_mesh_name);
      int mat_id = find_id(it->material_names, scene->world.material_registry.count, it->current_mat_name);
      if (mesh_id >= 0) world_add_mesh(&scene->world, e, mesh_id);
      if (mat_id >= 0) world_add_material(&scene->world, e, mat_id);
      break;
    }

    int n = sizeof(component_parsers) / sizeof(component_parsers[0]);
    for (int i = 0; i < n; i++) {
      if (strncmp(it->line, component_parsers[i].keyword, component_parsers[i].keyword_len) == 0) {
          component_parsers[i].parse(it, e, &scene->world);
      }
    }
  }
}

static void parse_mesh(SceneIterator *it, Scene *scene) {
  char name[64], path[256];
  sscanf(it->line, "mesh %s %s", name, path);

  strncpy(it->mesh_names[scene->world.mesh_registry.count], name, 63);
  it->mesh_names[scene->world.mesh_registry.count][63] = '\0';

  Mesh mesh = load_obj(path);
  RenderMesh rm = renderer_upload_mesh(&mesh);
  mesh_reg_add(&scene->world.mesh_registry, rm);

  free_mesh(&mesh);
}

static void parse_material(SceneIterator *it, Scene *scene) {
  char name[64];
  sscanf(it->line, "material %s", name);

  strncpy(it->material_names[scene->world.material_registry.count], name, 63);
  it->material_names[scene->world.material_registry.count][63] = '\0';

  Material mat = {0};

  while (next_line(it)) {
    if (strncmp(it->line, "color", 5) == 0) {
      Vec3f color;
      sscanf(it->line, "color %f %f %f", &color.x, &color.y, &color.z);
      mat.color = color;
    }
    if (strncmp(it->line, "texture", 7) == 0) {
      char path[256];
      sscanf(it->line, "texture %s", path);
      mat.texture_id = texture_load(path);
    }
    if (strncmp(it->line, "end", 3) == 0) {
      mat_reg_add(&scene->world.material_registry, mat);
      break;
    }
  }
}

static void parse_skybox(SceneIterator *it, Scene *scene) {
  while (next_line(it)) {
    if (strncmp(it->line, "color", 5) == 0) {
      Vec3f color;
      sscanf(it->line, "color %f %f %f", &color.x, &color.y, &color.z);
      scene->skybox.color = color;
    }
    if (strncmp(it->line, "end", 3) == 0) {
      break;
    }
  }
}

static const BlockParserEntry block_parsers[] = {
  { "object",     6,  parse_object   },
  { "mesh",       4,  parse_mesh     },
  { "material",   8,  parse_material },
  { "skybox",     8,  parse_skybox   },
};

int parse_scene_file(Scene *scene, char* filepath) {
  world_init(&scene->world);

  SceneIterator it;

  it.file = fopen(filepath, "r");
  if (!it.file) {
    printf("Failed to open scene file: %s\n", filepath);
    return 1;
  }

  while (next_line(&it)) {
    int n = sizeof(block_parsers) / sizeof(block_parsers[0]);
    for (int i = 0; i < n; i++) {
      if (strncmp(it.line, block_parsers[i].keyword, block_parsers[i].keyword_len) == 0) {
          block_parsers[i].parse(&it, scene);
      }
    }
  }

  return 0;
}
