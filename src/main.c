#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

SDL_Texture* load_texture(SDL_Renderer *rend, char *path)
{
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

int main() {
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
	SDL_Event event;
	while(true) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					goto cleanup;
			}
		}
		SDL_RenderClear(rend);
	}
	cleanup:
		//Initialize renderer color
		SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
}

