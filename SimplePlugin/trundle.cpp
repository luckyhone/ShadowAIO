#include "../plugin_sdk/plugin_sdk.hpp"
#include "trundle.h"
#include "utils.h"
#include "permashow.hpp"

namespace trundle
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
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_mode = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_mode = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_min_range = nullptr;
        TreeEntry* e_max_range = nullptr;
        TreeEntry* e_mode = nullptr;
        TreeEntry* e_only_if_target_is_moving = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_max_range = nullptr;
        TreeEntry* r_target_hp_under = nullptr;
        TreeEntry* r_dont_waste_if_target_hp_below = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
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
        TreeEntry* use_q_on_turret = nullptr;
        TreeEntry* use_w = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w;
    }

    namespace antigapclose
    {
        TreeEntry* use_e = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* e_hitchance = nullptr;
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
    void w_logic();
    void e_logic();
    void r_logic();
    void r_logic_semi();

    // Utils
    //
    vector get_pillar_position(game_object_script target);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, myhero->get_attack_range());
        w = plugin_sdk->register_spell(spellslot::w, 750);
        e = plugin_sdk->register_spell(spellslot::e, 1000);
        e->set_skillshot(0.75f, 112.5f, FLT_MAX, { }, skillshot_type::skillshot_circle);
        r = plugin_sdk->register_spell(spellslot::r, 650);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("trundle", "Trundle");
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
                    combo::q_mode = q_config->add_combobox(myhero->get_model() + ".combo.q.mode", "Q Mode", { {"Before AA", nullptr},{"After AA", nullptr } }, 1);
                }

                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());

                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_mode = w_config->add_combobox(myhero->get_model() + ".combo.w.mode", "W Mode", { {"Myhero Position", nullptr},{"Target Position", nullptr } }, 1);
                }

                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

                auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
                {
                    e_config->add_separator(myhero->get_model() + ".combo.e.separator1", "Range Settings");
                    combo::e_min_range = e_config->add_slider(myhero->get_model() + ".combo.e.min_range", "Minimum E range", myhero->get_attack_range() + 100, 1, e->range());
                    combo::e_max_range = e_config->add_slider(myhero->get_model() + ".combo.e.max_range", "Maximum E range", e->range() - 100, 1, e->range());

                    e_config->add_separator(myhero->get_model() + ".combo.e.separator2", "Usage Settings");
                    combo::e_mode = e_config->add_combobox(myhero->get_model() + ".combo.e.mode", "E Mode", { {"Standard", nullptr},{"Test 1", nullptr }, {"Test 2", nullptr } }, 0);
                    combo::e_only_if_target_is_moving = e_config->add_checkbox(myhero->get_model() + ".combo.e.only_if_target_is_moving", "Use E only if target is moving", false);
                    combo::e_only_if_target_is_moving->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                }

                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    r_config->add_separator(myhero->get_model() + ".combo.r.separator1", "Range Settings");
                    combo::r_max_range = r_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum R range", myhero->get_attack_range() + 125, 1, r->range());

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator2", "Usage Settings");
                    combo::r_target_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.target_hp_under", "Target HP is under (in %)", 65, 0, 100);
                    combo::r_dont_waste_if_target_hp_below = r_config->add_slider(myhero->get_model() + ".combo.r.dont_waste_if_target_hp_below", "Don't waste R if target hp is below (in %)", 25, 1, 100);

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator3", "Other Settings");
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_semi_manual_cast->set_tooltip("Automatically casts R on target with highest maximum health");

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
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", false);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_q_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q.on_turret", "Use Q On Turret", true);
                laneclear::use_q_on_turret->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", true);
                jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }


            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to ran away", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
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
            Permashow::Instance.AddElement("Semi Auto R", combo::r_semi_manual_cast);
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
        event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack);
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
            //Checking if the user is holding Semi Auto R key (Default T)
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
                // Get a target from a given range
                auto target = target_selector->get_target(e->range(), damage_type::physical);

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
                if (w->is_ready() && fleemode::use_w->get_bool())
                {
                    if (w->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
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
                        return !x->is_valid_target(q->range() + 300);
                    }), lane_minions.end());

                // You can use this function to delete monsters that aren't in the specified range
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(q->range() + 300);
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
                    if (w->is_ready() && laneclear::use_w->get_bool())
                    {
                        if (w->cast_on_best_farm_position(1))
                            return;
                    }
                }

                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->cast(monsters.front()->get_position()))
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
        auto target = target_selector->get_target(myhero->get_attack_range() + 50, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Checking if the target will die from q damage
            if (q->get_damage(target) >= target->get_health())
            {
                q->cast();
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
            w->cast(combo::w_mode->get_int() == 0 ? myhero->get_position() : target->get_position());
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && myhero->get_distance(target) > combo::e_min_range->get_int() && (target->is_moving() || !combo::e_only_if_target_is_moving->get_bool()))
        {
            if (combo::e_mode->get_int() == 0)
            {
                e->cast(target, utils::get_hitchance(hitchance::e_hitchance));
            }
            else
            {
                e->cast(get_pillar_position(target));
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && utils::enabled_in_map(combo::r_use_on, target))
        {
            if (target->get_health_percent() < combo::r_target_hp_under->get_int() && target->get_health_percent() > combo::r_dont_waste_if_target_hp_below->get_int())
            {
                r->cast(target);
            }
        }
    }
#pragma endregion

#pragma region r_logic_semi
    void r_logic_semi()
    {
        // Gets enemy heroes from the entitylist
        auto enemies = entitylist->get_enemy_heroes();

        // Delete invalid enemies
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return !x->is_valid_target(combo::r_max_range->get_int());
            }), enemies.end());

        // Sort enemies by max health
        std::sort(enemies.begin(), enemies.end(), [](game_object_script a, game_object_script b)
            {
                return a->get_max_health() > b->get_max_health();
            });

        if (!enemies.empty())
        {
            r->cast(enemies.front());
        }
    }
#pragma endregion

    void on_before_attack(game_object_script target, bool* process)
    {
        if (q->is_ready() && combo::q_mode->get_int() == 0)
        {
            // Using q before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (q->cast())
                {
                    return;
                }
            }

            // Using q before autoattack on minions
            if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_q->get_bool() && target->is_ai_minion())
            {
                if (q->cast())
                {
                    return;
                }
            }

            // Using q before autoattack on monsters
            if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool() && target->is_monster())
            {
                if (q->cast())
                {
                    return;
                }
            }
        }
    }

    void on_after_attack(game_object_script target)
    {
        if (q->is_ready())
        {
            // Using q after autoattack on enemies
            if (combo::q_mode->get_int() == 1)
            {
                if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using q after autoattack on minions
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_q->get_bool() && target->is_ai_minion())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using q after autoattack on monsters
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool() && target->is_monster())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
            }

            // Using q after autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_q_on_turret->get_bool() && target->is_ai_turret())
            {
                if (q->cast())
                {
                    return;
                }
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_e->get_bool() && e->is_ready())
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                e->cast(sender, utils::get_hitchance(hitchance::e_hitchance));
            }
        }
    }

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
            draw_manager->add_circle(myhero->get_position(), combo::e_max_range->get_int(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::r_max_range->get_int(), draw_settings::r_color->get_color());
    }

    vector to2D(vector vec)
    {
        return vector(vec.x, vec.y);
    }

    vector to3D(vector vec)
    {
        return vector(vec.x, vec.y, myhero->get_position().z);
    }

    vector add(vector vec, float fl)
    {
        return vector(vec.x + fl, vec.y + fl);
    }

    vector V2E(vector from, vector direction, float distance)
    {
        return to2D(from) + (to2D((direction - from).normalized()) * distance);
    }

    vector get_pillar_position(game_object_script target)
    {
        if (combo::e_mode->get_int() == 1)
        {
            return to3D(to2D(target->get_position()).extend(to2D(myhero->get_position()), -e->get_radius() / 2));
        }

        auto pos = myhero->get_position();
        return to3D(V2E(pos, target->get_position(), target->get_distance(pos) + 115));
    }
};