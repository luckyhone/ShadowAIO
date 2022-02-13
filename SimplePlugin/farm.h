#include "../plugin_sdk/plugin_sdk.hpp"

#pragma once
namespace farm
{

	bool cast_verify_range(script_spell* spell, game_object_script unit);
	bool cast_verify_range(script_spell* spell, game_object_script unit, hit_chance hitchance);
};

