#include "../plugin_sdk/plugin_sdk.hpp"
#include "thresh.h"
#include "utils.h"
#include "permashow.hpp"

namespace thresh
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
        TreeEntry* thresh_mode = nullptr;
        TreeEntry* allow_tower_dive = nullptr;

        TreeEntry* use_q = nullptr;
        TreeEntry* q_semi_manual_cast = nullptr;
        TreeEntry* q_min_range = nullptr;
        TreeEntry* q_max_range = nullptr;
        TreeEntry* q_force_use_selected_targe = nullptr;
        TreeEntry* use_q2 = nullptr;

        TreeEntry* use_w = nullptr;
        TreeEntry* w_semi_manual_cast = nullptr;

        TreeEntry* w_use_on_incoming_damage = nullptr;
        TreeEntry* w_incoming_damage_time = nullptr;
        TreeEntry* w_over_hp_in_percent = nullptr;

        TreeEntry* w_to_ally_on_q_hit = nullptr;
        TreeEntry* w_to_ally_on_q_hit_ally_is_above_range = nullptr;

        TreeEntry* w_to_ally_stunned = nullptr;
        TreeEntry* w_to_ally_stunned_target_is_below_range = nullptr;
        TreeEntry* w_to_ally_stunned_ally_is_above_range = nullptr;

        TreeEntry* use_e = nullptr;
        TreeEntry* e_auto_spell_interrupter = nullptr;
        TreeEntry* e_pull_if_distance_more_than = nullptr;

        TreeEntry* use_r = nullptr;
        TreeEntry* r_if_will_hit_x_targets = nullptr;
        TreeEntry* r_only_if_ally_nearby = nullptr;

        std::map<std::uint32_t, TreeEntry*> q_use_on;
        std::map<std::uint32_t, TreeEntry*> w_use_on;
        std::map<std::uint32_t, TreeEntry*> e_use_on;
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
        TreeEntry* use_e = nullptr;
        TreeEntry* e_minimum_minions = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_e;
    }

    namespace antigapclose
    {
        TreeEntry* use_e = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void w_logic_semi();
    void e_logic();
    void r_logic();

    // Utils
    //
    bool can_use_q_on(game_object_script target);
    bool can_use_w_on(game_object_script target);
    bool can_use_e_on(game_object_script target);
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    void cast_e_push(game_object_script target);
    void cast_e_pull(game_object_script target);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 1100);
        q->set_skillshot(0.50f, 70.0f, 1900.0f, { collisionable_objects::heroes, collisionable_objects::minions, collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
        w = plugin_sdk->register_spell(spellslot::w, 950);
        e = plugin_sdk->register_spell(spellslot::e, 480);
        r = plugin_sdk->register_spell(spellslot::r, 450);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("thresh", "Thresh");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::thresh_mode = combo->add_combobox(myhero->get_model() + ".combo.thresh.mode", "Thresh Mode", { {"Support", nullptr},{"ADC", nullptr } }, 0);
                combo::allow_tower_dive = combo->add_hotkey(myhero->get_model() + ".combo.allow_tower_dive", "Allow Tower Dive", TreeHotkeyMode::Toggle, 'A', true);
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());

                auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "Q Config");
                {
                    combo::q_semi_manual_cast = q_config->add_hotkey(myhero->get_model() + ".combo.q.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'X', true);
                    combo::q_min_range = q_config->add_slider(myhero->get_model() + ".combo.q.min_range", "Minimum Q distance to target", 125, 1, 1100); 
                    combo::q_max_range = q_config->add_slider(myhero->get_model() + ".combo.q.max_range", "Maximum Q distance to target", 1050, 1, 1100);
                    combo::q_force_use_selected_targe = q_config->add_checkbox(myhero->get_model() + ".combo.q.force_use_selected_targe", "Force use Q on selected target", true);

                    auto use_q_on_tab = q_config->add_tab(myhero->get_model() + ".combo.q.use_on", "Use Q On");
                    {
                        for (auto&& enemy : entitylist->get_enemy_heroes())
                        {
                            // In this case you HAVE to set should save to false since key contains network id which is unique per game
                            //
                            combo::q_use_on[enemy->get_network_id()] = use_q_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                            // Set texture to enemy square icon
                            //
                            combo::q_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                        }
                    }
                }

                combo::use_q2 = combo->add_checkbox(myhero->get_model() + ".combo.q2", "Use Q2", true);
                combo::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());

                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_use_on_incoming_damage = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_on_incoming_damage", "Use W on incoming damage", true);
                    combo::w_use_on_incoming_damage->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                    combo::w_incoming_damage_time = w_config->add_slider(myhero->get_model() + ".combo.w.incoming_damage_time", "Incoming damage time (in ms)", 750, 0, 1000);
                    combo::w_over_hp_in_percent = w_config->add_slider(myhero->get_model() + ".combo.w.over_hp_in_percent", "Incoming damage is over HP (in %)", 10, 0, 100);

                    w_config->add_separator(myhero->get_model() + ".combo.w.separator1", "W on Q hit Settings");

                    combo::w_to_ally_on_q_hit = w_config->add_checkbox(myhero->get_model() + ".combo.w.to_ally_on_q_hit", "Use W to ally on Q hit", true);
                    combo::w_to_ally_on_q_hit->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                    combo::w_to_ally_on_q_hit_ally_is_above_range = w_config->add_slider(myhero->get_model() + " combo.w.to_ally_on_q_hit_ally_is_above_range", "Use if ally distance to target is higher than", 275, 1, w->range());

                    w_config->add_separator(myhero->get_model() + ".combo.w.separator2", "W on CC Settings");

                    combo::w_to_ally_stunned = w_config->add_checkbox(myhero->get_model() + ".combo.w.to_ally_stunned", "Use W to ally when target is immobile", true);
                    combo::w_to_ally_stunned->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                    combo::w_to_ally_stunned_target_is_below_range = w_config->add_slider(myhero->get_model() + ".combo.w.to_ally_stunned_target_is_below_range", "Use if target is below range to myhero", 450, 1, 900);
                    combo::w_to_ally_stunned_ally_is_above_range = w_config->add_slider(myhero->get_model() + " combo.w.to_ally_stunned_ally_is_above_range", "Use if ally distance to target is more than", 275, 1, w->range());

                    w_config->add_separator(myhero->get_model() + ".combo.w.separator3", "Other Settings");

                    combo::w_semi_manual_cast = w_config->add_hotkey(myhero->get_model() + ".combo.w.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'G', true);

                    auto use_w_on_tab = w_config->add_tab(myhero->get_model() + ".combo.w.use_on", "Use W On");
                    {
                        for (auto&& ally : entitylist->get_ally_heroes())
                        {
                            // In this case you HAVE to set should save to false since key contains network id which is unique per game
                            //
                            combo::w_use_on[ally->get_network_id()] = use_w_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), ally->is_me() ? false : true, false);

                            // Set texture to ally square icon
                            //
                            combo::w_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
                        }
                    }
                }

                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

                auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
                {
                    combo::e_auto_spell_interrupter = e_config->add_checkbox(myhero->get_model() + ".combo.e.auto_spell_interrupter", "Auto E spell interrupter", true);
                    combo::e_auto_spell_interrupter->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    combo::e_pull_if_distance_more_than = e_config->add_slider(myhero->get_model() + ".combo.e.pull_if_distance_more_than", "Pull if distance to target more than", 300, 1, e->range());

                    auto use_e_on_tab = e_config->add_tab(myhero->get_model() + ".combo.e.use_on", "Use E On");
                    {
                        for (auto&& enemy : entitylist->get_enemy_heroes())
                        {
                            // In this case you HAVE to set should save to false since key contains network id which is unique per game
                            //
                            combo::e_use_on[enemy->get_network_id()] = use_e_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                            // Set texture to enemy square icon
                            //
                            combo::e_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                        }
                    }
                }   

                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_if_will_hit_x_targets = r_config->add_slider(myhero->get_model() + ".combo.r.if_will_hit_x_targets", "Use R if will hit x targets", 2, 1, 5);
                    combo::r_only_if_ally_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.only_if_ally_nearby", "Use R only if ally is nearby", true);

                    auto use_r_on_tab = r_config->add_tab(myhero->get_model() + ".combo.r.use_on", "Use R On");
                    {
                        for (auto&& enemy : entitylist->get_enemy_heroes())
                        {
                            // In this case you HAVE to set should save to false since key contains network id which is unique per game
                            //
                            combo::r_use_on[enemy->get_network_id()] = use_r_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                            // Set texture to enemy square icon
                            //
                            combo::r_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
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
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "Minimum minions", 2, 0, 5);
                }
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", false);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
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
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Semi Auto Q", combo::q_semi_manual_cast);
            Permashow::Instance.AddElement("Semi Auto W", combo::w_semi_manual_cast);
            Permashow::Instance.AddElement("Allow Tower Dive", combo::allow_tower_dive);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
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

        if (!myhero->is_recalling() && w->is_ready() && combo::use_w->get_bool() && combo::w_use_on_incoming_damage->get_bool())
        {
            for (auto& ally : entitylist->get_ally_heroes())
            {
                if (can_use_w_on(ally) && myhero->get_distance(ally) < w->range() && 
                    (health_prediction->get_incoming_damage(ally, combo::w_incoming_damage_time->get_int() / 1000.f, true) * 100.f) /
                    ally->get_max_health() > ally->get_health_percent() * (combo::w_over_hp_in_percent->get_int() / 100.f))
                {
                    if (w->cast(ally->get_position()))
                    {
                        return;
                    }
                }
            }
        }

        if (!myhero->is_recalling() && e->is_ready() && combo::use_e->get_bool() && combo::e_auto_spell_interrupter->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead())
                {
                    auto distance = myhero->get_distance(enemy);
                    if (distance < e->range() && enemy->is_casting_interruptible_spell())
                    {
                        if (distance >= combo::e_pull_if_distance_more_than->get_int())
                        {
                            cast_e_pull(enemy);
                        }
                        else
                        {
                            cast_e_push(enemy);
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
            if (q->is_ready() && combo::q_semi_manual_cast->get_bool())
            {
                q_logic();
            }

            if (w->is_ready() && combo::w_semi_manual_cast->get_bool())
            {
                w_logic_semi();
            }

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

                if (r->is_ready() && combo::use_r->get_bool())
                {
                    r_logic();
                }
            }

            //Checking if the user has selected harass() (Default C)
            if (orbwalker->harass())
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

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (e->is_ready() && fleemode::use_e->get_bool())
                {
                    e_logic();
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
                        if (e->cast_on_best_farm_position(laneclear::e_minimum_minions->get_int()))
                        {
                            return;
                        }
                    }
                }

                if (!monsters.empty())
                {
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast(monsters.front(), get_hitchance(hitchance::q_hitchance)))
                        {
                            return;
                        }
                    }
                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(1, true))
                        {
                            return;
                        }
                    }
                }
            }
        }
    }

#pragma region q_logic
    void q_logic()
    {        
        if (combo::use_q2->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && can_use_q_on(enemy) && (!enemy->is_under_ally_turret() || combo::allow_tower_dive->get_int()) && enemy->has_buff(buff_hash("ThreshQ")))
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
            }
        }

        // Get a target from a given range
        auto selected_target = target_selector->get_selected_target();
        auto target = selected_target != nullptr && selected_target->is_valid() && selected_target->is_valid_target(combo::q_max_range->get_int()) 
            ? selected_target : target_selector->get_target(combo::q_max_range->get_int(), damage_type::magical);

        // Minimum distance to target
        float min_distance = combo::thresh_mode->get_int() == 0 ? combo::q_min_range->get_int() : myhero->get_attack_range();

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_q_on(target) && myhero->get_distance(target) > min_distance && !target->has_buff(buff_hash("ThreshQ")))
        {
            q->cast(target, get_hitchance(hitchance::q_hitchance));
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        for (auto& target : entitylist->get_enemy_heroes())
        {
            if (target->is_valid() && !target->is_dead())
            {
                if (combo::w_to_ally_on_q_hit->get_bool() && target->has_buff(buff_hash("ThreshQ")))
                {
                    for (auto& ally : entitylist->get_ally_heroes())
                    {
                        if (ally->is_valid() && !ally->is_me() && !ally->is_dead() && can_use_w_on(ally) && ally->get_distance(target) > combo::w_to_ally_on_q_hit_ally_is_above_range->get_int() && myhero->get_distance(ally) < w->range())
                        {
                            if (w->cast(ally->get_position()))
                            {
                                return;
                            }
                        }
                    }
                }

                if (combo::w_to_ally_stunned->get_bool() && utils::has_crowd_control_buff(target) && myhero->get_distance(target) < combo::w_to_ally_stunned_target_is_below_range->get_int())
                {
                    for (auto& ally : entitylist->get_ally_heroes())
                    {
                        if (ally->is_valid() && !ally->is_me() && !ally->is_dead() && can_use_w_on(ally) && ally->get_distance(target) > combo::w_to_ally_stunned_ally_is_above_range->get_int() && myhero->get_distance(ally) < w->range())
                        {
                            if (w->cast(ally->get_position()))
                            {
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region w_logic_semi
    void w_logic_semi()
    {
        for (auto& ally : entitylist->get_ally_heroes())
        {
            if (ally->is_valid() && !ally->is_me() && !ally->is_dead() && can_use_w_on(ally) && myhero->get_distance(ally) < w->range())
            {
                if (w->cast(ally->get_position()))
                {
                    return;
                }
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_e_on(target))
        {
            if (!orbwalker->flee_mode() && myhero->get_distance(target) >= combo::e_pull_if_distance_more_than->get_int())
            {
                cast_e_pull(target);
            }
            else
            {
                cast_e_push(target);
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        if (myhero->count_enemies_in_range(r->range() - 100) >= combo::r_if_will_hit_x_targets->get_int() && (!combo::r_only_if_ally_nearby->get_bool() || myhero->count_allies_in_range(550) > 0))
        {
            r->cast();
        }
    }
#pragma endregion

#pragma region can_use_q_on
    bool can_use_q_on(game_object_script target)
    {
        auto it = combo::q_use_on.find(target->get_network_id());
        if (it == combo::q_use_on.end())
            return false;

        return it->second->get_bool();
    }
#pragma endregion

#pragma region can_use_w_on
    bool can_use_w_on(game_object_script target)
    {
        auto it = combo::w_use_on.find(target->get_network_id());
        if (it == combo::w_use_on.end())
            return false;

        return it->second->get_bool();
    }
#pragma endregion

#pragma region can_use_e_on
    bool can_use_e_on(game_object_script target)
    {
        auto it = combo::e_use_on.find(target->get_network_id());
        if (it == combo::e_use_on.end())
            return false;

        return it->second->get_bool();
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

    void cast_e_push(game_object_script target)
    {
        vector pos = target->get_position().extend(myhero->get_position(), myhero->get_distance(target->get_position()) - 200);
        e->cast(pos);
    }
    
    void cast_e_pull(game_object_script target)
    {
        vector pos = target->get_position().extend(myhero->get_position(), myhero->get_distance(target->get_position()) + 200);
        e->cast(pos);
    }

    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // Draw Q range
        if (q->is_ready() && draw_settings::draw_range_q->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::q_max_range->get_int(), draw_settings::q_color->get_color());

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_e->get_bool() && e->is_ready() && can_use_e_on(sender))
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                cast_e_push(sender);
            }
        }
    }
};