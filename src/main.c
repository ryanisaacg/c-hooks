#include "arcade.h"
#include "parson.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

SDL_Texture *player_texture, *hook_texture;
float player_jump, player_walk, player_gravity, player_gravity_hold, player_width, player_height, player_drag_x, player_max_x, player_max_y;
float hook_width, hook_height, hook_speed;
SDL_Renderer *rend;

typedef enum EntityType {ENTITY_PLAYER, ENTITY_HOOK} EntityType;

typedef struct {
	SDL_Texture *texture;
	enum EntityType type;
	union {
		struct { ArcadeObject *hook; } player;
		struct { ArcadeObject *parent, *target; } hook;
	} specific;
} EntityData;

ArcadeObject *new_entity(World *world, Vector2 position, EntityType type) {
	EntityData *data = malloc(sizeof(*data));
	data->type = type;
	Shape bounds;
	Vector2 acceleration = vec2_new(0, 0), drag = vec2_new(0, 0), max_velocity = vec2_new(0, 0);
	bool solid = false;
	switch(type) {
		case ENTITY_PLAYER:
			bounds = shape_rect(rect_new(position.x, position.y, player_width, player_height));
			acceleration.y = player_gravity;
			drag.x = player_drag_x;
			max_velocity = vec2_new(player_max_x, player_max_y);
			data->texture = player_texture;
			break;
		case ENTITY_HOOK:
			bounds = shape_rect(rect_new(position.x, position.y, hook_width, hook_height));
			data->texture = hook_texture;
			data->specific.hook.target = NULL;
			data->specific.hook.parent = NULL;
			break;
	}
	ArcadeObject obj = arcobj_new(bounds, solid, data);
	obj.acceleration = acceleration;
	obj.drag = drag;
	obj.max_velocity = max_velocity;
	size_t index = world_add(world, obj);
	ArcadeObject *current = world_get(*world, index);
	if(type == ENTITY_PLAYER) {
		ArcadeObject *hook = new_entity(world, position, ENTITY_HOOK);
		hook->alive = false;
		EntityData *hook_data = hook->data;
		data->specific.hook.parent = current;
		EntityData *player_data = current->data;
		player_data->specific.player.hook = hook;
	}
	return current;
}

SDL_Texture* load_texture(SDL_Renderer *rend, char *path) {
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image error: %s\n", path, IMG_GetError());
	} else {
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(rend, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}
	return newTexture;
}

void update(World world, ArcadeObject *obj) {
	EntityData *data = obj->data;
	if(data->type == ENTITY_PLAYER) {
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		bool leftPressed = keys[SDL_SCANCODE_A];
		bool rightPressed = keys[SDL_SCANCODE_D];
		bool jumpPressed = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W];
		if(leftPressed ^ rightPressed) {
			if(leftPressed) {
				obj->acceleration.x = -player_walk;
			} else  {
				obj->acceleration.x = player_walk;
			}
		} else {
			obj->acceleration.x = 0;
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
	}
}

void draw(ArcadeObject *obj) {
	Rect bounds = shape_bounding_box(obj->bounds);
	EntityData *data = obj->data;
	SDL_Rect rect = {bounds.x, bounds.y, bounds.width, bounds.height};
	SDL_RenderCopy(rend, data->texture, NULL, &rect);
}

void frame(World world, ArcadeObject *obj) {
	update(world, obj);
	draw(obj);
}

void collide(ArcadeObject *a, ArcadeObject *b) {

}

#undef main
int main() {
	// *** INITIALIZATION ***
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
	//Load the player texture
	player_texture 	= load_texture(rend, "../img/player.png");
	hook_texture 	= load_texture(rend, "../img/player.png");
	//Load the config file
	JSON_Object *config 		= json_value_get_object(json_parse_file("../data/config.json"));
	JSON_Object *player_config 	= json_object_get_object(config, "player");
	JSON_Object *hook_config 	= json_object_get_object(config, "hook");
	//Global values
	player_walk 		= json_object_get_number(player_config, "walk"); 
	player_jump 		= json_object_get_number(player_config, "jump");
	player_gravity 		= json_object_get_number(player_config, "gravity"); 
	player_gravity_hold = json_object_get_number(player_config, "hold-gravity");
	player_width 		= json_object_get_number(player_config, "width");
	player_height 		= json_object_get_number(player_config, "height");
	player_drag_x 		= json_object_get_number(player_config, "drag-x");
	player_max_x 		= json_object_get_number(player_config, "max-x");
	player_max_y 		= json_object_get_number(player_config, "max-y");
	hook_width			= json_object_get_number(hook_config, "width");
	hook_height			= json_object_get_number(hook_config, "height");
	hook_speed			= json_object_get_number(hook_config, "speed");
	//Create the simulation world
	World world = world_new(640, 480, 96);
	TileMap map = tl_new(sizeof(SDL_Texture*), 640, 480, 32);
	world_add_tilemap(&world, map);
	new_entity(&world, vec2_new(0, 0), ENTITY_PLAYER);
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

