#include "../plugin_sdk/plugin_sdk.hpp"
#include "farm.h"

namespace farm
{

	bool cast_verify_range(script_spell* spell, game_object_script unit);

	bool cast_verify_range(script_spell* spell, game_object_script unit) {
		if (unit->get_position().distance(myhero->get_position()) <= spell->range())
		{
			return spell->cast(unit);
		} 
		else
		{
			return false;
		}
	}

}