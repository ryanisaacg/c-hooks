#pragma once

typedef enum EntityType {ENTITY_PLAYER, ENTITY_HOOK, ENTITY_FISH, ENTITY_PUFFER, ENTITY_BLOCK} EntityType;

typedef struct {
	EntityType type;
	int health, iframes;
	int hook_index, parent_index, target_index;
} EntityData;

EntityData data_new(EntityType type, int health);
