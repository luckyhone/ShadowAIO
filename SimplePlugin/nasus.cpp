#include "../plugin_sdk/plugin_sdk.hpp"
#include "nasus.h"
#include "utils.h"
#include "permashow.hpp"

namespace nasus
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
        TreeEntry* draw_damage_q = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_mode = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_target_above_range = nullptr;
        TreeEntry* w_target_hp_under = nullptr;
        TreeEntry* w_dont_use_target_under_turret = nullptr;
        TreeEntry* w_check_if_target_is_not_facing = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;

        TreeEntry* r_use_on_low_hp = nullptr;
        TreeEntry* r_myhero_hp_under = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
        TreeEntry* r_hp_enemies_search_radius = nullptr;
        TreeEntry* r_calculate_incoming_damage = nullptr;
        TreeEntry* r_coming_damage_time = nullptr;

        TreeEntry* r_use_if_x_enemies_nearby = nullptr;
        TreeEntry* r_enemies_nearby_minimum = nullptr;
        TreeEntry* r_enemies_nearby_search_radius = nullptr;
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
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
    }
    
    namespace lasthit
    {
        TreeEntry* lasthit;
        TreeEntry* use_q;
    }

    namespace fleemode
    {
        TreeEntry* use_w = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* e_hitchance = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script target, bool* process);
    void on_after_attack_orbwalker(game_object_script target);
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void r_logic();

    // Utils
    //
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, myhero->get_attack_range() + 25);
        w = plugin_sdk->register_spell(spellslot::w, 700);
        e = plugin_sdk->register_spell(spellslot::e, 650);
        e->set_skillshot(0.25f, 200.0f, FLT_MAX, { }, skillshot_type::skillshot_circle);
        r = plugin_sdk->register_spell(spellslot::r, 0);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("nasus", "Nasus");
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
                    combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Target is above range", 35, 0, 800);
                    combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.target_hp_under", "Target HP is under (in %)", 50, 0, 100);
                    combo::w_dont_use_target_under_turret = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_target_under_turret", "Dont use if target is under turret", true);
                    combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Check if target is not facing myhero", true);
                }

                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    r_config->add_separator(myhero->get_model() + ".combo.r.separator1", "R on Low HP Settings");
                    
                    combo::r_use_on_low_hp = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_on_low_hp", "Use R on Low HP", true);
                    combo::r_use_on_low_hp->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under", "Myhero HP is under (in %)", 35, 0, 100);
                    combo::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.only_when_enemies_nearby", "Only when enemies are nearby", true);
                    combo::r_hp_enemies_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.hp_enemies_search_radius", "Enemies nearby search radius", 350, 150, 1600);
                    combo::r_calculate_incoming_damage = r_config->add_checkbox(myhero->get_model() + ".combo.r.calculate_incoming_damage", "Calculate incoming damage", true);
                    combo::r_coming_damage_time = r_config->add_slider(myhero->get_model() + ".combo.r.coming_damage_time", "Set coming damage time (in ms)", 1000, 0, 1000);

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator2", "R if enemies nearby Settings");

                    combo::r_use_if_x_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_if_x_enemies_nearby", "Use R if enemies nearby", true);
                    combo::r_use_if_x_enemies_nearby->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_enemies_nearby_minimum = r_config->add_slider(myhero->get_model() + ".combo.r.enemies_nearby_minimum", "Minimum enemies nearby", 2, 1, 5);
                    combo::r_enemies_nearby_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under_enemies_search_radius", "Enemies nearby search radius", 250, 150, 1600);
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
                laneclear::use_q_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q.on_turret", "Use Q On Turret", true);
                laneclear::use_q_on_turret->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
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
                lasthit::use_q = lasthit->add_checkbox(myhero->get_model() + ".lasthit.q", "Use Q", true);
                lasthit::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }


            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to slow enemies", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_w = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.w", "Use W", true);
                antigapclose::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                draw_settings->add_separator(myhero->get_model() + ".draw.separator1", "Spell Drawings");
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings->add_separator(myhero->get_model() + ".draw.separator2", "Damage Drawings");
                draw_settings::draw_damage_q = draw_settings->add_checkbox(myhero->get_model() + "draw.q.damage", "Draw Q Damage", true);
                draw_settings::draw_damage_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
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
            if (r->is_ready() && combo::use_r->get_bool())
            {
                r_logic();
            }

            //Checking if the user has combo_mode() (Default SPACE
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
                if (w->is_ready() && fleemode::use_w->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(w->range(), damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        w->cast(target);
                    }
                }
            }

            // Lasthit logic
            if ((orbwalker->last_hit_mode() || orbwalker->harass() || orbwalker->lane_clear_mode()) && lasthit::lasthit->get_bool())
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
                        if (minion->is_valid() && minion->is_valid_target(175))
                        {
                            if (q->is_ready() && lasthit::use_q->get_bool() && q->get_damage(minion) + myhero->get_auto_attack_damage(minion) > minion->get_health())
                            {
                                if (q->cast())
                                {
                                    return;
                                }
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


                // Logic responsible for minions
                if (!lane_minions.empty())
                {
                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(1))
                            return;
                    }
                }

                // Logic responsible for monsters
                if (!monsters.empty())
                {
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
        // Get a target from a given range
        auto target = target_selector->get_target(myhero->get_attack_range() + 25, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && q->get_damage(target) + myhero->get_auto_attack_damage(target) > target->get_real_health())
        {
            q->cast();
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
            if (target->get_health_percent() < combo::w_target_hp_under->get_int() && target->get_distance(myhero) > combo::w_target_above_range->get_int())
            {
                if (!combo::w_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
                {
                    if (!combo::w_check_if_target_is_not_facing->get_bool() || !target->is_facing(myhero))
                    {
                        w->cast(target);
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
        auto target = target_selector->get_target(e->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            e->cast(target, get_hitchance(hitchance::e_hitchance));
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        if (!utils::has_unkillable_buff(myhero))
        {
            if (combo::r_use_on_low_hp->get_bool())
            {
                if ((!combo::r_only_when_enemies_nearby->get_bool() || myhero->count_enemies_in_range(combo::r_hp_enemies_search_radius->get_int()) != 0))
                {
                    if ((myhero->get_health_percent() < combo::r_myhero_hp_under->get_int()) || (combo::r_calculate_incoming_damage->get_bool() && health_prediction->get_incoming_damage(myhero, combo::r_coming_damage_time->get_int() / 1000.0f, true) >= myhero->get_health()))
                    {
                        if (r->cast())
                        {
                            return;
                        }
                    }
                }
            }

            if (combo::r_use_if_x_enemies_nearby->get_bool())
            {
                if (myhero->count_enemies_in_range(combo::r_enemies_nearby_search_radius->get_int()) >= combo::r_enemies_nearby_minimum->get_int())
                {
                    r->cast();
                }
            }
        }
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

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_w->get_bool() && w->is_ready())
        {
            if (sender->is_valid_target(w->range() + sender->get_bounding_radius()))
            {
                w->cast(sender);
            }
        }
        if (antigapclose::use_e->get_bool() && e->is_ready())
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                e->cast(sender, get_hitchance(hitchance::e_hitchance));
            }
        }
    }

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
        if (q->is_ready())
        {
            if (combo::q_mode->get_int() == 0)
            {
                // Using w before autoattack on enemies
                if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w before autoattack on minions
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_q->get_bool() && target->is_ai_minion())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w before autoattack on monsters
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool() && target->is_monster())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
            }
            else
            {
                // Using w before autoattack on enemies to lasthit
                if (target->is_ai_hero() && q->get_damage(target) + myhero->get_auto_attack_damage(target) > target->get_health() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w before autoattack on minions to lasthit
                if ((orbwalker->lane_clear_mode() || orbwalker->last_hit_mode() || orbwalker->harass()) && lasthit::lasthit->get_bool() && lasthit::use_q->get_bool() && target->is_ai_minion() && q->get_damage(target) + myhero->get_auto_attack_damage(target) > target->get_health())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w before autoattack on monsters to lasthit
                if (orbwalker->lane_clear_mode() && lasthit::lasthit->get_bool() && lasthit::use_q->get_bool() && target->is_monster() && q->get_damage(target) + myhero->get_auto_attack_damage(target) > target->get_health())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
            }
        }
    }

    void on_after_attack_orbwalker(game_object_script target)
    {
        if (q->is_ready())
        {
            // Using q after autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_q_on_turret->get_bool() && target->is_ai_turret())
            {
                if (q->cast())
                {
                    return;
                }
            }

            if (combo::q_mode->get_int() == 1)
            {
                // Using w after autoattack on enemies
                if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w after autoattack on minions
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_q->get_bool() && target->is_ai_minion())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }

                // Using w after autoattack on monsters
                if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool() && target->is_monster())
                {
                    if (q->cast())
                    {
                        return;
                    }
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

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw Q damage
        if (draw_settings::draw_damage_q->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    float damage = myhero->get_auto_attack_damage(enemy);

                    if (q->is_ready())
                        damage += q->get_damage(enemy);

                    draw_dmg_rl(enemy, damage, 0x8000ff00);
                }
            }
        }
    }
};