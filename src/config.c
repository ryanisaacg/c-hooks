#include "config.h"
#include "parson.h"

void config_load(char *filename) {
	//Load the config file
	JSON_Object *config 		= json_value_get_object(json_parse_file(filename));
	JSON_Object *player_config 	= json_object_get_object(config, "player");
	JSON_Object *hook_config 	= json_object_get_object(config, "hook");
	JSON_Object *fish_config	= json_object_get_object(config, "fish");
	//Global values
	player_walk 				= json_object_get_number(player_config, "walk"); 
	player_jump 				= json_object_get_number(player_config, "jump");
	player_gravity 				= json_object_get_number(player_config, "gravity"); 
	player_gravity_hold 		= json_object_get_number(player_config, "hold-gravity");
	player_width 				= json_object_get_number(player_config, "width");
	player_height 				= json_object_get_number(player_config, "height");
	player_drag_x 				= json_object_get_number(player_config, "drag-x");
	player_max_x 				= json_object_get_number(player_config, "max-x");
	player_max_y 				= json_object_get_number(player_config, "max-y");
	player_reel_max_x			= json_object_get_number(player_config, "reel-max-x"); 
	player_reel_max_y			= json_object_get_number(player_config, "reel-max-y"); 
	player_idle_animation_speed = json_object_get_number(player_config, "idle-anim-speed");
	player_walk_animation_speed = json_object_get_number(player_config, "walk-anim-speed");
	hook_width					= json_object_get_number(hook_config, "width");
	hook_height					= json_object_get_number(hook_config, "height");
	hook_speed					= json_object_get_number(hook_config, "speed");
	hook_reel_speed				= json_object_get_number(hook_config, "reel");
	fish_radius					= json_object_get_number(fish_config, "radius");
	fish_gravity				= json_object_get_number(fish_config, "gravity");
	fish_leap					= json_object_get_number(fish_config, "leap");
	fish_variance				= json_object_get_number(fish_config, "variance");
	fish_max_x					= json_object_get_number(fish_config, "max-x");
	fish_max_y					= json_object_get_number(fish_config, "max-y");
}
