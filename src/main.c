#include "arcade.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	SDL_Texture *texture;
	enum {PLAYER} type;
} EntityData;

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

static inline float clamp(float value, float high, float low) {
	if(value > high) return high;
	else if(value < low) return low;
	else return value;
}

void update(ArcadeObject *obj) {
	EntityData *data = obj->data;
	if(data->type == PLAYER) {
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		bool leftPressed = keys[SDL_SCANCODE_A];
		bool rightPressed = keys[SDL_SCANCODE_D];
		if(leftPressed ^ rightPressed) {
			if(leftPressed) {
				obj->acceleration.x = -0.01f;
			} else  {
				obj->acceleration.x = 0.01f;
			}
			printf("%f:%f, %f:%f\n", obj->velocity.x, obj->velocity.y, obj->acceleration.x, obj->acceleration.y);
		}
		obj->velocity.x = clamp(obj->velocity.x, -1, 1);
	}
}

void collide(ArcadeObject *a, ArcadeObject *b) {

}

int main() {
	// *** INITIALIZATION ***
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	//Create window
	SDL_Window *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
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
	SDL_Renderer *rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (rend == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
	//Load the player texture
	SDL_Texture *texture = load_texture(rend, "../img/player.png");
	//Create the simulation world
	World world = world_new(640, 480, 96);
	TileMap map = tl_new(sizeof(SDL_Texture*), 640, 480, 32);
	world_add_tilemap(&world, map);
	EntityData *data = malloc(sizeof(data));
	*data = (EntityData){ texture, PLAYER };
	ArcadeObject obj = arcobj_new(shape_rect(rect_new(0, 0, 32, 32)), false, data);
	obj.acceleration.y = 0.5f;
	world_add(&world, obj);
	while(true) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					goto cleanup;
			}
		}
		world_update(world, 1, &update, &collide);
		SDL_RenderClear(rend);
		for(size_t i = 0; i < qt_len(world.entities); i++) {
			ArcadeObject *obj = world_get(world, i);
			Rect bounds = shape_bounding_box(obj->bounds);
			EntityData *data = obj->data;
			SDL_Rect rect = {bounds.x, bounds.y, bounds.width, bounds.height};
			SDL_RenderCopy(rend, data->texture, NULL, &rect);
		}
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

