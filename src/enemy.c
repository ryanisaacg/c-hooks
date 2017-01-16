#include "enemy.h"

#include "config.h"
#include "sprite.h"

extern Animation fish_anim;
extern Group *enemy_group;

static float rand_num(float min, float max) {
	float diff = max - min;
	float value = rand() * diff / RAND_MAX;
	return value + min;
}

size_t spawn_fish(World *world, Vector2 position) {
	Shape bounds = shape_circ(circ_new(position.x, position.y, fish_radius));
	ArcadeObject fish_obj = arcobj_new(bounds, false, spr_new_animated(fish_anim, shape_bounding_box(bounds)));
	spr_center_origin(&(fish_obj.sprite));
	EntityData fish_data = data_new(ENTITY_FISH, 2);
	fish_obj.max_velocity = vec2_new(fish_max_x, fish_max_y);
	fish_obj.velocity.x = rand_num(-fish_leap, fish_leap);
	fish_obj.acceleration.y = fish_gravity;
	fish_data.health = 2;
	fish_obj.group = enemy_group;
	fish_obj.bounce = true;
	return world_add(world, fish_obj, &fish_data);
}

void update_fish(World world, ArcadeObject *obj, EntityData *data) {
	obj->velocity.x *= rand_num(0.9, 1.5);
	obj->sprite.angle = vec2_angle(obj->velocity);
	float rotation = obj->sprite.angle;
	if(rotation > 90 && rotation < 270) {
		obj->sprite.flip_y = true;
	} else {
		obj->sprite.flip_y = false;
	}
}
