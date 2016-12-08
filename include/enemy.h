#pragma once

#include "arcade.h"
#include "data.h"

size_t spawn_fish(World *world, Vector2 position);
void update_fish(World world, ArcadeObject *obj, EntityData *data);

