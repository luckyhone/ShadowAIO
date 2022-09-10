#include "../plugin_sdk/plugin_sdk.hpp"
#include "draven.h"
#include "utils.h"
#include "permashow.hpp"

namespace draven
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
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* e_color = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* r_color = nullptr;
        TreeEntry* draw_damage_r = nullptr;
        TreeEntry* draw_axe_radius = nullptr;
        TreeEntry* draw_axe_move_radius = nullptr;
        TreeEntry* draw_axe_number = nullptr;
        TreeEntry* draw_axe_expire_time = nullptr;
        TreeEntry* draw_line_to_axe = nullptr;
        TreeEntry* axe_radius_color = nullptr;
        TreeEntry* axe_move_radius_color = nullptr;
        TreeEntry* axe_number_color = nullptr;
        TreeEntry* axe_expire_time_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_max_active_axes = nullptr;
        TreeEntry* q_cast_to_keep_two_axes = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_max_range = nullptr;
        TreeEntry* w_cast_while_chasing = nullptr;
        TreeEntry* w_target_above_range = nullptr;
        TreeEntry* w_cast_in_fight = nullptr;
        TreeEntry* w_cast_before_catching_axe = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_semi_manual_cast = nullptr;
        TreeEntry* e_mode = nullptr;
        TreeEntry* e_max_range = nullptr;
        TreeEntry* e_spell_interrupter = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_min_distance = nullptr;
        TreeEntry* r_max_range = nullptr;
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
        TreeEntry* e_minimum_minions = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace catch_axes_settings
    {
        TreeEntry* catch_axes = nullptr;
        TreeEntry* catch_mode = nullptr;
        TreeEntry* catch_only_if_orbwalker_active = nullptr;
        TreeEntry* catch_axes_under_turret = nullptr;
        TreeEntry* axe_maximum_distance = nullptr;
        TreeEntry* move_to_axe_if_distance_higher_than = nullptr;
        TreeEntry* block_aa_if_distance_to_axe_smaller_than = nullptr;
        TreeEntry* block_aa_while_staying_in_axe = nullptr;
        TreeEntry* dont_catch_axes = nullptr;
        TreeEntry* dont_catch_axes_if_killable_by_x_aa = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* e_hitchance = nullptr;
        TreeEntry* r_hitchance = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);
    void on_before_attack(game_object_script target, bool* process);
    void on_after_attack(game_object_script target);
    void on_create_object(game_object_script sender);
    void on_delete_object(game_object_script sender);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void e_logic_semi();
    void r_logic();
    void r_logic_semi();

    // Utils
    //
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    int get_draven_q_stacks();
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    struct axe
    {
        game_object_script object;
        int axe_id;
        float start_time;
        float expire_time;

        axe(game_object_script obj, int id)
        {
            object = obj;
            axe_id = id;
            start_time = gametime->get_time() + 0.05f;
            expire_time = gametime->get_time() + 1.2f;
        }
    };

    // Others
    //
    std::vector<axe> axes;

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 0);
        q->set_spell_lock(false);
        w = plugin_sdk->register_spell(spellslot::w, 0);
        e = plugin_sdk->register_spell(spellslot::e, 1100);
        e->set_skillshot(0.25f, 130.0f, 1400.0f, { }, skillshot_type::skillshot_line);
        r = plugin_sdk->register_spell(spellslot::r, 15000);
        r->set_skillshot(0.50f, 160.0f, 2000.0f, { }, skillshot_type::skillshot_line);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("draven", "Draven");
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
                    combo::q_max_active_axes = q_config->add_slider(myhero->get_model() + ".combo.q.max_active_axes", "Max Active Axes", 2, 1, 2);
                    combo::q_cast_to_keep_two_axes = q_config->add_slider(myhero->get_model() + ".combo.q.cast_to_keep_two_axes", "Cast to keep X axes if expiring (0 = disabled)", 2, 0, 2);
                }
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
                {
                    combo::w_max_range = w_config->add_slider(myhero->get_model() + ".combo.w.max_range", "W maximum range", 1100, 1, 1600);
                    combo::w_cast_while_chasing = w_config->add_checkbox(myhero->get_model() + ".combo.w.cast_while_chasing", "Cast W while chasing enemy", true);
                    combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".combo.w.target_above_range", "Cast if target is above range", 550, 0, 900);
                    combo::w_cast_in_fight = w_config->add_checkbox(myhero->get_model() + ".combo.w.cast_in_fight", "Cast W in fight", true);
                    combo::w_cast_before_catching_axe = w_config->add_checkbox(myhero->get_model() + ".combo.w.cast_before_catching_axe", "Cast W before catching axe", false);
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + ".combo.e.config", "E Config");
                {
                    combo::e_semi_manual_cast = e_config->add_hotkey(myhero->get_model() + ".combo.e.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'X', true);
                    combo::e_mode = e_config->add_combobox(myhero->get_model() + ".combo.e.mode", "E Mode", { {"In Combo", nullptr},{"After AA", nullptr } }, 1);
                    combo::e_max_range = e_config->add_slider(myhero->get_model() + ".combo.e.max_range", "Maximum E Range", 800.0f, 1, e->range());
                    combo::e_spell_interrupter = e_config->add_checkbox(myhero->get_model() + ".combo.e.spell_interrupter", "Auto E spell interrupter", true);
                }
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + ".combo.r.config", "R Config");
                {
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_min_distance = r_config->add_slider(myhero->get_model() + ".combo.r.min_distance", "Minimum distance to target", 750, 1, 1600);
                    combo::r_max_range = r_config->add_slider(myhero->get_model() + ".combo.r.max_range", "Maximum R Range", 2200.0f, 1200.0f, 5000.0f);

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
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", false);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", false);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "Minimum minions", 2, 0, 5);
                }
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

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to speed up", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E to slow enemies", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_w = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.w", "Use W", true);
                antigapclose::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                antigapclose::use_e = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.e", "Use E", true);
                antigapclose::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto catch_axes_settings = main_tab->add_tab(myhero->get_model() + ".misc", "Catch Axes Settings");
            {
                catch_axes_settings::catch_axes = catch_axes_settings->add_checkbox(myhero->get_model() + ".axe.catch_axes", "Catch Axes", true);
                catch_axes_settings::catch_axes->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                catch_axes_settings::catch_mode = catch_axes_settings->add_combobox(myhero->get_model() + ".axe.catch_mode", "Catch Axes Mode", { {"Near Myhero", nullptr},{"Near Mouse", nullptr } }, 1);
                catch_axes_settings::catch_only_if_orbwalker_active = catch_axes_settings->add_checkbox(myhero->get_model() + ".axe.catch_only_if_orbwalker_active", "Catch Axes only if Orbwalker active", true);
                catch_axes_settings::catch_axes_under_turret = catch_axes_settings->add_hotkey(myhero->get_model() + ".axe.catch_axes_under_turret", "Catch Axes under turret", TreeHotkeyMode::Toggle, 'A', true);
                catch_axes_settings::axe_maximum_distance = catch_axes_settings->add_slider(myhero->get_model() + ".axe.maximum_distance", "Maximum Distance to Axe", 1600, 1, 1600);
                catch_axes_settings::move_to_axe_if_distance_higher_than = catch_axes_settings->add_slider(myhero->get_model() + ".axe.move_to_axe_if_distance_higher_than", "Move to Axe if distance higher than", 70, 1, 120);
                catch_axes_settings::block_aa_if_distance_to_axe_smaller_than = catch_axes_settings->add_slider(myhero->get_model() + ".axe.block_aa_if_distance_to_axe_smaller_than", "Block AA if distance smaller than", 200, 1, 500);
                catch_axes_settings::block_aa_while_staying_in_axe = catch_axes_settings->add_checkbox(myhero->get_model() + ".axe.block_aa_while_staying_in_axe", "Block AA while staying in axe", false);
                catch_axes_settings::dont_catch_axes = catch_axes_settings->add_hotkey(myhero->get_model() + ".axe.dont_catch_axes.key", "Don't catch Axes Key", TreeHotkeyMode::Hold, 'Z', false);
                catch_axes_settings::dont_catch_axes_if_killable_by_x_aa = catch_axes_settings->add_slider(myhero->get_model() + ".axe.dont_catch_axes_if_killable_by_x_aa", "Don't catch Axes if target killable by x AA (0 = disabled)", 0, 0, 4);
             }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::e_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.e", "Hitchance E", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
                hitchance::r_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.r", "Hitchance R", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 3);
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
                draw_settings::draw_damage_r = draw_settings->add_checkbox(myhero->get_model() + "draw.r.damage", "Draw R Damage", true);
                draw_settings::draw_damage_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings->add_separator(myhero->get_model() + "draw.separator.1", "");
                draw_settings::draw_axe_radius = draw_settings->add_checkbox(myhero->get_model() + ".draw.axe_radius", "Draw Axe Radius", true);
                draw_settings::draw_axe_radius->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::draw_axe_move_radius = draw_settings->add_checkbox(myhero->get_model() + ".draw.axe_move_radius", "Draw Axe Move Radius", true);
                draw_settings::draw_axe_move_radius->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::draw_axe_number = draw_settings->add_checkbox(myhero->get_model() + ".draw.axe_number", "Draw Axe Number", true);
                draw_settings::draw_axe_number->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::draw_axe_expire_time = draw_settings->add_checkbox(myhero->get_model() + ".draw.axe_expire_time", "Draw Axe Expire Time", true);
                draw_settings::draw_axe_expire_time->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::draw_line_to_axe = draw_settings->add_checkbox(myhero->get_model() + ".draw.line_to_axe", "Draw Line To Axe", true);
                draw_settings::draw_line_to_axe->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings->add_separator(myhero->get_model() + "draw.separator.2", "");
                draw_settings::axe_radius_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.axe.radius_color", "Axe Radius Color", color);
                draw_settings::axe_move_radius_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.axe.move_radius_color", "Axe Move Radius Color", color);
                float color1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::axe_number_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.axe.number_color", "Axe Number Color", color1);
                draw_settings::axe_expire_time_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.axe.expire_time_color", "Axe Expire Time Color", color1);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Catch Under Turret", catch_axes_settings::catch_axes_under_turret);
            Permashow::Instance.AddElement("Don't Catch Axes", catch_axes_settings::dont_catch_axes);
            Permashow::Instance.AddElement("Semi Auto R", combo::r_semi_manual_cast);
            Permashow::Instance.AddElement("Semi Auto E", combo::e_semi_manual_cast);
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
        event_handler<events::on_create_object>::add_callback(on_create_object);
        event_handler<events::on_delete_object>::add_callback(on_delete_object);

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
        event_handler<events::on_create_object>::remove_handler(on_create_object);
        event_handler<events::on_delete_object>::remove_handler(on_delete_object);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        //console->print("[ShadowAIO] [DEBUG] Buff list:");
        //for (auto&& buff : myhero->get_bufflist())
        //{
        //    if (buff->is_valid() && buff->is_alive())
        //    {
        //        console->print("[ShadowAIO] [DEBUG] Buff name %s, count: %d", buff->get_name_cstr(), buff->get_count());
        //    }
        //}

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            if (e->is_ready() && combo::e_semi_manual_cast->get_bool())
            {
                e_logic_semi();
            }

            if (r->is_ready() && combo::r_semi_manual_cast->get_bool())
            {
                r_logic_semi();
            }

            if (catch_axes_settings::catch_axes->get_bool() && !catch_axes_settings::dont_catch_axes->get_bool() && ((!orbwalker->none_mode() && !orbwalker->flee_mode()) || !catch_axes_settings::catch_only_if_orbwalker_active->get_bool()))
            {
                int value = catch_axes_settings::dont_catch_axes_if_killable_by_x_aa->get_int();
                bool should_catch = !orbwalker->flee_mode();

                if (value != 0 && (orbwalker->combo_mode() || orbwalker->harass()))
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(myhero->get_attack_range() + 50, damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr && target->is_valid_target() && myhero->get_auto_attack_damage(target) * value > target->get_real_health())
                    {
                        should_catch = false;
                    }
                }

                if (should_catch)
                {
                    axes.erase(std::remove_if(axes.begin(), axes.end(), [](axe x)
                        {
                            return !x.object->is_valid() || x.object->is_dead() || gametime->get_time() > x.expire_time;
                        }), axes.end());

                    axes.erase(std::remove_if(axes.begin(), axes.end(), [](axe x)
                        {
                            return x.object->get_position().is_under_enemy_turret() && !catch_axes_settings::catch_axes_under_turret->get_bool();
                        }), axes.end());

                    axes.erase(std::remove_if(axes.begin(), axes.end(), [](axe x)
                        {
                            vector loc = catch_axes_settings::catch_mode->get_int() == 0 ? myhero->get_position() : hud->get_hud_input_logic()->get_game_cursor_position();
                            return loc.distance(x.object->get_position()) > catch_axes_settings::axe_maximum_distance->get_int();
                        }), axes.end());

                    std::sort(axes.begin(), axes.end(), [](axe a, axe b)
                        {
                            return a.object->get_position().distance(catch_axes_settings::catch_mode->get_int() == 0 ? myhero->get_position() : hud->get_hud_input_logic()->get_game_cursor_position()) < b.object->get_position().distance(myhero->get_position());
                        });

                    if (!axes.empty())
                    {
                        axe front = axes.front();

                        if (gametime->get_time() > front.start_time)
                        {
                            float distance_to_axe = myhero->get_distance(front.object);
                            if (distance_to_axe < catch_axes_settings::block_aa_if_distance_to_axe_smaller_than->get_int())
                            {
                                orbwalker->set_attack(false);
                            }
                            if (w->is_ready() && combo::use_w->get_bool() && combo::w_cast_before_catching_axe->get_bool() && orbwalker->combo_mode() && distance_to_axe < 175 && !myhero->has_buff(buff_hash("dravenfurybuff")))
                            {
                                // Get a target from a given range
                                auto target = target_selector->get_target(combo::w_max_range->get_int(), damage_type::physical);

                                // Always check an object is not a nullptr!
                                if (target != nullptr)
                                {
                                    w->cast();
                                }
                            }
                            if (distance_to_axe > catch_axes_settings::move_to_axe_if_distance_higher_than->get_int())
                            {
                                orbwalker->set_movement(false);
                                myhero->issue_order(front.object->get_position());
                                return;
                            }
                            else
                            {
                                int stacks = 0;
                                auto buff = myhero->get_buff(buff_hash("DravenSpinningAttack"));
                                if (buff != nullptr && buff->is_valid() && buff->is_alive())
                                {
                                    stacks = buff->get_count();
                                }
                                if (catch_axes_settings::block_aa_while_staying_in_axe->get_bool() && distance_to_axe < 125 && stacks == 0)
                                {
                                    orbwalker->set_attack(false);
                                }
                                else
                                {
                                    orbwalker->set_attack(true);
                                }
                                orbwalker->set_movement(true);
                            }
                        }
                    }
                    else
                    {
                        orbwalker->set_attack(true);
                        orbwalker->set_movement(true);
                    }
                }
                else
                {
                    orbwalker->set_attack(true);
                    orbwalker->set_movement(true);
                }
            }

            if (e->is_ready() && combo::e_spell_interrupter->get_bool())
            {
                for (auto& enemy : entitylist->get_enemy_heroes())
                {
                    if (enemy->is_valid() && enemy->is_valid_target(e->range()) && enemy->is_casting_interruptible_spell())
                    {
                        if (e->cast(enemy))
                        {
                            return;
                        }
                    }
                }
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

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (w->is_ready() && fleemode::use_w->get_bool())
                {
                    w->cast();
                }
                if (e->is_ready() && fleemode::use_e->get_bool())
                {
                    e_logic();
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
                    if (q->is_ready() && laneclear::use_q->get_bool() && get_draven_q_stacks() < combo::q_max_active_axes->get_int())
                    {
                        if (q->cast())
                            return;
                    }
                    if (w->is_ready() && laneclear::use_w->get_bool() && !myhero->has_buff(buff_hash("dravenfurybuff")))
                    {
                        if (w->cast())
                            return;
                    }
                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (e->cast_on_best_farm_position(laneclear::e_minimum_minions->get_int()))
                            return;
                    }
                }

                if (!monsters.empty())
                {
                    if (q->is_ready() && jungleclear::use_q->get_bool() && get_draven_q_stacks() < combo::q_max_active_axes->get_int())
                    {
                        if (q->cast())
                            return;
                    }
                    if (w->is_ready() && jungleclear::use_w->get_bool() && !myhero->has_buff(buff_hash("dravenfurybuff")))
                    {
                        if (w->cast())
                            return;
                    }
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
        int value = combo::q_cast_to_keep_two_axes->get_int();

        if (value != 0)
        {
            auto buff = myhero->get_buff(buff_hash("DravenSpinningAttack"));
            if (buff != nullptr && buff->is_valid() && buff->is_alive() && buff->get_count() >= value && buff->get_remaining_time() < 0.25)
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
        auto target = target_selector->get_target(combo::w_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && combo::w_cast_while_chasing->get_bool() && !myhero->has_buff(buff_hash("dravenfurybuff")) 
            && target->get_distance(myhero) > combo::w_target_above_range->get_int())
        {
            w->cast();
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        if (combo::e_mode->get_int() == 0 || orbwalker->flee_mode())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                e->cast(target, get_hitchance(hitchance::e_hitchance));
            }
        }
    }
#pragma endregion

#pragma region e_logic_semi
    void e_logic_semi()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::e_max_range->get_int(), damage_type::physical);

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
        if (myhero->has_buff(buff_hash("DravenRDoublecast")))
        {
            r->cast();
            return;
        }
        for (auto& enemy : entitylist->get_enemy_heroes())
        {
            if (can_use_r_on(enemy) && !utils::has_unkillable_buff(enemy) && !enemy->is_valid_target(combo::r_min_distance->get_int()) && enemy->is_valid_target(combo::r_max_range->get_int()))
            {
                if (r->get_damage(enemy) * 2.0f > enemy->get_real_health())
                {
                    if (r->cast(enemy, get_hitchance(hitchance::r_hitchance)))
                    {
                        return;
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic_semi
    void r_logic_semi()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::r_max_range->get_int(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && can_use_r_on(target))
        {
            r->cast(target, get_hitchance(hitchance::r_hitchance));
        }
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


    int get_draven_q_stacks()
    {
        auto buff = myhero->get_buff(buff_hash("DravenSpinningAttack"));
        if (buff != nullptr && buff->is_valid() && buff->is_alive())
        {
            return buff->get_count() + axes.size();
        }
        return 0;
    }

    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::e_max_range->get_int(), draw_settings::e_color->get_color());

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), combo::r_max_range->get_int(), draw_settings::r_color->get_color());

        // Draw R damage
        if (r->is_ready() && draw_settings::draw_damage_r->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    draw_dmg_rl(enemy, r->get_damage(enemy) * 2.0f, 0x8000ff00);
                }
            }
        }

        // Draw Axes
        if (draw_settings::draw_axe_radius->get_bool() && catch_axes_settings::catch_axes->get_bool())
        {
            for (axe axe : axes)
            {
                if (axe.object->is_valid() && !axe.object->is_dead())
                {
                    if (draw_settings::draw_axe_radius->get_bool())
                    {
                        draw_manager->add_circle(axe.object->get_position(), 125, draw_settings::axe_radius_color->get_color(), 2.0f);
                    }
                    if (draw_settings::draw_axe_move_radius->get_bool())
                    {
                        draw_manager->add_circle(axe.object->get_position(), catch_axes_settings::move_to_axe_if_distance_higher_than->get_int(), draw_settings::axe_move_radius_color->get_color());
                    }
                    if (draw_settings::draw_axe_number->get_bool())
                    {
                        auto pos = axe.object->get_position() + vector(-20, 40);
                        renderer->world_to_screen(pos, pos);
                        draw_manager->add_text_on_screen(pos, draw_settings::axe_number_color->get_color(), 64, "%d", axe.axe_id);
                    }
                    if (draw_settings::draw_axe_expire_time->get_bool())
                    {
                        auto pos = axe.object->get_position() + vector(-80, -80);
                        renderer->world_to_screen(pos, pos);
                        draw_manager->add_text_on_screen(pos, draw_settings::axe_expire_time_color->get_color(), 18, "Expire Time: [%.1fs]", axe.expire_time - gametime->get_time());
                    }
                    if (draw_settings::draw_line_to_axe->get_bool())
                    {
                        draw_manager->add_line(myhero->get_position(), axe.object->get_position(), draw_settings::axe_radius_color->get_color(), 1.5f);
                    }
                }
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_w->get_bool() && w->is_ready())
        {
            if (sender->is_valid_target(myhero->get_attack_range() + sender->get_bounding_radius()) && !myhero->has_buff(buff_hash("dravenfurybuff")))
            {
                w->cast();
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

    void on_before_attack(game_object_script target, bool* process)
    {
        if (q->is_ready())
        {
            // Using Q before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (get_draven_q_stacks() < combo::q_max_active_axes->get_int())
                {
                    q->cast();
                }
            }
        }
    }


    void on_after_attack(game_object_script target)
    {
        if (w->is_ready() && target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_w->get_bool()) || (orbwalker->harass() && harass::use_w->get_bool())) && combo::w_cast_in_fight->get_bool() && !myhero->has_buff(buff_hash("dravenfurybuff")))
        {
            if (target->get_distance(myhero) <= myhero->get_attack_range())
            {
                w->cast();
            }
        }
        if (e->is_ready() && combo::e_mode->get_int() == 1)
        {
            // Using E after autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_e->get_bool()) || (orbwalker->harass() && harass::use_e->get_bool())))
            {
                e->cast(target, get_hitchance(hitchance::e_hitchance));
            }
        }
    }

    void on_create_object(game_object_script sender)
    {
        if (sender->get_name().find("Q_reticle_self") != std::string::npos)
        {
            axes.push_back(axe(sender, axes.size() + 1));
        }
    }

    void on_delete_object(game_object_script sender)
    {
        if (sender->get_name().find("Q_reticle_self") != std::string::npos)
        {
            axes.erase(std::remove_if(axes.begin(), axes.end(), [sender](axe x)
                {
                    return x.object->get_network_id() == sender->get_network_id();
                }), axes.end());
        }
    }
};