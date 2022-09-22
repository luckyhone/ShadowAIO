#include "../plugin_sdk/plugin_sdk.hpp"
#include "kayle.h"
#include "utils.h"
#include "dmg_lib.h"
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

		namespace draw_damage_settings
		{
			TreeEntry* draw_damage = nullptr;
			TreeEntry* q_damage = nullptr;
			TreeEntry* e_damage = nullptr;
			TreeEntry* r_damage = nullptr;
		}
	}

	namespace combo
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* q_mode = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* w_use_on_low_hp = nullptr;
		TreeEntry* w_only_if_enemies_nearby = nullptr;
		TreeEntry* w_myhero_hp_under = nullptr;
		TreeEntry* w_ally_hp_under = nullptr;
		std::map<std::uint32_t, TreeEntry*> w_use_on;
		TreeEntry* w_use_while_chasing = nullptr;
		TreeEntry* w_target_above_range = nullptr;
		TreeEntry* w_target_hp_under = nullptr;
		TreeEntry* w_dont_use_target_under_turret = nullptr;
		TreeEntry* w_check_if_target_is_not_facing = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* e_mode = nullptr;
		TreeEntry* e_ignore_mode_before_lvl_6 = nullptr;
		TreeEntry* use_r = nullptr;
		TreeEntry* r_on_low_hp = nullptr;
		TreeEntry* r_myhero_hp_under = nullptr;
		TreeEntry* r_ally_hp_under = nullptr;
		TreeEntry* r_low_hp_only_when_enemies_nearby = nullptr;
		TreeEntry* r_low_hp_enemies_search_radius = nullptr;
		TreeEntry* r_calculate_incoming_damage = nullptr;
		TreeEntry* r_incoming_damage_time = nullptr;
		TreeEntry* r_incoming_damage_over_my_hp_in_percent = nullptr;
		TreeEntry* r_incoming_damage_only_when_enemies_nearby = nullptr;
		TreeEntry* r_incoming_damage_enemies_search_radius = nullptr;
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

	namespace antigapclose
	{
		TreeEntry* use_q = nullptr;
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
	void on_unkillable_minion(game_object_script minion);
	void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

	// Declaring functions responsible for spell-logic
	//
	void q_logic();
	void w_logic();
	void e_logic();
	void r_logic();

	// Utils
	//
	inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

	void load()
	{
		// Registering a spells
		//
		q = plugin_sdk->register_spell(spellslot::q, 900);
		q->set_skillshot(0.25f, 75.0f, 1600.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
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
				auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "Q Config");
				{
					combo::q_mode = q_config->add_combobox(myhero->get_model() + ".combo.q.mode", "Q Mode", { {"If enemy above AA range or After AA", nullptr}, {"In Combo", nullptr}, {"After AA", nullptr } }, 0);
				}
				combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
				combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
				{
					w_config->add_separator(myhero->get_model() + ".combo.w.separator1", "W on Low HP Options");
					combo::w_use_on_low_hp = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_on_low_hp", "Use W on Low HP", true);
					combo::w_use_on_low_hp->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
					combo::w_only_if_enemies_nearby = w_config->add_checkbox(myhero->get_model() + ".combo.w.only_if_enemies_nearby", "Use W only if enemies nearby", true);
					combo::w_only_if_enemies_nearby->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
					combo::w_myhero_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.myhero_hp_under", "Myhero HP is under (in %)", 30, 0, 100);
					combo::w_ally_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.ally_hp_under", "Ally HP is under (in %)", 25, 0, 100);

					auto use_w_on_tab = w_config->add_tab(myhero->get_model() + ".combo.w.use_on", "Use W on");
					{
						for (auto&& ally : entitylist->get_ally_heroes())
						{
							// In this case you HAVE to set should save to false since key contains network id which is unique per game
							//
							combo::w_use_on[ally->get_network_id()] = use_w_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, false);

							// Set texture to enemy square icon
							//
							combo::w_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
						}
					}

					w_config->add_separator(myhero->get_model() + ".combo.w.separator2", "W while chasing Options");
					combo::w_use_while_chasing = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_while_chasing", "Use W while chasing an enemy", true);
					combo::w_use_while_chasing->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
					combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Target is above range", 525, 0, 800);
					combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.target_hp_under", "Target HP is under (in %)", 50, 0, 100);
					combo::w_dont_use_target_under_turret = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_target_under_turret", "Dont use if target is under turret", true);
					combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Check if target is not facing myhero", true);
				}
				combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
				combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
				{
					combo::e_mode = e_config->add_combobox(myhero->get_model() + ".combo.e.mode", "E Mode", { {"Before AA", nullptr},{"After AA", nullptr } }, 1);
					combo::e_ignore_mode_before_lvl_6 = e_config->add_checkbox(myhero->get_model() + ".combo.e.ignore_mode_before_lvl_6", "Ignore E mode before level 6", true);
					combo::e_ignore_mode_before_lvl_6->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				}
				combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
				combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
				auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
				{
					r_config->add_separator(myhero->get_model() + ".combo.r.separator1", "R on Low HP Options");
					combo::r_on_low_hp = r_config->add_checkbox(myhero->get_model() + ".combo.r.on_low_hp", "Use R on Low HP", true);
					combo::r_on_low_hp->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
					combo::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under", "Myhero HP is under (in %)", 15, 0, 100);
					combo::r_ally_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.ally_hp_under", "Ally HP is under (in %)", 15, 0, 100);
					combo::r_low_hp_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.low_hp_only_when_enemies_nearby", "Use R only when enemies are nearby", true);
					combo::r_low_hp_only_when_enemies_nearby->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
					combo::r_low_hp_enemies_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.low_hp_enemies_search_radius", "Enemies nearby search radius", 750, 300, 1600);

					r_config->add_separator(myhero->get_model() + ".combo.r.separator2", "R on Incoming Damage Options");
					combo::r_calculate_incoming_damage = r_config->add_checkbox(myhero->get_model() + ".combo.r.calculate_incoming_damage", "Use R on Incoming Damage", true);
					combo::r_calculate_incoming_damage->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
					combo::r_incoming_damage_time = r_config->add_slider(myhero->get_model() + ".combo.r.coming_damage_time", "Incoming damage time (in ms)", 750, 0, 1000);
					combo::r_incoming_damage_over_my_hp_in_percent = r_config->add_slider(myhero->get_model() + ".combo.r.incoming_damage_over_my_hp_in_percent", "Incoming damage is over my HP (in %)", 90, 0, 100);
					combo::r_incoming_damage_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.incoming_damage_only_when_enemies_nearby", "Use R only when enemies are nearby", false);
					combo::r_incoming_damage_only_when_enemies_nearby->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
					combo::r_incoming_damage_enemies_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.incoming_damage_enemies_search_radius", "Enemies nearby search radius", 750, 300, 1600);

					r_config->add_separator(myhero->get_model() + ".combo.r.separator3", "R Usage Options");
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
				laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
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

			auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
			{
				antigapclose::use_q = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.q", "Use Q", true);
				antigapclose::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
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

				auto draw_damage = draw_settings->add_tab(myhero->get_model() + ".draw.damage", "Draw Damage");
				{
					draw_settings::draw_damage_settings::draw_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.enabled", "Draw Combo Damage", true);
					draw_settings::draw_damage_settings::q_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.q", "Draw Q Damage", true);
					draw_settings::draw_damage_settings::q_damage->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
					draw_settings::draw_damage_settings::e_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.e", "Draw E Damage", true);
					draw_settings::draw_damage_settings::e_damage->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
					draw_settings::draw_damage_settings::r_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.r", "Draw R Damage", true);
					draw_settings::draw_damage_settings::r_damage->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
				}
			}
		}

		// Permashow initialization
		//
		{
			Permashow::Instance.Init(main_tab);
			Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
			Permashow::Instance.AddElement("Last Hit", lasthit::lasthit);
		}

		// Add anti gapcloser handler
		//
		antigapcloser::add_event_handler(on_gapcloser);

		// To add a new event you need to define a function and call add_calback
		//
		event_handler<events::on_update>::add_callback(on_update);
		event_handler<events::on_draw>::add_callback(on_draw);
		event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
		event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
		event_handler<events::on_unkillable_minion>::add_callback(on_unkillable_minion);

		// Chat message after load
		//
		utils::on_load();
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

		// Remove anti gapcloser handler
		//
		antigapcloser::remove_event_handler(on_gapcloser);

		// VERY important to remove always ALL events
		//
		event_handler<events::on_update>::remove_handler(on_update);
		event_handler<events::on_draw>::remove_handler(on_draw);
		event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
		event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
		event_handler<events::on_unkillable_minion>::remove_handler(on_unkillable_minion);
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

		if ((orbwalker->last_hit_mode() || orbwalker->harass() || orbwalker->lane_clear_mode()) && lasthit::lasthit->get_bool())
		{
			// Gets enemy minions from the entitylist
			auto lane_minions = entitylist->get_enemy_minions();

			// You can use this function to delete minions that aren't in the specified range
			lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
				{
					return !x->is_valid_target(e->range() + 50);
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
					if (minion->get_health() > myhero->get_auto_attack_damage(minion) || myhero->is_winding_up() || myhero->get_distance(minion) > myhero->get_attack_range())
					{
						if (!lasthit::dont_lasthit_below_aa_range->get_bool() || !minion->is_valid_target(myhero->get_attack_range()))
						{
							if (q->is_ready() && lasthit::use_q->get_bool() && dmg_lib::get_damage(q, minion) > minion->get_health())
							{
								if (q->cast(minion, utils::get_hitchance(hitchance::q_hitchance)))
								{
									return;
								}
							}
							if (e->is_ready() && lasthit::use_e->get_bool() && dmg_lib::get_damage(e, minion) > minion->get_health())
							{
								if (e->cast())
								{
									myhero->issue_order(minion);
									return;
								}
							}
						}
					}
				}
			}
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
						if (q->cast(target, utils::get_hitchance(hitchance::q_hitchance))) {
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
				}


				if (!monsters.empty())
				{
					// Logic responsible for monsters
					if (q->is_ready() && jungleclear::use_q->get_bool())
					{
						if (q->cast_on_best_farm_position(1, true))
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
			auto q_mode = combo::q_mode->get_int();
			if ((q_mode == 0 && myhero->get_distance(target) > myhero->get_attack_range()) || q_mode == 1 || q->get_damage(target) > target->get_real_health())
			{
				q->cast(target, utils::get_hitchance(hitchance::q_hitchance));
			}
		}
	}
#pragma endregion

#pragma region w_logic
	void w_logic()
	{
		if (combo::w_use_on_low_hp->get_bool())
		{
			for (auto&& ally : entitylist->get_ally_heroes())
			{
				if (ally->is_valid() && !ally->is_dead() && utils::enabled_in_map(combo::w_use_on, ally) && ally->get_distance(myhero->get_position()) <= w->range())
				{
					if (!utils::has_unkillable_buff(ally) && (!combo::w_only_if_enemies_nearby->get_bool() || ally->count_enemies_in_range(w->range()) != 0))
					{
						if ((ally->get_health_percent() < (ally->is_me() ? combo::w_myhero_hp_under->get_int() : combo::w_ally_hp_under->get_int())))
						{
							if (w->cast(ally))
							{
								return;
							}
						}
					}
				}
			}
		}

		if (combo::w_use_while_chasing->get_bool())
		{
			// Get a target from a given range
			auto target = target_selector->get_target(w->range(), damage_type::magical);

			// Always check an object is not a nullptr!
			if (target != nullptr)
			{
				if (target->get_health_percent() < combo::w_target_hp_under->get_int() && target->get_distance(myhero) > combo::w_target_above_range->get_int())
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
		auto target = target_selector->get_target((r->level() == 0 ? e->range() : myhero->get_attack_range()) + 50, damage_type::magical);

		// Always check an object is not a nullptr!
		if (target != nullptr)
		{
			if ((r->level() == 0 && combo::e_ignore_mode_before_lvl_6->get_bool()) || dmg_lib::get_damage(e, target) >= target->get_health())
			{
				if (e->cast())
				{
					myhero->issue_order(target);
				}
			}
		}
	}

#pragma endregion

#pragma region r_logic
	void r_logic()
	{
		for (auto&& ally : entitylist->get_ally_heroes())
		{
			if (ally->is_valid() && !ally->is_dead() && utils::enabled_in_map(combo::r_use_on, ally) && ally->get_distance(myhero->get_position()) <= r->range() && !utils::has_unkillable_buff(ally))
			{
				if (combo::r_on_low_hp->get_bool() && (!combo::r_low_hp_only_when_enemies_nearby->get_bool() || 
					ally->count_enemies_in_range(combo::r_low_hp_enemies_search_radius->get_int()) != 0))
				{
					if (ally->get_health_percent() < (ally->is_me() ? combo::r_myhero_hp_under->get_int() : combo::r_ally_hp_under->get_int()))
					{
						if (r->cast(ally))
						{
							//myhero->print_chat(1, "R Casted due to Low HP");
							return;
						}
					}
				}

				if (combo::r_calculate_incoming_damage->get_bool() && (!combo::r_incoming_damage_only_when_enemies_nearby->get_bool() ||
					ally->count_enemies_in_range(combo::r_incoming_damage_enemies_search_radius->get_int()) != 0))
				{
					if ((health_prediction->get_incoming_damage(ally, combo::r_incoming_damage_time->get_int() / 1000.f, true) * 100.f) /
						ally->get_max_health() > ally->get_health_percent() * (combo::r_incoming_damage_over_my_hp_in_percent->get_int() / 100.f))
					{
						if (r->cast(ally))
						{
							//myhero->print_chat(1, "R Casted due to Incoming DMG");
							return;
						}
					}
				}
			}
		}
	}
#pragma endregion

	inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color)
	{
		if (target != nullptr && target->is_valid() && target->is_visible_on_screen() && target->is_hpbar_recently_rendered())
		{
			auto bar_pos = target->get_hpbar_pos();

			if (bar_pos.is_valid() && !target->is_dead() && target->is_visible())
			{
				const auto health = target->get_health();

				bar_pos = vector(bar_pos.x + (105 * (health / target->get_max_health())), bar_pos.y -= 10);

				auto damage_size = (105 * (damage / target->get_max_health()));

				if (damage >= health)
				{
					damage_size = (105 * (health / target->get_max_health()));
				}

				if (damage_size > 105)
				{
					damage_size = 105;
				}

				const auto size = vector(bar_pos.x + (damage_size * -1), bar_pos.y + 11);

				draw_manager->add_filled_rect(bar_pos, size, color);
			}
		}
	}

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

			// Using e before autoattack on minions
			if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_e->get_bool() && target->is_ai_minion())
			{
				if (e->cast())
				{
					return;
				}
			}

			// Using e before autoattack on monsters
			if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_e->get_bool() && target->is_monster())
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
		// Use q after autoattack on enemies
		if (q->is_ready() && combo::q_mode->get_int() != 1 && target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
		{
			q->cast(target, utils::get_hitchance(hitchance::q_hitchance));
		}

		if (e->is_ready())
		{
			if (combo::e_mode->get_int() == 1)
			{
				// Using e after autoattack on enemies
				if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
				{
					if (e->cast())
					{
						return;
					}
				}

				// Using e after autoattack on minions
				if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_e->get_bool() && target->is_ai_minion())
				{
					if (e->cast())
					{
						return;
					}
				}

				// Using e after autoattack on monsters
				if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_e->get_bool() && target->is_monster())
				{
					if (e->cast())
					{
						return;
					}
				}
			}

			// Using e after autoattack on turrets
			if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_e_on_turret->get_bool() && target->is_ai_turret())
			{
				if (e->cast())
				{
					return;
				}
			}
		}
	}

	void on_unkillable_minion(game_object_script minion)
	{
		if (e->is_ready() && lasthit::lasthit->get_bool() && lasthit::use_e->get_bool() && minion->is_valid_target(e->range() + 50) && dmg_lib::get_damage(e, minion) >= minion->get_health())
		{
			if (e->cast())
			{
				myhero->issue_order(minion);
			}
		}
	}

	void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
	{
		if (antigapclose::use_q->get_bool() && q->is_ready() && !myhero->is_under_enemy_turret())
		{
			if (sender->is_valid_target(q->range() + sender->get_bounding_radius()))
			{
				q->cast(sender, utils::get_hitchance(hitchance::q_hitchance));
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

		// Draw combo damage
		if (draw_settings::draw_damage_settings::draw_damage->get_bool())
		{
			for (auto& enemy : entitylist->get_enemy_heroes())
			{
				if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
				{
					float damage = 0.0f;

					if (q->is_ready() && draw_settings::draw_damage_settings::q_damage->get_bool())
						damage += dmg_lib::get_damage(q, enemy);

					if (e->is_ready() && draw_settings::draw_damage_settings::e_damage->get_bool())
						damage += dmg_lib::get_damage(e, enemy);

					if (r->is_ready() && draw_settings::draw_damage_settings::r_damage->get_bool())
						damage += dmg_lib::get_damage(r, enemy);

					if (damage != 0.0f)
						draw_dmg_rl(enemy, damage, 0x8000ff00);
				}
			}
		}
	}
};