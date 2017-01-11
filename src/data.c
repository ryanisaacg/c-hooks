#include "data.h"

EntityData data_new(EntityType type, int health) {
	EntityData data;
	data.type = type;
	data.health = health;
	data.iframes = 0;
	data.hook_index = data.parent_index = data.target_index = -1;
	return data;
}
