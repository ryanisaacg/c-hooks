#pragma once

#include "arcade.h"
#include "SDL.h"
#include <SDL_image.h>

typedef struct {
	Rect region;
	SDL_Texture *texture;
	Vector2 origin;
} Texture;

typedef struct {
	ArrayList frames;
	int ticks_per_frame, current_ticks, current_frame;
} Animation;

SDL_Texture *load_texture(SDL_Renderer *rend, char *path);
Texture texture_new(SDL_Texture *texture);
Texture texture_new_region(SDL_Texture *texture, Rect region);
Texture texture_get_subtexture(Texture texture, Rect region);
void texture_draw(Texture texture, SDL_Renderer *rend, Rect destination);
void texture_draw_ex(Texture texture, SDL_Renderer *rend, Rect destination, double angle, bool fip_x, bool flip_y);
Animation animation_new(int ticks_per_frame);
Animation animation_from_texture(Texture tex);
Animation animation_from_array(Texture *array, int length, int ticks_per_frame);
Animation animation_from_spritesheet(Texture tex, int frame_width, int ticks_per_frame);
void animation_add_frame(Animation *anim, Texture frame);
Texture *animation_get_current_frame(Animation anim);
void animation_next_tick(Animation *anim);
void animation_draw(Animation anim, SDL_Renderer *rend, Rect destination);
void animation_draw_ex(Animation anim, SDL_Renderer *rend, Rect destination, double angle, bool fip_x, bool flip_y);
