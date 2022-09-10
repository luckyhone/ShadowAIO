#include "../plugin_sdk/plugin_sdk.hpp"
#include "teemo.h"
#include "utils.h"
#include "permashow.hpp"

namespace teemo
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* r = nullptr;

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_q = nullptr;
        TreeEntry* q_color = nullptr;
        TreeEntry* draw_damage_q = nullptr;
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
        TreeEntry* r_draw_best_locations = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_mode = nullptr;
        TreeEntry* q_auto_harass = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_target_is_above_range = nullptr;
        TreeEntry* w_check_if_target_is_not_facing = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_target_hp_under = nullptr;
        TreeEntry* r_auto_on_cc = nullptr;
        TreeEntry* r_auto_on_best_locations = nullptr;
        std::map<std::uint32_t, TreeEntry*> q_use_on;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
    }

    namespace lasthit
    {
        TreeEntry* lasthit = nullptr;
        TreeEntry* use_q = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_r = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* r_hitchance = nullptr;
    }

    // Champion data
    //
    float last_r_time = 0.0f;
    float r_ranges[] = { 600.0f, 750.0f, 900.0f };
    vector r_best_locations[] =
    {
        vector(1170.0f, 12330.0f),
        vector(1690.0f, 13020.0f),
        vector(2392.0f, 13530.0f),
        vector(4458.0f, 11860.0f),
        vector(2956.0f, 11120.0f),
        vector(3008.0f, 9088.0f),
        vector(2306.0f, 9800.0f),
        vector(4684.0f, 10050.0f),
        vector(5230.0f, 9170.0f),
        vector(5058.0f, 8530.0f),
        vector(4778.0f, 7140.0f),
        vector(3384.0f, 7820.0f),
        vector(8266.0f, 10280.0f),
        vector(6748.0f, 11480.0f),
        vector(7992.0f, 11820.0f),
        vector(6538.0f, 8350.0f),
        vector(9224.0f, 11400.0f),
        vector(11500.0f, 71500.0f),
        vector(5676.0f, 12780.0f),
        vector(5634.0f, 3540.0f),
        vector(6876.0f, 3160.0f),
        vector(8090.0f, 3520.0f),
        vector(9218.0f, 2190.0f),
        vector(10390.0f, 3110.0f),
        vector(8580.0f, 4740.0f),
        vector(11870.0f, 3940.0f),
        vector(10190.0f, 4820.0f),
        vector(9430.0f, 5690.0f),
        vector(9858.0f, 6490.0f),
        vector(8390.0f, 6510.0f),
        vector(10012.0f, 7958.0f),
        vector(11484.0f, 7150.0f),
        vector(11922.0f, 5910.0f),
        vector(12532.0f, 5270.0f),
        vector(6566.0f, 4750.0f),
        vector(12410.0f, 1400.0f),
        vector(13030.0f, 1950.0f),
        vector(13534.0f, 2660.0f)
    };

    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);
    void on_after_attack(game_object_script target);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void q_logic_auto();
    void w_logic();
    void r_logic();
    void r_logic_auto();
    void update_range();

    // Utils
    //
    bool can_use_q_on(game_object_script target);
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);
    bool is_trap_placed_in_loc(vector loc);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 680);
        w = plugin_sdk->register_spell(spellslot::w, 0);
        r = plugin_sdk->register_spell(spellslot::r, r_ranges[0]);
        r->set_skillshot(0.25f, 75.0f, 1600.0f, { }, skillshot_type::skillshot_circle);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("teemo", "Teemo");
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
                    combo::q_auto_harass = q_config->add_hotkey(myhero->get_model() + ".combo.q.config", "Auto Q harass", TreeHotkeyMode::Toggle, 'A', false);

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
                auto w_config = combo->add_tab(myhero->get_model() + "combo.w.config", "W Config");
                {
                    combo::w_target_is_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_is_above_range", "Target is above range", 400, 0, 800);
                    combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Only if target is not facing myhero", true);
                }
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_target_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.target_hp_under", "Use R if target HP is under (in %)", 65, 0, 100);
                    combo::r_auto_on_cc = r_config->add_checkbox(myhero->get_model() + ".combo.r.auto_on_cc", "Use R on CC", true);
                    combo::r_auto_on_cc->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_auto_on_best_locations = r_config->add_checkbox(myhero->get_model() + ".combo.r.auto_on_best_locations", "Use R on best locations", true);
                    combo::r_auto_on_best_locations->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

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
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
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
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_q = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.q", "Use Q", true);
                antigapclose::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                antigapclose::use_w = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.w", "Use W", true);
                antigapclose::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                antigapclose::use_r = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.r", "Use R", true);
                antigapclose::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
                draw_settings::draw_damage_q = draw_settings->add_checkbox(myhero->get_model() + "draw.q.damage", "Draw Q Damage", true);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
                draw_settings::r_draw_best_locations = draw_settings->add_checkbox(myhero->get_model() + ".draw.r.best_locations", "Draw R best locations", true);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Last Hit", lasthit::lasthit);
            Permashow::Instance.AddElement("Auto Q Harass", combo::q_auto_harass);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
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
        event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        //console->print("X: %f, Y: %f", myhero->get_position().x, myhero->get_position().y);

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (q->is_ready() && combo::q_auto_harass->get_bool())
            {
                q_logic_auto();
            }

            if (r->is_ready() && combo::use_r->get_bool())
            {
                r_logic_auto();
                update_range();
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

                if (r->is_ready() && combo::use_r->get_bool())
                {
                    r_logic();
                }
            }

            if (orbwalker->last_hit_mode() || orbwalker->harass() || orbwalker->lane_clear_mode())
            {
                if (lasthit::lasthit->get_bool())
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
                                if (minion->get_health() > myhero->get_auto_attack_damage(minion) || !orbwalker->can_attack())
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
                        if (q->cast(lane_minions.front()))
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
        if (target != nullptr && can_use_q_on(target))
        {
            auto q_mode = combo::q_mode->get_int();
            if ((q_mode == 0 && myhero->get_distance(target) > myhero->get_attack_range()) || q_mode == 1 || q->get_damage(target) > target->get_real_health())
            {
                q->cast(target);
            }
        }
    }
#pragma endregion

#pragma region q_logic_auto
    void q_logic_auto()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(q->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_q_on(target))
        {
            q->cast(target);
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(myhero->get_attack_range() + 150, damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (myhero->get_distance(target) >= combo::w_target_is_above_range->get_int() && (!combo::w_check_if_target_is_not_facing->get_bool() || !target->is_facing(myhero)))
            {
                w->cast();
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_r_on(target))
        {
            if (target->get_health_percent() < combo::r_target_hp_under->get_int() && gametime->get_time() > last_r_time)
            {
                if (r->cast(target, get_hitchance(hitchance::r_hitchance)))
                {
                    last_r_time = gametime->get_time() + 2.0f;
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic_auto
    void r_logic_auto()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_r_on(target))
        {
            r->cast(target, hit_chance::immobile);
        }
        else if (combo::r_auto_on_best_locations->get_bool() && !orbwalker->flee_mode() && !myhero->is_recalling() && gametime->get_time() > last_r_time)
        {
            for (auto& loc : r_best_locations)
            {
                if (!is_trap_placed_in_loc(loc) && r->is_in_range(loc, r->range()))
                {
                    if (r->cast(loc))
                    {
                        last_r_time = gametime->get_time() + 2.0f;
                        return;
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region update_range
    void update_range()
    {
        if (r->is_ready())
        {
            r->set_range(r_ranges[r->level() - 1]);
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


    bool is_trap_placed_in_loc(vector loc)
    {
        for (auto& obj : entitylist->get_other_minion_objects())
        {
            if (obj->is_valid() && obj->get_name().compare("Noxious Trap") == 0)
            {
                if (obj->get_distance(loc) < 75)
                {
                    return true;
                }
            }
        }
        return false;
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

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());

        if (q->is_ready() && draw_settings::draw_damage_q->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    draw_dmg_rl(enemy, q->get_damage(enemy), 0x8000ff00);
                }
            }
        }

        if (draw_settings::r_draw_best_locations->get_bool())
        {
            for (auto& loc : r_best_locations)
            {
                draw_manager->add_circle(loc, 75, is_trap_placed_in_loc(loc) ? 0xFF00FF00 : 0xFF0000FF);
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (sender->is_hpbar_recently_rendered())
        {
            if (q->is_ready() && antigapclose::use_q->get_bool())
            {
                if (can_use_q_on(sender) && sender->is_valid_target(q->range() + sender->get_bounding_radius()))
                {
                    q->cast(sender);
                }
            }

            if (w->is_ready() && antigapclose::use_w->get_bool())
            {
                w->cast();
            }

            if (r->is_ready() && antigapclose::use_r->get_bool())
            {
                if (sender->is_valid_target(r->range() + sender->get_bounding_radius()))
                {
                    r->cast(sender, get_hitchance(hitchance::r_hitchance));
                }
            }
        }
    }

    void on_after_attack(game_object_script target)
    {
        if (q->is_ready() && combo::q_mode->get_int() != 1)
        {
            // Use Q after AA
            if (target->is_ai_hero() && can_use_q_on(target) && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                q->cast(target);
            }
        }
    }
};