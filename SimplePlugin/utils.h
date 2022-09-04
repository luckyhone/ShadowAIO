#include "../plugin_sdk/plugin_sdk.hpp"

#pragma once
namespace utils
{

	// Buff checks
	//
	bool has_unkillable_buff(game_object_script target);
	bool has_untargetable_buff(game_object_script target);
	bool has_crowd_control_buff(game_object_script target);

	// Fast spell casting
	//
	bool fast_cast(script_spell* spell);
	bool fast_cast(script_spell* spell, vector position);
	bool fast_cast(script_spell* spell, game_object_script unit, hit_chance minimum, bool aoe, int min_targets);
	bool fast_cast(script_spell* spell, int minMinions, bool is_jugnle_mobs);
};

