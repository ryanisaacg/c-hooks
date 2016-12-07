#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "arcade.h"
#include "parson.h"
#include <SDL.h>

#include "config.h"
#include "data.h"
#include "player.h"
#include "textures.h"

Animation player_anim_idle, player_anim_walk, player_anim_jump, hook_anim, fish_anim;
SDL_Renderer *rend;
Group *player_group, *enemy_group;

float rand_num(float min, float max) {
	float diff = max - min;
	float value = rand() * diff / RAND_MAX;
	return value + min;
}

size_t new_entity(World *world, Vector2 position, EntityType type) {
	EntityData data;
	data.iframes = 0;
	data.type = type;
	data.flip_x = data.flip_y = false;
	data.hook_index = data.parent_index = data.target_index = -1;
	Shape bounds;
	Vector2 acceleration = vec2_new(0, 0), drag = vec2_new(0, 0), max_velocity = vec2_new(-1, -1);
	Group *group = NULL;
	bool solid = false, bounce = false;
	switch(type) {
		case ENTITY_PLAYER:
			bounds = shape_rect(rect_new(position.x, position.y, player_width, player_height));
			acceleration.y = player_gravity;
			drag.x = player_drag_x;
			max_velocity = vec2_new(player_max_x, player_max_y);
			data.current = player_anim_idle;
			data.health = 5;
			group = player_group;
			break;
		case ENTITY_HOOK:
			bounds = shape_rect(rect_new(position.x, position.y, hook_width, hook_height));
			data.current = hook_anim;
			data.target_index = -1;
			group = player_group;
			break;
		case ENTITY_FISH:
			bounds = shape_circ(circ_new(position.x, position.y, fish_radius));
			max_velocity = vec2_new(fish_max_x, fish_max_y);
			data.current = fish_anim;
			acceleration.y = fish_gravity;
			data.health = 2;
			group = enemy_group;
			bounce = true;
			break;
		case ENTITY_PUFFER:
			bounds = shape_circ(circ_new(position.x, position.y, puffer_radius));

	}
	ArcadeObject obj = arcobj_new(bounds, solid);
	obj.acceleration = acceleration;
	obj.drag = drag;
	obj.max_velocity = max_velocity;
	obj.group = group;
	obj.bounce = bounce;
	size_t index = world_add(world, obj, &data);
	ArcadeObject *current = world_get(*world, index);
	if(type == ENTITY_PLAYER) {
		size_t hook_index = new_entity(world, position, ENTITY_HOOK);
		EntityData *player_data = world_get_data(*world, index);
		ArcadeObject *hook = world_get(*world, hook_index);
		EntityData *hook_data = world_get_data(*world, hook_index);
		hook->alive = false;
		hook->group = player_group;
		hook_data->parent_index = index;
		player_data->hook_index = hook_index;
	}
	return index;
}

void hurt(ArcadeObject *object, EntityData *data, int damage, int iframes) {
	if(data->health > 0 && data->iframes == 0) {
		data->health -= damage;
		data->iframes = iframes;
		if(data->health <= 0) {
			object->alive = false;
		}
	}
}

void update_fish(World world, ArcadeObject *obj, EntityData *data) {
	/*Rect bounds = shape_bounding_box(obj->bounds);
	bounds.x += obj->velocity.x;
	if(!world_region_free(world, shape_rect(bounds), obj)) {
		obj->velocity.x *= -1;
		obj->velocity.x += rand_num(-fish_variance, fish_variance);
		bounds.x += 2 * obj->velocity.x;
	}
	bounds.y += obj->velocity.y;
	if(!world_region_free(world, shape_rect(bounds), obj)) {
		obj->velocity.y *= -1.5;
		obj->velocity.x += rand_num(-fish_variance, fish_variance);
	}*/
	float rotation = vec2_angle(obj->velocity);
	if(rotation > 90 && rotation < 270) {
		data->flip_y = true;
	} else {
		data->flip_y = false;
	}
	shape_set_rotation(&obj->bounds, vec2_angle(obj->velocity));
}

void update(World world, ArcadeObject *obj, EntityData *data) {
	if(data->iframes > 0) {
		data->iframes--;
	}
	switch(data->type) {
		case ENTITY_PLAYER:
			update_player(world, obj, data);
			break;
		case ENTITY_HOOK:
			update_hook(world, obj, data);
			break;
		case ENTITY_FISH:
			update_fish(world, obj, data);
			break;
	}
}

void collide(World world, ArcadeObject *a, void *a_data_ptr, ArcadeObject *b, void *b_data_ptr) {
	EntityData *aData = a_data_ptr;
	EntityData *bData = b_data_ptr;
	if(aData->type == ENTITY_PLAYER) {
		EntityData *hookData = world_get_data(world, aData->hook_index);
		ArcadeObject *hookTarget = hookData->target_index == -1 ? NULL : world_get(world, hookData->target_index);
		if(hookTarget == NULL && bData->iframes == 0) {
			hurt(a, aData, 1, 60);
		} else {
			hurt(b, bData, 1, 30);
		}
	} else if(aData->type == ENTITY_HOOK && aData->target_index == -1) {
		aData->target_index = b->index;
		shape_set_position(&b->bounds, shape_get_position(a->bounds));
		a->velocity = vec2_new(0, 0);
		hurt(b, bData, 1, 0);
	}
}

void draw(ArcadeObject *obj, EntityData *data) {
	animation_next_tick(&data->current);	
	Vector2 position = shape_get_position(obj->bounds);
	Texture *current_texture = animation_get_current_frame(data->current);
	Rect bounds = current_texture->region;
	bounds.x = position.x;
	bounds.y = position.y;
	float rotation = shape_get_rotation(obj->bounds);
	Uint8 alpha = (data->iframes > 0) ? 0x88 : 0xff;
	animation_draw_ex(data->current, rend, bounds, rotation, data->flip_x, data->flip_y, alpha);
}

void frame(World world, ArcadeObject *obj, void *ptr) {
	update(world, obj, ptr);
	draw(obj, ptr);
}

#undef main
int main() {
	// *** INITIALIZATION ***
	srand(time(NULL)); //seed the RNG with the current system time
	config_load("../data/config.json"); //load the configuration
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	//Create window
	SDL_Window *window = SDL_CreateWindow("Hooks", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_Image could not be initialized! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	//Create renderer for window
	rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (rend == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	if( SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND)) {
		printf("Failed to set the blend mode! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	//Load the game animations
	player_anim_idle 	= animation_from_spritesheet(texture_new(load_texture(rend, "../img/player_idle.png")), 22, player_idle_animation_speed);
	player_anim_walk 	= animation_from_spritesheet(texture_new(load_texture(rend, "../img/player_walk.png")), 27, player_walk_animation_speed);
	player_anim_jump 	= animation_from_texture(texture_new(load_texture(rend, "../img/player_fall.png")));
	hook_anim 			= animation_from_texture(texture_new(load_texture(rend, "../img/hook.png")));
	fish_anim 			= animation_from_texture(texture_new(load_texture(rend, "../img/fish.png")));
	//Create the simulation world
	World world = world_new(640, 480, 96, sizeof(EntityData));
	TileMap map = tl_new(sizeof(SDL_Texture*), 640, 480, 32);
	player_group = world_add_group(&world, group_new());
	enemy_group = world_add_group(&world, group_new());
	group_blacklist_self(player_group);
	world_add_tilemap(&world, map);
	spawn_player(&world, vec2_new(0, 0));
	for(int i = 0; i < 10; i++) {
		new_entity(&world, vec2_new(i + 20, 100), ENTITY_FISH);
	}
	while(true) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					goto cleanup;
			}
		}
		SDL_SetRenderDrawColor(rend, 0x7e, 0xc0, 0xee, 0xFF);
		SDL_RenderClear(rend);
		world_update(world, 1, &frame, &collide);
		SDL_RenderPresent(rend);
		SDL_Delay(10);
	}
	cleanup:
		world_destroy(world);
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
}

