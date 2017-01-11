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
#include "objects.h"
#include "player.h"

Animation player_anim_idle, player_anim_walk, player_anim_jump, fish_anim, block_anim;
TextureRegion hook_tex;

SDL_Renderer *rend;
Group *player_group, *enemy_group, *object_group;

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

void frame(World world, ArcadeObject *obj, void *ptr) {
	update(world, obj, ptr);
}

Animation animation_from_texture(TextureRegion region) {
	TextureRegion *regions = malloc(sizeof(TextureRegion));
	*regions = region;
	return anim_new(regions, 1, 1);
}

#undef main
int main() {
	// *** INITIALIZATION ***
	srand(time(NULL)); //seed the RNG with the current system time
	config_load("../data/config.json"); //load the configuration
	Window window = window_new(window_config_new(640, 480, "Example"));
	//Load the game animations
	player_anim_idle 	= anim_new_linear_sheet(texregion_new(texture_new(window, "../img/player_idle.png")), 22, player_idle_animation_speed);
	player_anim_walk 	= anim_new_linear_sheet(texregion_new(texture_new(window, "../img/player_walk.png")), 27, player_walk_animation_speed);
	player_anim_jump 	= animation_from_texture(texregion_new(texture_new(window, "../img/player_fall.png")));
	hook_tex 			= texregion_new(texture_new(window, "../img/hook.png"));
	fish_anim 			= animation_from_texture(texregion_new(texture_new(window, "../img/fish.png")));
	block_anim			= animation_from_texture(texregion_new(texture_new(window, "../img/crate.png")));
	//Create the simulation world
	World world = world_new(&window, 640, 480, 96, sizeof(EntityData));
	Texture tex = texture_new(window, "../img/rock.png");
	SpatialMap map = sm_new(sizeof(Texture), 640, 480, 32, 32);
	sm_set(&map, &tex, 300, 300);
	player_group = world_add_group(&world, group_new());
	enemy_group = world_add_group(&world, group_new());
	object_group = world_add_group(&world, group_new());
	group_blacklist_self(player_group);
	world_add_map(&world, map);
	spawn_player(&world, vec2_new(0, 0));
	for(int i = 0; i < 10; i++) {
		spawn_fish(&world, vec2_new(i + 20, 100));
	}
	spawn_block(&world, vec2_new(400, 0));
	while(true) {
		window_events(&window);
		world_update(world, 1, &frame, &collide);
		window_start_draw(&window, 0, 0, 0xff);
		world_draw(world);
		window_end_draw(window);
	}
	world_destroy(world);
	window_destroy(window);
}

