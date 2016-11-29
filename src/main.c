#include "arcade.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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

void update(ArcadeObject *obj) {

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
	ArcadeObject obj = arcobj_new(shape_rect(rect_new(0, 0, 32, 32)), false, texture);
	obj.acceleration.y = 0.5f;
	world_add(&world, obj);
	SDL_Event event;
	ArcadeObject *ptr = qt_get(world.entities, 0);
	while(true) {
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
			SDL_Rect rect = {bounds.x, bounds.y, bounds.width, bounds.height};
			SDL_RenderCopy(rend, obj->data, NULL, &rect);
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

