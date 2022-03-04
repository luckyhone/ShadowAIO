#include "../plugin_sdk/plugin_sdk.hpp"
#include "masteryi.h"

namespace masteryi
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
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_dont_use_target_under_turret = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr; 
        TreeEntry* r_use_before_aa = nullptr;
        TreeEntry* r_use_to_chase = nullptr;
        TreeEntry* r_chase_search_range = nullptr;
        TreeEntry* r_dont_use_target_under_turret = nullptr;
        TreeEntry* r_only_when_enemies_more_than = nullptr;
        TreeEntry* r_check_for_enemies_nearby_before_aa = nullptr;
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
        TreeEntry* q_only_when_minions_more_than = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_r = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
        TreeEntry* r_myhero_hp_under = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_q = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);
    void on_before_attack(game_object_script target, bool* process);
    void on_after_attack(game_object_script target);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void r_logic();


    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 600);
        w = plugin_sdk->register_spell(spellslot::w, 0);
        e = plugin_sdk->register_spell(spellslot::e, 0);
        r = plugin_sdk->register_spell(spellslot::r, 0);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("masteryi", "Master Yi");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = combo->add_tab(myhero->get_model() + ".combo.q.config", "Q Config");
                {
                    combo::q_dont_use_target_under_turret = q_config->add_checkbox(myhero->get_model() + ".combo.q.dont_use_under_enemy_turret", "Dont use under enemy turret", true);
                }
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W to reset AA", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_use_before_aa = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_before_aa", "Use before AA", true);
                    combo::r_use_to_chase = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_to_chase", "Use to chase enemies", true);
                    combo::r_chase_search_range = r_config->add_slider(myhero->get_model() + ".combo.r.chase_search_range", "Chase enemies search range", 1200, 300, 1600);
                    combo::r_dont_use_target_under_turret = r_config->add_checkbox(myhero->get_model() + ".combo.r.dont_use_target_under_turret", "Dont use if target is under turret", true);
                    combo::r_only_when_enemies_more_than = r_config->add_slider(myhero->get_model() + ".combo.r.use_only_when_enemies_more_than", "Use only when enemies more than", 2, 0, 4);
                    combo::r_check_for_enemies_nearby_before_aa = r_config->add_checkbox(myhero->get_model() + ".combo.r.check_for_enemies_nearby_before_aa", "Check for enemies nearby before AA", true);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W to reset AA", false);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 'H', true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = laneclear->add_tab(myhero->get_model() + ".laneclear.q.config", "Q Config");
                {
                    laneclear::q_only_when_minions_more_than = q_config->add_slider(myhero->get_model() + ".laneclear.q.use_only_when_minions_more_than", "Use only when minions more than", 3, 0, 5);
                }
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", false);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_r = fleemode->add_checkbox(myhero->get_model() + ".flee.r", "Use R to escape", true);
                fleemode::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = fleemode->add_tab(myhero->get_model() + ".flee.r.config", "R Config");
                {
                    fleemode::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".flee.r.only_when_enemies_nearby", "Use only when enemies nearby", true);
                    fleemode::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".flee.r.myhero_hp_under", "Myhero HP is under (in %)", 40, 0, 100);
                }
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_q = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.q", "Use Q", true);
                antigapclose::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }   


            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
            }
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
        event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack);
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
        menu->delete_tab("masteryi");

        // Remove anti gapcloser handler
        //
        antigapcloser::remove_event_handler(on_gapcloser);

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
            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (q->is_ready() && combo::use_q->get_bool())
                {
                    q_logic();
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
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (r->is_ready() && fleemode::use_r->get_bool())
                {
                    if (myhero->get_health_percent() < fleemode::r_myhero_hp_under->get_int())
                    {
                        if (!fleemode::r_only_when_enemies_nearby->get_bool() || myhero->count_enemies_in_range(900) != 0)
                        {
                            if (r->cast())
                            {
                                return;
                            }
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

                if (!lane_minions.empty() && lane_minions.size() >= laneclear::q_only_when_minions_more_than->get_int())
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
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast(monsters.front()))
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
            if (!combo::q_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
            {
                if (q->cast(target))
                {
                    return;
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        if (r->is_ready())
        {
            if (combo::r_use_to_chase->get_bool() && myhero->count_enemies_in_range(combo::r_chase_search_range->get_int()) >= combo::r_only_when_enemies_more_than->get_int())
            {
                // Get a target from a given range
                auto target = target_selector->get_target(combo::r_chase_search_range->get_int(), damage_type::physical);

                // Always check an object is not a nullptr!
                if (target != nullptr)
                {
                    if (!combo::r_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
                    {
                        if (r->cast())
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
#pragma endregion

    void on_before_attack(game_object_script target, bool* process)
    {
        if (e->is_ready())
        {
            // Use e before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
            {
                if (e->cast())
                {
                    return;
                }
            }

            // Use e before autoattack on lane minions
            if (target->is_minion() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_e->get_bool())) {
                if (e->cast())
                {
                    return;
                }
            }

            // Use e before autoattack on monsters
            if (target->is_monster() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_e->get_bool())) {
                if (e->cast())
                {
                    return;
                }
            }
        }

        if (r->is_ready()) {
            // Use r before autoattack on enemies
            if (target->is_ai_hero() && (orbwalker->combo_mode() && combo::use_r->get_bool()))
            {
                if ((myhero->count_enemies_in_range(combo::r_chase_search_range->get_int()) >= combo::r_only_when_enemies_more_than->get_int()) || !combo::r_check_for_enemies_nearby_before_aa->get_bool())
                {
                    if (!combo::r_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
                    {
                        if (r->cast())
                        {
                            return;
                        }
                    }
                }
            }
        }
    }

    void on_after_attack(game_object_script target)
    {
        if (w->is_ready())
        {
            // Use w to reset AA
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_w->get_bool())))
            {
                if (w->cast())
                {
                    return;
                }
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_q->get_bool() && q->is_ready())
        {
            if (sender->is_valid_target(q->range() + sender->get_bounding_radius()))
            {
                q->cast(sender);
            }
        }
    }

    void on_draw()
    {

        if (myhero->is_dead())
        {
            return;
        }

        // Draw q range
        if (q->is_ready() && draw_settings::draw_range_q->get_bool())
            draw_manager->add_circle(myhero->get_position(), q->range(), draw_settings::q_color->get_color());

        auto pos = myhero->get_position();
        renderer->world_to_screen(pos, pos);
        auto spellfarm = laneclear::spell_farm->get_bool();
        draw_manager->add_text_on_screen(pos + vector(0, 40), (spellfarm ? 0xFF00FF00 : 0xFF0000FF), 14, "FARM %s", (spellfarm ? "ON" : "OFF"));
    }
};