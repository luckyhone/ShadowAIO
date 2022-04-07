#include "../plugin_sdk/plugin_sdk.hpp"
#include "gwen.h"

namespace gwen
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
            TreeEntry* r_damage = nullptr;
        }
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_only_on_stacks = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_damage_time = nullptr;
        TreeEntry* w_over_my_hp_in_percent = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_mode = nullptr;
        TreeEntry* e_only_above_aa_range = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_max_range = nullptr;
        TreeEntry* r_target_hp_under = nullptr;
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
        TreeEntry* q_only_on_stacks = nullptr;
        TreeEntry* q_minimum_minions = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_e_on_turret = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_only_on_stacks = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_e;
    }

    namespace hitchance
    {
        TreeEntry* r_hitchance = nullptr;
    }

    float last_r_time = 0.0f;

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script target, bool* process);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void r_logic();
    void r_logic_semi();

    // Utils
    //
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);
    int get_gwen_q_stacks();

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 450);
        q->set_skillshot(0.5f, 250.0f, FLT_MAX, { }, skillshot_type::skillshot_line);
        w = plugin_sdk->register_spell(spellslot::w, 425);
        e = plugin_sdk->register_spell(spellslot::e, 350);
        r = plugin_sdk->register_spell(spellslot::r, 1200);
        r->set_skillshot(0.25f, 150.0f, 1800.0f, { collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("gwen", "Gwen");
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
                    combo::q_only_on_stacks = q_config->add_slider(myhero->get_model() + ".combo.only_on_stacks", "Use Q only on x stacks", 4, 1, 4);
                }

                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());

                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_damage_time = w_config->add_slider(myhero->get_model() + ".combo.w.damage_time", "Set coming damage time (in ms)", 750, 0, 1000);
                    combo::w_over_my_hp_in_percent = w_config->add_slider(myhero->get_model() + ".combo.w.over_my_hp_in_percent", "Coming damage is over my HP (in %)", 10, 0, 100);
                }

                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    combo::e_mode = e_config->add_combobox(myhero->get_model() + ".combo.e.mode", "E Mode", { {"Cursor Position", nullptr},{"Enemy Position", nullptr } }, 0);
                    combo::e_only_above_aa_range = e_config->add_checkbox(myhero->get_model() + ".combo.e.only_above_aa_range", "Use only above AA range", true);
                }

                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_max_range = r_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum R range", 525, 100, r->range());
                    combo::r_target_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.target_hp_under", "Target HP is under (in %)", 50, 0, 100);

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
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", false);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", false);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = laneclear->add_tab(myhero->get_model() + ".laneclear.q.config", "Q Config");
                {
                    laneclear::q_only_on_stacks = q_config->add_slider(myhero->get_model() + ".laneclear.q.only_on_stacks", "Use Q only on x stacks", 4, 1, 4);
                    laneclear::q_minimum_minions = q_config->add_slider(myhero->get_model() + ".laneclear.q.minimum_minions", "Minimum minions", 2, 1, 5);
                }
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                laneclear::use_e_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e.on_turret", "Use E On Turret", true);
                laneclear::use_e_on_turret->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = jungleclear->add_tab(myhero->get_model() + ".jungleclear.q.config", "Q Config");
                {
                    jungleclear::q_only_on_stacks = q_config->add_slider(myhero->get_model() + ".jungleclear.q.only_on_stacks", "Use Q only on x stacks", 4, 1, 4);
                }
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
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


                auto draw_damage = draw_settings->add_tab(myhero->get_model() + ".draw.damage", "Draw Damage");
                {
                    draw_settings::draw_damage_settings::draw_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.enabled", "Draw Combo Damage", true);
                    draw_settings::draw_damage_settings::q_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.q", "Draw Q Damage", true);
                    draw_settings::draw_damage_settings::q_damage->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    draw_settings::draw_damage_settings::r_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.r", "Draw R Damage", true);
                    draw_settings::draw_damage_settings::r_damage->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                }
            }
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
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

        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            //for (auto&& buff : myhero->get_bufflist())
            //{
            //    if (buff->is_valid() && buff->is_alive())
            //    {
            //        console->print("[ShadowAIO] [DEBUG] Buff name %s", buff->get_name_cstr());
            //    }
            //}

            if (w->is_ready() && combo::use_w->get_bool())
            {
                w_logic();
            }

            if (r->is_ready() && combo::use_r->get_bool() && combo::r_semi_manual_cast->get_bool())
            {
                r_logic_semi();
            }

            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (q->is_ready() && combo::use_q->get_bool())
                {
                    q_logic();
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
                    if (q->is_ready() && laneclear::use_q->get_bool() && get_gwen_q_stacks() >= laneclear::q_only_on_stacks->get_int())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (q->cast_on_best_farm_position(laneclear::q_minimum_minions->get_int()))
                                {
                                    return;
                                }
                            }
                        }
                        if (q->cast_on_best_farm_position(laneclear::q_minimum_minions->get_int()))
                            return;
                    }

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                                {
                                    return;
                                }
                            }
                        }
                        if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                            return;
                    }
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (q->is_ready() && jungleclear::use_q->get_bool() && get_gwen_q_stacks() >= jungleclear::q_only_on_stacks->get_int())
                    {
                        if (q->cast_on_best_farm_position(1, true))
                            return;
                    }

                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
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
            if (get_gwen_q_stacks() >= combo::q_only_on_stacks->get_int())
            {
                q->cast(target);
            }
        }
    }
#pragma endregion


#pragma region w_logic
    void w_logic()
    {
        if (!myhero->has_buff(buff_hash("gwenwuntargetabilitymanager")))
        {
            if ((health_prediction->get_incoming_damage(myhero, combo::w_damage_time->get_int() / 1000.f, true) * 100.f) /
                myhero->get_max_health() > myhero->get_health_percent() * (combo::w_over_my_hp_in_percent->get_int() / 100.f))
            {
                auto enemies = entitylist->get_enemy_heroes();

                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
                    {
                        return myhero->get_distance(x) > 900;
                    }), enemies.end());

                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
                    {
                        return myhero->get_distance(x) < w->range();
                    }), enemies.end());

                if (!enemies.empty())
                {
                    w->cast();
                }
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range() + myhero->get_attack_range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (!combo::e_only_above_aa_range->get_bool() || myhero->get_distance(target) > myhero->get_attack_range() + 50)
            {
                if (combo::e_mode->get_int() == 0)
                {
                    e->cast(hud->get_hud_input_logic()->get_game_cursor_position());
                }
                else
                {
                    e->cast(target);
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        bool is_recast = myhero->has_buff(buff_hash("GwenRRecast"));
        
        // Get a target from a given range
        auto target = target_selector->get_target(is_recast ? r->range() : combo::r_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_r_on(target))
        {
            if (target->get_health_percent() < combo::r_target_hp_under->get_int() || is_recast)
            {
                prediction_input x;

                x._from = myhero->get_position();
                x.unit = target;
                x.delay = is_recast ? 0.50f : r->delay;
                x.radius = r->radius;
                x.speed = r->speed;
                x.collision_objects = r->get_collision_flags();
                x.range = r->range();
                x.type = skillshot_type::skillshot_cone;
                x.spell_slot = r->get_slot();
                x.use_bounding_radius = true;

                auto output = prediction->get_prediction(&x);

                if (output.hitchance >= get_hitchance(hitchance::r_hitchance))
                {
                    if (gametime->get_time() - last_r_time > 1.0f)
                    {
                        if (r->cast(output.get_cast_position()))
                        {
                            last_r_time = gametime->get_time();
                        }
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic_semi
    void r_logic_semi()
    {
        bool is_recast = myhero->has_buff(buff_hash("GwenRRecast"));

        // Get a target from a given range
        auto target = target_selector->get_target(is_recast ? r->range() : combo::r_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_r_on(target))
        {
            prediction_input x;

            x._from = myhero->get_position();
            x.unit = target;
            x.delay = is_recast ? 0.50f : r->delay;
            x.radius = r->radius;
            x.speed = r->speed;
            x.collision_objects = r->get_collision_flags();
            x.range = r->range();
            x.type = skillshot_type::skillshot_cone;
            x.spell_slot = r->get_slot();
            x.use_bounding_radius = true;

            auto output = prediction->get_prediction(&x);

            if (output.hitchance >= get_hitchance(hitchance::r_hitchance))
            {
                if (gametime->get_time() - last_r_time > 1.0f)
                {
                    if (r->cast(output.get_cast_position()))
                    {
                        last_r_time = gametime->get_time();
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

    void on_before_attack(game_object_script target, bool* process)
    {
        if (e->is_ready())
        {
            // Using e before autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_e_on_turret->get_bool() && target->is_ai_turret())
            {
                if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                {
                    return;
                }
            }
        }
    }

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


    int get_gwen_q_stacks()
    {
        auto buff = myhero->get_buff(buff_hash("GwenQ"));
        if (buff != nullptr && buff->is_valid() && buff->is_alive())
        {
            return buff->get_count();
        }
        return 0;
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
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::r_max_range->get_int(), draw_settings::r_color->get_color());

        auto pos = myhero->get_position();
        renderer->world_to_screen(pos, pos);
        if (combo::use_r->get_bool())
        {
            auto semi = combo::r_semi_manual_cast->get_bool();
            draw_manager->add_text_on_screen(pos + vector(0, 24), (semi ? 0xFF00FF00 : 0xFF0000FF), 14, "SEMI R %s", (semi ? "ON" : "OFF"));
        }
        auto spellfarm = laneclear::spell_farm->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 40), (spellfarm ? 0xFF00FF00 : 0xFF0000FF), 14, "FARM %s", (spellfarm ? "ON" : "OFF"));

        if (draw_settings::draw_damage_settings::draw_damage->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (!enemy->is_dead() && enemy->is_valid() && enemy->is_hpbar_recently_rendered())
                {
                    int damage = 0;

                    if (q->is_ready() && draw_settings::draw_damage_settings::q_damage->get_bool())
                        damage += q->get_damage(enemy);

                    if (r->is_ready() && can_use_r_on(enemy) && draw_settings::draw_damage_settings::r_damage->get_bool())
                        damage += r->get_damage(enemy) * 3.0f;

                    if (damage != 0)
                        draw_dmg_rl(enemy, damage, 0x8000ff00);
                }
            }
        }
    }
};