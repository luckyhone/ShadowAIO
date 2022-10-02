#include "../plugin_sdk/plugin_sdk.hpp"
#include "vex.h"
#include "farm.h"
#include "utils.h"
#include "permashow.hpp"

namespace vex
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
        TreeEntry* draw_range_r_minimap = nullptr;
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
        TreeEntry* allow_tower_dive = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_killable_by_combo = nullptr;
        TreeEntry* r_use_only_passive_ready = nullptr;
        TreeEntry* r_target_hp_under = nullptr;
        TreeEntry* r_target_above_range = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
        TreeEntry* use_r2 = nullptr;
        TreeEntry* r2_maxiumum_enemies_nearby = nullptr;
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
        TreeEntry* farm_only_when_minions_more_than = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_w = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
        TreeEntry* e_hitchance = nullptr;
        TreeEntry* r_hitchance = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void r_logic();
    void r_logic_semi();
    void update_range();

    // Utils
    //
    float get_damage(game_object_script target);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    // Champion data
    //
    float r_ranges[] = { 2000.0f, 2500.0f, 3000.0f };

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 1200);
        q->set_skillshot(0.15f, 80.f, 600.0f, { collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line); //width 160-80, speed 600-3200
        w = plugin_sdk->register_spell(spellslot::w, 475); //550 against dashing enemies
        e = plugin_sdk->register_spell(spellslot::e, 800);
        e->set_skillshot(0.25f, 100.0f, 1300.0f, { }, skillshot_type::skillshot_circle);
        r = plugin_sdk->register_spell(spellslot::r, r_ranges[0]);
        r->set_skillshot(0.25f, 130.0f, 1600.0f, { collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("vex", "Vex");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::allow_tower_dive = combo->add_hotkey(myhero->get_model() + ".combo.allow_tower_dive", "Allow Tower Dive", TreeHotkeyMode::Toggle, 'A', true);
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_killable_by_combo = r_config->add_checkbox(myhero->get_model() + ".combo.r.killable_by_combo", "Use R if target is killable by combo", true);
                    combo::r_killable_by_combo->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_use_only_passive_ready = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_only_passive_ready", "Use R only if passive is ready", false);
                    combo::r_use_only_passive_ready->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_target_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.target_hp_under", "Target HP is under (in %)", 30, 0, 100);
                    combo::r_target_above_range = r_config->add_slider(myhero->get_model() + ".combo.r.target_is_above_range", "Target is above range", 300, 0, 800);

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

                combo::use_r2 = combo->add_checkbox(myhero->get_model() + ".combo.r2", "Use R2", true);
                combo::use_r2->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r2_config = combo->add_tab(myhero->get_model() + ".combo.r2.config", "R2 Config");
                {
                    combo::r2_maxiumum_enemies_nearby = r2_config->add_slider(myhero->get_model() + ".combo.r2.maxiumum_enemies_nearby", "Maximum enemies nearby target", 2, 1, 5);
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
                laneclear::farm_only_when_minions_more_than = laneclear->add_slider(myhero->get_model() + ".laneclearFarmOnlyWhenMinionsMoreThan", "Farm only when minions more than", 2, 0, 5);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", false);
                jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", false);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_w = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.w", "Use W", true);
                antigapclose::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 3);
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
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings::draw_range_r_minimap = draw_settings->add_checkbox(myhero->get_model() + ".draw.r.minimap", "Draw R range on minimap", true);
                draw_settings::draw_range_r_minimap->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
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
	        Permashow::Instance.AddElement("Semi Manual R", combo::r_semi_manual_cast);
            Permashow::Instance.AddElement("Allow Tower Dive", combo::allow_tower_dive);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);

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
            update_range();
            r_logic_semi();
        }


        //Checking if the user has combo_mode() (Default SPACE
        if (orbwalker->combo_mode())
        {
            if (r->is_ready() && combo::use_r->get_bool())
            {
                r_logic();
            }

            if (e->is_ready() && combo::use_e->get_bool())
            {
                e_logic();
            }

            if (q->is_ready() && combo::use_q->get_bool())
            {
                q_logic();
            }

            if (w->is_ready() && combo::use_w->get_bool())
            {
                w_logic();
            }
        }

        //Checking if the user has selected harass() (Default C)
        if (orbwalker->harass())
        {
            if (!myhero->is_under_enemy_turret())
            {
                if (e->is_ready() && harass::use_e->get_bool())
                {
                    e_logic();
                }

                if (q->is_ready() && harass::use_q->get_bool())
                {
                    q_logic();
                }

                if (w->is_ready() && harass::use_w->get_bool())
                {
                    w_logic();
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

            if (!lane_minions.empty() && lane_minions.size() >= laneclear::farm_only_when_minions_more_than->get_int())
            {
                if (q->is_ready() && laneclear::use_q->get_bool())
                {
                    q->cast_on_best_farm_position(laneclear::farm_only_when_minions_more_than->get_int());
                }

                if (w->is_ready() && laneclear::use_w->get_bool())
                {
                    farm::cast_verify_range(w, lane_minions.front());
                }

                if (e->is_ready() && laneclear::use_e->get_bool())
                {
                    e->cast_on_best_farm_position(laneclear::farm_only_when_minions_more_than->get_int());
                }
            }

            if (!monsters.empty())
            {
                if (q->is_ready() && jungleclear::use_q->get_bool())
                {
                    q->cast_on_best_farm_position(1, true);
                }

                if (w->is_ready() && jungleclear::use_w->get_bool())
                {
                    farm::cast_verify_range(w, monsters.front());
                }

                if (e->is_ready() && jungleclear::use_e->get_bool())
                {
                    e->cast_on_best_farm_position(1, true);
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
            q->cast(target, utils::get_hitchance(hitchance::q_hitchance));
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            w->cast();
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
            e->cast(target, utils::get_hitchance(hitchance::e_hitchance));
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        if (combo::use_r2->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && utils::enabled_in_map(combo::r_use_on, enemy) && (!enemy->is_under_ally_turret() || combo::allow_tower_dive->get_bool()))
                {
                    int max_enemies = combo::r2_maxiumum_enemies_nearby->get_int();
                    if (max_enemies >= enemy->count_allies_in_range(800) && enemy->has_buff(buff_hash("VexRTarget")))
                    {
                        if (r->cast())
                        {
                            return;
                        }
                    }
                }
            }
        }

        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && utils::enabled_in_map(combo::r_use_on, target) && (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool()))
        {
            if (target->get_distance(myhero) > combo::r_target_above_range->get_int() && !target->has_buff(buff_hash("vexr2timer")))
            {
                if ((target->get_health_percent() < combo::r_target_hp_under->get_int()) || (get_damage(target) > target->get_real_health() && combo::r_killable_by_combo->get_bool()))
                {
                    if (!combo::r_use_only_passive_ready->get_bool() || myhero->has_buff(buff_hash("vexpdoom")))
                    {
                        r->cast(target, utils::get_hitchance(hitchance::r_hitchance));
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic_semi
    void r_logic_semi()
    {
        if (combo::r_semi_manual_cast->get_bool())
        {
            if (combo::use_r2->get_bool())
            {
                for (auto& enemy : entitylist->get_enemy_heroes())
                {
                    if (enemy->is_valid() && !enemy->is_dead() && utils::enabled_in_map(combo::r_use_on, enemy) && (!enemy->is_under_ally_turret() || combo::allow_tower_dive->get_bool()))
                    {
                        int max_enemies = combo::r2_maxiumum_enemies_nearby->get_int();
                        if (max_enemies >= enemy->count_allies_in_range(800) && enemy->has_buff(buff_hash("VexRTarget")))
                        {
                            if (r->cast())
                            {
                                return;
                            }
                        }
                    }
                }
            }

            // Get a target from a given range
            auto target = target_selector->get_target(r->range(), damage_type::magical);

            // Always check an object is not a nullptr!
            if (target != nullptr && utils::enabled_in_map(combo::r_use_on, target) && (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool()))
            {
                if (target->get_distance(myhero) > combo::r_target_above_range->get_int() && !target->has_buff(buff_hash("vexr2timer")))
                {
                    if (!combo::r_use_only_passive_ready->get_bool() || myhero->has_buff(buff_hash("vexpdoom")))
                    {
                        r->cast(target, utils::get_hitchance(hitchance::r_hitchance));
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

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_w->get_bool() && w->is_ready())
        {
            if (sender->is_valid_target(w->range() + sender->get_bounding_radius()))
            {
                w->cast();
            }
        }
    }


    float get_damage(game_object_script target)
    {
        float damage = 0.0f;

        if (q->is_ready())
            damage += q->get_damage(target);

        if (w->is_ready())
            damage += w->get_damage(target);

        if (e->is_ready())
            damage += e->get_damage(target);

        if (r->is_ready() && utils::enabled_in_map(combo::r_use_on, target))
            damage += r->get_damage(target);

        damage += myhero->get_auto_attack_damage(target);
        return damage;
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
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());

        // Draw R range on minimap
        if (r->is_ready() && draw_settings::draw_range_r_minimap->get_bool())
            draw_manager->draw_circle_on_minimap(myhero->get_position(), r->range(), draw_settings::r_color->get_color());

        if (draw_settings::draw_damage_settings::draw_damage->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    float damage = 0.0f;

                    if (q->is_ready() && draw_settings::draw_damage_settings::q_damage->get_bool())
                        damage += q->get_damage(enemy);

                    if (w->is_ready() && draw_settings::draw_damage_settings::w_damage->get_bool())
                        damage += w->get_damage(enemy);

                    if (e->is_ready() && draw_settings::draw_damage_settings::e_damage->get_bool())
                        damage += e->get_damage(enemy);

                    if (r->is_ready() && utils::enabled_in_map(combo::r_use_on, enemy) && draw_settings::draw_damage_settings::r_damage->get_bool())
                        damage += r->get_damage(enemy);

                    if (damage != 0.0f)
                        draw_dmg_rl(enemy, damage, 0x8000ff00);
                }
            }
        }
    }
};