#include "arcade.h"
#include "config.h"
#include "parson.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "textures.h"
#include <time.h>

Animation player_anim_idle, player_anim_walk, hook_anim, fish_anim;
SDL_Renderer *rend;
Group *player_group, *enemy_group;

typedef enum EntityType {ENTITY_PLAYER, ENTITY_HOOK, ENTITY_FISH} EntityType;

typedef struct {
	Animation current;
	EntityType type;
	int health, iframes;
	bool flip_x, flip_y;
	union {
		struct { ArcadeObject *hook; } player;
		struct { ArcadeObject *parent, *target; } hook;
		struct { } fish;
	} specific;
} EntityData;

float rand_num(float min, float max) {
	float diff = max - min;
	float value = rand() * diff / RAND_MAX;
	return value + min;
}

ArcadeObject *new_entity(World *world, Vector2 position, EntityType type) {
	EntityData *data = malloc(sizeof(*data));
	data->iframes = 0;
	data->type = type;
	data->flip_x = false;
	data->flip_y = false;
	Shape bounds;
	Vector2 acceleration = vec2_new(0, 0), drag = vec2_new(0, 0), max_velocity = vec2_new(-1, -1);
	Group *group = NULL;
	bool solid = false;
	switch(type) {
		case ENTITY_PLAYER:
			bounds = shape_rect(rect_new(position.x, position.y, player_width, player_height));
			acceleration.y = player_gravity;
			drag.x = player_drag_x;
			max_velocity = vec2_new(player_max_x, player_max_y);
			data->current = player_anim_idle;
			data->health = 5;
			group = player_group;
			break;
		case ENTITY_HOOK:
			bounds = shape_rect(rect_new(position.x, position.y, hook_width, hook_height));
			data->current = hook_anim;
			data->specific.hook.target = NULL;
			data->specific.hook.parent = NULL;
			data->health = -1;
			group = player_group;
			break;
		case ENTITY_FISH:
			bounds = shape_circ(circ_new(position.x, position.y, fish_radius));
			max_velocity = vec2_new(fish_max_x, fish_max_y);
			data->current = fish_anim;
			acceleration.y = fish_gravity;
			data->health = 2;
			group = enemy_group;
			break;
	}
	ArcadeObject obj = arcobj_new(bounds, solid, data);
	obj.acceleration = acceleration;
	obj.drag = drag;
	obj.max_velocity = max_velocity;
	obj.group = group;
	size_t index = world_add(world, obj);
	ArcadeObject *current = world_get(*world, index);
	if(type == ENTITY_PLAYER) {
		ArcadeObject *hook = new_entity(world, position, ENTITY_HOOK);
		hook->alive = false;
		hook->group = player_group;
		EntityData *hook_data = hook->data;
		data->specific.hook.parent = current;
		EntityData *player_data = current->data;
		player_data->specific.player.hook = hook;
	}
	return current;
}

void hurt(ArcadeObject *object, int damage, int iframes) {
	EntityData *data = object->data;
	if(data->health > 0 && data->iframes == 0) {
		data->health -= damage;
		data->iframes = iframes;
		if(data->health <= 0) {
			object->alive = false;
		}
	}
}

void update_player(World world, ArcadeObject *obj) {
	EntityData *data = obj->data;
	ArcadeObject *hook = data->specific.player.hook;
	EntityData *hook_data = hook->data;
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
	} else if(hook_data->specific.hook.target == NULL 
			&& world_region_free(world, shape_rect(shape_bounding_box(hook->bounds)), hook)) {
		Rect bounds = shape_bounding_box(hook->bounds);
		bounds.x += hook->velocity.x;
		bounds.y += hook->velocity.y;
		if(!world_region_free(world, shape_rect(bounds), hook)) {
			shape_set_position(&hook->bounds, vec2_add(hook->velocity, shape_get_position(hook->bounds)));
		}
	} else {
		Vector2 diff = vec2_sub(shape_get_position(hook->bounds), shape_get_position(obj->bounds));
		if(vec2_len(diff) <= 90) {
			if(hook_data->specific.hook.target != NULL) {
				shape_set_position(&hook_data->specific.hook.target->bounds, shape_get_position(hook->bounds));
			}
			hook_data->specific.hook.target = NULL;
			hook->alive = false;
			obj->max_velocity = vec2_new(player_max_x, player_max_y);
		} else {
			obj->velocity = vec2_with_len(diff, hook_reel_speed);
			obj->max_velocity = vec2_new(player_reel_max_x, player_reel_max_y);
		}
	}
	Rect region = shape_bounding_box(obj->bounds);
	region.y += 1;
	if(jumpPressed && !world_region_free(world, shape_rect(region), obj)) {
		obj->velocity.y = -player_jump;
		obj->acceleration.y = player_gravity_hold;
	}
	if(!jumpPressed) {
		obj->acceleration.y = player_gravity;
	}
	//Restore animation progress
	data->current.current_frame = frame % data->current.frames.length;
	data->current.current_ticks = progress;
}

void update_hook(ArcadeObject *obj) {
	EntityData *data = obj->data;
	ArcadeObject *target = data->specific.hook.target;
	if(target != NULL) {
		shape_set_position(&target->bounds, shape_get_position(obj->bounds));
		target->velocity = obj->velocity;
	}
}

void update_fish(World world, ArcadeObject *obj) {
	Rect bounds = shape_bounding_box(obj->bounds);
	bounds.x += obj->velocity.x;
	if(!world_region_free(world, shape_rect(bounds), obj)) {
		obj->velocity.x *= -1;
		obj->velocity.x += rand_num(-fish_variance, fish_variance);
		bounds.x += 2 * obj->velocity.x;
	}
	bounds.y += obj->velocity.y;
	if(!world_region_free(world, shape_rect(bounds), obj)) {
		obj->velocity.y *= -1;
		obj->velocity.x += rand_num(-fish_variance, fish_variance);
	}
}

void update(World world, ArcadeObject *obj) {
	EntityData *data = obj->data;
	if(data->iframes > 0) {
		data->iframes--;
	}
	switch(data->type) {
		case ENTITY_PLAYER:
			update_player(world, obj);
			break;
		case ENTITY_HOOK:
			update_hook(obj);
			break;
		case ENTITY_FISH:
			update_fish(world, obj);
			break;
	}
}

void collide(ArcadeObject *a, ArcadeObject *b) {
	EntityData *aData = a->data;
	EntityData *bData = b->data;
	if(aData->type == ENTITY_PLAYER) {
		EntityData *hookData = aData->specific.player.hook->data;
		ArcadeObject *hookTarget = hookData->specific.hook.target;
		if(hookTarget == NULL && bData->iframes == 0) {
			hurt(a, 1, 60);
		} else {
			hurt(b, 1, 30);
		}
	} else if(aData->type == ENTITY_HOOK && aData->specific.hook.target == NULL) {
		aData->specific.hook.target = b;
		shape_set_position(&b->bounds, shape_get_position(a->bounds));
		a->velocity = vec2_new(0, 0);
		hurt(b, 1, 0);
	}
}

void draw(ArcadeObject *obj) {
	EntityData *data = obj->data;
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

void frame(World world, ArcadeObject *obj) {
	update(world, obj);
	draw(obj);
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
	//Load the player texture
	player_anim_idle = animation_from_spritesheet(texture_new(load_texture(rend, "../img/player_idle.png")), 22, player_idle_animation_speed);
	player_anim_walk = animation_from_spritesheet(texture_new(load_texture(rend, "../img/player_walk.png")), 27, player_walk_animation_speed);
	hook_anim 	= animation_from_texture(texture_new(load_texture(rend, "../img/hook.png")));
	fish_anim 	= animation_from_texture(texture_new(load_texture(rend, "../img/fish.png")));
	//Create the simulation world
	World world = world_new(640, 480, 96);
	TileMap map = tl_new(sizeof(SDL_Texture*), 640, 480, 32);
	player_group = world_add_group(&world, group_new());
	enemy_group = world_add_group(&world, group_new());
	group_blacklist_self(player_group);
	world_add_tilemap(&world, map);
	new_entity(&world, vec2_new(0, 0), ENTITY_PLAYER);
	new_entity(&world, vec2_new(100, 100), ENTITY_FISH);
	while(true) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					goto cleanup;
			}
		}
		SDL_RenderClear(rend);
		world_update(world, 1, &frame, &collide);
		SDL_RenderPresent(rend);
		SDL_Delay(10);
	}
	cleanup:
		//Initialize renderer color
		SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
}

