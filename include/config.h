#pragma once

float player_jump, player_walk, player_gravity, player_gravity_hold, player_width, player_height, player_drag_x, player_max_x, player_max_y,
	  player_reel_max_x, player_reel_max_y;
float hook_width, hook_height, hook_speed, hook_reel_speed;
float fish_radius, fish_gravity, fish_leap, fish_variance, fish_max_x, fish_max_y;
int player_idle_animation_speed, player_walk_animation_speed;
void config_load(char *filename);
