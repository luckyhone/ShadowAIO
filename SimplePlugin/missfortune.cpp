#include "../plugin_sdk/plugin_sdk.hpp"
#include "missfortune.h"
#include "dmg_lib.h"
#include "utils.h"
#include "permashow.hpp"

namespace missfortune
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* q1 = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
        TreeEntry* draw_damage_r = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_mode = nullptr;
        TreeEntry* q_auto_harass = nullptr;
        TreeEntry* q_use_on_minion_to_crit_on_enemy = nullptr;
        TreeEntry* q_use_on_minion_to_crit_on_enemy_use_on_moving = nullptr;
        TreeEntry* q_use_on_minion_to_crit_on_enemy_only_on_killable = nullptr;
        TreeEntry* q_use_on_minion_to_crit_maximum_range_minion_to_enemy = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_mode = nullptr;
        TreeEntry* w_use_while_chasing = nullptr;
        TreeEntry* w_target_above_range = nullptr;
        TreeEntry* w_target_below_range = nullptr;
        TreeEntry* w_target_hp_under = nullptr;
        TreeEntry* w_dont_use_target_under_turret = nullptr;
        TreeEntry* w_check_if_target_is_not_facing = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_save = nullptr;
        TreeEntry* r_min_range = nullptr;
        TreeEntry* r_max_range = nullptr;
        TreeEntry* r_use_if_killable_by_x_waves = nullptr;
        TreeEntry* r_dont_waste_if_target_hp_below = nullptr;
        TreeEntry* r_auto_if_enemies_more_than = nullptr;
        TreeEntry* r_try_to_use_e_before_r = nullptr;
        TreeEntry* r_auto_on_cc = nullptr;
        TreeEntry* r_cancel_if_nobody_inside = nullptr;
        TreeEntry* r_block_mouse_move = nullptr;
        TreeEntry* r_disable_evade = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
        bool previous_evade_state = false;
        bool previous_orbwalker_state = false;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_only_if_mana_more_than = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_w_on_turret = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_minimum_minions = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace lasthit
    {
        TreeEntry* lasthit = nullptr;
        TreeEntry* use_q = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w;
        TreeEntry* use_e;
    }

    namespace antigapclose
    {
        TreeEntry* use_e = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
        TreeEntry* e_hitchance = nullptr;
        TreeEntry* r_hitchance = nullptr;
    }

    float last_r_time = 0.0f;

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack_orbwalker(game_object_script target, bool* process);
    void on_after_attack_orbwalker(game_object_script target);
    void on_issue_order(game_object_script& target, vector& pos, _issue_order_type& type, bool* process);
    void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void q_logic_auto();
    void q_logic_minion();
    void w_logic();
    void e_logic();
    bool r_logic();
    bool r_logic_auto();
    bool r_logic_semi();

    // Utils
    //
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    // Other
    //
    game_object_script last_lasthit_target;

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, myhero->get_attack_range());
        q1 = plugin_sdk->register_spell(spellslot::q, myhero->get_attack_range());
        q1->set_skillshot(0.0f, 60.0f, 1400.0f, { collisionable_objects::heroes }, skillshot_type::skillshot_line);
        w = plugin_sdk->register_spell(spellslot::w, 0);
        e = plugin_sdk->register_spell(spellslot::e, 1000);
        e->set_skillshot(0.25f, 100.0f, FLT_MAX, { }, skillshot_type::skillshot_circle);
        r = plugin_sdk->register_spell(spellslot::r, 1450);
        r->set_skillshot(0.0f, 200.0f, FLT_MAX, { }, skillshot_type::skillshot_line);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("missfortune", "Miss Fortune");
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
                    q_config->add_separator(myhero->get_model() + ".combo.q.separator1", "Usage Settings");
                    combo::q_mode = q_config->add_combobox(myhero->get_model() + ".combo.q.mode", "Q Mode", { {"In Combo", nullptr},{"After AA", nullptr } }, 1);
                    combo::q_auto_harass = q_config->add_hotkey(myhero->get_model() + ".combo.q.config", "Auto Q harass", TreeHotkeyMode::Toggle, 'A', true);

                    q_config->add_separator(myhero->get_model() + ".combo.q.separator2", "Q Minion Logic Settings");

                    combo::q_use_on_minion_to_crit_on_enemy = q_config->add_checkbox(myhero->get_model() + ".combo.q.use_on_minion_to_crit_on_enemy", "Use Q on minions to crit on enemy", true);
                    combo::q_use_on_minion_to_crit_on_enemy->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::q_use_on_minion_to_crit_on_enemy_use_on_moving = q_config->add_checkbox(myhero->get_model() + ".combo.q.use_on_minion_to_crit_on_enemy_use_on_moving", "^ Use Q on moving minions", false);
                    combo::q_use_on_minion_to_crit_on_enemy_use_on_moving->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::q_use_on_minion_to_crit_on_enemy_only_on_killable = q_config->add_checkbox(myhero->get_model() + ".combo.q.use_on_minion_to_crit_on_enemy_only_on_killable", "^ Use Q only on killable minions", true);
                    combo::q_use_on_minion_to_crit_on_enemy_only_on_killable->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::q_use_on_minion_to_crit_maximum_range_minion_to_enemy = q_config->add_slider(myhero->get_model() + ".combo.q.use_on_minion_to_crit_maximum_range_minion_to_enemy", "^ Maximum minion distance to target", 500, 1, 525);
                }

                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);

                auto w_config = combo->add_tab(myhero->get_model() + "combo.w.config", "W Config");
                {
                    w_config->add_separator(myhero->get_model() + ".combo.w.separator1", "W on AA Settings");
                    combo::w_mode = w_config->add_combobox(myhero->get_model() + ".combo.w.mode", "W Mode", { {"Before AA", nullptr},{"After AA", nullptr } }, 0);

                    w_config->add_separator(myhero->get_model() + ".combo.w.separator2", "W while chasing Options");
                    combo::w_use_while_chasing = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_while_chasing", "Use W while chasing an enemy", true);
                    combo::w_use_while_chasing->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                    combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Target is above range", 525, 0, 800);
                    combo::w_target_below_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_below_range", "Target is below range", 950, 0, 1100);
                    combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".combo.w.target_hp_under", "Target HP is under (in %)", 50, 0, 100);
                    combo::w_dont_use_target_under_turret = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_target_under_turret", "Dont use if target is under turret", true);
                    combo::w_check_if_target_is_not_facing = w_config->add_checkbox(myhero->get_model() + ".combo.w.check_if_target_is_not_facing", "Check if target is not facing myhero", true);
                }

                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    r_config->add_separator(myhero->get_model() + ".combo.r.separator1", "Range Settings");
                    combo::r_min_range = r_config->add_slider(myhero->get_model() + ".combo.r.min_range", "Minimum R range", 400, 1, r->range());
                    combo::r_max_range = r_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum R range", 1150, 550, r->range());

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator2", "Usage Settings");
                    combo::r_dont_waste_if_target_hp_below = r_config->add_slider(myhero->get_model() + ".combo.r.dont_waste_if_target_hp_below", "Don't waste R if target hp is below (in %)", 15, 1, 100);
                    combo::r_use_if_killable_by_x_waves = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_killable_by_x_waves", "Use R if target killable by x waves", 6, 1, 14);
                    combo::r_auto_if_enemies_more_than = r_config->add_slider(myhero->get_model() + ".combo.r.auto_if_enemies_more_than", "Use R if will hit enemies more than", 3, 1, 5);
                    combo::r_try_to_use_e_before_r = r_config->add_checkbox(myhero->get_model() + ".combo.r.r_try_to_use_e_before_r", "Try to use E before R", true);
                    combo::r_try_to_use_e_before_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    combo::r_auto_on_cc = r_config->add_checkbox(myhero->get_model() + ".combo.r.auto_on_cc", "Use R on CC", false);
                    combo::r_auto_on_cc->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_cancel_if_nobody_inside = r_config->add_checkbox(myhero->get_model() + ".combo.r.cancel_if_nobody_inside", "Cancel R if nobody inside", true);
                    combo::r_cancel_if_nobody_inside->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator3", "Other Settings");
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_save = r_config->add_hotkey(myhero->get_model() + ".combo.r.save", "Save R", TreeHotkeyMode::Toggle, 'G', false);
                    combo::r_block_mouse_move = r_config->add_checkbox(myhero->get_model() + ".combo.r.block_mouse_move", "Block Mouse Move on R", false);
                    combo::r_block_mouse_move->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_disable_evade = r_config->add_checkbox(myhero->get_model() + ".combo.r.disable_evade", "Disable Evade on R", true);
                    combo::r_disable_evade->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

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
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", false);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = harass->add_tab(myhero->get_model() + "harass.r.config", "E Config");
                {
                    harass::e_only_if_mana_more_than = e_config->add_slider(myhero->get_model() + ".harass.e.only_if_mana_more_than", "Use E if mana more than (in %)", 40, 0, 100);
                }
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_w_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w.on_turret", "Use W On Turret", true);
                laneclear::use_w_on_turret->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "Minimum minions", 3, 0, 5);
                }
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
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E to slow enemies", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q (minion -> enemy)", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
                draw_settings::draw_damage_r = draw_settings->add_checkbox(myhero->get_model() + "draw.R.damage", "Draw R Damage", true);
            }
        }

        // Permashow initialization
        //
        {
	        Permashow::Instance.Init(main_tab);
	        Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
	        Permashow::Instance.AddElement("Last Hit", lasthit::lasthit);
            Permashow::Instance.AddElement("Auto Q Harass", combo::q_auto_harass);
	        Permashow::Instance.AddElement("Semi Auto R", combo::r_semi_manual_cast);
            Permashow::Instance.AddElement("Save R", combo::r_save);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack_orbwalker);
        event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
        event_handler<events::on_issue_order>::add_callback(on_issue_order);
        event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);

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
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack_orbwalker);
        event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
        event_handler<events::on_issue_order>::remove_handler(on_issue_order);
        event_handler<events::on_process_spell_cast>::remove_handler(on_process_spell_cast);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (myhero->is_casting_interruptible_spell() || myhero->has_buff(buff_hash("missfortunebulletsound")) || gametime->get_time() - last_r_time < 0.3f)
        {
            std::vector<game_object_script> hit_by_r;

            if (combo::r_cancel_if_nobody_inside->get_bool())
            {
                for (auto& enemy : entitylist->get_enemy_heroes())
                {
                    if (enemy->is_valid() && !enemy->is_dead() && enemy->is_valid_target(r->range()))
                    {
                        if (r->get_prediction(enemy).hitchance >= hit_chance::low)
                        {
                            hit_by_r.push_back(enemy);
                        }
                    }
                }
            }

            bool should_cancel_r = false;

            if (!hit_by_r.empty())
            {
                if (!combo::previous_orbwalker_state)
                {
                    orbwalker->set_attack(false);
                    orbwalker->set_movement(false);
                    combo::previous_orbwalker_state = true;
                }

                if (!combo::previous_evade_state && combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                {
                    evade->disable_evade();
                    combo::previous_evade_state = true;
                }

                return;
            }
            else if (combo::r_cancel_if_nobody_inside->get_bool())
            {
                should_cancel_r = true;
            }

            if (!should_cancel_r)
            {
                return;
            }
        }

        if (last_r_time != 0.0f)
        {
            last_r_time = 0.0f;
        }

        if (combo::previous_orbwalker_state)
        {
            orbwalker->set_attack(true);
            orbwalker->set_movement(true);
            combo::previous_orbwalker_state = false;
        }
        if (combo::previous_evade_state)
        {
            evade->enable_evade();
            combo::previous_evade_state = false;
        }

        if ((orbwalker->last_hit_mode() || orbwalker->harass() || orbwalker->lane_clear_mode()) && lasthit::lasthit->get_bool())
        {
            // Gets enemy minions from the entitylist
            auto lane_minions = entitylist->get_enemy_minions();

            // You can use this function to delete minions that aren't in the specified range
            lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                {
                    return !x->is_valid_target(q->range() + 50);
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
                        if (dmg_lib::get_damage(q, minion) > minion->get_health())
                        {
                            if (q->cast(minion))
                            {
                                last_lasthit_target = minion;
                                return;
                            }
                        }
                    }
                }
            }
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (r->is_ready() && combo::use_r->get_bool())
            {
                if (r_logic_auto())
                {
                    return;
                }
                if (r_logic_semi())
                {
                    return;
                }
            }

            if (q->is_ready() && combo::q_auto_harass->get_bool() && !myhero->is_under_enemy_turret())
            {
                q_logic_auto();
            }

            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (r->is_ready() && combo::use_r->get_bool())
                {
                    if (r_logic())
                    {
                        return;
                    }
                }

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

                    if (e->is_ready() && harass::use_e->get_bool() && myhero->get_mana_percent() > harass::e_only_if_mana_more_than->get_int())
                    {
                        e_logic();
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
                if (e->is_ready() && fleemode::use_e->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(e->range(), damage_type::magical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (e->cast(target, get_hitchance(hitchance::e_hitchance)))
                        {
                            return;
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
                        return !x->is_valid_target(q->range() + 50);
                    }), lane_minions.end());

                // You can use this function to delete monsters that aren't in the specified range
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(q->range() + 50);
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

                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(laneclear::e_minimum_minions->get_int()))
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

                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(1, true))
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
        auto target = target_selector->get_target(myhero->get_attack_range() + 50, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Checking if the target will die from Q damage
            if (combo::q_mode->get_int() == 0 || dmg_lib::get_damage(q, target) >= target->get_health())
            {
                q->cast(target);
            }
        }
        else if (combo::q_use_on_minion_to_crit_on_enemy->get_bool())
        {
            q_logic_minion();
        }
    }
#pragma endregion

#pragma region q_logic
    void q_logic_auto()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(myhero->get_attack_range() + 50, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (orbwalker->combo_mode() || orbwalker->harass())
            {
                // Checking if the target will die from Q damage
                if (combo::q_mode->get_int() == 0 || dmg_lib::get_damage(q, target) >= target->get_health())
                {
                    q->cast(target);
                }
            }
            else
            {
                q->cast(target);
            }
        }
        else if (combo::q_use_on_minion_to_crit_on_enemy->get_bool())
        {
            q_logic_minion();
        }
    }
#pragma endregion

#pragma region q_logic_minion
    void q_logic_minion()
    {
        // Gets enemy minions from the entitylist
        auto lane_minions = entitylist->get_enemy_minions();

        // You can use this function to delete minions that aren't in the specified range
        lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
            {
                return !x->is_valid_target(myhero->get_attack_range() + 50);
            }), lane_minions.end());

        // Delete moving minions
        lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
            {
                return !combo::q_use_on_minion_to_crit_on_enemy_use_on_moving->get_bool() && x->is_moving();
            }), lane_minions.end());

        // Delete unkillable minions
        lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
            {
                return combo::q_use_on_minion_to_crit_on_enemy_only_on_killable->get_bool() && x->get_health() > dmg_lib::get_damage(q, x);
            }), lane_minions.end());

        if (!lane_minions.empty())
        {
            for (auto& minion : lane_minions)
            {
                // Gets enemies from the entitylist
                auto enemies = entitylist->get_enemy_heroes();

                // Delete invalid enemies
                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target();
                    }), enemies.end());

                // You can use this function to delete minions that aren't in the specified range
                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [minion](game_object_script x)
                    {
                        return x->get_distance(minion) > 525;
                    }), enemies.end());

                if (!enemies.empty())
                {
                    for (auto& enemy : enemies)
                    {
                        prediction_input x;

                        x._from = minion->get_position();
                        x._range_check_from = myhero->get_position();
                        x.unit = enemy;
                        x.delay = q1->delay;
                        x.radius = q1->radius;
                        x.speed = q1->speed;
                        x.collision_objects = q1->collision_flags;
                        x.range = myhero->get_attack_range() + myhero->get_distance(minion);
                        x.type = q1->type;
                        x.aoe = false;
                        x.spell_slot = q1->slot;
                        x.use_bounding_radius = q1->type != skillshot_type::skillshot_circle;

                        auto pred = prediction->get_prediction(&x);

                        if (pred.hitchance >= get_hitchance(hitchance::q_hitchance))
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
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        if (combo::w_use_while_chasing->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::w_target_below_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (target->get_health_percent() < combo::w_target_hp_under->get_int() && target->get_distance(myhero) > combo::w_target_above_range->get_int())
                {
                    if (!combo::w_dont_use_target_under_turret->get_bool() || !target->is_under_ally_turret())
                    {
                        if (!combo::w_check_if_target_is_not_facing->get_bool() || !target->is_facing(myhero))
                        {
                            w->cast();
                        }
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
    bool r_logic()
    {
        if (combo::r_save->get_bool())
            return false;

        // Get a target from a given range
        auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && !target->is_zombie() && target->is_attack_allowed_on_target() && !utils::has_unkillable_buff(target) && can_use_r_on(target) && myhero->get_distance(target) > combo::r_min_range->get_int())
        {
            if (target->get_health_percent() > combo::r_dont_waste_if_target_hp_below->get_int())
            {
                if (dmg_lib::get_damage(r, target) * combo::r_use_if_killable_by_x_waves->get_int() > target->get_health())
                {
                    auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                    if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
                    {
                        orbwalker->set_attack(false);
                        orbwalker->set_movement(false);
                        combo::previous_orbwalker_state = true;

                        if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                        {
                            evade->disable_evade();
                            combo::previous_evade_state = true;
                        }
                        if (combo::r_try_to_use_e_before_r->get_bool() && e->is_ready() && target->is_valid_target(e->range()))
                        {
                            e->cast(pred.get_unit_position());
                        }
                        if (r->cast(pred.get_unit_position()))
                        {
                            last_r_time = gametime->get_time();
                            return true;
                        }
                    }

                }
            }
        }

        return false;
    }
#pragma endregion

#pragma region r_logic_auto
    bool r_logic_auto()
    {
        if (combo::r_save->get_bool())
            return false;

        std::vector<game_object_script> hit_by_r;

        for (auto& enemy : entitylist->get_enemy_heroes())
        {
            if (enemy->is_valid() && !enemy->is_dead() && !enemy->is_zombie() && !utils::has_unkillable_buff(enemy) && enemy->is_valid_target(combo::r_max_range->get_int()))
            {
                auto pred = prediction->get_prediction(enemy, r->get_delay(), r->get_radius(), r->get_speed());
                if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
                {
                    hit_by_r.push_back(enemy);
                }
            }
        }

        if (hit_by_r.size() >= combo::r_auto_if_enemies_more_than->get_int())
        {
            auto pred = prediction->get_prediction(hit_by_r.front(), r->get_delay(), r->get_radius(), r->get_speed());
            if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
            {
                orbwalker->set_attack(false);
                orbwalker->set_movement(false);
                combo::previous_orbwalker_state = true;

                if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                {
                    evade->disable_evade();
                    combo::previous_evade_state = true;
                }
                if (combo::r_try_to_use_e_before_r->get_bool() && e->is_ready() && hit_by_r.front()->is_valid_target(e->range()))
                {
                    e->cast(pred.get_unit_position());
                }
                if (r->cast(pred.get_unit_position()))
                {
                    last_r_time = gametime->get_time();
                    return true;
                }
            }
        }

        if (combo::r_auto_on_cc->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr && can_use_r_on(target) && myhero->get_distance(target) > combo::r_min_range->get_int())
            {
                auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                if (pred.hitchance >= hit_chance::immobile)
                {
                    orbwalker->set_attack(false);
                    orbwalker->set_movement(false);
                    combo::previous_orbwalker_state = true;

                    if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                    {
                        evade->disable_evade();
                        combo::previous_evade_state = true;
                    }
                    if (combo::r_try_to_use_e_before_r->get_bool() && e->is_ready() && target->is_valid_target(e->range()))
                    {
                        e->cast(pred.get_unit_position());
                    }
                    if (r->cast(pred.get_unit_position()))
                    {
                        last_r_time = gametime->get_time();
                        return true;
                    }
                }
            }
        }

        return false;
    }
#pragma endregion

#pragma region r_logic_semi
    bool r_logic_semi()
    {
        if (combo::r_semi_manual_cast->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(r->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr && can_use_r_on(target) && myhero->get_distance(target) > combo::r_min_range->get_int())
            {
                auto pred = prediction->get_prediction(target, r->get_delay(), r->get_radius(), r->get_speed());
                if (pred.hitchance >= get_hitchance(hitchance::r_hitchance))
                {
                    orbwalker->set_attack(false);
                    orbwalker->set_movement(false);
                    combo::previous_orbwalker_state = true;

                    if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
                    {
                        evade->disable_evade();
                        combo::previous_evade_state = true;
                    }
                    if (combo::r_try_to_use_e_before_r->get_bool() && e->is_ready() && target->is_valid_target(e->range()))
                    {
                        e->cast(pred.get_unit_position());
                    }
                    if (r->cast(pred.get_unit_position()))
                    {
                        last_r_time = gametime->get_time();
                        return true;
                    }
                }
            }
        }

        return false;
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

    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::r_max_range->get_int(), draw_settings::r_color->get_color());

        if (r->is_ready() && draw_settings::draw_damage_r->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    draw_dmg_rl(enemy, dmg_lib::get_damage(r, enemy) * combo::r_use_if_killable_by_x_waves->get_int(), 0x8000ff00);
                }
            }
        }
    }

    void on_before_attack_orbwalker(game_object_script target, bool* process)
    {
        if (last_lasthit_target == target)
        {
            *process = false;
            return;
        }

        if (q->is_ready())
        {
            // Use q before autoattack on lane minions (lasthit)
            if (target->is_minion() && dmg_lib::get_damage(q, target) >= target->get_health() && (orbwalker->lane_clear_mode() || orbwalker->harass() || orbwalker->last_hit_mode()) && lasthit::lasthit->get_bool() && lasthit::use_q->get_bool())
            {
                *process = false;
                if (q->cast(target))
                {
                    return;
                }
            }
        }

        if (w->is_ready() && combo::w_mode->get_int() == 0)
        {
            // Use w before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_w->get_bool())))
            {
                if (w->cast())
                {
                    return;
                }
            }

            // Use w before autoattack on lane minions
            if (target->is_minion() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_w->get_bool())) {
                if (w->cast())
                {
                    return;
                }
            }

            // Use w before autoattack on monsters
            if (target->is_monster() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_w->get_bool())) {
                if (w->cast())
                {
                    return;
                }
            }
        }
    }

    void on_after_attack_orbwalker(game_object_script target)
    {
        if (q->is_ready() && combo::q_mode->get_int() == 1)
        {
            // Using q after autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (q->cast(target))
                {
                    return;
                }
            }
        }

        if (w->is_ready() && combo::w_mode->get_int() == 1)
        {
            // Use w after autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_w->get_bool())))
            {
                if (w->cast())
                {
                    return;
                }
            }

            // Use w after autoattack on lane minions
            if (target->is_minion() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_w->get_bool())) {
                if (w->cast())
                {
                    return;
                }
            }

            // Use w after autoattack on monsters
            if (target->is_monster() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_w->get_bool())) {
                if (w->cast())
                {
                    return;
                }
            }
        }

        if (w->is_ready())
        {
            // Use w before autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_w_on_turret->get_bool() && target->is_ai_turret())
            {
                if (w->cast())
                {
                    return;
                }
            }
        }
    }

    void on_issue_order(game_object_script& target, vector& pos, _issue_order_type& type, bool* process)
    {
        if (combo::r_block_mouse_move->get_bool() && combo::previous_orbwalker_state && last_r_time != 0.0f && (myhero->is_casting_interruptible_spell() || myhero->has_buff(buff_hash("missfortunebulletsound")) || gametime->get_time() - last_r_time < 0.3f))
        {
            *process = false;
        }
    }

    void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
    {
        if (sender->is_me() && spell->get_spellslot() == r->get_slot())
        {
            last_r_time = gametime->get_time();
            orbwalker->set_attack(false);
            orbwalker->set_movement(false);
            combo::previous_orbwalker_state = true;

            if (combo::r_disable_evade->get_bool() && evade->is_evade_registered() && !evade->is_evade_disabled())
            {
                evade->disable_evade();
                combo::previous_evade_state = true;
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if ((myhero->get_active_spell() != nullptr && myhero->get_active_spell()->is_channeling()) || gametime->get_time() - last_r_time < 0.3f)
        {
            return;
        }

        if (antigapclose::use_e->get_bool() && e->is_ready() && !myhero->is_under_enemy_turret())
        {
            if (sender->is_valid_target(e->range() + sender->get_bounding_radius()))
            {
                e->cast(sender, get_hitchance(hitchance::e_hitchance));
            }
        }
    }
};