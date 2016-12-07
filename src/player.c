#include "player.h"

#include "config.h"
#include "data.h"
#include "textures.h"

extern Animation player_anim_idle, player_anim_walk, player_anim_jump, hook_anim;
extern Group *player_group;

size_t spawn_player(World *world, Vector2 position) {
	EntityData data = data_new(ENTITY_PLAYER, 5);
	EntityData hook_data = data_new(ENTITY_HOOK, -1);
	ArcadeObject player_obj = arcobj_new(shape_rect(rect_new(position.x, position.y, player_width, player_height)), false);
	player_obj.acceleration.y = player_gravity;
	player_obj.drag.x = player_drag_x;
	player_obj.group = player_group;
	player.max_velocity = vec2_new(player_max_x, player_max_y);
	data.current = player_anim_idle;
	ArcadeObject hook_obj = arcobj_new(shape_rect(rect_new(position.x, position.y, hook_width, hook_height)));
	hook_data.current = hook_anim;
	obj_obj.group = group;
	size_t player_index = world_add(world, player_obj, &data);
	size_t hook_index = world_add(world, hook_obj, &hook_data);
	World w = *world;
	EntityData *player_data_ptr = world_get_data(w, player_index);
	EntityData *hook_data_ptr = world_get_data(w, hook_index);
	player_data_ptr->hook = hook_index;
	hook_data_ptr->parent = player_index;
	return player_index;
}
void update_player(World world, ArcadeObject *obj, EntityData *data) {
	int hook_index = data->hook_index;
	ArcadeObject *hook = hook_index == -1 ? NULL : world_get(world, hook_index);
	EntityData *hook_data = hook_index == -1 ? NULL : world_get_data(world, hook_index);
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	bool leftPressed = keys[SDL_SCANCODE_A];
	bool rightPressed = keys[SDL_SCANCODE_D];
	bool jumpPressed = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W];
	int mouse_x, mouse_y;
	Uint8 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
	//store animation progress so animations play
	int frame = data->current.current_frame;
	int progress = data->current.current_ticks;
	data->current = player_anim_idle;
	if(leftPressed ^ rightPressed) {
		if(leftPressed) {
			obj->acceleration.x = -player_walk;
			data->flip_x = true;
		} else  {
			obj->acceleration.x = player_walk;
			data->flip_x = false;
		}
		data->current = player_anim_walk;
	} else {
		obj->acceleration.x = 0;
	}
	if(!hook->alive) {
		//Throw the hook
		if(mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			hook->alive = true;
			shape_set_position(&hook->bounds, shape_get_center(obj->bounds));
			hook->velocity = vec2_with_len(vec2_sub(vec2_new(mouse_x, mouse_y), shape_get_position(obj->bounds)), hook_speed);
			shape_set_rotation(&hook->bounds, vec2_angle(hook->velocity));
		}
	} else if(hook_data->target_index == -1
			&& world_region_free(world, shape_rect(shape_bounding_box(hook->bounds)), hook)) {
		Rect bounds = shape_bounding_box(hook->bounds);
		bounds.x += hook->velocity.x;
		bounds.y += hook->velocity.y;
		if(!world_region_free(world, shape_rect(bounds), hook)) {
			shape_set_position(&hook->bounds, vec2_add(hook->velocity, shape_get_position(hook->bounds)));
		}
	} else {
		Vector2 diff = vec2_sub(shape_get_position(hook->bounds), shape_get_position(obj->bounds));
		if(vec2_len(diff) <= 60) {
			ArcadeObject *target = hook_data->target_index == -1 ? NULL : world_get(world, hook_data->target_index);
			if(target != NULL) {
				shape_set_position(&target->bounds, shape_get_position(hook->bounds));
			}
			hook_data->target_index = -1;
			hook->alive = false;
			obj->max_velocity = vec2_new(player_max_x, player_max_y);
		} else {
			obj->velocity = vec2_with_len(diff, hook_reel_speed);
			obj->max_velocity = vec2_new(player_reel_max_x, player_reel_max_y);
		}
	}
	Rect region = shape_bounding_box(obj->bounds);
	region.y += 1;
	bool supported = !world_region_free(world, shape_rect(region), obj);
	if(jumpPressed && supported) {
		obj->velocity.y = -player_jump;
		obj->acceleration.y = player_gravity_hold;
	}
	if(!jumpPressed) {
		obj->acceleration.y = player_gravity;
	}
	if(!supported) {
		data->current = player_anim_jump;
	}
	//Restore animation progress
	data->current.current_frame = frame % data->current.frames.length;
	data->current.current_ticks = progress;
}
void update_hook(World world, ArcadeObject *obj, EntityData *data) {
	if(data->target_index != -1) {
		ArcadeObject *target = world_get(world, data->target_index);
		shape_set_position(&target->bounds, shape_get_position(obj->bounds));
		target->velocity = obj->velocity;
	}
}
