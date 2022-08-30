#include "../plugin_sdk/plugin_sdk.hpp"

#pragma once
namespace dmg_lib
{

	float calculate(float ad_dmg, float ap_dmg, float true_dmg, bool include_aa, game_object_script target);

	float get_damage(script_spell* spell, game_object_script target);
};

