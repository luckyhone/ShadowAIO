#include "../plugin_sdk/plugin_sdk.hpp"
#include "rengar.h"
#include "utils.h"
#include "permashow.hpp"

namespace rengar
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
        TreeEntry* draw_range_r_tracing = nullptr;
        TreeEntry* r_tracing_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* empowered_spell_priority = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* q_before_leap = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_max_range = nullptr;
        TreeEntry* w_use_empowered_if_immobile = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_max_range = nullptr;
        TreeEntry* e_use_prediction = nullptr;
        TreeEntry* e_use_empowered_if_chasing = nullptr;
        TreeEntry* e_use_midair_on_leap = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_leap_use_on;
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
        TreeEntry* save_empowered_spell_if_enemy_nearby = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* q_use_empowered = nullptr;
        TreeEntry* q_use_on_turret = nullptr;
        TreeEntry* use_w = nullptr;
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
        TreeEntry* use_e = nullptr;
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
    void on_after_attack_orbwalker(game_object_script target);

    // Declaring functions responsible for spell-logic
    //
    bool q_logic();
    bool w_logic();
    bool e_logic();

    // Utils
    //
    bool is_empowered();
    bool is_on_r();

    // Champion data
    //
    float r_tracking_radius[] = { 2500.0f, 3000.0f, 3500.0f };

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 0);
        w = plugin_sdk->register_spell(spellslot::w, 450);
        e = plugin_sdk->register_spell(spellslot::e, 1000);
        e->set_skillshot(0.25f, 80.0f, 1500.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
        r = plugin_sdk->register_spell(spellslot::r, 745);

        // Disabling spell lock
        //
        q->set_spell_lock(false);
        w->set_spell_lock(false);
        e->set_spell_lock(false);
        r->set_spell_lock(false);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("rengar", "Rengar");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::empowered_spell_priority = combo->add_combobox(myhero->get_model() + ".combo.empowered_spell_priority", "Empowered Spell Priority", { {"Q", nullptr},{"W", nullptr },{"E", nullptr } }, 0);
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = combo->add_tab(myhero->get_model() + ".combo.q.config", "Q Config");
                {
                    combo::q_before_leap = q_config->add_checkbox(myhero->get_model() + ".combo.q.before_leap", "Use Q before leap", true);
                    combo::q_before_leap->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                }
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_max_range = w_config->add_slider(myhero->get_model() + ".combo.w.max_range", "Maximum W range", w->range() - 50, 1, w->range());
                    combo::w_use_empowered_if_immobile = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_empowered_if_immobile", "Use empowered W if immobile", true);
                    combo::w_use_empowered_if_immobile->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    e_config->add_separator(myhero->get_model() + ".combo.e.separator1", "Range Settings");
                    combo::e_max_range = e_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum E range", e->range() - 50, 1, e->range());

                    e_config->add_separator(myhero->get_model() + ".combo.e.separator2", "Usage Settings");
                    combo::e_use_prediction = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_prediction", "Use prediction in E", true);
                    combo::e_use_prediction->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    combo::e_use_empowered_if_chasing = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_empowered_if_chasing", "Use empowered E when chasing enemy", true);
                    combo::e_use_empowered_if_chasing->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    combo::e_use_midair_on_leap = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_midair_on_leap", "Use E midair on leap", true);
                    combo::e_use_midair_on_leap->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                }
                auto use_r_on_tab = combo->add_tab(myhero->get_model() + ".combo.r.leap_use_on", "Use R Leap On");
                {
                    for (auto&& enemy : entitylist->get_enemy_heroes())
                    {
                        // In this case you HAVE to set should save to false since key contains network id which is unique per game
                        //
                        combo::r_leap_use_on[enemy->get_network_id()] = use_r_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                        // Set texture to enemy square icon
                        //
                        combo::r_leap_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                    }
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::save_empowered_spell_if_enemy_nearby = laneclear->add_hotkey(myhero->get_model() + ".laneclear.save_empowered_spell_if_enemy_nearby", "Save empowered spell if enemy nearby", TreeHotkeyMode::Toggle, 'H', true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = laneclear->add_tab(myhero->get_model() + ".laneclear.q.config", "Q Config");
                {
                    laneclear::q_use_empowered = q_config->add_hotkey(myhero->get_model() + ".laneclear.q.empowered", "Use Empowered Q", TreeHotkeyMode::Toggle, 'J', true);
                    laneclear::q_use_empowered->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    laneclear::q_use_on_turret = q_config->add_checkbox(myhero->get_model() + ".laneclear.q.use_on_turret", "Use Q on turret", true);
                    laneclear::q_use_on_turret->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                }
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", true);
                jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E to slow enemy", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
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
                draw_settings::draw_range_r_tracing = draw_settings->add_checkbox(myhero->get_model() + ".draw.r.tracing", "Draw R tracing range on minimap", true);
                draw_settings::draw_range_r_tracing->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings::r_tracing_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.tracing.color", "R Color", color);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Save empowered if enemy nearby", laneclear::save_empowered_spell_if_enemy_nearby);
            Permashow::Instance.AddElement("Empowered Q farm", laneclear::q_use_empowered);
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

        //Checking if the user has combo_mode() (Default SPACE
        if (orbwalker->combo_mode())
        {
            if (q->is_ready() && combo::use_q->get_bool())
            {
                q_logic();
            }

            if (!is_on_r())
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
        }

        //Checking if the user has selected harass() (Default C)
        if (orbwalker->harass())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (!myhero->is_under_enemy_turret())
                {
                    if (q->is_ready() && harass::use_q->get_bool())
                    {
                        q_logic();
                    }

                    if (!is_on_r())
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
        }

        // Checking if the user has selected flee_mode() (Default Z)
        if (orbwalker->flee_mode())
        {
            if (!is_on_r() && e->is_ready() && fleemode::use_e->get_bool())
            {
                // Get a target from a given range
                auto target = target_selector->get_target(e->range(), damage_type::physical);

                // Always check an object is not a nullptr!
                if (target != nullptr)
                {
                    utils::fast_cast(e, target, utils::get_hitchance(hitchance::e_hitchance), false, 0);
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
                    return !x->is_valid_target(w->range());
                }), lane_minions.end());

            // You can use this function to delete monsters that aren't in the specified range
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return !x->is_valid_target(w->range());
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
                if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                {
                    return;
                }
                if (q->is_ready() && laneclear::use_q->get_bool() && laneclear::q_use_empowered->get_bool() && is_empowered())
                {
                    if (utils::fast_cast(q))
                        return;
                }
                if (!is_empowered() && !is_on_r())
                {
                    if (w->is_ready() && laneclear::use_w->get_bool())
                    {
                        if (utils::fast_cast(w))
                            return;
                    }
                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (utils::fast_cast(e, 1, false))
                            return;
                    }
                }
            }

            // Logic responsible for monsters
            if (!monsters.empty())
            {
                if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                {
                    return;
                }
                if (q->is_ready() && jungleclear::use_q->get_bool())
                {
                    if (utils::fast_cast(q))
                        return;
                }
                if (!is_empowered() && !is_on_r())
                {
                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (utils::fast_cast(w))
                            return;
                    }
                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (utils::fast_cast(e, 1, true))
                            return;
                    }
                }
            }
        }
    }

#pragma region q_logic
    bool q_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::q_before_leap->get_bool() ? myhero->get_attack_range() + 25 : 175, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (!is_empowered() || combo::empowered_spell_priority->get_int() == 0)
            {
                return utils::fast_cast(q);
            }
        }

        return false;
    }
#pragma endregion

#pragma region w_logic
    bool w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::w_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (!is_empowered() || combo::empowered_spell_priority->get_int() == 1 || (utils::has_crowd_control_buff(myhero) && combo::w_use_empowered_if_immobile->get_bool()))
            {
                return utils::fast_cast(w);
            }
        }

        return false;
    }
#pragma endregion

#pragma region e_logic
    bool e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (!is_empowered() || combo::empowered_spell_priority->get_int() == 2 || (combo::e_use_empowered_if_chasing->get_bool() && target->get_distance(myhero) > myhero->get_attack_range() + 325 && target->is_moving()))
            {
                if (combo::e_use_prediction->get_bool())
                {
                    return utils::fast_cast(e, target, utils::get_hitchance(hitchance::e_hitchance), false, 0);
                }
                
                return utils::fast_cast(e, target->get_position());
            }
        }

        return false;
    }
#pragma endregion

    bool is_empowered()
    {
        return q->name() == "RengarQEmp"
            || w->name() == "RengarWEmp"
            || e->name() == "RengarEEmp"
            || myhero->get_mana() >= 4.0f;
    }

    bool is_on_r()
    {
        auto buff = myhero->get_buff(buff_hash("RengarR"));
        return buff != nullptr && buff->is_valid() && buff->is_alive();
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_e->get_bool() && e->is_ready() && !is_on_r())
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                utils::fast_cast(e, sender, utils::get_hitchance(hitchance::e_hitchance), false, 0);
            }
        }
    }

    void on_before_attack(game_object_script target, bool* process)
    {
        if (target->is_ai_hero() && is_on_r() && !utils::enabled_in_map(combo::r_leap_use_on, target))
        {
            *process = false;
            return;
        }

        if (e->is_ready() && !is_on_r() && combo::e_use_midair_on_leap->get_bool())
        {
            // Using E while middle of a jump on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
            {
                if (myhero->get_attack_range() >= r->range() && (!is_empowered() || combo::empowered_spell_priority->get_int() == 2))
                {
                    utils::fast_cast(e, target, utils::get_hitchance(hitchance::e_hitchance), false, 0);
                }
            }
        }

        if (q->is_ready())
        {
            // Using q before autoattack on enemies
            if (target->is_ai_hero() && (combo::q_before_leap->get_bool() || target->is_valid_target(175)) && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (!is_empowered() || combo::empowered_spell_priority->get_int() == 0)
                {
                    if (utils::fast_cast(q))
                    {
                        return;
                    }
                }
            }

            if (laneclear::spell_farm->get_bool())
            {
                // Using q before autoattack on lane minions
                if (orbwalker->lane_clear_mode() && laneclear::use_q->get_bool() && target->is_lane_minion())
                {
                    if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                    {
                        return;
                    }
                    if (!is_empowered() || laneclear::q_use_empowered->get_bool())
                    {
                        if (utils::fast_cast(q))
                        {
                            return;
                        }
                    }
                }

                // Using q before autoattack on monsters
                if (orbwalker->lane_clear_mode() && jungleclear::use_q->get_bool() && target->is_monster())
                {
                    if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                    {
                        return;
                    }
                    if (utils::fast_cast(q))
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
            // Using q after autoattack on enemies
            if (target->is_ai_hero() && (combo::q_before_leap->get_bool() || target->is_valid_target(175)) && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (!is_empowered() || combo::empowered_spell_priority->get_int() == 0)
                {
                    if (utils::fast_cast(q))
                    {
                        return;
                    }
                }
            }

            // Using q after autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::q_use_on_turret->get_bool() && target->is_ai_turret())
            {
                if (utils::fast_cast(q))
                {
                    return;
                }
            }

            if (laneclear::spell_farm->get_bool())
            {
                // Using q after autoattack on lane minions
                if (orbwalker->lane_clear_mode() && laneclear::use_q->get_bool() && target->is_lane_minion())
                {
                    if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                    {
                        return;
                    }
                    if (!is_empowered() || laneclear::q_use_empowered->get_bool())
                    {
                        if (utils::fast_cast(q))
                        {
                            return;
                        }
                    }
                }

                // Using q after autoattack on monsters
                if (orbwalker->lane_clear_mode() && jungleclear::use_q->get_bool() && target->is_monster())
                {
                    if (is_empowered() && laneclear::save_empowered_spell_if_enemy_nearby && myhero->count_enemies_in_range(900) != 0)
                    {
                        return;
                    }
                    if (utils::fast_cast(q))
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
            draw_manager->add_circle(myhero->get_position(), combo::w_max_range->get_int(), draw_settings::w_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::e_max_range->get_int(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());

        // Draw R tracing range on minimap
        if (r->is_ready() && draw_settings::draw_range_r_tracing->get_bool())
            draw_manager->draw_circle_on_minimap(myhero->get_position(), r_tracking_radius[r->level() - 1], draw_settings::r_tracing_color->get_color());
    }
};