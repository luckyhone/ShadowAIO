#include "../plugin_sdk/plugin_sdk.hpp"
#include "jax.h"

namespace jax
{

// To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;
    script_spell* ward = nullptr;

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
        TreeEntry* draw_range_ward = nullptr;
        TreeEntry* ward_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_only_when_e_ready = nullptr;
        TreeEntry* q_dont_use_under_enemy_turret = nullptr;
        TreeEntry* q_target_above_range = nullptr;
        std::map<std::uint32_t, TreeEntry*> q_use_on;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_auto_recast_if_enemy_leaving_range = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_myhero_hp_under = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
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
        TreeEntry* use_w_on_turret = nullptr;
        TreeEntry* use_e = nullptr;
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
        TreeEntry* q_jump_on_ally_champions = nullptr;
        TreeEntry* q_jump_on_ally_minions = nullptr;
        TreeEntry* q_ward_jump = nullptr;
    }

    namespace misc
    {
        TreeEntry* ward_jump = nullptr;
        TreeEntry* ward_jump_key = nullptr;
    }

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
    void ward_jump_logic();

    // Utils
    //
    bool can_use_q_on(game_object_script target);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 700);
        w = plugin_sdk->register_spell(spellslot::w, myhero->get_attack_range() + 50);
        e = plugin_sdk->register_spell(spellslot::e, 300);
        r = plugin_sdk->register_spell(spellslot::r, 0);
        ward = plugin_sdk->register_spell(spellslot::trinket, 600);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("jax", "Jax");
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
                    combo::q_only_when_e_ready = q_config->add_checkbox(myhero->get_model() + ".combo.q.only_when_e_ready", "Use Q only when E is ready", false);
                    combo::q_dont_use_under_enemy_turret = q_config->add_checkbox(myhero->get_model() + ".combo.q.dont_use_under_enemy_turret", "Dont use under enemy turret", true);
                    combo::q_target_above_range = q_config->add_slider(myhero->get_model() + ".combo.q.target_above_range", "Only if target is above range", myhero->get_attack_range(), 0, q->range());

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
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    combo::e_auto_recast_if_enemy_leaving_range = e_config->add_checkbox(myhero->get_model() + ".combo.e.auto_recast_if_enemy_leaving_range", "Auto E2 if enemy leaving E range", true);
                }
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under", "Myhero HP is under (in %)", 50, 0, 100);
                    combo::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.only_when_enemies_nearby", "Only when enemies are nearby", true);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", false);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", false);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 'H', true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", false);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", true);
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


            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "Use Q to jump on ally", true);
                fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = fleemode->add_tab(myhero->get_model() + ".flee.q.config", "Q Config");
                {
                    fleemode::q_jump_on_ally_champions = q_config->add_checkbox(myhero->get_model() + ".flee.q.jump_on_ally_champions", "Jump on ally champions", true);
                    fleemode::q_jump_on_ally_minions = q_config->add_checkbox(myhero->get_model() + ".flee.q.jump_on_ally_minions", "Jump on ally minions", true);
                    fleemode::q_ward_jump = q_config->add_checkbox(myhero->get_model() + ".flee.q.ward_jump", "Ward jump", true);
                }
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Miscellaneous Settings");
            {
                misc::ward_jump = misc->add_checkbox(myhero->get_model() + ".misc.ward_jump", "Ward Jump", true);
                misc::ward_jump_key = misc->add_hotkey(myhero->get_model() + ".misc.stealth_recall.key", "Ward Jump Key", TreeHotkeyMode::Hold, 'T', true);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.ewcolor", "W Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_ward = draw_settings->add_checkbox(myhero->get_model() + ".draw.ward", "Draw Ward range", true);
                draw_settings::ward_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.ward.color", "Ward Color", color);
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

        if (r->is_ready() && combo::use_r->get_bool())
        {
            r_logic();
        }

        if (e->is_ready() && combo::e_auto_recast_if_enemy_leaving_range->get_bool() && myhero->has_buff(buff_hash("JaxCounterStrike")))
        {
            // Get a target from a given range
            auto target = target_selector->get_target(e->range() + 25, damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr && target->is_attack_allowed_on_target())
            {
                if (myhero->get_distance(target) >= e->range() - 30)
                {
                    if (e->cast())
                    {
                        return;
                    }
                }
            }
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (q->is_ready() && misc::ward_jump->get_bool() && misc::ward_jump_key->get_bool())
            {
                ward_jump_logic();
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
            }

            //Checking if the user has selected harass() (Default C)
            if (orbwalker->harass())
            {
                // Get a target from a given range
                auto target = target_selector->get_target(e->range(), damage_type::physical);

                // Always check an object is not a nullptr!
                if (target != nullptr)
                {
                    if (!target->is_under_ally_turret())
                    {
                        if (q->is_ready() && harass::use_q->get_bool())
                        {
                            q_logic();
                        }
                    }

                    if (!myhero->is_under_enemy_turret())
                    {
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
                    std::vector<game_object_script> allies;

                    if (fleemode::q_jump_on_ally_champions->get_bool())
                    {
                        auto champions = entitylist->get_ally_heroes();
                        allies.insert(allies.end(), champions.begin(), champions.end());
                    }

                    if (fleemode::q_jump_on_ally_minions->get_bool())
                    {
                        auto minions = entitylist->get_ally_minions();
                        allies.insert(allies.end(), minions.begin(), minions.end());
                    }

                    //std::sort -> sort lane minions by distance
                    std::sort(allies.begin(), allies.end(), [](game_object_script a, game_object_script b)
                        {
                            return a->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < b->get_distance(hud->get_hud_input_logic()->get_game_cursor_position());
                        });

                    // You can use this function to delete allies that aren't in the specified range
                    allies.erase(std::remove_if(allies.begin(), allies.end(), [](game_object_script x)
                        {
                            return x == myhero || x->get_distance(myhero->get_position()) > q->range();
                        }), allies.end());

                    if (!allies.empty())
                    {
                        if (q->cast(allies.front()))
                        {
                            return;
                        }
                    }

                    ward_jump_logic();
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
                                if (q->cast(lane_minions.front()))
                                {
                                    return;
                                }
                            }
                        }
                        if (q->cast(lane_minions.front()))
                            return;
                    }

                    if (w->is_ready() && laneclear::use_w->get_bool())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (w->cast())
                                {
                                    return;
                                }
                            }
                        }
                        if (w->cast())
                            return;
                    }

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (e->cast())
                                {
                                    return;
                                }
                            }
                        }
                        if (e->cast())
                            return;
                    }
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast(monsters.front()))
                            return;
                    }

                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->cast())
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
        if (target != nullptr && can_use_q_on(target))
        {
            if (!combo::q_dont_use_under_enemy_turret->get_bool() || !target->is_under_ally_turret())
            {
                if (target->get_distance(myhero) > combo::q_target_above_range->get_int())
                {
                    if (combo::q_only_when_e_ready->get_bool())
                    {
                        if (e->is_ready())
                        {
                            e->cast();
                            q->cast(target);
                        }
                        return;
                    }

                    if (e->is_ready())
                    {
                        e->cast();
                    }

                    if (q->cast(target))
                        return;
                }
            }
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(myhero->get_attack_range() + 50, damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Checking if the target will die from W damage
            if (w->get_damage(target) >= target->get_health())
            {
                if (w->cast())
                    return;
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
            if (!myhero->has_buff(buff_hash("JaxCounterStrike")))
            {
                e->cast();
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        if (r->is_ready() && combo::use_r->get_bool())
        {
            if (combo::r_only_when_enemies_nearby->get_bool() && myhero->count_enemies_in_range(1000) == 0)
            {
                return;
            }

            if (myhero->get_health_percent() < combo::r_myhero_hp_under->get_int())
            {
                if (r->cast())
                {
                    return;
                }
            }
        }
    }
#pragma endregion

#pragma region ward_jump_logic
    void ward_jump_logic()
    {
        game_object_script near_ward = nullptr;

        for (auto& object : entitylist->get_other_minion_objects())
        {
            if (object->is_valid())
            {
                if (object->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < 100)
                {
                    if (myhero->is_facing(object))
                    {
                        if (object->get_name().compare("SightWard") == 0)
                        {
                            near_ward = object;
                        }
                    }
                }
            }
        }

        if (near_ward == nullptr)
        {
            if (ward->is_ready() && fleemode::q_ward_jump->get_bool())
            {
                if (ward->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                {
                    return;
                }
            }
        }
        else
        {
            q->cast(near_ward);
        }
    }
#pragma endregion

    void on_before_attack(game_object_script target, bool* process)
    {
        if (w->is_ready())
        {
            // Using w before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_w->get_bool())))
            {
                if (w->cast())
                {
                    return;
                }
            }

            // Using w before autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_w_on_turret->get_bool() && target->is_ai_turret())
            {
                if (w->cast())
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
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw Ward range
        if (ward->is_ready() && draw_settings::draw_range_ward->get_bool())
            draw_manager->add_circle(myhero->get_position(), ward->range(), draw_settings::ward_color->get_color());

        auto pos = myhero->get_position();
        renderer->world_to_screen(pos, pos);
        auto spellfarm = laneclear::spell_farm->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 40), (spellfarm ? 0xFF00FF00 : 0xFF0000FF), 14, "FARM %s", (spellfarm ? "ON" : "OFF"));
    }

#pragma region can_use_q_on
    bool can_use_q_on(game_object_script target)
    {
        auto it = combo::q_use_on.find(target->get_network_id());
        if (it == combo::q_use_on.end())
            return false;

        return it->second->get_bool();
    }
#pragma endregion
};