#include "../plugin_sdk/plugin_sdk.hpp"
#include "ivern.h"
#include "permashow.hpp"

namespace ivern
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;

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
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_use_on = nullptr;;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_incoming_damage_time = nullptr;
        TreeEntry* e_over_hp_in_percent = nullptr;
        std::map<std::uint32_t, TreeEntry*> e_use_on;
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

    namespace antigapclose
    {
        TreeEntry* use_q = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();

    // Utils
    //
    game_object_script get_best_target_prority(const std::vector<game_object_script>& possible_targets, TreeEntry* priority);
    bool can_use_e_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);

    vector last_w_position = vector(0, 0);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 1100);
        q->set_skillshot(0.25f, 160.0f, 1300.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
        w = plugin_sdk->register_spell(spellslot::w, 1000);
        e = plugin_sdk->register_spell(spellslot::e, 750);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("ivern", "Ivern");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", false);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    std::vector<ProrityCheckItem> prority_items;

                    for (auto&& ally : entitylist->get_ally_heroes())
                    {
                        prority_items.push_back({ ally->get_network_id(), ally->get_model(), true, ally->get_square_icon_portrait() });
                    }

                    // In this case you HAVE to set should save to false since prority key contains network id which is unique per game
                    //
                    combo::w_use_on = w_config->add_prority_list(myhero->get_model() + ".combo.w.use_on", "Use W On", prority_items, false, false);
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    combo::e_incoming_damage_time = e_config->add_slider(myhero->get_model() + ".misc.e.incoming_damage_time", "Set coming damage time (in ms)", 200, 0, 1000);
                    combo::e_over_hp_in_percent = e_config->add_slider(myhero->get_model() + ".misc.e.over_hp_in_percent", "Coming damage is over HP (in %)", 10, 0, 100);

                    auto use_e_on_tab = e_config->add_tab(myhero->get_model() + ".combo.e.use_on", "Use E On");
                    {
                        for (auto&& ally : entitylist->get_ally_heroes())
                        {
                            // In this case you HAVE to set should save to false since key contains network id which is unique per game
                            //
                            combo::e_use_on[ally->get_network_id()] = use_e_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), true, false);

                            // Set texture to ally square icon
                            //
                            combo::e_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
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

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_q = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.q", "Use Q", true);
                antigapclose::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
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
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (!myhero->is_recalling() && e->is_ready() && combo::use_e->get_bool())
        {
            for (auto& ally : entitylist->get_ally_heroes())
            {
                if (can_use_e_on(ally) && myhero->get_distance(ally) < e->range())
                {
                    if (health_prediction->get_incoming_damage(ally, combo::e_incoming_damage_time->get_int() / 1000.f, true) * 100.f /
                        ally->get_max_health() > ally->get_health_percent() * (combo::e_over_hp_in_percent->get_int() / 100.f))
                    {
                        e->cast(ally);
                    }
                }
            }
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
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
                    }
                }
            }

            // Checking if the user has selected lane_clear_mode() (Default V)
            if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool())
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
                    if (q->is_ready() && laneclear::use_q->get_bool())
                    {
                        q->cast(lane_minions.front(), get_hitchance(hitchance::q_hitchance));
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
            q->cast(target, get_hitchance(hitchance::q_hitchance));
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        std::vector<game_object_script> allies = entitylist->get_ally_heroes();

        allies.erase(std::remove_if(allies.begin(), allies.end(), [](game_object_script x)
            {
                return x->get_distance(myhero) > w->range();
            }), allies.end());

        allies.erase(std::remove_if(allies.begin(), allies.end(), [](game_object_script x)
            {
                return x->count_enemies_in_range(750) == 0;
            }), allies.end());

        if (!allies.empty())
        {
            auto target = get_best_target_prority(allies, combo::w_use_on);

            if (target->is_valid() && target->count_enemies_in_range(150) == 0)
            {
                auto position = target->get_position();

                if (position.distance(last_w_position) > 250 && w->cast(position))
                {
                    last_w_position = position;
                }
            }
        }
    }
#pragma endregion

    game_object_script get_best_target_prority(const std::vector<game_object_script>& possible_targets, TreeEntry* priority)
    {
        std::vector<std::pair<game_object_script, std::int32_t>> valid_targets;

        for (auto&& target : possible_targets)
        {
            auto target_prority = priority->get_prority(target->get_network_id());

            if (target_prority.first == -1 || !target_prority.second)
                continue;

            valid_targets.push_back({ target, target_prority.first });
        }

        std::sort(valid_targets.begin(), valid_targets.end(), [](const std::pair<game_object_script, std::int32_t>& a, const std::pair<game_object_script, std::int32_t>& b)
            {
                // Sort by prority value
                //
                //   First in vector = top
                //
                return a.second < b.second;
            });

        // Return best target
        //
        //   If list is empty return nullptr
        //
        return valid_targets.empty() ? nullptr : valid_targets.front().first;
    }

    bool can_use_e_on(game_object_script target)
    {
        auto it = combo::e_use_on.find(target->get_network_id());
        if (it == combo::e_use_on.end())
            return false;

        return it->second->get_bool();
    }

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
        if (antigapclose::use_q->get_bool() && q->is_ready())
        {
            if (sender->is_valid_target(q->range() + sender->get_bounding_radius()))
            {
                q->cast(sender, get_hitchance(hitchance::q_hitchance));
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
    }
};