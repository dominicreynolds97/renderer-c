#include "System.h"
#include "ecs/World.h"
#include "maths/Maths3D.h"
#include "vendor/uthash.h"
#include <math.h>

#define DOWN (Vec3f){0.0f, -1.0f, 0.0f}
#define GRAVITY 9.8f

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

static void update_player(World *world) {
  PositionComponent *player_pos = world_get_position(world, world->player.entity);
  if (player_pos) world->camera.pos = player_pos->position;
}

static int aabb_overlap(Vec3f pos_a, Vec3f half_a, Vec3f pos_b, Vec3f half_b) {
  return fabsf(pos_a.x - pos_b.x) < half_a.x + half_b.x &&
         fabsf(pos_a.y - pos_b.y) < half_a.y + half_b.y &&
         fabsf(pos_a.z - pos_b.z) < half_a.z + half_b.z;
}

static void resolve_collisions(World *world) {
  ColliderComponent *a, *tmp1;
  HASH_ITER(hh, world->colliders, a, tmp1) {
    if (a->is_static) continue;

    PositionComponent *ap = world_get_position(world, a->entity);
    if (!ap) continue;

    ColliderComponent *b, *tmp2;
    HASH_ITER(hh, world->colliders, b, tmp2) {
      if (a->entity == b->entity) continue;

      PositionComponent *bp = world_get_position(world, b->entity);
      if (!bp) continue;

      if (!aabb_overlap(ap->position, a->half_extents, bp->position, b->half_extents))
        continue;

      float dx = (a->half_extents.x + b->half_extents.x) - fabsf(ap->position.x - bp->position.x);
      float dy = (a->half_extents.y + b->half_extents.y) - fabsf(ap->position.y - bp->position.y);
      float dz = (a->half_extents.z + b->half_extents.z) - fabsf(ap->position.z - bp->position.z);

      if (dx < dy && dx < dz) {
        ap->position.x += ap->position.x < bp->position.x ? -dx : dx;
      } else if (dy < dx && dy < dz) {
        ap->position.y += ap->position.y < bp->position.y ? -dy : dy;
        MassComponent *mc = world_get_mass(world, a->entity);
        if (mc) mc->grounded_entity = b->entity;
      } else {
        ap->position.z += ap->position.z < bp->position.z ? -dz : dz;
      }
    }
  }
}

static void apply_acceleration(World *world, Entity e, Vec3f dir, float accelleration, float dt) {
  VelocityComponent *vc = world_get_velocity(world, e);
  Vec3f velocity = vc ? vc->velocity : vec3f_identity();

  Vec3f normalized_dir = vec3f_normalize(dir);

  velocity = vec3f_add(
    velocity,
    vec3f_scale(normalized_dir, accelleration * dt)
  );

  if (!vc) {
    world_add_velocity(world, e, velocity);
  } else {
    vc->velocity = velocity;
  }
}

static void apply_friction(World *world, MassComponent *mc, float dt) {
  VelocityComponent *vc = world_get_velocity(world, mc->entity);
  if (!vc) return;

  ColliderComponent *ca = world_get_collider(world, mc->entity);
  ColliderComponent *cb = world_get_collider(world, mc->entity);

  float friction_a = ca ? ca->friction : 0.8f;
  float friction_b = cb ? cb->friction : 0.8f;
  float friction   = friction_a * friction_b;

  float threshold = friction * 0.1f;

  float decel = friction * dt;
  vc->velocity.x *= fmaxf(0.0f, 1.0f - decel);
  vc->velocity.z *= fmaxf(0.0f, 1.0f - decel);

  if (fabsf(vc->velocity.x) < threshold) vc->velocity.x = 0;
  if (fabsf(vc->velocity.y) < threshold) vc->velocity.y = 0;
}

static void apply_gravity_and_friction(World *world, float dt) {
  MassComponent *mc, *tmp1;
  HASH_ITER(hh, world->masses, mc, tmp1) {
    if (mc->grounded_entity == -1) {
      apply_acceleration(world, mc->entity, DOWN, GRAVITY, dt);
    } else {
      mc->grounded_entity = -1;
      apply_friction(world, mc, dt);
    }
  }
}

void apply_thrust(World *world, Entity e, Vec3f dir, float dt) {
  LocomotionComponent *lc = world_get_locomotion(world, e);
  if (!lc) return;
  apply_acceleration(world, e, dir, lc->thrust, dt);
}

void jump(World *world, Entity e) {
  MassComponent *mc = world_get_mass(world, e);
  JumpComponent *jc = world_get_jump(world, e);
  if (!jc || !mc || mc->grounded_entity == -1) return;
  VelocityComponent *vc = world_get_velocity(world, e);
  if (vc) {
    vc->velocity.y += jc->jump_force;
  } else {
    world_add_velocity(world, e, (Vec3f){0.0f, jc->jump_force, 0.0f});
  }
}

void update_systems(World *world, float dt) {
  apply_paths(world);
  apply_gravity_and_friction(world, dt);
  apply_velocities(world, dt);
  resolve_collisions(world);
  update_player(world);
}
