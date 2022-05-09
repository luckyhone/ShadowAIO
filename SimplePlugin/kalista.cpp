#include "../plugin_sdk/plugin_sdk.hpp"
#include "kalista.h"
#include "permashow.hpp"

namespace kalista
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    vector dragon_location = vector(9800.0f, 4400.0f);
    vector baron_location = vector(4950.0f, 10400.0f);

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_q = nullptr;
        TreeEntry* q_color = nullptr;
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* draw_range_w_minimap = nullptr;
        TreeEntry* w_color = nullptr;
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
        TreeEntry* draw_damage_e = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_auto_on_dragon_location = nullptr;
        TreeEntry* w_auto_on_baron_location = nullptr;
        TreeEntry* w_dont_use_if_enemies_nearby = nullptr;
        TreeEntry* w_enemies_search_radius = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_use_before_death = nullptr;
        TreeEntry* e_before_death_use_on_x_stacks = nullptr;
        TreeEntry* e_before_death_myhero_under_hp = nullptr;
        TreeEntry* e_before_death_calculate_incoming_damage = nullptr;
        TreeEntry* e_before_death_damage_time = nullptr;
        TreeEntry* e_before_death_over_my_hp_in_percent = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_ally_hp_under = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
        TreeEntry* r_enemies_search_radius = nullptr;
        TreeEntry* r_calculate_incoming_damage = nullptr;
        TreeEntry* r_coming_damage_time = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_only_on_x_stacks = nullptr;
        TreeEntry* e_if_nearby_minion_killable = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_use_if_killable_minions = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_epic_monsters_only = nullptr;
    }

    namespace misc
    {
        TreeEntry* jump_on_minions_when_chasing_enemy = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    //void on_after_attack_orbwalker(game_object_script target);

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
    int get_kalista_e_stacks(game_object_script target);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 1200);
        q->set_skillshot(0.25f, 80.0f, 2400.0f, { collisionable_objects::yasuo_wall, collisionable_objects::heroes, collisionable_objects::minions }, skillshot_type::skillshot_line);
        w = plugin_sdk->register_spell(spellslot::w, 5000);
        e = plugin_sdk->register_spell(spellslot::e, 1100);
        r = plugin_sdk->register_spell(spellslot::r, 1200);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("kalista", "Kalista");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + "combo.w.config", "W Config");
                {
                    combo::w_auto_on_dragon_location = w_config->add_checkbox(myhero->get_model() + ".combo.w.auto_on_dragon_location", "Auto W on Dragon Location", true);
                    combo::w_auto_on_baron_location  = w_config->add_checkbox(myhero->get_model() + ".combo.w.auto_on_baron_location", "Auto W on Baron Location", true);
                    combo::w_dont_use_if_enemies_nearby = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_if_enemies_nearby", "Dont use if enemies nearby", true);
                    combo::w_enemies_search_radius = w_config->add_slider(myhero->get_model() + ".combo.r.enemies_search_radius", "Enemies nearby search radius", 1200, 300, 1600);
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E on Killable", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
                {
                    combo::e_use_before_death = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_before_death", "Use before death", true);
                    auto before_death_config = e_config->add_tab(myhero->get_model() + "combo.e.before_death.config", "Use before death Config");
                    {
                        combo::e_before_death_use_on_x_stacks = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_use_on_x_stacks", "Use on x stacks", 6, 1, 16);
                        combo::e_before_death_myhero_under_hp = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_myhero_under_hp", "Myhero HP is under (in %)", 10, 0, 100);
                        combo::e_before_death_calculate_incoming_damage = before_death_config->add_checkbox(myhero->get_model() + ".combo.e.before_death_calculate_incoming_damage", "Calculate incoming damage", true);
                        combo::e_before_death_damage_time = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_damage_time", "Incoming damage time (in ms)", 600, 0, 1000);
                        combo::e_before_death_over_my_hp_in_percent = before_death_config->add_slider(myhero->get_model() + ".combo.w.before_death_over_my_hp_in_percent", "Coming damage is over my HP (in %)", 90, 0, 100);
                    }
                }
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R to save ally", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_ally_hp_under = r_config->add_slider(myhero->get_model() + ".combo.r.myhero_hp_under", "Ally HP is under (in %)", 20, 0, 100);
                    combo::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".combo.r.only_when_enemies_nearby", "Only when enemies are nearby", true);
                    combo::r_enemies_search_radius = r_config->add_slider(myhero->get_model() + ".combo.r.enemies_search_radius", "Enemies nearby search radius", 900, 300, 1600);
                    combo::r_calculate_incoming_damage = r_config->add_checkbox(myhero->get_model() + ".combo.r.calculate_incoming_damage", "Calculate incoming damage", true);
                    combo::r_coming_damage_time = r_config->add_slider(myhero->get_model() + ".combo.r.coming_damage_time", "Set coming damage time (in ms)", 1000, 0, 1000);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = harass->add_tab(myhero->get_model() + ".harass.e.config", "E Config");
                {
                    harass::e_only_on_x_stacks = e_config->add_slider(myhero->get_model() + ".harass.e.only_onx_stacks", "Use E only on x stacks", 6, 1, 16);
                    harass::e_only_on_x_stacks->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    harass::e_if_nearby_minion_killable = e_config->add_checkbox(myhero->get_model() + ".harass.e.if_nearby_minion_killable", "Use E if nearby minion killable", true);
                }
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_use_if_killable_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.use_if_killable_minions", "Use only when killable minions more than", 2, 1, 5);
                }
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = jungleclear->add_tab(myhero->get_model() + ".jungleclear.e.config", "E Config");
                {
                    jungleclear::e_epic_monsters_only = e_config->add_checkbox(myhero->get_model() + ".jungleclear.e.epic_monsters_only", "Use on epic monsters only", false);
                }
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Miscellaneous Settings");
            {
                misc::jump_on_minions_when_chasing_enemy = misc->add_checkbox(myhero->get_model() + ".misc.jump_on_minions_when_chasing_enemy", "Jump on minions when chasing enemy", true);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::draw_range_w_minimap = draw_settings->add_checkbox(myhero->get_model() + ".draw.w.minimap", "Draw W range on minimap", true);
                draw_settings::draw_range_w_minimap->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
                draw_settings::draw_damage_e = draw_settings->add_checkbox(myhero->get_model() + "draw.e.damage", "Draw E Damage", true);
            }
        }

        // Permashow initialization
        //
        {
	        Permashow::Instance.Init(main_tab);
	        Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        //event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
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

        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        //event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (e->is_ready())
        {
            e_logic();
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
            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (q->is_ready() && combo::use_q->get_bool())
                {
                    q_logic();
                }

                //console->print("AA range: %d | In E range: %d | In 1200 range: %d | Target: %s", myhero->count_enemies_in_range(myhero->get_attack_range()), myhero->count_enemies_in_range(e->range()), myhero->count_enemies_in_range(1200), orbwalker->get_target() == nullptr ? "null" : orbwalker->get_target()->get_name_cstr());

                if (misc::jump_on_minions_when_chasing_enemy->get_bool() && orbwalker->get_target() == nullptr && myhero->count_enemies_in_range(myhero->get_attack_range() + 50) == 0 && myhero->count_enemies_in_range(e->range()) != 0 && myhero->can_attack())
                {
                    // Gets enemy minions from the entitylist
                    auto lane_minions = entitylist->get_enemy_minions();

                    // You can use this function to delete minions that aren't in the specified range
                    lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                        {
                            return !x->is_valid_target(myhero->get_attack_range());
                        }), lane_minions.end());

                    //std::sort -> sort lane minions by distance
                    std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
                        {
                            return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
                        });

                    if (!lane_minions.empty())
                    {
                        //orbwalker->set_orbwalking_target(lane_minions.front());
                        myhero->issue_order(lane_minions.front(), true, true);
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

            if (!orbwalker->flee_mode() && !myhero->is_recalling())
            {
                if (w->is_ready() && combo::use_w->get_bool())
                {
                    w_logic();
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
                        if (q->cast_on_best_farm_position(1))
                        {
                            return;
                        }
                    }

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        int killable_minions = 0;

                        for (auto& minion : lane_minions)
                        {
                            if (e->get_damage(minion) > minion->get_health())
                            {
                                killable_minions++;
                            }
                        }

                        if (killable_minions >= laneclear::e_use_if_killable_minions->get_int())
                        {
                            if (e->cast())
                            {
                                return;
                            }
                        }
                    }
                }

                if (!monsters.empty())
                {
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (q->cast_on_best_farm_position(1, true))
                        {
                            return;
                        }
                    }

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        for (auto& monster : monsters)
                        {
                            if ((monster->is_epic_monster() || !jungleclear::e_epic_monsters_only->get_bool()) && e->get_damage(monster) > monster->get_health())
                            {
                                if (e->cast())
                                    return;
                            }
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
            q->cast(target, get_hitchance(hitchance::q_hitchance));
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        auto sentinels = entitylist->get_other_minion_objects();

        sentinels.erase(std::remove_if(sentinels.begin(), sentinels.end(), [](game_object_script x)
            {
                return !x->is_valid();
            }), sentinels.end());

        sentinels.erase(std::remove_if(sentinels.begin(), sentinels.end(), [](game_object_script x)
            {
                return x->get_model().compare("KalistaSpawn") != 0;
            }), sentinels.end());

        bool sentinel_alive_on_dragon = false;
        bool sentinel_alive_on_baron = false;

        for (auto& sentinel : sentinels)
        {
            if (sentinel->get_distance(dragon_location) < 1600)
            {
                sentinel_alive_on_dragon = true;
            }
            if (sentinel->get_distance(baron_location) < 1600)
            {
                sentinel_alive_on_baron = true;
            }
        }

        if (!combo::w_dont_use_if_enemies_nearby->get_bool() || myhero->count_enemies_in_range(combo::w_enemies_search_radius->get_int()) == 0)
        {
            if (combo::w_auto_on_dragon_location->get_bool() && !sentinel_alive_on_dragon)
            {
                auto dragon_distance = myhero->get_distance(dragon_location);
                if (w->range() - 50 > dragon_distance && dragon_distance > 1450)
                {
                    if (w->cast(dragon_location))
                        return;
                }
            }

            if (combo::w_auto_on_baron_location->get_bool() && !sentinel_alive_on_baron)
            {
                auto baron_distance = myhero->get_distance(baron_location);
                if (w->range() - 50 > baron_distance && baron_distance > 1450)
                {   
                    if (w->cast(baron_location))
                        return;
                }
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        auto enemies = entitylist->get_enemy_heroes();

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return !x->is_valid();
            }), enemies.end());

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return x->is_dead();
            }), enemies.end());

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return !x->is_valid_target(e->range());
            }), enemies.end());

        if ((orbwalker->harass() || orbwalker->lane_clear_mode()) && harass::use_e->get_bool())
        {
            for (auto& enemy : enemies)
            {
                auto stacks = get_kalista_e_stacks(enemy);

                if (stacks != 0)
                {
                    if (stacks >= harass::e_only_on_x_stacks->get_int())
                    {
                        if (e->cast())
                            return;
                    }

                    if (harass::e_if_nearby_minion_killable->get_bool())
                    {
                        // Gets enemy minions from the entitylist
                        auto lane_minions = entitylist->get_enemy_minions();

                        // You can use this function to delete minions that aren't in the specified range
                        lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                            {
                                return !x->is_valid_target(e->range());
                            }), lane_minions.end());

                        // Remove unkillable minions
                        lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                            {
                                return e->get_damage(x) < x->get_health();
                            }), lane_minions.end());

                        if (!lane_minions.empty())
                        {
                            if (e->cast())
                                return;
                        }
                    }
                }
            }
        }

        if (combo::use_e->get_bool())
        {
            for (auto& enemy : enemies)
            {
                if (e->get_damage(enemy) > enemy->get_real_health())
                {
                    e->cast();
                }
                else if (combo::e_use_before_death->get_bool()
                    && (myhero->get_health_percent() <= combo::e_before_death_myhero_under_hp->get_int()
                        || (combo::e_before_death_calculate_incoming_damage->get_bool() && (health_prediction->get_incoming_damage(myhero, combo::e_before_death_damage_time->get_int() / 1000.f, true) * 100.f) /
                            myhero->get_max_health() > myhero->get_health_percent() * (combo::e_before_death_over_my_hp_in_percent->get_int() / 100.f))) && get_kalista_e_stacks(enemy) >= combo::e_before_death_use_on_x_stacks->get_int())
                {
                    e->cast();
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        for (auto&& ally : entitylist->get_ally_heroes())
        {
            if (ally->get_distance(myhero->get_position()) <= r->range())
            {
                if (ally->has_buff(buff_hash("kalistacoopstrikeally")))
                {
                    if ((ally->get_health_percent() < combo::r_ally_hp_under->get_int()) || (combo::r_calculate_incoming_damage->get_bool() && health_prediction->get_incoming_damage(ally, combo::r_coming_damage_time->get_int() / 1000.0f, true) >= ally->get_health()))
                    {
                        if (!combo::r_only_when_enemies_nearby->get_bool() || ally->count_enemies_in_range(combo::r_enemies_search_radius->get_int()) != 0)
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

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

        // Draw W range on minimap
        if (w->is_ready() && draw_settings::draw_range_w_minimap->get_bool())
            draw_manager->draw_circle_on_minimap(myhero->get_position(), w->range(), draw_settings::w_color->get_color());

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color());

        if (e->is_ready() && draw_settings::draw_damage_e->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    draw_dmg_rl(enemy, e->get_damage(enemy), 0x8000ff00);
                }
            }
        }
    }

    //void on_after_attack_orbwalker(game_object_script target)
    //{
        //if (orbwalker->combo_mode() && misc::jump_on_minions_when_chasing_enemy->get_bool() && target->is_valid() && target->is_ai_minion())
        //{
        //    orbwalker->set_orbwalking_target(nullptr);
        //}
    //}

    int get_kalista_e_stacks(game_object_script target)
    {
        if (target->is_valid())
        {
            auto buff = target->get_buff(buff_hash("kalistaexpungemarker"));
            if (buff != nullptr && buff->is_valid() && buff->is_alive())
            {
                return buff->get_count();
            }
        }
        return 0;
    }
};