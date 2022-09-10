#include "../plugin_sdk/plugin_sdk.hpp"
#include "utils.h"

#pragma once
namespace utils
{

	void on_load()
	{
		std::string msg = "<b><font color=\"#7289da\">[ShadowAIO]</font></b><font color=\"#FFFFFF\">: Loaded champion</font> <b><font color=\"#7289da\">" + myhero->get_model() + "</font></b>";
		myhero->print_chat(1, msg.c_str());
		msg = "<b><font color=\"#7289da\">[ShadowAIO]</font></b><font color=\"#FFFFFF\">: Bugs or suggestions please send on Discord - </font> <b><font color=\"#7289da\">racism gaming#0375</font></b>";
		myhero->print_chat(1, msg.c_str());
	}

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

	bool fast_cast(script_spell* spell)
	{
		console->print("[%f] FAST CAST (SPELL) %s", gametime->get_time(), spell->name().c_str());
		myhero->cast_spell(spell->slot, true, spell->is_charged_spell);
		return true;
	}

	bool fast_cast(script_spell* spell, vector position)
	{
		console->print("[%f] FAST CAST (SPELL, POSITION) %s", gametime->get_time(), spell->name().c_str());
		if (!spell->is_charged_spell)
		{
			myhero->cast_spell(spell->slot, position);
			return true;
		}
		if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
		{
			myhero->update_charged_spell(spell->slot, position, true);
			return true;
		}
		return spell->start_charging();
	}

	bool fast_cast(script_spell* spell, game_object_script unit, hit_chance minimum, bool aoe, int min_targets)
	{
		console->print("[%f] FAST CAST (FARM) %s", gametime->get_time(), spell->name().c_str());

		vector cast_position;

		prediction_input x;

		if (!spell->from.is_valid())
			x._from = myhero->get_position();
		else
			x._from = spell->from;

		x.unit = unit;
		x.delay = spell->delay;
		x.radius = spell->radius;
		x.speed = spell->speed;
		x.collision_objects = spell->collision_flags;
		x.range = spell->range();
		x.type = spell->type;
		x.aoe = aoe;
		x.spell_slot = spell->slot;
		x.use_bounding_radius = spell->type != skillshot_type::skillshot_circle;

		auto output = prediction->get_prediction(&x);

		if (output.hitchance >= minimum && output.aoe_targets_hit_count() >= min_targets)
		{
			cast_position = output.get_cast_position();

			if (!spell->is_charged_spell)
			{
				myhero->cast_spell(spell->slot, cast_position);
				return true;
			}

			if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
			{
				myhero->update_charged_spell(spell->slot, cast_position, true);
				return true;
			}
		}
		return false;
	}

	bool fast_cast(script_spell* spell, int minMinions, bool is_jugnle_mobs)
	{
		auto best_pos = spell->get_cast_on_best_farm_position(minMinions, is_jugnle_mobs);

		if (best_pos.is_valid())
		{
			if (!spell->is_charged_spell)
			{
				myhero->cast_spell(spell->slot, best_pos);
				return true;
			}

			if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
			{
				myhero->update_charged_spell(spell->slot, best_pos, true);
				return true;
			}
		}

		return false;
	}
};

