#include "../plugin_sdk/plugin_sdk.hpp"
#include "chogath.h"
#include "permashow.hpp"

namespace chogath
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;
    script_spell* flash = nullptr;

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_q = nullptr;
        TreeEntry* q_color = nullptr;
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* w_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;

        namespace draw_damage_settings
        {
            TreeEntry* draw_damage = nullptr;
            TreeEntry* q_damage = nullptr;
            TreeEntry* w_damage = nullptr;
            TreeEntry* e_damage = nullptr;
            TreeEntry* r_damage = nullptr;
        }
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_max_range = nullptr;
        TreeEntry* q_auto_on_cc = nullptr;
        TreeEntry* q_auto_dashing = nullptr;
        TreeEntry* q_try_to_hit_with_the_center = nullptr;
        std::map<std::uint32_t, TreeEntry*> q_use_on;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_max_range = nullptr;
        TreeEntry* w_use_prediction = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_use_flash_r = nullptr;
        TreeEntry* r_use_on_epic_monsters = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
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
        TreeEntry* use_e = nullptr;
        TreeEntry* use_e_on_turret = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_q;
    }

    namespace antigapclose
    {
        TreeEntry* use_q = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
        TreeEntry* w_hitchance = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script target, bool* process);
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void q_logic_auto();
    void w_logic();
    void e_logic();
    void r_logic();

    // Utils
    //
    bool can_use_q_on(game_object_script target);
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 950);
        q->set_skillshot(1.125f, 100.0f, FLT_MAX, { }, skillshot_type::skillshot_circle);
        w = plugin_sdk->register_spell(spellslot::w, 650);
        w->set_skillshot(0.5f, 100.0f, FLT_MAX, { }, skillshot_type::skillshot_line);
        e = plugin_sdk->register_spell(spellslot::e, myhero->get_attack_range());
        r = plugin_sdk->register_spell(spellslot::r, 325);

        if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner1, 400.f);
        else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner2, 400.f);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("chogath", "Cho'Gath");
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
                    combo::q_max_range = q_config->add_slider(myhero->get_model() + ".combo.q.max_range", "Maximum Q range", q->range() - 50, 300, q->range());
                    combo::q_auto_on_cc = q_config->add_checkbox(myhero->get_model() + ".combo.q.auto_on_cc", "Auto Q on CC", true);
                    combo::q_auto_on_cc->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::q_auto_dashing = q_config->add_checkbox(myhero->get_model() + ".combo.q.auto_dashing", "Auto Q dashing", true);
                    combo::q_auto_dashing->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::q_try_to_hit_with_the_center = q_config->add_checkbox(myhero->get_model() + ".combo.q.try_to_hit_with_the_center", "Try to hit Q in the center of target", false);
                    combo::q_try_to_hit_with_the_center->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());

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
                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_max_range = w_config->add_slider(myhero->get_model() + ".combo.w.max_range", "Maximum W range", w->range() - 50, 1, q->range());
                    combo::w_use_prediction = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_prediction", "Use Prediction on W", true);
                    combo::w_use_prediction->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                }
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_use_flash_r = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_flash_r", "Use Flash + R above R range", true);
                    combo::r_use_flash_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_use_on_epic_monsters = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_on_epic_monsters", "Auto R on epic monsters", true);
                    combo::r_use_on_epic_monsters->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

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
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", false);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", false);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", true);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                laneclear::use_e_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e.on_turret", "Use E On Turret", true);
                laneclear::use_e_on_turret->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
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
                fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "Use Q", true);
                fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_q = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.q", "Use Q", true);
                antigapclose::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
                hitchance::w_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance W", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
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
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);

                auto draw_damage = draw_settings->add_tab(myhero->get_model() + ".draw.damage", "Draw Damage");
                {
                    draw_settings::draw_damage_settings::draw_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.enabled", "Draw Combo Damage", true);
                    draw_settings::draw_damage_settings::q_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.q", "Draw Q Damage", true);
                    draw_settings::draw_damage_settings::q_damage->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    draw_settings::draw_damage_settings::w_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.w", "Draw W Damage", true);
                    draw_settings::draw_damage_settings::w_damage->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                    draw_settings::draw_damage_settings::e_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.e", "Draw E Damage", true);
                    draw_settings::draw_damage_settings::e_damage->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    draw_settings::draw_damage_settings::r_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.r", "Draw R Damage", true);
                    draw_settings::draw_damage_settings::r_damage->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                }
            }
        }

        // Permashow initialization
		//
	    {
	        Permashow::Instance.Init(main_tab);
	        Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

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
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (q->is_ready() && combo::use_q->get_bool())
        {
            q->set_radius(combo::q_try_to_hit_with_the_center->get_bool() ? 10.0f : 100.0f);
        }

        if (r->is_ready() && combo::use_r->get_bool())
        {
            r_logic();
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (q->is_ready() && combo::use_q->get_bool())
            {
                q_logic_auto();
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
                    q_logic();
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
                                if (q->cast_on_best_farm_position(1))
                                {
                                    return;
                                }
                            }
                        }
                        if (q->cast_on_best_farm_position(1))
                            return;
                    }

                    if (w->is_ready() && laneclear::use_w->get_bool())
                    {
                        if (lane_minions.front()->is_under_ally_turret())
                        {
                            if (myhero->count_enemies_in_range(900) == 0)
                            {
                                if (w->cast_on_best_farm_position(1))
                                {
                                    return;
                                }
                            }
                        }
                        if (w->cast_on_best_farm_position(1))
                            return;
                    }
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast_on_best_farm_position(1, true))
                            return;
                    }

                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->cast_on_best_farm_position(1, true))
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
        auto target = target_selector->get_target(combo::q_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_q_on(target))
        {
            q->cast(target, get_hitchance(hitchance::q_hitchance));
        }
    }
#pragma endregion

#pragma region q_logic_auto
    void q_logic_auto()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::q_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_q_on(target))
        {
            if (combo::q_auto_on_cc->get_bool())
            {
                if (q->cast(target, hit_chance::immobile))
                {
                    return;
                }
            }

            if (combo::q_auto_dashing->get_bool())
            {
                if (q->cast(target, hit_chance::dashing))
                {
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
        auto target = target_selector->get_target(combo::w_max_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (combo::w_use_prediction->get_bool())
            {
                w->cast(target, get_hitchance(hitchance::w_hitchance));
            }
            else
            {
                w->cast(target->get_position());
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(myhero->get_attack_range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Checking if the target will die from E damage
            if (e->get_damage(target) >= target->get_health())
            {
                e->cast();
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::true_dmg);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (r->get_damage(target) > target->get_health() && can_use_r_on(target))
            {
                if (r->cast(target))
                {
                    return;
                }
            }
        }
        else if (flash && flash->is_ready() && combo::r_use_flash_r->get_bool())
        {
            auto target = target_selector->get_target(r->range() + flash->range(), damage_type::magical);
            if (target != nullptr && can_use_r_on(target) && myhero->get_distance(target) > r->range() + 50 && r->get_damage(target) > target->get_health())
            {
                if (flash->cast(target) && r->cast(target))
                {
                    return;
                }
            }
        }

        if (combo::r_use_on_epic_monsters->get_bool())
        {
            // Gets jugnle mobs from the entitylist
            auto monsters = entitylist->get_jugnle_mobs_minions();

            // You can use this function to delete monsters that aren't in the specified range
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return x->get_distance(myhero) > r->range();
                }), monsters.end());

            // Remove non epic monsters
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return !x->is_epic_monster();
                }), monsters.end());

            // Remove unkillable monsters
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return x->get_health() > r->get_damage(x);
                }), monsters.end());

            //std::sort -> sort monsters by max health
            std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
                {
                    return a->get_max_health() > b->get_max_health();
                });

            if (!monsters.empty())
            {
                auto monster = monsters.front();
                if (monster->get_name().compare("MiniKrugA") != 0 && monster->get_name().compare("MiniKrugB") != 0)
                {
                    r->cast(monster);
                }
            }
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

    void on_before_attack(game_object_script target, bool* process)
    {
        if (e->is_ready())
        {
            // Using e before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
            {
                if (e->cast())
                {
                    return;
                }
            }

            // Using e before autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_e_on_turret->get_bool() && target->is_ai_turret())
            {
                if (e->cast())
                {
                    return;
                }
            }

            // Using e before autoattack on minions
            if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_e->get_bool() && target->is_ai_minion())
            {
                if (e->cast())
                {
                    return;
                }
            }

            // Using e before autoattack on monsters
            if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_e->get_bool() && target->is_monster())
            {
                if (e->cast())
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
                q->cast(sender, get_hitchance(hitchance::q_hitchance));
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
            draw_manager->add_circle(myhero->get_position(), combo::w_max_range->get_int(), draw_settings::w_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());
        
        if (draw_settings::draw_damage_settings::draw_damage->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    int damage = 0;

                    if (q->is_ready() && can_use_q_on(enemy) && draw_settings::draw_damage_settings::q_damage->get_bool())
                        damage += q->get_damage(enemy);

                    if (w->is_ready() && draw_settings::draw_damage_settings::w_damage->get_bool())
                        damage += w->get_damage(enemy);

                    if (e->is_ready() && draw_settings::draw_damage_settings::e_damage->get_bool())
                        damage += e->get_damage(enemy);

                    if (r->is_ready() && can_use_r_on(enemy) && draw_settings::draw_damage_settings::r_damage->get_bool())
                        damage += r->get_damage(enemy);

                    if (damage != 0)
                        draw_dmg_rl(enemy, damage, 0x8000ff00);
                }
            }
        }
    }
};