#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "arcade.h"
#include "parson.h"
#include <SDL.h>

#include "config.h"
#include "data.h"
#include "enemy.h"
#include "player.h"
#include "textures.h"

Animation player_anim_idle, player_anim_walk, player_anim_jump, hook_anim, fish_anim;
SDL_Renderer *rend;
Group *player_group, *enemy_group;

void hurt(ArcadeObject *object, EntityData *data, int damage, int iframes) {
	if(data->health > 0 && data->iframes == 0) {
		data->health -= damage;
		data->iframes = iframes;
		if(data->health <= 0) {
			object->alive = false;
		}
	}
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
	if(data->type == ENTITY_PLAYER) {
		for(int i = 0; i < data->health; i++) {
			animation_draw_ex(hook_anim, rend, rect_new(32 + i * 48, 32, 32, 18), 45, false, true, 0xff);
		}
	}
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
	Texture tex = texture_new(load_texture(rend, "../img/floor.png"));
	SpatialMap map = sm_new(sizeof(Texture), 640, 480, 32, 32);
	sm_set(&map, &tex, 300, 300);
	player_group = world_add_group(&world, group_new());
	enemy_group = world_add_group(&world, group_new());
	group_blacklist_self(player_group);
	world_add_map(&world, map);
	spawn_player(&world, vec2_new(0, 0));
	for(int i = 0; i < 10; i++) {
		spawn_fish(&world, vec2_new(i + 20, 100));
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
		for(int x = 0; x < map.width; x += map.tile_width) {
			for(int y = 0; y < map.height; y += map.tile_height) {
				if(sm_has(map, x, y)) {
					Texture *tex = sm_get(map, x, y);
					if(tex->texture != NULL) {
						texture_draw(*tex, rend, rect_new(x, y, tex->region.width, tex->region.height));
					}
				}
			}
		}
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

