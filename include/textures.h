#pragma once

#include "arcade.h"
#include "SDL.h"

typedef struct {
	Rect region;
	SDL_Texture *texture;
} Texture;

typedef struct {
	ArrayList frames;
	int ticks_per_frame, current_ticks, current_frame;
} Animation;

Texture texture_new(SDL_Texture *texture, Rect region);
void texture_draw(Texture texture, SDL_Renderer *rend, Rect destination);
Animation animation_new(int ticks_per_frame);
Animation animation_from_texture(Texture tex);
Animation animation_from_array(Texture *array, int length, int ticks_per_frame);
void animation_add_frame(Animation *anim, Texture frame);
void animation_next_tick(Animation *anim);
void animation_draw(Animation *anim, SDL_Renderer *rend, Rect destination);
