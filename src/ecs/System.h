#ifndef SYSTEM_H
#define SYSTEM_H

#include "ecs/World.h"
#include "maths/Maths3D.h"

void update_systems(World *world, float dt);
void apply_thrust(World *world, Entity e, Vec3f dir, float dt);
void jump(World *world, Entity e);

#endif
