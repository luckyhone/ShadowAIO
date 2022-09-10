#include "../plugin_sdk/plugin_sdk.hpp"
#include "tryndamere.h"
#include "utils.h"
#include "permashow.hpp"

namespace tryndamere
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
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* w_color = nullptr;
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_myhero_hp_under = nullptr;
        TreeEntry* q_only_when_no_enemies_nearby = nullptr;
        TreeEntry* q_enemies_search_radius = nullptr;
        TreeEntry* q_use_if_fury_above = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_target_above_range = nullptr;
        TreeEntry* w_target_hp_under = nullptr;
        TreeEntry* w_only_when_e_ready = nullptr;
        TreeEntry* w_dont_use_target_under_turret = nullptr;
        TreeEntry* w_check_if_target_is_not_facing = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_dont_use_under_enemy_turret = nullptr;
        TreeEntry* e_use_prediction = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_myhero_hp_under = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
        TreeEntry* r_enemies_search_radius = nullptr;
        TreeEntry* r_calculate_incoming_damage = nullptr;
        TreeEntry* r_coming_damage_time = nullptr;
        TreeEntry* r_disable_evade = nullptr;
        bool previous_evade_state = false;
    }

    namespace harass
    {
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_only_when_minions_more_than = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_e;
    }

    namespace hitchance
    {
        TreeEntry* e_hitchance = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    bool r_logic();


    // Utils
    //
    hit_chance get_hitchance(TreeEntry* entry);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 0);
        w = plugin_sdk->register_spell(spellslot::w, 850);
        e = plugin_sdk->register_spell(spellslot::e, 660);
        e->set_skillshot(0.0f, 112.0f, FLT_MAX, { }, skillshot_type::skillshot_line);
        r = plugin_sdk->register_spell(spellslot::r, 0);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("tryndamere", "Tryndamere");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = combo->add_tab(myhero->get_model() + ".combo.q.config", "Q Config");
                {
                    combo::q_myhero_hp_under = q_config->add_slider(myhero->get_model() + ".combo.q.myhero_hp_under", "Myhero HP is under (in %)", 20, 0, 100);
                    combo::q_only_when_no_enemies_nearby = q_config->add_checkbox(myhero->get_model() + ".combo.q.only_when_no_enemies_nearby", "Only when no enemies are nearby", true);
                    combo::q_enemies_search_radius = q_config->add_slider(myhero->get_model() + ".combo.q.enemies_search_radius", "Enemies nearby search radius", 900, 300, 1600);
                    combo::q_use_if_fury_above = q_config->add_slider(myhero->get_model() + ".combo.q.use_if_fury_above", "Use if fury is above", 50, 0, 100);
                }
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W on escaping enemies", true);
                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Target is above range", 300, 0, 800);
                    combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.target_hp_under", "Target HP is under (in %)", 80, 0, 100);
                    combo::w_only_when_e_ready = w_config->add_checkbox(myhero->get_model() + ".combo.w.only_when_e_ready", "Use W only when E is ready", true);
                    combo::w_dont_use_target_under_turret = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_target_under_turret", "Dont use if target is under turret", true);
                    combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Check if target is not facing myhero", true);
                }
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    combo::e_dont_use_under_enemy_turret = e_config->add_checkbox(myhero->get_model() + ".combo.e.dont_use_under_enemy_turret", "Dont use if target is under turret", true);
                    combo::e_use_prediction = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_prediction", "Use prediction", false);
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
                    combo::r_disable_evade = r_config->add_checkbox(myhero->get_model() + ".combo.r.disable_evade", "Disable evade on R", false);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_only_when_minions_more_than = e_config->add_slider(myhero->get_model() + ".laneclear.e.use_only_when_minions_more_than", "Use only when minions more than", 3, 0, 5);
                }
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }


            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E to ran away", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);

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

        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (!r_logic())
        {
            q_logic();
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
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
                if (e->is_ready() && harass::use_e->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(e->range(), damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (!target->is_under_ally_turret())
                        {
                            if (combo::e_use_prediction->get_bool())
                            {
                                if (e->cast(target, get_hitchance(hitchance::e_hitchance)))
                                {
                                    return;
                                }
                            }
                            else
                            {
                                if (e->cast(target))
                                {
                                    return;
                                }
                            }
                        }
                    }
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (e->is_ready() && fleemode::use_e->get_bool())
                {
                    if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
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
                        return !x->is_valid_target(e->range());
                    }), lane_minions.end());

                // You can use this function to delete monsters that aren't in the specified range
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range());
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
                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (e->cast_on_best_farm_position(laneclear::e_only_when_minions_more_than->get_int()))
                                {
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if (e->cast_on_best_farm_position(laneclear::e_only_when_minions_more_than->get_int()))
                            {
                                return;
                            }
                        }
                    }
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(1, true))
                            return;
                    }
                }
            }
        }
    }

#pragma region q_logic
    void q_logic()
    {
        if (myhero->is_recalling())
        {
            return;
        }

        if (q->is_ready() && combo::use_q->get_bool())
        {
            if (!myhero->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") }))
            {
                if (myhero->get_health_percent() < combo::q_myhero_hp_under->get_int())
                {
                    if (!combo::q_only_when_no_enemies_nearby->get_bool() || myhero->count_enemies_in_range(combo::q_enemies_search_radius->get_int()) != 0)
                    {
                        if (myhero->get_mana() >= combo::q_use_if_fury_above->get_int())
                        {
                            q->cast();
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
        auto target = target_selector->get_target(w->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (target->get_health_percent() < combo::w_target_hp_under->get_int())
            {
                if (target->get_distance(myhero) > combo::w_target_above_range->get_int())
                {
                    if (!combo::w_only_when_e_ready->get_bool() || e->is_ready())
                    {
                        if (!combo::w_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
                        {
                            if (!combo::w_check_if_target_is_not_facing->get_bool() || !target->is_facing(myhero))
                            {
                                w->cast();
                            }
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
        auto target = target_selector->get_target(e->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (!combo::e_dont_use_under_enemy_turret->get_bool() || !target->is_under_ally_turret())
            {
                if (e->cast(target))
                {
                    return;
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    bool r_logic()
    {
        if (combo::r_disable_evade->get_bool())
        {
            if (myhero->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") }))
            {
                if (!evade->is_evade_disabled() && !combo::previous_evade_state)
                {
                    evade->disable_evade();
                    combo::previous_evade_state = true;
                }
            }
            else if (combo::previous_evade_state)
            {
                evade->enable_evade();
                combo::previous_evade_state = false;
            }
        }

        if (r->is_ready() && combo::use_r->get_bool())
        {
            if (!myhero->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") }))
            {
                if ((myhero->get_health_percent() < combo::r_myhero_hp_under->get_int()) || (combo::r_calculate_incoming_damage->get_bool() && health_prediction->get_incoming_damage(myhero, combo::r_coming_damage_time->get_int() / 1000.0f, true) >= myhero->get_health()))
                {
                    if (!combo::r_only_when_enemies_nearby->get_bool() || myhero->count_enemies_in_range(combo::r_enemies_search_radius->get_int()) != 0)
                    {
                        return r->cast();
                    }
                }
            }
        }
        return false;
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

    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());
    }
};