#include "../plugin_sdk/plugin_sdk.hpp"
#include "kayle.h"
#include "permashow.hpp"

namespace kayle
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
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* w_target_above_range = nullptr;
		TreeEntry* w_target_hp_under = nullptr;
		TreeEntry* w_dont_use_target_under_turret = nullptr;
		TreeEntry* w_check_if_target_is_not_facing = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* e_mode = nullptr;
		TreeEntry* use_r = nullptr;
		TreeEntry* r_myhero_hp_under = nullptr;
		TreeEntry* r_only_when_enemies_nearby = nullptr;
		TreeEntry* r_enemies_search_radius = nullptr;
		TreeEntry* r_calculate_incoming_damage = nullptr;
		TreeEntry* r_coming_damage_time = nullptr;
		std::map<std::uint32_t, TreeEntry*> r_use_on;
	}

	namespace harass
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_e = nullptr;
	}

	namespace laneclear
	{
		TreeEntry* spell_farm = nullptr;
		TreeEntry* use_q = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* use_e_on_turret = nullptr;
	}

	namespace jungleclear
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_e = nullptr;
	}

	namespace lasthit
	{
		TreeEntry* lasthit = nullptr;
		TreeEntry* dont_lasthit_below_aa_range = nullptr;
		TreeEntry* use_q = nullptr;
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
	}


	// Event handler functions
	void on_update();
	void on_draw();
	void on_before_attack(game_object_script target, bool* process);
	void on_after_attack_orbwalker(game_object_script target);

	// Declaring functions responsible for spell-logic
	//
	void q_logic();
	void w_logic();
	void e_logic();
	void r_logic();

	// Utils
	//
	bool can_use_r_on(game_object_script target);
	hit_chance get_hitchance(TreeEntry* entry);

	void load()
	{
		// Registering a spells
		//
		q = plugin_sdk->register_spell(spellslot::q, 900);
		q->set_skillshot(0.25f, 150.0f, 1600.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
		w = plugin_sdk->register_spell(spellslot::w, 900);
		e = plugin_sdk->register_spell(spellslot::e, 575);
		r = plugin_sdk->register_spell(spellslot::r, 900);


		// Create a menu according to the description in the "Menu Section"
		//
		main_tab = menu->create_tab("kayle", "Kayle");
		main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
		{
			// Info
			//
			main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

			auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
			{
				combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
				combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
				combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
				{
					combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Target is above range", 500, 0, 800);
					combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.target_hp_under", "Target HP is under (in %)", 50, 0, 100);
					combo::w_dont_use_target_under_turret = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_target_under_turret", "Dont use if target is under turret", true);
					combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Check if target is not facing myhero", true);
				}
				combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
				combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
				{
					combo::e_mode = e_config->add_combobox(myhero->get_model() + ".combo.e.mode", "E Mode", { {"Before AA", nullptr},{"After AA", nullptr } }, 0);
				}
				combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
				combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
				auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
				{
					combo::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under", "Myhero HP is under (in %)", 20, 0, 100);
					combo::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.only_when_enemies_nearby", "Only when enemies are nearby", true);
					combo::r_enemies_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.enemies_search_radius", "Enemies nearby search radius", 900, 300, 1600);
					combo::r_calculate_incoming_damage = r_config->add_checkbox(myhero->get_model() + ".combo.r.calculate_incoming_damage", "Calculate incoming damage", true);
					combo::r_coming_damage_time = r_config->add_slider(myhero->get_model() + ".combo.r.coming_damage_time", "Set coming damage time (in ms)", 1000, 0, 1000);

					auto use_r_on_tab = r_config->add_tab(myhero->get_model() + ".combo.r.use_on", "Use R on");
					{
						for (auto&& ally : entitylist->get_ally_heroes())
						{
							// In this case you HAVE to set should save to false since key contains network id which is unique per game
							//
							combo::r_use_on[ally->get_network_id()] = use_r_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, false);

							// Set texture to enemy square icon
							//
							combo::r_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
						}
					}
				}
			}

			auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
			{
				harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
				harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
				harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
			{
				laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
				laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", false);
				laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
				laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				laneclear::use_e_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e.on_turret", "Use E On Turret", true);
				laneclear::use_e_on_turret->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
			{
				jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
				jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
				jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto lasthit = main_tab->add_tab(myhero->get_model() + ".lasthit", "Last Hit Settings");
			{
				lasthit::lasthit = lasthit->add_hotkey(myhero->get_model() + ".lasthit.enabled", "Toggle Last Hit", TreeHotkeyMode::Toggle, 'J', true);
				lasthit::dont_lasthit_below_aa_range = lasthit->add_checkbox(myhero->get_model() + ".lasthit.dont_lasthit_below_aa_range", "Dont lasthit below autoattack range", false);
				lasthit::use_q = lasthit->add_checkbox(myhero->get_model() + ".lasthit.q", "Use Q", false);
				lasthit::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				lasthit::use_e = lasthit->add_checkbox(myhero->get_model() + ".lasthit.e", "Use E", true);
				lasthit::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}

			auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
			{
				fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "Use Q to slow enemies", true);
				fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to ran away", true);
				fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
			}

			auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
			{
				hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
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
			Permashow::Instance.AddElement("Last Hit", lasthit::lasthit);
		}

		// To add a new event you need to define a function and call add_calback
		//
		event_handler<events::on_update>::add_callback(on_update);
		event_handler<events::on_draw>::add_callback(on_draw);
		event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
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
		event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
		event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
	}

	// Main update script function
	void on_update()
	{
		if (myhero->is_dead())
		{
			return;
		}

		if (r->is_ready() && combo::use_r->get_bool())
		{
			r_logic();
		}

		// Very important if can_move ( extra_windup ) 
		// Extra windup is the additional time you have to wait after the aa
		// Too small time can interrupt the attack
		if (orbwalker->can_move(0.05f))
		{
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

			if ((orbwalker->last_hit_mode() || orbwalker->mixed_mode() || orbwalker->lane_clear_mode()) && lasthit::lasthit->get_bool())
			{
				// Gets enemy minions from the entitylist
				auto lane_minions = entitylist->get_enemy_minions();

				// You can use this function to delete minions that aren't in the specified range
				lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
					{
						return !x->is_valid_target(q->range());
					}), lane_minions.end());

				//std::sort -> sort lane minions by distance
				std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
					{
						return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
					});

				if (!lane_minions.empty())
				{
					for (auto&& minion : lane_minions)
					{
						if (minion->get_health() > myhero->get_auto_attack_damage(minion) || !orbwalker->can_attack())
						{
							if (!lasthit::dont_lasthit_below_aa_range->get_bool() || !minion->is_valid_target(myhero->get_attack_range()))
							{
								if (q->is_ready() && lasthit::use_q->get_bool() && q->get_damage(minion) > minion->get_health())
								{
									if (q->cast(minion, get_hitchance(hitchance::q_hitchance)))
									{
										return;
									}
								}
								if (e->is_ready() && lasthit::use_e->get_bool() && e->get_damage(minion) + myhero->get_auto_attack_damage(minion) > minion->get_health())
								{
									if (e->cast())
									{
										return;
									}
								}
							}
						}
					}
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
				// Get a target from a given range
				auto target = target_selector->get_target(q->range(), damage_type::magical);

				// Always check an object is not a nullptr!
				if (target != nullptr)
				{
					if (q->is_ready() && fleemode::use_q->get_bool())
					{
						if (q->cast(target, get_hitchance(hitchance::q_hitchance))) {
							return;
						}
					}
				}

				if (w->is_ready() && fleemode::use_w->get_bool())
				{
					if (w->cast(myhero))
					{
						return;
					}
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
						if (q->cast_on_best_farm_position(1))
							return;
					}

					if (e->is_ready() && laneclear::use_e->get_bool())
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
		auto target = target_selector->get_target(q->range(), damage_type::magical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			// Check if the distance between myhero and enemy is smaller than q range
			if (target->get_distance(myhero) <= q->range())
			{
				q->cast(target, get_hitchance(hitchance::q_hitchance));
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
			if (target->get_health_percent() < combo::w_target_hp_under->get_int())
			{
				if (target->get_distance(myhero) > combo::w_target_above_range->get_int())
				{
					if (!combo::w_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
					{
						if (!combo::w_check_if_target_is_not_facing->get_bool() || !target->is_facing(myhero))
						{
							w->cast(myhero);
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region e_logic
	void e_logic()
	{
		// Get a target from a given range
		auto target = target_selector->get_target(r->level() == 0 ? e->range() : myhero->get_attack_range(), damage_type::magical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			if (r->level() == 0 || e->get_damage(target) >= target->get_health())
			{
				e->cast();
			}
		}
	}

#pragma endregion

#pragma region r_logic
	void r_logic()
	{
		for (auto&& ally : entitylist->get_ally_heroes())
		{
			if (can_use_r_on(ally))
			{
				if (ally->get_distance(myhero->get_position()) <= r->range())
				{
					if (!ally->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") }))
					{
						if ((ally->get_health_percent() < combo::r_myhero_hp_under->get_int()) || (combo::r_calculate_incoming_damage->get_bool() && health_prediction->get_incoming_damage(ally, combo::r_coming_damage_time->get_int() / 1000.0f, true) >= ally->get_health()))
						{
							if (!combo::r_only_when_enemies_nearby->get_bool() || ally->count_enemies_in_range(combo::r_enemies_search_radius->get_int()) != 0)
							{
								if (r->cast(ally))
								{
									return;
								}
							}
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region can_use_r_on
	bool can_use_r_on(game_object_script target)
	{
		auto it = combo::r_use_on.find(target->get_network_id());
		if (it == combo::r_use_on.end())
			return false;

		return it->second->get_bool();
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

	void on_before_attack(game_object_script target, bool* process)
	{
		if (e->is_ready() && combo::e_mode->get_int() == 0)
		{
			// Using e before autoattack on enemies
			if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
			{
				if (e->cast())
				{
					return;
				}
			}
		}
	}

	void on_after_attack_orbwalker(game_object_script target)
	{
		if (e->is_ready())
		{
			if (combo::e_mode->get_int() == 1)
			{
				// Using e before autoattack on enemies
				if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
				{
					if (e->cast())
					{
						return;
					}
				}
			}

			// Using e before autoattack on turrets
			if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_e_on_turret->get_bool() && target->is_ai_turret())
			{
				if (e->cast())
				{
					return;
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
		if (e->is_ready() && draw_settings::draw_range_e->get_bool() && r->level() == 0)
			draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

		// Draw R range
		if (r->is_ready() && draw_settings::draw_range_r->get_bool())
			draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());
	}
};