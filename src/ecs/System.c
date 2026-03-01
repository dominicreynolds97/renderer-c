#include "System.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "vendor/uthash.h"

static void apply_paths(World *world) {
  PathComponent *p, *tmp1;
  HASH_ITER(hh, world->paths, p, tmp1) {
    SpeedComponent *sc = world_get_speed(world, p->entity);
    if (!sc) continue;
    PositionComponent *pc = world_get_position(world, p->entity);
    if (!pc) {
      world_add_position(world, p->entity, p->path.starting_pos);
      continue;
    }

    int prev = p->path.current_waypoint == 0
      ? p->path.waypoint_count - 1
      : p->path.current_waypoint - 1;

    Vec3f previous_waypoint = prev == 0 && !p->path.initialized
      ? p->path.starting_pos
      : vec3f_add(p->path.waypoints[prev], p->path.starting_pos);

    if (!p->path.initialized) {
      p->path.initialized = 1;
      p->path.current_waypoint = 1;
    }

    Vec3f target = vec3f_add(p->path.starting_pos, p->path.waypoints[p->path.current_waypoint]);

    Vec3f to_target = vec3f_sub(target, pc->position);
    Vec3f travel_dir = vec3f_sub(target, previous_waypoint);

    Vec3f direction = vec3f_normalize(to_target);

    float dot = vec3f_dot(to_target, travel_dir);

    if (dot < 0.0f) {
      p->path.current_waypoint++;

      if (p->path.current_waypoint >= p->path.waypoint_count) {
        if (p->path.is_loop) {
          p->path.current_waypoint = 0;
        } else {
          VelocityComponent *vc = world_get_velocity(world, p->entity);
          if (vc) vc->velocity = vec3f_identity();
          world_destroy_path(world, p);
          continue;
        }
      }

      target = vec3f_add(p->path.starting_pos, p->path.waypoints[p->path.current_waypoint]);
      to_target = vec3f_sub(target, pc->position);
      direction = vec3f_normalize(to_target);
    }

    Vec3f velocity = vec3f_scale(direction, sc->speed);
    VelocityComponent *vc = world_get_velocity(world, p->entity);
    if (!vc) {
      world_add_velocity(world, p->entity, velocity);
    } else {
      vc->velocity = velocity;
    }
  }
}

static void apply_velocities(World *world, float dt) {
  VelocityComponent *v, *tmp1;
  HASH_ITER(hh, world->velocities, v, tmp1) {
    PositionComponent *pc = world_get_position(world, v->entity);
    if (!pc) continue;

    Vec3f distance = vec3f_scale(v->velocity, dt);

    pc->position = vec3f_add(pc->position, distance);
  }
}

void update_systems(World *world, float dt) {
  apply_paths(world);
  apply_velocities(world, dt);
}
