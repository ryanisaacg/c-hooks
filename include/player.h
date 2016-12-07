#pragma once

#include "arcade.h"

#include "data.h"

size_t spawn_player(World *world, Vector2 position);
void update_player(World world, ArcadeObject *player, EntityData *player_data);
void update_hook(World world, ArcadeObject *hook, EntityData *hook_data);
