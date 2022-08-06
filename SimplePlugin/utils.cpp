#include "../plugin_sdk/plugin_sdk.hpp"
#include "utils.h"

#pragma once
namespace utils
{

	bool has_unkillable_buff(game_object_script target)
	{
		return target->is_zombie() || target->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") });;
	}

	bool has_untargetable_buff(game_object_script target)
	{
		return target->has_buff({ buff_hash("JaxCounterStrike"), buff_hash("ShenW") });;
	}

	bool has_crowd_control_buff(game_object_script target)
	{
		return target->has_buff_type({
			buff_type::Stun,
			buff_type::Knockup,
			buff_type::Asleep,
			buff_type::Berserk,
			buff_type::Charm,
			buff_type::Flee,
			buff_type::Fear, 
			buff_type::Snare, 
			buff_type::Suppression, 
			buff_type::Polymorph,
			buff_type::Taunt
		});
	}
};

