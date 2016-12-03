#include <stdlib.h>
#include "textures.h"

Texture texture_new(SDL_Texture *texture, Rect region) {
	return (Texture) { region, texture };
}
void texture_draw(Texture texture, SDL_Renderer *rend, Rect dest) {
	if(SDL_RenderCopy(rend, texture.texture, &texture.region, &dest) != 0) {
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
