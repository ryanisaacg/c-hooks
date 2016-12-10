#include "objects.h"

#include "config.h"
#include "data.h"
#include "textures.h"

extern Animation block_anim;
extern Group *object_group;

size_t spawn_block(World *world, Vector2 position) {
	ArcadeObject block_obj = arcobj_new(shape_rect(rect_new(position.x, position.y, box_width, box_height)), false);
	EntityData block_data = data_new(ENTITY_BLOCK, 2);
	block_data.current = block_anim;
	block_obj.acceleration.y = box_gravity;
	block_obj.group = object_group;
	return world_add(world, block_obj, &block_data);
}
