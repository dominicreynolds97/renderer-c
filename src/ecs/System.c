#include "System.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "vendor/uthash.h"

void apply_velocities(World *world, float dt) {
  VelocityComponent *v, *tmp1;
  HASH_ITER(hh, world->velocities, v, tmp1) {
    PositionComponent *pc = world_get_position(world, v->entity);
    if (!pc) continue;

    Vec3f distance = vec3f_scale(v->velocity, dt);

    pc->position = vec3f_add(pc->position, distance);
  }
}

void update_systems(World *world, float dt) {
  apply_velocities(world, dt);
}
