#include "enemy.h"

#include "config.h"

extern Animation fish_anim;
extern Group *enemy_group;

static float rand_num(float min, float max) {
	float diff = max - min;
	float value = rand() * diff / RAND_MAX;
	return value + min;
}

size_t spawn_fish(World *world, Vector2 position) {
	ArcadeObject fish_obj = arcobj_new(shape_circ(circ_new(position.x, position.y, fish_radius)), false);
	EntityData fish_data = data_new(ENTITY_FISH, 2);
	fish_obj.max_velocity = vec2_new(fish_max_x, fish_max_y);
	fish_data.current = fish_anim;
	fish_obj.velocity.x = rand_num(-fish_leap, fish_leap);
	fish_obj.acceleration.y = fish_gravity;
	fish_data.health = 2;
	fish_obj.group = enemy_group;
	fish_obj.bounce = true;
	return world_add(world, fish_obj, &fish_data);
}

void update_fish(World world, ArcadeObject *obj, EntityData *data) {
	obj->velocity.x *= rand_num(0.9, 1.2);
	float rotation = vec2_angle(obj->velocity);
	if(rotation > 90 && rotation < 270) {
		data->flip_y = true;
	} else {
		data->flip_y = false;
	}
	shape_set_rotation(&obj->bounds, vec2_angle(obj->velocity));
}
