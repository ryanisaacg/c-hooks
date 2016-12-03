#include <stdlib.h>
#include "textures.h"

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
Texture texture_new(SDL_Texture *texture) {
	int width, height;
	if(SDL_QueryTexture(texture, NULL, NULL, &width, &height) != 0) {
		printf("Unable to query texture! SDL Error: %s\n", SDL_GetError());
	}
	Rect region = {0, 0, width, height};
	return texture_new_region(texture, region); 
}
Texture texture_new_region(SDL_Texture *texture, Rect region) {
	return (Texture) { region, texture };
}
void texture_draw(Texture texture, SDL_Renderer *rend, Rect dest) {
	SDL_Rect destination = { (int)dest.x, (int)dest.y, (int)dest.width, (int)dest.height };
	if(SDL_RenderCopy(rend, texture.texture, &texture.region, &destination) != 0) {
		printf("RenderCopy call failed! SDL Error: %s\n", SDL_GetError());
		exit(-1);
	}
}
Animation animation_new(int ticks_per_frame) {
	return (Animation) { al_new(sizeof(Texture)), ticks_per_frame, 0 };
}
Animation animation_from_texture(Texture tex) {
	Animation anim = animation_new(1);
	animation_add_frame(&anim, tex);
	return anim;
}
Animation animation_from_array(Texture *array, int length, int ticks_per_frame) {
	Animation anim = animation_new(ticks_per_frame);
	for(int i = 0; i < length; i++) {
		animation_add_frame(&anim, array[i]);
	}
	return anim;
}
void animation_add_frame(Animation *anim, Texture frame) {
	al_add(&anim->frames, &frame);
}
void animation_next_tick(Animation *anim) {
	anim->current_ticks++;
	if(anim->current_ticks > anim->ticks_per_frame) {
		anim->current_ticks = 0;
		anim->current_frame++;
		if(anim->current_frame > anim->frames.length) {
			anim->current_frame = 0;
		}
	}
}
void animation_draw(Animation *anim, SDL_Renderer *rend, Rect dest) {
	Texture *current = al_get(anim->frames, anim->current_frame);
	texture_draw(*current, rend, dest);
}
