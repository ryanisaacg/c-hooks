#pragma once

#include "textures.h"

typedef enum EntityType {ENTITY_PLAYER, ENTITY_HOOK, ENTITY_FISH, ENTITY_PUFFER} EntityType;

typedef struct {
	Animation current;
	EntityType type;
	int health, iframes;
	bool flip_x, flip_y;
	int hook_index, parent_index, target_index;
} EntityData;

EntityData data_new(EntityType type, int health);
