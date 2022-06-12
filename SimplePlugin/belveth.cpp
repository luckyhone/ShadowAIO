#include "../plugin_sdk/plugin_sdk.hpp"
#include "belveth.h"
#include "permashow.hpp"

namespace belveth
{

	// To declare a spell, it is necessary to create an object and registering it in load function
	script_spell* q = nullptr;
	script_spell* w = nullptr;
	script_spell* e = nullptr;
	script_spell* r = nullptr;

	// Declaration of menu objects
	TreeTab* main_tab = nullptr;

	namespace draw_settings
	{
		TreeEntry* draw_range_q = nullptr;
		TreeEntry* q_color = nullptr;
		TreeEntry* draw_range_w = nullptr;
		TreeEntry* w_color = nullptr;
		TreeEntry* draw_range_e = nullptr;
		TreeEntry* e_color = nullptr;
		TreeEntry* draw_range_r = nullptr;
		TreeEntry* r_color = nullptr;
	}

	namespace combo
	{
		TreeEntry* allow_tower_dive = nullptr;
		TreeEntry* use_q = nullptr;
		TreeEntry* q_mode = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* e_cancel_if_nobody_inside = nullptr;
		TreeEntry* e_max_range = nullptr;
		TreeEntry* use_r = nullptr;
	}

	namespace harass
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_e = nullptr;
	}

	namespace laneclear
	{
		TreeEntry* spell_farm = nullptr;
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* e_minimum_minions = nullptr;
	}

	namespace jungleclear
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_e = nullptr;
	}

	namespace fleemode
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
	}

	namespace hitchance
	{
		TreeEntry* q_hitchance = nullptr;
		TreeEntry* w_hitchance = nullptr;
	}


	// Event handler functions
	void on_update();
	void on_draw();
	void on_after_attack_orbwalker(game_object_script target);

	// Declaring functions responsible for spell-logic
	//
	void q_logic();
	void w_logic();
	void e_logic();
	void r_logic();

	// Utils
	//
	hit_chance get_hitchance(TreeEntry* entry);

	// Champion data
	//
	float q_speeds[] = { 800.0f, 850.0f, 900.0f, 950.0f, 1000.0f };

	void load()
	{
		// Registering a spells
		//
		q = plugin_sdk->register_spell(spellslot::q, 400);
		q->set_skillshot(0.0f, 100.f, 800.0f, { collisionable_objects::walls }, skillshot_type::skillshot_line);
		w = plugin_sdk->register_spell(spellslot::w, 660);
		w->set_skillshot(0.75f, 200.0f, FLT_MAX, { }, skillshot_type::skillshot_line);
		e = plugin_sdk->register_spell(spellslot::e, 500);
		r = plugin_sdk->register_spell(spellslot::r, 500);


		// Create a menu according to the description in the "Menu Section"
		//
		main_tab = menu->create_tab("belveth", "Bel'Veth");
		main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
		{
			// Info
			//
			main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

			auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
			{
				combo::allow_tower_dive = combo->add_hotkey(myhero->get_model() + ".combo.allow_tower_dive", "Allow Tower Dive", TreeHotkeyMode::Toggle, 'A', true);
				combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
				combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				auto q_config = combo->add_tab(myhero->get_model() + ".combo.q.config", "Q Config");
				{
					combo::q_mode = q_config->add_combobox(myhero->get_model() + ".combo.q.mode", "Q Mode", { {"In Combo", nullptr},{"After AA", nullptr } }, 1);
				}
				combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
				combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
				combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
				{
					combo::e_cancel_if_nobody_inside = e_config->add_checkbox(myhero->get_model() + ".combo.e.cancel_if_nobody_inside", "Cancel E if nobody inside", true);
					combo::e_max_range = e_config->add_slider(myhero->get_model() + ".combo.e.max_range", "E maximum range", 400, 1, e->range());
				}
				combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
				combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
			}

			auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
			{
				harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
				harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
				harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", false);
				harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
			{
				laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
				laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
				laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", true);
				laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
				laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
				{
					laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "E minimum minions", 3, 1, 5);
				}
			}

			auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
			{
				jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
				jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", true);
				jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
				jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
			{
				fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "Use Q to dash", true);
				fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to slow enemies", true);
				fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
			}

			auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
			{
				hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
				hitchance::w_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.w", "Hitchance W", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
			}

			auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
			{
				float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
				draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
				draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
				draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
				draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);
				draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
				draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
				draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
				draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
				draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
			}
		}

		// Permashow initialization
		//
		{
			Permashow::Instance.Init(main_tab);
			Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
			Permashow::Instance.AddElement("Allow Tower Dive", combo::allow_tower_dive);
		}

		// To add a new event you need to define a function and call add_calback
		//
		event_handler<events::on_update>::add_callback(on_update);
		event_handler<events::on_draw>::add_callback(on_draw);
		event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
	}

	void unload()
	{
		// Always remove all declared spells
		//
		plugin_sdk->remove_spell(q);
		plugin_sdk->remove_spell(w);
		plugin_sdk->remove_spell(e);
		plugin_sdk->remove_spell(r);

		// Remove menu tab
		//
		menu->delete_tab(main_tab);

		// Remove permashow
		//
		Permashow::Instance.Destroy();

		// VERY important to remove always ALL events
		//
		event_handler<events::on_update>::remove_handler(on_update);
		event_handler<events::on_draw>::remove_handler(on_draw);
		event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
	}

	// Main update script function
	void on_update()
	{
		if (myhero->is_dead())
		{
			return;
		}

		//console->print("[ShadowAIO] [DEBUG] Buff list:");
		//for (auto&& buff : myhero->get_bufflist())
		//{
		//    if (buff->is_valid() && buff->is_alive())
		//    {
		//        console->print("[ShadowAIO] [DEBUG] Buff name %s, count: %d", buff->get_name_cstr(), buff->get_count());
		//    }
		//}

		/*for (auto& object : entitylist->get_other_minion_objects())
		{
			if (object->is_valid() && !object->is_dead())
			{
				console->print("[ShadowAIO] [DEBUG] Object name %s", object->get_model_cstr());
			}
		}*/
		
		if (q->is_ready())
		{
			q->set_speed(q_speeds[q->level() - 1] + myhero->get_move_speed());
		}

		if (r->is_ready() && combo::use_r->get_bool() && !orbwalker->flee_mode())
		{
			r_logic();
		}

		// Very important if can_move ( extra_windup ) 
		// Extra windup is the additional time you have to wait after the aa
		// Too small time can interrupt the attack
		if (orbwalker->can_move(0.05f))
		{
			bool e_active = myhero->has_buff(buff_hash("BelvethE"));

			if (e->is_ready() && e_active && combo::e_cancel_if_nobody_inside->get_bool())
			{
				if ((orbwalker->combo_mode() || orbwalker->harass()) && myhero->count_enemies_in_range(e->range()) == 0)
				{
					if (e->cast())
						return;
				}
				if (orbwalker->lane_clear_mode())
				{
					// Gets enemy minions from the entitylist
					auto lane_minions = entitylist->get_enemy_minions();

					// Gets jugnle mobs from the entitylist
					auto monsters = entitylist->get_jugnle_mobs_minions();
						
					// You can use this function to delete minions that aren't in the specified range
					lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
						{
							return !x->is_valid_target(e->range());
						}), lane_minions.end());

					// You can use this function to delete monsters that aren't in the specified range
					monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
						{
							return !x->is_valid_target(e->range());
						}), monsters.end());

					if (lane_minions.empty() && monsters.empty())
					{
						if (e->cast())
							return;
					}
					if (orbwalker->flee_mode())
					{
						if (e->cast())
							return;
					}
				}
			}

			if (e_active)
				return;

			//Checking if the user has combo_mode() (Default SPACE)
			if (orbwalker->combo_mode())
			{
				if (q->is_ready() && combo::use_q->get_bool())
				{
					q_logic();
				}

				if (w->is_ready() && combo::use_w->get_bool())
				{
					w_logic();
				}

				if (e->is_ready() && combo::use_e->get_bool())
				{
					e_logic();
				}
			}

			//Checking if the user has selected harass() (Default C)
			if (orbwalker->harass())
			{
				// Get a target from a given range
				auto target = target_selector->get_target(q->range(), damage_type::magical);

				// Always check an object is not a nullptr!
				if (target != nullptr)
				{
					if (!myhero->is_under_enemy_turret())
					{
						if (q->is_ready() && harass::use_q->get_bool())
						{
							q_logic();
						}

						if (w->is_ready() && harass::use_w->get_bool())
						{
							w_logic();
						}

						if (e->is_ready() && harass::use_e->get_bool())
						{
							e_logic();
						}
					}
				}
			}

			// Checking if the user has selected flee_mode() (Default Z)
			if (orbwalker->flee_mode())
			{
				if (q->is_ready() && fleemode::use_q->get_bool())
				{
					if (q->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
					{
						return;
					}
				}

				if (w->is_ready() && fleemode::use_w->get_bool())
				{
					w_logic();
				}
			}

			// Checking if the user has selected lane_clear_mode() (Default V)
			if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool())
			{
				// Gets enemy minions from the entitylist
				auto lane_minions = entitylist->get_enemy_minions();

				// Gets jugnle mobs from the entitylist
				auto monsters = entitylist->get_jugnle_mobs_minions();

				// You can use this function to delete minions that aren't in the specified range
				lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
					{
						return !x->is_valid_target(q->range());
					}), lane_minions.end());

				// You can use this function to delete monsters that aren't in the specified range
				monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
					{
						return !x->is_valid_target(q->range());
					}), monsters.end());

				//std::sort -> sort lane minions by distance
				std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
					});

				//std::sort -> sort monsters by max health
				std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_max_health() > b->get_max_health();
					});

				if (!lane_minions.empty())
				{
					if (q->is_ready() && laneclear::use_q->get_bool())
					{
						if (lane_minions.front()->is_under_ally_turret())
						{
							if (myhero->count_enemies_in_range(900) == 0)
							{
								if (q->cast_on_best_farm_position(1))
								{
									return;
								}
							}
						}
						else
						{
							if (q->cast_on_best_farm_position(1))
							{
								return;
							}
						}
					}

					if (w->is_ready() && laneclear::use_w->get_bool())
					{
						if (w->cast_on_best_farm_position(1))
							return;
					}

					if (e->is_ready() && laneclear::use_e->get_bool() && lane_minions.size() >= laneclear::e_minimum_minions->get_int())
					{
						if (e->cast())
							return;
					}
				}


				if (!monsters.empty())
				{
					// Logic responsible for monsters
					if (q->is_ready() && jungleclear::use_q->get_bool())
					{
						if (q->cast_on_best_farm_position(1, true))
							return;
					}

					if (w->is_ready() && jungleclear::use_w->get_bool())
					{
						if (w->cast_on_best_farm_position(1, true))
							return;
					}

					if (e->is_ready() && jungleclear::use_e->get_bool())
					{
						if (e->cast())
							return;
					}
				}
			}
		}
	}

#pragma region q_logic
	void q_logic()
	{
		// Get a target from a given range
		auto target = target_selector->get_target(q->range(), damage_type::physical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			if (target->get_distance(myhero) > myhero->get_attack_range() || combo::q_mode->get_int() == 0)
			{
				if (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool())
				{
					auto pred = q->get_prediction(target);
					if (pred.hitchance >= get_hitchance(hitchance::q_hitchance))
					{
						auto pos = pred.get_cast_position();
						if (!evade->is_dangerous(pos))
						{
							q->cast(pos);
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region w_logic
	void w_logic()
	{
		// Get a target from a given range
		auto target = target_selector->get_target(w->range(), damage_type::magical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			w->cast(target, get_hitchance(hitchance::w_hitchance));
		}
	}
#pragma endregion

#pragma region e_logic
	void e_logic()
	{
		// Get a target from a given range
		auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			e->cast();
		}
	}

#pragma endregion

#pragma region r_logic
	void r_logic()
	{
		for (auto& object : entitylist->get_other_minion_objects())
		{
			if (object->is_valid() && !object->is_dead() && object->get_model().compare("BelvethSpore") == 0)
			{
				if (!object->is_under_enemy_turret() || combo::allow_tower_dive->get_bool())
				{
					if (myhero->get_distance(object) < r->range())
					{
						if (r->cast(object))
						{
							return;
						}
					}
				}
			}
		}
	}
#pragma endregion


#pragma region get_hitchance
	hit_chance get_hitchance(TreeEntry* entry)
	{
		switch (entry->get_int())
		{
			case 0:
				return hit_chance::low;
			case 1:
				return hit_chance::medium;
			case 2:
				return hit_chance::high;
			case 3:
				return hit_chance::very_high;
		}
		return hit_chance::medium;
	}
#pragma endregion

	void on_after_attack_orbwalker(game_object_script target)
	{
		if (q->is_ready() && combo::q_mode->get_int() == 1)
		{
			// Using e after autoattack on enemies
			if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
			{
				if (!evade->is_dangerous(target->get_position()))
				{
					if (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool())
					{
						auto pred = q->get_prediction(target);
						if (pred.hitchance >= get_hitchance(hitchance::q_hitchance))
						{
							auto pos = pred.get_cast_position();
							if (!evade->is_dangerous(pos))
							{
								q->cast(pos);
							}
						}
					}
				}
			}
		}
	}

	void on_draw()
	{
		if (myhero->is_dead())
		{
			return;
		}

		// Draw Q range
		if (q->is_ready() && draw_settings::draw_range_q->get_bool())
			draw_manager->add_circle(myhero->get_position(), q->range(), draw_settings::q_color->get_color());

		// Draw W range
		if (w->is_ready() && draw_settings::draw_range_w->get_bool())
			draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

		// Draw E range
		if (e->is_ready() && draw_settings::draw_range_e->get_bool())
			draw_manager->add_circle(myhero->get_position(), combo::e_max_range->get_int(), draw_settings::e_color->get_color());

		// Draw R range
		if (r->is_ready() && draw_settings::draw_range_r->get_bool())
			draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());
	}
};