#include "../plugin_sdk/plugin_sdk.hpp"
#include "missfortune.h"

namespace missfortune
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
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
        TreeEntry* draw_damage_r = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_max_range = nullptr;
        TreeEntry* r_use_if_killable_by_x_waves = nullptr;
        TreeEntry* r_auto_if_enemies_more_than = nullptr;
        TreeEntry* r_auto_on_cc = nullptr;
        TreeEntry* r_cancel_if_nobody_inside = nullptr;
        TreeEntry* r_disable_orbwalker_moving = nullptr;
        TreeEntry* r_disable_evade = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
        bool previous_evade_state = false;
        bool previous_orbwalker_state = false;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_only_if_mana_more_than = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_w_on_turret = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace lasthit
    {
        TreeEntry* lasthit = nullptr;
        TreeEntry* use_q = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w;
        TreeEntry* use_e;
    }

    namespace antigapclose
    {
        TreeEntry* use_e = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* e_hitchance = nullptr;
        TreeEntry* r_hitchance = nullptr;
    }

    float last_r_time = 0.0f;

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack_orbwalker(game_object_script target, bool* process);
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void e_logic();
    bool r_logic();
    bool r_logic_auto();
    bool r_logic_semi();

    // Utils
    //
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, myhero->get_attack_range());
        w = plugin_sdk->register_spell(spellslot::w, 0);
        e = plugin_sdk->register_spell(spellslot::e, 1000);
        e->set_skillshot(0.25f, 200.f, FLT_MAX, { }, skillshot_type::skillshot_circle);
        r = plugin_sdk->register_spell(spellslot::r, 1450);
        r->set_skillshot(0.0f, 1450.0f, 2000.0f, { }, skillshot_type::skillshot_line);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("missfortune", "Miss Fortune");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W before AA", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R on killable", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_max_range = r_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum R range", 1200, 550, r->range());
                    combo::r_use_if_killable_by_x_waves = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_killable_by_x_waves", "Use if killable by x waves", 6, 1, 14);
                    combo::r_auto_if_enemies_more_than = r_config->add_slider(myhero->get_model() + ".combo.r.auto_if_enemies_more_than", "Auto R if hit enemies more than", 2, 1, 5);
                    combo::r_auto_on_cc = r_config->add_checkbox(myhero->get_model() + ".combo.r.auto_on_cc", "Auto R on CC", false);;
                    combo::r_cancel_if_nobody_inside = r_config->add_checkbox(myhero->get_model() + ".combo.r.cancel_if_nobody_inside", "Cancel R if nobody inside", false);
                    combo::r_disable_orbwalker_moving = r_config->add_checkbox(myhero->get_model() + ".combo.r.disable_orbwalker_moving", "Disable Orbwalker Moving on R", true);
                    combo::r_disable_evade = r_config->add_checkbox(myhero->get_model() + ".combo.r.disable_evade", "Disable Evade on R", true);

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
                auto e_config = harass->add_tab(myhero->get_model() + "harass.r.config", "E Config");
                {
                    harass::e_only_if_mana_more_than = e_config->add_slider(myhero->get_model() + ".harass.e.only_if_mana_more_than", "Use only if mana more than (in %)", 50, 0, 100);
                }
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 'H', true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_w_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w.on_turret", "Use W On Turret", true);
                laneclear::use_w_on_turret->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", true);
                jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", false);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto lasthit = main_tab->add_tab(myhero->get_model() + ".lasthit", "Last Hit Settings");
            {
                lasthit::lasthit = lasthit->add_hotkey(myhero->get_model() + ".lasthit.enabled", "Toggle Last Hit", TreeHotkeyMode::Toggle, 'J', true);
                lasthit::use_q = lasthit->add_checkbox(myhero->get_model() + ".lasthit.q", "Use Q", true);
                lasthit::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E to slow enemies", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
                draw_settings::draw_damage_r = draw_settings->add_checkbox(myhero->get_model() + "draw.R.damage", "Draw R Damage", true);
            }
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack_orbwalker);
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

        // Remove anti gapcloser handler
        //
        antigapcloser::remove_event_handler(on_gapcloser);

        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack_orbwalker);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if ((myhero->get_active_spell() != nullptr && myhero->get_active_spell()->is_channeling()) || gametime->get_time() - last_r_time < 0.3f)
        {
            if (combo::r_disable_orbwalker_moving->get_bool())
            {
                orbwalker->set_attack(false);
                orbwalker->set_movement(false);
                combo::previous_orbwalker_state = true;
            }
            if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
            {
                evade->disable_evade();
                combo::previous_evade_state = true;
            }

            if (combo::r_cancel_if_nobody_inside->get_bool())
            {
                std::vector<game_object_script> hit_by_r;

                for (auto& enemy : entitylist->get_enemy_heroes())
                {
                    if (enemy->is_valid() && enemy->is_valid_target(r->range()))
                    {
                        auto pred = prediction->get_prediction(enemy, r->get_delay(), r->get_radius(), r->get_speed());
                        if (pred.hitchance >= hit_chance::impossible)
                        {
                            hit_by_r.push_back(enemy);
                        }
                    }
                }

                if (hit_by_r.empty())
                {
                    if (combo::previous_orbwalker_state)
                    {
                        orbwalker->set_attack(true);
                        orbwalker->set_movement(true);
                        combo::previous_orbwalker_state = false;
                    }
                    if (combo::previous_evade_state)
                    {
                        evade->enable_evade();
                        combo::previous_evade_state = false;
                    }
                }
            }

            return;
        }

        if (myhero->get_active_spell() == nullptr)
        {
            if (combo::previous_orbwalker_state)
            {
                orbwalker->set_attack(true);
                orbwalker->set_movement(true);
                combo::previous_orbwalker_state = false;
            }
            if (combo::previous_evade_state)
            {
                evade->enable_evade();
                combo::previous_evade_state = false;
            }
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (r->is_ready() && combo::use_r->get_bool())
            {
                if (r_logic_auto())
                {
                    return;
                }
                if (r_logic_semi())
                {
                    return;
                }
            }

            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (r->is_ready() && combo::use_r->get_bool())
                {
                    if (r_logic())
                    {
                        return;
                    }
                }

                if (q->is_ready() && combo::use_q->get_bool())
                {
                    q_logic();
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
                    if (q->is_ready() && lasthit::use_q->get_bool())
                    {
                        for (auto&& minion : lane_minions)
                        {
                            if (q->get_damage(minion) > minion->get_health())
                            {
                                if (q->cast(minion))
                                {
                                    return;
                                }
                            }
                        }
                    }
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
                        if (myhero->get_mana_percent() > harass::e_only_if_mana_more_than->get_int())
                        {
                            e_logic();
                        }
                    }
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (w->is_ready() && fleemode::use_w->get_bool())
                {
                    if (w->cast())
                    {
                        return;
                    }
                }
                if (e->is_ready() && fleemode::use_e->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(e->range(), damage_type::magical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (e->cast(target, get_hitchance(hitchance::e_hitchance)))
                        {
                            return;
                        }
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
                    if (q->is_ready() && laneclear::use_q->get_bool())
                    {
                        if (q->cast(lane_minions.front()))
                        {
                            return;
                        }
                    }

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(1))
                        {
                            return;
                        }
                    }
                }


                if (!monsters.empty())
                {
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast(monsters.front()))
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
        // Get a target from a given range
        auto target = target_selector->get_target(q->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            q->cast(target);
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            e->cast(target, get_hitchance(hitchance::e_hitchance));
        }
    }
#pragma endregion

#pragma region r_logic
    bool r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && target->is_attack_allowed_on_target())
        {
            if (can_use_r_on(target))
            {
                if (r->get_damage(target) * combo::r_use_if_killable_by_x_waves->get_int() > target->get_health())
                {
                    auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                    if (pred.hitchance >= get_hitchance(hitchance::e_hitchance))
                    {
                        if (combo::r_disable_orbwalker_moving->get_bool())
                        {
                            orbwalker->set_attack(false);
                            orbwalker->set_movement(false);
                            combo::previous_orbwalker_state = true;
                        }
                        if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                        {
                            evade->disable_evade();
                            combo::previous_evade_state = true;
                        }
                        if (r->cast(pred.get_unit_position()))
                        {
                            last_r_time = gametime->get_time();
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
#pragma endregion

#pragma region r_logic_auto
    bool r_logic_auto()
    {
        std::vector<game_object_script> hit_by_r;

        for (auto& enemy : entitylist->get_enemy_heroes())
        {
            if (enemy->is_valid() && enemy->is_valid_target(combo::r_max_range->get_int()))
            {
                auto pred = prediction->get_prediction(enemy, r->get_delay(), r->get_radius(), r->get_speed());
                if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
                {
                    hit_by_r.push_back(enemy);
                }
            }
        }

        if (hit_by_r.size() >= combo::r_auto_if_enemies_more_than->get_int())
        {
            auto pred = prediction->get_prediction(hit_by_r.front(), r->get_delay(), r->get_radius(), r->get_speed());
            if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
            {
                if (combo::r_disable_orbwalker_moving->get_bool())
                {
                    orbwalker->set_attack(false);
                    orbwalker->set_movement(false);
                    combo::previous_orbwalker_state = true;
                }
                if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                {
                    evade->disable_evade();
                    combo::previous_evade_state = true;
                }
                if (r->cast(pred.get_unit_position()))
                {
                    last_r_time = gametime->get_time();
                    return true;
                }
            }
        }

        if (combo::r_auto_on_cc->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (can_use_r_on(target))
                {
                    auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                    if (pred.hitchance >= hit_chance::immobile)
                    {
                        if (combo::r_disable_orbwalker_moving->get_bool())
                        {
                            orbwalker->set_attack(false);
                            orbwalker->set_movement(false);
                            combo::previous_orbwalker_state = true;
                        }
                        if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                        {
                            evade->disable_evade();
                            combo::previous_evade_state = true;
                        }
                        if (r->cast(pred.get_unit_position()))
                        {
                            last_r_time = gametime->get_time();
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
#pragma endregion

#pragma region r_logic_semi
    bool r_logic_semi()
    {
        if (combo::r_semi_manual_cast->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(r->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (can_use_r_on(target))
                {
                    auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                    if (pred.hitchance >= get_hitchance(hitchance::e_hitchance))
                    {
                        if (combo::r_disable_orbwalker_moving->get_bool())
                        {
                            orbwalker->set_attack(false);
                            orbwalker->set_movement(false);
                            combo::previous_orbwalker_state = true;
                        }
                        if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                        {
                            evade->disable_evade();
                            combo::previous_evade_state = true;
                        }
                        if (r->cast(pred.get_unit_position()))
                        {
                            last_r_time = gametime->get_time();
                            return true;
                        }
                    }
                }
            }
        }

        return false;
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
            break;
        case 1:
            return hit_chance::medium;
            break;
        case 2:
            return hit_chance::high;
            break;
        case 3:
            return hit_chance::very_high;
            break;
        }
        return hit_chance::medium;
    }
#pragma endregion

    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color)
    {
        if (target != nullptr && target->is_valid() && target->is_hpbar_recently_rendered())
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

    void on_draw()
    {

        if (myhero->is_dead())
        {
            return;
        }

        // Draw Q range
        if (q->is_ready() && draw_settings::draw_range_q->get_bool())
            draw_manager->add_circle(myhero->get_position(), q->range(), draw_settings::q_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::r_max_range->get_int(), draw_settings::r_color->get_color());

        auto pos = myhero->get_position();
        renderer->world_to_screen(pos, pos);
        auto semi = combo::r_semi_manual_cast->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 8), (semi ? 0xFF00FF00 : 0xFF0000FF), 14, "SEMI R %s", (semi ? "ON" : "OFF"));
        auto lasthit = lasthit::lasthit->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 24), (lasthit ? 0xFF00FF00 : 0xFF0000FF), 14, "LASTHIT % s", (lasthit ? "ON" : "OFF"));
        auto spellfarm = laneclear::spell_farm->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 40), (spellfarm ? 0xFF00FF00 : 0xFF0000FF), 14, "FARM %s", (spellfarm ? "ON" : "OFF"));

        if (draw_settings::draw_damage_r->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (!enemy->is_dead() && enemy->is_valid() && enemy->is_hpbar_recently_rendered() && r->is_ready())
                {
                    draw_dmg_rl(enemy, r->get_damage(enemy) * combo::r_use_if_killable_by_x_waves->get_int(), 0x8000ff00);
                }
            }
        }
    }

    void on_before_attack_orbwalker(game_object_script target, bool* process)
    {
        // Use w before autoattack on enemies
        if (target->is_ai_hero())
        {
            if (combo::use_w->get_bool())
            {
                if (w->cast())
                {
                    return;
                }
            }
        }

        // Use w before autoattack on lane minions
        if (target->is_minion() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_w->get_bool())) {
            if (w->cast())
            {
                return;
            }
        }

        // Use w before autoattack on monsters
        if (target->is_monster() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_w->get_bool())) {
            if (w->cast())
            {
                return;
            }
        }

        // Use w before autoattack on turrets
        if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_w_on_turret->get_bool() && target->is_ai_turret())
        {
            if (w->cast())
            {
                return;
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_e->get_bool() && e->is_ready())
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                e->cast(sender, get_hitchance(hitchance::e_hitchance));
            }
        }
    }
};