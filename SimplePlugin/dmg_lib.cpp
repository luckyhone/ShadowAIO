#include "../plugin_sdk/plugin_sdk.hpp"
#include "dmg_lib.h"

namespace dmg_lib
{

	float calculate(float ad_dmg, float ap_dmg, float true_dmg, bool include_aa, game_object_script target)
	{

		damage_input input;
		input.raw_physical_damage = ad_dmg;
		input.raw_magical_damage = ap_dmg;
		input.raw_true_damage = true_dmg;

		float calculated = damagelib->calculate_damage_on_unit(myhero, target, &input);

		if (include_aa)
		{
			calculated += myhero->get_auto_attack_damage(target);
		}

		return calculated;
	}

	float get_damage(script_spell* spell, game_object_script target)
	{
		std::string spell_name = spell->name();

		// Kayle
		//
		if (spell_name == "KayleQ")
		{
			float dmg[] = { 60.0f, 100.0f, 140.0f, 180.0f, 220.0f };
			float base_magic_dmg = dmg[spell->level() - 1] + (0.6 * myhero->get_additional_attack_damage()) + (0.5 * myhero->get_total_ability_power());
			return calculate(0.0f, base_magic_dmg, 0.0f, false, target);
		}

		else if (spell_name == "KayleE")
		{
			float dmg[] = { 8.0f, 8.5f, 9.0f, 9.5f, 10.0f };
			float bonus_magic_dmg = dmg[spell->level() - 1] + (0.015 * myhero->get_total_ability_power());
			float missing_hp = target->get_max_health() - target->get_health();
			float missing_hp_dmg = (bonus_magic_dmg / 100) * missing_hp;

			if (target->is_monster() && missing_hp_dmg > 400.0f)
			{
				missing_hp_dmg = 400.0f;
			}

			//if (target->is_visible_on_screen())
			//{
			//	console->print("[Target] %s | [Bonus Magic DMG] %f | [Missing HP] %f | [Missing HP DMG] %f | [Total DMG] %f", target->get_model_cstr(), bonus_magic_dmg, missing_hp, missing_hp_dmg, calc);
			//}

			return calculate(0.0f, missing_hp_dmg, 0.0f, true, target);
		}

		else if (spell_name == "KayleR")
		{
			float dmg[] = { 200.0f, 350.0f, 500.0f };
			float base_magic_dmg = dmg[spell->level() - 1] + (myhero->get_additional_attack_damage()) + (0.8 * myhero->get_total_ability_power());
			return calculate(0.0f, base_magic_dmg, 0.0f, false, target);
		}

		// Miss Fortune
		//
		else if (spell_name == "MissFortuneRicochetShot")
		{
			float dmg[] = { 20.0f, 45.0f, 70.0f, 95.0f, 120.0f };
			float base_dmg = myhero->get_total_attack_damage();
			float base_magic_dmg = 0.35 * myhero->get_total_ability_power();
			float final_dmg = dmg[spell->level() - 1] + base_dmg + base_magic_dmg;
			return calculate(final_dmg, 0.0f, 0.0f, false, target);
		}

		else if (spell_name == "MissFortuneBulletTime")
		{
			float base_dmg = 0.75 * myhero->get_total_attack_damage();
			float base_magic_dmg = 0.20 * myhero->get_total_ability_power();
			float final_dmg = base_dmg + base_magic_dmg;
			return calculate(final_dmg, 0.0f, 0.0f, false, target);
		}

		// Default, not included in my dmglib
		//
		return damagelib->get_spell_damage(myhero, target, spell->slot, false);
	}
}