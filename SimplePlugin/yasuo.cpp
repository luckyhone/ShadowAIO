#include "../plugin_sdk/plugin_sdk.hpp"
#include "yasuo.h"
#include "utils.h"
#include "permashow.hpp"

namespace yasuo
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* q3 = nullptr;
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
        TreeEntry* r_color = nullptr;

        namespace draw_damage_settings
        {
            TreeEntry* draw_damage = nullptr;
            TreeEntry* q_damage = nullptr;
            TreeEntry* e_damage = nullptr;
            TreeEntry* r_damage = nullptr;
            TreeEntry* aa_damage = nullptr;
        }
    }

    namespace combo
    {
        TreeEntry* allow_tower_dive = nullptr;
        TreeEntry* use_q1 = nullptr;
        TreeEntry* use_q2 = nullptr;
        TreeEntry* use_q3 = nullptr;
        TreeEntry* q_allow_exploit = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_on_targetted_spells = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_safety_checks = nullptr;
        TreeEntry* e_fast = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_use_if_killable_by_combo_and_x_aa = nullptr;
        TreeEntry* r_use_if_target_hp_below = nullptr;
        TreeEntry* r_use_if_hit_x_enemies = nullptr;
        TreeEntry* r_cast_if_x_percent_knockup_remaining = nullptr;
        TreeEntry* r_cast_if_x_time_knockup_remaining = nullptr;
        TreeEntry* r_allow_airblade = nullptr;
        TreeEntry* r_airblade_delay = nullptr;
        TreeEntry* r_force_use = nullptr;
        TreeEntry* r_force_eqr = nullptr;
        bool previous_evade_state = false;
    }

    namespace harass
    {
        TreeEntry* use_q1 = nullptr;
        TreeEntry* use_q2 = nullptr;
        TreeEntry* use_q3 = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q1 = nullptr;
        TreeEntry* use_q2 = nullptr;
        TreeEntry* use_q3 = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q1 = nullptr;
        TreeEntry* use_q2 = nullptr;
        TreeEntry* use_q3 = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_q;
        TreeEntry* use_e;
    }

    namespace misc
    {
        TreeEntry* show_mastery_after_kill;
    }

    namespace hitchance
    {
        TreeEntry* q1_hitchance = nullptr;
        TreeEntry* q2_hitchance = nullptr;
        TreeEntry* q3_hitchance = nullptr;
    }

    namespace debug
    {
        TreeEntry* debug_messages = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack_orbwalker(game_object_script target, bool* process);
    void on_after_attack_orbwalker(game_object_script target);
    void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
    void on_object_dead(game_object_script target);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void r_logic();

    // Champ utilities
    //
    vector get_e_dash_position(game_object_script target);
    void draw_dash_positions(std::vector<std::vector<game_object_script>> lists);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

    // Other
    //
    vector exploit_pos = vector(9999999, 9999999, 9999999);
    float last_eq_time;
    float last_exploit_time;
    float last_eq3_exploit_time;
    float last_e_time;
    game_object_script last_e_target;
    vector last_e_pos;

    struct eq
    {
        game_object_script target;
        bool should_exploit;
    };

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 450);
        q->set_skillshot(0.40f, 40.0f, FLT_MAX, { }, skillshot_type::skillshot_line); //todo cast time based on BONUS attack speed
        q3 = plugin_sdk->register_spell(spellslot::q, 1050);
        q3->set_skillshot(0.40f, 60.0f, 1200.0f, { }, skillshot_type::skillshot_line); //todo cast time based on BONUS attack speed
        w = plugin_sdk->register_spell(spellslot::w, 400);
        e = plugin_sdk->register_spell(spellslot::e, 475);
        r = plugin_sdk->register_spell(spellslot::r, 1400);

        // Disabling spell lock
        //
        q->set_spell_lock(false);
        q3->set_spell_lock(false);
        w->set_spell_lock(false);
        e->set_spell_lock(false);
        r->set_spell_lock(false);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("yasuo", "Yasuo");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::allow_tower_dive = combo->add_hotkey(myhero->get_model() + ".combo.allow_tower_dive", "Allow Tower Dive", TreeHotkeyMode::Toggle, 'A', true);

                auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "(Q) - Steel Tempest");
                {
                    q_config->add_separator(myhero->get_model() + ".combo.q.separator1", "Enabled Options");
                    combo::use_q1 = q_config->add_checkbox(myhero->get_model() + ".combo.q1", "Use Q", true);
                    combo::use_q1->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::use_q2 = q_config->add_checkbox(myhero->get_model() + ".combo.q2", "Use Q2", true);
                    combo::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::use_q3 = q_config->add_checkbox(myhero->get_model() + ".combo.q3", "Use Q3", true);
                    combo::use_q3->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());

                    q_config->add_separator(myhero->get_model() + ".combo.q.separator2", "Exploit Options");
                    combo::q_allow_exploit = q_config->add_checkbox(myhero->get_model() + ".combo.q.exploit", "Use EQ exploit", true);
                    combo::q_allow_exploit->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                }

                auto w_config = combo->add_tab(myhero->get_model() + "combo.w.config", "(W) - Wind Wall");
                {
                    w_config->add_separator(myhero->get_model() + ".combo.w.separator1", "Enabled Options");
                    combo::use_w = w_config->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                    combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());

                    w_config->add_separator(myhero->get_model() + ".combo.w.separator2", "Usage Options");
                    combo::w_on_targetted_spells = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_on_targetted_spells", "Use W on targetted spells (SoonTM)", true);
                    combo::w_on_targetted_spells->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                }

                auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "(E) - Sweeping Blade");
                {
                    e_config->add_separator(myhero->get_model() + ".combo.e.separator1", "Enabled Options");
                    combo::use_e = e_config->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                    combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

                    e_config->add_separator(myhero->get_model() + ".combo.e.separator2", "Usage Options");
                    combo::e_safety_checks = e_config->add_checkbox(myhero->get_model() + ".combo.e.safety_checks", "Use E safety checks", true);
                    combo::e_safety_checks->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    combo::e_fast = e_config->add_checkbox(myhero->get_model() + ".combo.e.fast", "Use E Fast", false);
                    combo::e_fast->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                }

                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "(R) - Last Breath");
                {
                    r_config->add_separator(myhero->get_model() + ".combo.r.separator1", "Enabled Options");
                    combo::use_r = r_config->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                    combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator2", "Usage Options");
                    combo::r_use_if_killable_by_combo_and_x_aa = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_killable", "Use R if target killable by combo + x AA", 2, 0, 3);
                    combo::r_use_if_target_hp_below = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_hp_below", "Use R if target HP below (in %)", 65, 0, 100);
                    combo::r_use_if_hit_x_enemies = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_hit_x_enemies", "Use R if hit x enemies", 2, 1, 5);

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator3", "Cast Options");
                    combo::r_cast_if_x_percent_knockup_remaining = r_config->add_slider(myhero->get_model() + ".combo.r.cast_if_percent_remaining", "Cast R if x% knockup remaining", 50, 15, 100);
                    combo::r_cast_if_x_time_knockup_remaining = r_config->add_slider(myhero->get_model() + ".combo.r.cast_if_time_remaining", "Cast R if x time (/100) knockup remaining", 15, 5, 100);

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator4", "Exploit Options");
                    combo::r_allow_airblade = r_config->add_checkbox(myhero->get_model() + ".combo.r.airblade", "Use Airblade if possible", true);
                    combo::r_allow_airblade->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_airblade_delay = r_config->add_slider(myhero->get_model() + ".combo.r.airblade_delay", "Airblade delay", 15, 0, 30);

                    r_config->add_separator(myhero->get_model() + ".combo.r.separator5", "Hotkey Options");
                    combo::r_force_use = r_config->add_hotkey(myhero->get_model() + ".combo.r.force_r", "Force Use R", TreeHotkeyMode::Toggle, 'G', true);
                    combo::r_force_eqr = r_config->add_hotkey(myhero->get_model() + ".combo.r.force_eqr", "Force Use EQR", TreeHotkeyMode::Toggle, 'O', false);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                auto q_config = harass->add_tab(myhero->get_model() + "harass.q.config", "(Q) - Steel Tempest");
                {
                    q_config->add_separator(myhero->get_model() + ".harass.q.separator1", "Enabled Options");
                    harass::use_q1 = q_config->add_checkbox(myhero->get_model() + ".harass.q1", "Use Q", true);
                    harass::use_q1->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    harass::use_q2 = q_config->add_checkbox(myhero->get_model() + ".harass.q2", "Use Q2", true);
                    harass::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    harass::use_q3 = q_config->add_checkbox(myhero->get_model() + ".harass.q3", "Use Q3", true);
                    harass::use_q3->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                }

                auto e_config = harass->add_tab(myhero->get_model() + "combo.e.config", "(E) - Sweeping Blade");
                {
                    e_config->add_separator(myhero->get_model() + ".combo.e.separator1", "Enabled Options");
                    harass::use_e = e_config->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                    harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                }
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q1 = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q1", "Use Q", true);
                laneclear::use_q1->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_q2 = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q2", "Use Q2", true);
                laneclear::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_q3 = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q3", "Use Q3", true);
                laneclear::use_q3->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q1 = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q1", "Use Q", true);
                jungleclear::use_q1->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_q2 = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q2", "Use Q2", true);
                jungleclear::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_q3 = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q3", "Use Q3", true);
                jungleclear::use_q3->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
            {
                fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "Use Q", true);
                fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Misc Settings");
            {
                misc::show_mastery_after_kill = misc->add_checkbox(myhero->get_model() + ".misc.show_mastery_after_kill", "Show mastery after kill", true);
                misc::show_mastery_after_kill->set_texture(myhero->get_passive_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q1_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q1", "Hitchance Q1", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
                hitchance::q2_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q2", "Hitchance Q2", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
                hitchance::q3_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q3", "Hitchance Q3", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);

                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", false);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);

                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);

                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);

                auto draw_damage = draw_settings->add_tab(myhero->get_model() + ".draw.damage", "Draw Damage");
                {
                    draw_settings::draw_damage_settings::draw_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.enabled", "Draw Combo Damage", true);
                    draw_settings::draw_damage_settings::q_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.q", "Draw Q Damage", true);
                    draw_settings::draw_damage_settings::q_damage->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    draw_settings::draw_damage_settings::e_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.e", "Draw E Damage", true);
                    draw_settings::draw_damage_settings::e_damage->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                    draw_settings::draw_damage_settings::r_damage = draw_damage->add_checkbox(myhero->get_model() + ".draw.damage.r", "Draw R Damage", true);
                    draw_settings::draw_damage_settings::r_damage->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    draw_settings::draw_damage_settings::aa_damage = draw_damage->add_slider(myhero->get_model() + ".draw.damage.aa", "Draw x AA Damage", 2, 0, 4);
                }
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Allow Tower Dive", combo::allow_tower_dive);
            Permashow::Instance.AddElement("Force Use R", combo::r_force_use);
            Permashow::Instance.AddElement("Force Use EQR", combo::r_force_eqr);
        }

        // Debug menu initialization
        //
        {
            auto debug_settings = main_tab->add_tab(myhero->get_model() + ".debug", "Debug Settings");
            {
                debug_settings->add_separator(myhero->get_model() + ".debug.separator1", "ShadowAIO : " + myhero->get_model());
                debug_settings->add_separator(myhero->get_model() + ".debug.separator2", "Test version - 10/1/2022");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator3", "");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator4", "Created by Kuezese");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator5", "Helped with plugin:");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator6", "Klee");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator7", "div");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator8", "KissMyMp5");
                debug_settings->add_separator(myhero->get_model() + ".debug.separator9", "");
                debug::debug_messages = debug_settings->add_checkbox(myhero->get_model() + ".debug.messages", "Debug Messages", true);
            }
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        //event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack_orbwalker);
        //event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
        event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);
        event_handler<events::on_object_dead>::add_callback(on_object_dead);

        // Chat message after load
        //
        utils::on_load();
    }

    void unload()
    {
        // Always remove all declared spells
        //
        plugin_sdk->remove_spell(q);
        plugin_sdk->remove_spell(q3);
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
        //event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack_orbwalker);
        //event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack_orbwalker);
        event_handler<events::on_process_spell_cast>::remove_handler(on_process_spell_cast);
        event_handler<events::on_object_dead>::remove_handler(on_object_dead);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (gametime->get_time() - last_eq_time < 0.65f)
        {
            orbwalker->set_attack(false);
        }
        else
        {
            orbwalker->set_attack(true);
        }

        if (q->is_ready())
        {
            float bonusAA = myhero->mPercentAttackSpeedMod() * 100.0f;
            float val = 0.f;
            //XD
            for (float f = 0.f; f < bonusAA; f += 1.67f)
            {
                val += 0.01f;
            }
            float delay = std::max(0.133f, 0.4f * (1 - val));
            q->set_delay(delay);
            q3->set_delay(delay);
            //myhero->print_chat(1, "Bonus AA speed: %.3f | Q Cast Time: %.3f", bonusAA, delay);
        }

        if (r->is_ready() && combo::use_r->get_bool() && !orbwalker->flee_mode())
        {
            r_logic();
        }

        //Checking if the user has combo_mode() (Default SPACE)
        if (orbwalker->combo_mode())
        {
            if (q->is_ready())
            {
                q_logic();
            }

            //if (w->is_ready() && combo::use_w->get_bool())
            //{
            //    w_logic();
            //} 

            if (e->is_ready() && combo::use_e->get_bool())
            {
                e_logic();
            }
        }

        //Checking if the user has selected harass() (Default C)
        if (orbwalker->harass())
        {
            if (q->is_ready())
            {
                q_logic();
            }

            if (e->is_ready() && harass::use_e->get_bool())
            {
                e_logic();
            }
        }

        // Checking if the user has selected flee_mode() (Default Z)
        if (orbwalker->flee_mode())
        {
            auto spell_hash = q->name_hash();
            bool is_q3 = spell_hash == spell_hash("YasuoQ3Wrapper");

            if (is_q3 && q->is_ready() && fleemode::use_q->get_bool())
            {
                // Get a target from a given range
                auto target = target_selector->get_target(q3->range(), damage_type::physical);

                // Always check an object is not a nullptr!
                if (target != nullptr)
                {
                    if (myhero->is_dashing() || gametime->get_time() - last_e_time < 0.55f)
                    {
                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Cant cast Q3 (Flee) to target because in E");
                        return;
                    }

                    if (myhero->is_dashing() && last_e_pos.is_valid() && last_e_pos.count_enemies_in_range(225) != 0)
                    {
                        if (q->cast())
                        {
                            if (debug::debug_messages->get_bool())
                                myhero->print_chat(1, "Q3 casted (detected enemy in last_e_pos in flee_mode())");
                        }
                        return;
                    }
                    
                    if (q3->cast(target, utils::get_hitchance(hitchance::q3_hitchance)))
                    {
                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Q3 casted (Flee)");
                        return;
                    }
                }
            }

            if (e->is_ready() && fleemode::use_e->get_bool())
            {
                // Gets enemy minions from the entitylist
                auto lane_minions = entitylist->get_enemy_minions();

                // You can use this function to delete minions that aren't in the specified range
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range());
                    }), lane_minions.end());

                // Delete minions with cooldown
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return x->has_buff(buff_hash("YasuoE"));
                    }), lane_minions.end());

                // Delete minions that myhero is not facing
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !myhero->is_facing(x);
                    }), lane_minions.end());

                // Delete minions under turret
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !combo::allow_tower_dive->get_bool() && get_e_dash_position(x).is_under_enemy_turret();
                    }), lane_minions.end());

                //std::sort -> sort lane minions by distance
                std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
                    {
                        auto pos = hud->get_hud_input_logic()->get_game_cursor_position();
                        //return a->get_position().distance(pos) < b->get_position().distance(pos);
                        auto pos_a = get_e_dash_position(a);    
                        auto pos_b = get_e_dash_position(b);
                        return pos_a.distance(pos) < pos_b.distance(pos);
                    });

                if (!lane_minions.empty())
                {
                    for (auto& minion : lane_minions)
                    {
                        if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(minion->get_position()))
                        {
                            if (e->cast(minion))
                            {
                                last_e_target = minion;
                                if (!is_q3 && q->is_ready() && fleemode::use_q->get_bool())
                                {
                                    scheduler->delay_action(0.15f, [] { q->cast(exploit_pos); });
                                }
                                return;
                            }
                        }
                        else if (debug::debug_messages->get_bool())
                        {
                            myhero->print_chat(1, "Dangerous E pos (%s)", minion->get_name_cstr());
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
                    return !x->is_valid_target(q3->range());
                }), lane_minions.end());

            // You can use this function to delete monsters that aren't in the specified range
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return !x->is_valid_target(q3->range());
                }), monsters.end());

            //std::sort -> sort lane minions by distance
            std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
                {
                    return a->get_position().distance(hud->get_hud_input_logic()->get_game_cursor_position()) < b->get_position().distance(hud->get_hud_input_logic()->get_game_cursor_position());
                });

            //std::sort -> sort monsters by max health
            std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
                {
                    return a->get_max_health() > b->get_max_health();
                });

            if (!lane_minions.empty())
            {
                // Ignore when exploit issued
                if (combo::q_allow_exploit->get_bool() && q->name_hash() != spell_hash("YasuoQ3Wrapper"))
                {
                    if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid() && !last_e_target->is_ai_hero())
                    {
                        myhero->print_chat(1, "666 (4)");
                        return;
                    }
                }

                if (q->is_ready() && e->is_ready())
                {
                    for (auto& minion : lane_minions)
                    {
                        if (myhero->get_distance(minion) < e->range() && !minion->has_buff(buff_hash("YasuoE")))
                        {
                            if (utils::count_minions_in_range(get_e_dash_position(minion), 225) > 2)
                            {
                                if (combo::allow_tower_dive->get_bool() || !get_e_dash_position(minion).is_under_enemy_turret())
                                {
                                    if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(minion->get_position()))
                                    {
                                        if (e->cast(minion))
                                        {
                                            last_e_target = minion;
                                            if (q->is_ready())
                                            {
                                                if (combo::q_allow_exploit->get_bool() && q->name_hash() == spell_hash("YasuoQ2Wrapper") && (combo::allow_tower_dive->get_bool() || !get_e_dash_position(minion).is_under_enemy_turret()))
                                                {
                                                    if (q->cast(exploit_pos))
                                                    {
                                                        last_exploit_time = gametime->get_time();
                                                        if (debug::debug_messages->get_bool())
                                                            myhero->print_chat(1, "EQ exploit casted (4)");
                                                        return;
                                                    }
                                                }
                                                else
                                                {
                                                    scheduler->delay_action(0.15f, [] { q->cast(); });
                                                }
                                            }
                                            return;
                                        }
                                    }
                                    else if (debug::debug_messages->get_bool())
                                    {
                                        myhero->print_chat(1, "Dangerous E pos (%s)", minion->get_name_cstr());
                                    }
                                }
                            }
                        }
                    }
                }

                if (q->is_ready())
                {
                    auto spell_hash = q->name_hash();

                    // Q1 logic
                    if (laneclear::use_q1->get_bool() && spell_hash == spell_hash("YasuoQ1Wrapper"))
                    {
                        q->cast_on_best_farm_position(1);
                        return;
                    }

                    // Q2 logic
                    if (laneclear::use_q2->get_bool() && spell_hash == spell_hash("YasuoQ2Wrapper"))
                    {
                        q->cast_on_best_farm_position(1);
                        return;
                    }

                    // Q3 logic
                    if (laneclear::use_q3->get_bool() && spell_hash == spell_hash("YasuoQ3Wrapper") && myhero->count_enemies_in_range(q3->range()) == 0)
                    {
                        q3->cast_on_best_farm_position(1);
                        return;
                    }
                }
            }

            if (!monsters.empty())
            {
                // Ignore when exploit issued
                if (combo::q_allow_exploit->get_bool() && q->name_hash() != spell_hash("YasuoQ3Wrapper"))
                {
                    if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid() && !last_e_target->is_ai_hero())
                    {
                        myhero->print_chat(1, "666 (5)");
                        return;
                    }
                }

                if (q->is_ready() && e->is_ready() && jungleclear::use_e->get_bool())
                {
                    for (auto& monster : monsters)
                    {
                        if (monster->is_valid_target(e->range()) && !monster->has_buff(buff_hash("YasuoE")))
                        {
                            if (utils::count_monsters_in_range(get_e_dash_position(monster), 225) > 0)
                            {
                                if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(monster->get_position()))
                                {
                                    if (e->cast(monster))
                                    {
                                        last_e_target = monster;
                                        if (q->is_ready())
                                        {
                                            if (combo::q_allow_exploit->get_bool() && q->name_hash() == spell_hash("YasuoQ2Wrapper") && (combo::allow_tower_dive->get_bool() || !get_e_dash_position(monster).is_under_enemy_turret()))
                                            {
                                                if (q->cast(exploit_pos))
                                                {
                                                    last_exploit_time = gametime->get_time();
                                                    if (debug::debug_messages->get_bool())
                                                        myhero->print_chat(1, "EQ exploit casted (5)");
                                                    return;
                                                }
                                            }
                                            else
                                            {
                                                scheduler->delay_action(0.15f, [] { q->cast(); });
                                            }
                                        }
                                        return;
                                    }
                                }
                                else if (debug::debug_messages->get_bool())
                                {
                                    myhero->print_chat(1, "Dangerous E pos (%s)", monster->get_name_cstr());
                                }
                            }
                        }
                    }
                }

                if (q->is_ready())
                {
                    auto spell_hash = q->name_hash();

                    // Q1 logic
                    if (jungleclear::use_q1->get_bool() && spell_hash == spell_hash("YasuoQ1Wrapper"))
                    {
                        q->cast_on_best_farm_position(1, true);
                        return;
                    }

                    // Q2 logic
                    if (jungleclear::use_q2->get_bool() && spell_hash == spell_hash("YasuoQ2Wrapper"))
                    {
                        q->cast_on_best_farm_position(1, true);
                        return;
                    }

                    // Q3 logic
                    if (jungleclear::use_q3->get_bool() && spell_hash == spell_hash("YasuoQ3Wrapper"))
                    {
                        q3->cast_on_best_farm_position(1, true);
                        return;
                    }
                }
            }
        }
    }

#pragma region q_logic
    void q_logic()
    {
        auto spell_hash = q->name_hash();

        // Ignore when exploit issued
        if (combo::q_allow_exploit->get_bool() && spell_hash != spell_hash("YasuoQ3Wrapper"))
        {
            if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid())
            {
                myhero->print_chat(1, "666 (1)");
                return;
            }
        }

        // Q1 logic
        if (combo::use_q1->get_bool() && spell_hash == spell_hash("YasuoQ1Wrapper"))
        {
            // Get a target from a given range
            auto target = target_selector->get_target(q->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (myhero->is_dashing() || gametime->get_time() - last_e_time < 0.55f)
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Cant cast Q1 (Combo) to target because in E");
                    return;
                }
                
                if (myhero->is_dashing() && last_e_pos.is_valid() && last_e_pos.count_enemies_in_range(225) != 0)
                {
                    if (q->cast())
                    {
                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Q1 casted (detected enemy in last_e_pos in q_logic())");
                    }
                    return;
                }

                if (q->cast(target, utils::get_hitchance(hitchance::q3_hitchance)))
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Q1 casted (Combo)");
                    return;
                }
            }
        }

        // Q2 logic
        if (combo::use_q2->get_bool() && spell_hash == spell_hash("YasuoQ2Wrapper"))
        {
            // Get a target from a given range
            auto target = target_selector->get_target(q->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (myhero->is_dashing() || gametime->get_time() - last_e_time < 0.55f)
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Cant cast Q2 (Combo) to target because in E");
                    return;
                }

                if (myhero->is_dashing() && last_e_pos.is_valid() && last_e_pos.count_enemies_in_range(225) != 0)
                {
                    if (q->cast())
                    {
                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Q2 casted (detected enemy in last_e_pos in q_logic())");
                    }
                    return;
                }

                if (combo::q_allow_exploit->get_bool())
                {
                    vector dash_pos = get_e_dash_position(target);

                    if (!target->has_buff(buff_hash("YasuoE")) && (combo::allow_tower_dive->get_bool() || !dash_pos.is_under_enemy_turret()) && dash_pos.count_enemies_in_range(225) > 0 && e->is_ready() && e->cast(target))
                    {
                        last_e_target = target;

                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "E casted (EQ Exploit)");

                        if (q->cast(exploit_pos))
                        {
                            last_exploit_time = gametime->get_time();
                            if (debug::debug_messages->get_bool())
                                myhero->print_chat(1, "EQ exploit casted (1)");
                            return;
                        }
                    }

                    //if ((myhero->is_dashing() || gametime->get_time() - last_e_time < 0.65f) && last_e_target == target)
                    //{
                    //    if (debug::debug_messages->get_bool())
                    //        myhero->print_chat(1, "Ignoring Q2 other logic, becasuse of exploit...");
                    //    return;
                    //}
                }

                if (q->cast(target, utils::get_hitchance(hitchance::q1_hitchance)))
                {
                    return;
                }
            }
        }

        // Q3 logic
        if (combo::use_q3->get_bool() && spell_hash == spell_hash("YasuoQ3Wrapper"))
        {
            // Get a target from a given range
            auto target = target_selector->get_target(q3->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (e->is_ready() && combo::use_e->get_bool() && target->is_valid_target(e->range()) && !target->has_buff(buff_hash("YasuoE")) && (combo::allow_tower_dive->get_bool() || !get_e_dash_position(target).is_under_enemy_turret()))
                {
                    auto pos = get_e_dash_position(target);
                    if (pos.count_enemies_in_range(225) > 0)
                    {
                        if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(target->get_position()))
                        {
                            if (e->cast(target))
                            {
                                last_e_target = target;

                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "E casted (EQ3)");
                                
                                if (combo::q_allow_exploit->get_bool())
                                {
                                    if (q->cast(exploit_pos))
                                    {
                                        last_eq3_exploit_time = gametime->get_time();
                                    }
                                }
                                else
                                {
                                    scheduler->delay_action(0.15f, [] { q->cast(); });
                                }

                                return;
                            }
                        }
                        else if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Dangerous E pos (%s)", target->get_model_cstr());
                    }
                }

                // Ignore when exploit issued
                if (combo::q_allow_exploit->get_bool())
                {
                    if (gametime->get_time() - last_eq3_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid())
                    {
                        myhero->print_chat(1, "666 (7)");
                        return;
                    }
                }

                if (myhero->is_dashing() || gametime->get_time() - last_e_time < 0.55f)
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Cant cast Q3 (Combo) to target because in E");
                    return;
                }

                if (myhero->is_dashing() && last_e_pos.is_valid() && last_e_pos.count_enemies_in_range(225) != 0)
                {
                    if (q->cast())
                    {
                        if (debug::debug_messages->get_bool())
                            myhero->print_chat(1, "Q3 casted (detected enemy in last_e_pos in q_logic())");
                    }
                    return;
                }

                if (combo::r_force_eqr->get_bool() && r->is_ready() && !target->has_buff(buff_hash("YasuoE")))
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Cant cast Q3 (Combo) to target because forced EQR");
                    return;
                }

                if (q3->cast(target, utils::get_hitchance(hitchance::q3_hitchance)))
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Q3 casted (Combo)");
                    return;
                }
            }
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        if (myhero->count_enemies_in_range(e->range()) > 2)
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid_target(e->range()) && !enemy->is_dead())
                {
                    if (combo::allow_tower_dive->get_bool() || !get_e_dash_position(enemy).is_under_enemy_turret())
                    {
                        if (!enemy->has_buff(buff_hash("YasuoE")))
                        {
                            if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(enemy->get_position()))
                            {
                                auto pos = get_e_dash_position(enemy);
                                if (pos.count_enemies_in_range(225) > 0)
                                {
                                    if (e->cast(enemy))
                                    {
                                        last_e_target = enemy;

                                        if (debug::debug_messages->get_bool())
                                            myhero->print_chat(1, "E casted (Enemies > 2)");

                                        if (q->is_ready())
                                        {
                                            vector dash_pos = get_e_dash_position(enemy);

                                            if (combo::q_allow_exploit->get_bool() && q->name_hash() == spell_hash("YasuoQ2Wrapper") && (combo::allow_tower_dive->get_bool() || !dash_pos.is_under_enemy_turret()) && dash_pos.count_enemies_in_range(225) > 0)
                                            {
                                                if (q->cast(exploit_pos))
                                                {
                                                    last_exploit_time = gametime->get_time();
                                                    if (debug::debug_messages->get_bool())
                                                        myhero->print_chat(1, "EQ exploit casted (2)");
                                                    return;
                                                }
                                            }
                                            else
                                            {
                                                // Ignore when exploit issued
                                                if (combo::q_allow_exploit->get_bool() && q->name_hash() != spell_hash("YasuoQ3Wrapper"))
                                                {
                                                    if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid() && last_e_target->is_ai_hero())
                                                    {
                                                        myhero->print_chat(1, "666 (2)");
                                                        return;
                                                    }
                                                }

                                                scheduler->delay_action(0.15f, [] { q->cast(); });
                                            }
                                        }
                                        return;
                                    }
                                }
                            }
                            else
                            {
                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "Dangerous E pos (%s)", enemy->get_model_cstr());
                            }
                        }
                    }
                }
            }
        }

        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::physical);

        if (target != nullptr && !target->is_valid_target(myhero->get_attack_range() + 150) && !target->has_buff(buff_hash("YasuoE")) && !target->is_facing(myhero) && myhero->is_facing(target))
        {
            if (combo::allow_tower_dive->get_bool() || !get_e_dash_position(target).is_under_enemy_turret())
            {
                if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(target->get_position()))
                {
                    auto pos = get_e_dash_position(target);
                    if (pos.count_enemies_in_range(225) > 0)
                    {
                        if (e->cast(target))
                        {
                            last_e_target = target;

                            if (debug::debug_messages->get_bool())
                                myhero->print_chat(1, "E casted (Gapclose)");

                            if (q->is_ready())
                            {
                                vector dash_pos = get_e_dash_position(target);

                                if (combo::q_allow_exploit->get_bool() && q->name_hash() == spell_hash("YasuoQ2Wrapper") && (combo::allow_tower_dive->get_bool() || !dash_pos.is_under_enemy_turret()) && dash_pos.count_enemies_in_range(225) > 0)
                                {
                                    if (q->cast(exploit_pos))
                                    {
                                        last_exploit_time = gametime->get_time();
                                        if (debug::debug_messages->get_bool())
                                            myhero->print_chat(1, "EQ exploit casted (3)");
                                        return;
                                    }
                                }
                                else
                                {
                                    // Ignore when exploit issued
                                    if (combo::q_allow_exploit->get_bool() && q->name_hash() != spell_hash("YasuoQ3Wrapper"))
                                    {
                                        if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid() && last_e_target->is_ai_hero())
                                        {
                                            myhero->print_chat(1, "666 (3)");
                                            return;
                                        }
                                    }

                                    scheduler->delay_action(0.15f, [] { q->cast(); });
                                }
                            }
                            return;
                        }
                    }
                }
                else
                {
                    if (debug::debug_messages->get_bool())
                        myhero->print_chat(1, "Dangerous E pos (%s)", target->get_model_cstr());
                }
            }
        }
        else if (orbwalker->combo_mode())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(1600, damage_type::physical);

            if (target != nullptr && !target->is_valid_target(e->range()))
            {
                auto spell_hash = q->name_hash();

                if (target->is_valid_target(725) && spell_hash == spell_hash("YasuoQ3Wrapper"))
                {
                    return;
                }

                // Gets enemy minions from the entitylist
                auto lane_minions = entitylist->get_enemy_minions();

                // You can use this function to delete minions that aren't in the specified range
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range());
                    }), lane_minions.end());

                // Delete minions with cooldown
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return x->has_buff(buff_hash("YasuoE"));
                    }), lane_minions.end());

                // Delete minions that myhero is not facing
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !myhero->is_facing(x);
                    }), lane_minions.end());

                // Delete minions under turret
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !combo::allow_tower_dive->get_bool() && get_e_dash_position(x).is_under_enemy_turret();
                    }), lane_minions.end());

                //std::sort -> sort lane minions by distance
                std::sort(lane_minions.begin(), lane_minions.end(), [target](game_object_script a, game_object_script b)
                    {
                        /*auto pos = hud->get_hud_input_logic()->get_game_cursor_position();
                        return a->get_position().distance(pos) < b->get_position().distance(pos);*/
                        auto pos_a = get_e_dash_position(a);
                        auto pos_b = get_e_dash_position(b);
                        return pos_a.distance(target) < pos_b.distance(target);
                    });

                if (!lane_minions.empty())
                {
                    for (auto& minion : lane_minions)
                    {
                        if (!combo::e_safety_checks->get_bool() || !evade->is_dangerous(minion->get_position()))
                        {
                            if (e->cast(minion))
                            {
                                last_e_target = minion;
                                if (q->is_ready())
                                {
                                    auto spell_hash = q->name_hash();
                                    if (spell_hash != spell_hash("YasuoQ3Wrapper"))
                                    {
                                        if (q->is_ready())
                                        {
                                            // Ignore when exploit issued
                                            if (combo::q_allow_exploit->get_bool() && q->name_hash() != spell_hash("YasuoQ3Wrapper"))
                                            {
                                                if (gametime->get_time() - last_exploit_time < 0.65f && last_e_target != nullptr && last_e_target->is_valid() && !last_e_target->is_ai_hero())
                                                {
                                                    myhero->print_chat(1, "666 (6)");
                                                    return;
                                                }
                                            }

                                            if (combo::q_allow_exploit->get_bool() && q->name_hash() == spell_hash("YasuoQ2Wrapper") && utils::count_minions_in_range(get_e_dash_position(minion), 225) > 0)
                                            {
                                                if (q->cast(exploit_pos))
                                                {
                                                    last_exploit_time = gametime->get_time();
                                                    if (debug::debug_messages->get_bool())
                                                        myhero->print_chat(1, "EQ exploit casted (6)");
                                                    return;
                                                }
                                            }
                                            else
                                            {
                                                scheduler->delay_action(0.15f, [] { q->cast(); });
                                            }
                                        }
                                    }
                                }
                                return;
                            }
                        }
                        else
                        {
                            if (debug::debug_messages->get_bool())
                                myhero->print_chat(1, "Dangerous E pos (%s)", minion->get_name_cstr());
                        }
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        auto enemies = entitylist->get_enemy_heroes();

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return !x->is_valid_target(r->range()) || x->is_dead() || x->is_zombie() || utils::has_unkillable_buff(x) || !x->has_buff_type({ buff_type::Knockup, buff_type::Knockback });
            }), enemies.end());

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return !combo::allow_tower_dive->get_bool() && x->is_under_ally_turret();
            }), enemies.end());

        if (!combo::r_force_use->get_bool() && enemies.size() < combo::r_use_if_hit_x_enemies->get_int())
        {
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
                {
                    bool is_killable_by_combo = utils::get_damage(x, { q, e, r }, combo::r_use_if_killable_by_combo_and_x_aa->get_int()) > x->get_real_health();
                    bool is_hp_below = x->get_health_percent() <= combo::r_use_if_target_hp_below->get_int();
                    return !is_killable_by_combo && !is_hp_below;
                }), enemies.end());
        }

        std::sort(enemies.begin(), enemies.end(), [](game_object_script a, game_object_script b)
            {
                return a->get_health() < b->get_health();
            });

        if (!enemies.empty())
        {
            auto& enemy = enemies.front();
            auto buff = enemy->get_buff_by_type({ buff_type::Knockup, buff_type::Knockback });

            if (buff && buff->is_valid() && buff->is_alive())
            {
                game_object_script eqr_target = nullptr;

                // Airblade Logic
                if (combo::r_allow_airblade->get_bool() && e->is_ready())
                {
                    //eq->r (heroes)
                    if (eqr_target == nullptr || !eqr_target->is_valid())
                    {
                        for (auto& other : entitylist->get_enemy_heroes())
                        {
                            if (other->is_valid_target(e->range()) && other->get_handle() != enemy->get_handle() && !other->has_buff(buff_hash("YasuoE")))
                            {
                                eqr_target = other;
                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "detected possible eqr (hero) | eq->r start");
                            }
                        }
                    }

                    //eq->r (minions)
                    if (eqr_target == nullptr || !eqr_target->is_valid())
                    {
                        for (auto& other : entitylist->get_enemy_minions())
                        {
                            if (other->is_valid_target(e->range()) && !other->has_buff(buff_hash("YasuoE")))
                            {
                                eqr_target = other;
                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "detected possible eqr (minion) | eq->r start");
                            }
                        }
                    }

                    //eq->r (monsters)
                    if (eqr_target == nullptr || !eqr_target->is_valid())
                    {
                        for (auto& other : entitylist->get_jugnle_mobs_minions())
                        {
                            if (other->is_valid_target(e->range()) && !other->has_buff(buff_hash("YasuoE")))
                            {
                                eqr_target = other;
                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "detected possible eqr (monster) | eq->r start");
                            }
                        }
                    }

                    //eq->r (same target)
                    if (eqr_target == nullptr || !eqr_target->is_valid())
                    {
                        if (enemy->is_valid_target(e->range()) && !enemy->has_buff(buff_hash("YasuoE")))
                        {
                            eqr_target = enemy;
                            if (debug::debug_messages->get_bool())
                                myhero->print_chat(1, "detected possible eqr (same target) | eq->r start");
                        }
                    }
                }

                float remaining_time = buff->get_remaining_time();

                //myhero->print_chat(1, "Start: [%f] End: [%f] Remaining: [%f] Percent: [%.2f] Q cooldown: [%.2f]", buff->get_start(), buff->get_end(), buff->get_remaining_time(), remaining_percent, q->handle()->cooldown());

                if (eqr_target != nullptr && eqr_target->is_valid())
                {
                    bool ignore = true;

                    if (!q->is_ready() && remaining_time < q->handle()->cooldown() - 0.57f)
                    {
                        ignore = false;
                        eqr_target = nullptr;
                    }

                    if (ignore)
                    {
                        if ((q->is_ready() || q->handle()->cooldown() <= 0.57f) && e->cast(eqr_target))
                        {
                            last_e_target = eqr_target;
                            if (q->cast())
                            {
                                //scheduler->delay_action(0.15f, [eqr_target, enemy] 
                                scheduler->delay_action(combo::r_airblade_delay->get_int() / 100.0f, [eqr_target, enemy] {
                                    r->cast(enemy);
                                    if (debug::debug_messages->get_bool())
                                        myhero->print_chat(1, "eq->r casted (%s->%s)", eqr_target->get_name_cstr(), enemy->get_model_cstr());
                                });

                                if (debug::debug_messages->get_bool())
                                    myhero->print_chat(1, "eq->r scheduled (%s->%s)", eqr_target->get_name_cstr(), enemy->get_model_cstr());
                            }
                        }
                        else if (debug::debug_messages->get_bool())
                        {
                            myhero->print_chat(1, "eqr is possible (%s->%s), waiting for ready... ignoring buff check", eqr_target->get_name_cstr(), enemy->get_model_cstr());
                        }
   
                        return;
                    }
                }
                 
                if (debug::debug_messages->get_bool())
                    myhero->print_chat(1, "eqr not possible, checking buff");

                float max_time = buff->get_end() - buff->get_start();
                float remaining_percent = remaining_time / max_time;
                //myhero->print_chat(1, "Buff on %s [%s] Max: %.2f | Remaining: %.2f | Percent: %.2f", enemy->get_model_cstr(), buff->get_name_cstr(), max_time, remaining_time, remaining_percent);

                if (remaining_percent <= combo::r_cast_if_x_percent_knockup_remaining->get_int() / 100.0f || remaining_time <= combo::r_cast_if_x_time_knockup_remaining->get_int() / 100.0f)
                {
                    if (r->cast(enemy))
                    {
                        return;
                    }
                }
            }
        }
    }
#pragma endregion

    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        auto spell_hash = q->name_hash();

        if (spell_hash != spell_hash("YasuoQ3Wrapper"))
        {
            // Draw Q12 range
            if (draw_settings::draw_range_q->get_bool())
            {
                if (q->is_ready())
                {
                    draw_manager->add_circle(myhero->get_position(), q->range(), MAKE_COLOR(0, 155, 249, 255));
                }
                else
                {
                    auto range = (q->range() / q->handle()->get_total_cooldown()) * (q->handle()->get_total_cooldown() - q->handle()->cooldown());
                    draw_manager->add_circle(myhero->get_position(), range, MAKE_COLOR(0, 155, 249, 100));
                }
            }
        }
        else
        {
            // Draw Q3 range
            if (draw_settings::draw_range_q->get_bool())
            {
                if (q3->is_ready())
                {
                    draw_manager->add_circle(myhero->get_position(), q3->range(), MAKE_COLOR(0, 199, 249, 255));
                }
                else
                {
                    auto range = (q3->range() / q3->handle()->get_total_cooldown()) * (q3->handle()->get_total_cooldown() - q3->handle()->cooldown());
                    draw_manager->add_circle(myhero->get_position(), range, MAKE_COLOR(0, 155, 249, 100));
                }
            }
        }


        // Draw W range
        if (draw_settings::draw_range_w->get_bool())
        {
            if (w->is_ready())
            {
                draw_manager->add_circle(myhero->get_position(), w->range(), MAKE_COLOR(0, 199, 249, 255));
            }
            else
            {
                auto range = (w->range() / w->handle()->get_total_cooldown()) * (w->handle()->get_total_cooldown() - w->handle()->cooldown());
                draw_manager->add_circle(myhero->get_position(), range, MAKE_COLOR(0, 155, 249, 100));
            }
        }

        // Draw E range
        if (draw_settings::draw_range_e->get_bool())
        {
            if (e->is_ready())
            {
                draw_manager->add_circle(myhero->get_position(), e->range(), MAKE_COLOR(0, 155, 249, 255));
            }
            else
            {
                auto range = (e->range() / e->handle()->get_total_cooldown()) * (e->handle()->get_total_cooldown() - e->handle()->cooldown());
                draw_manager->add_circle(myhero->get_position(), range, MAKE_COLOR(0, 155, 249, 100));
            }
        }

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), MAKE_COLOR(0, 155, 249, 255));

        // Draw E dash positions
        if (e->is_ready())
        {
            draw_dash_positions( { entitylist->get_enemy_heroes(), entitylist->get_enemy_minions(), entitylist->get_jugnle_mobs_minions() } );
        }

        // Draw last E dash position
        if (last_e_pos.is_valid())
        {
            if (gametime->get_time() - last_e_time < 0.55f || gametime->get_time() - last_eq_time < 0.65f)
            {
                draw_manager->add_circle(last_e_pos, 225.0f, MAKE_COLOR(137, 207, 240, 255));
                {
                    auto pos = last_e_pos - vector(60, -30);
                    renderer->world_to_screen(pos, pos);
                    draw_manager->add_text_on_screen(pos, MAKE_COLOR(137, 207, 240, 255), 18, "Last E pos");
                }
                {
                    auto pos = last_e_pos - vector(85, 30);
                    renderer->world_to_screen(pos, pos);
                    draw_manager->add_text_on_screen(pos, MAKE_COLOR(137, 207, 240, 255), 18, "Enemies EQ [%d]", last_e_pos.count_enemies_in_range(225));
                }
            }
            else
            {
                last_e_pos = vector::zero;
            }
        }

        // Draw damage
        if (draw_settings::draw_damage_settings::draw_damage->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead())
                {
                    float damage = 0.0f;

                    if (q->is_ready() && draw_settings::draw_damage_settings::q_damage->get_bool())
                        damage += q->get_damage(enemy);

                    if (q->is_ready() && draw_settings::draw_damage_settings::q_damage->get_bool())
                        damage += e->get_damage(enemy);

                    if (r->is_ready() && draw_settings::draw_damage_settings::r_damage->get_bool())
                        damage += r->get_damage(enemy);

                    for (int i = 0; i < draw_settings::draw_damage_settings::aa_damage->get_int(); i++)
                        damage += myhero->get_auto_attack_damage(enemy);

                    if (damage != 0.0f)
                        draw_dmg_rl(enemy, damage, MAKE_COLOR(0, 155, 249, 149));
                }
            }
        }
    }

    void on_before_attack_orbwalker(game_object_script target, bool* process)
    {
    }

    void on_after_attack_orbwalker(game_object_script target)
    {
    }

    void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
    {
        if (sender->is_me())
        {
            auto hash = spell->get_spell_data()->get_name_hash();
            if (hash == spell_hash("YasuoEDash"))
            {
                last_e_time = gametime->get_time();
                last_e_pos = utils::to_2d(spell->get_end_position());
                if (combo::e_fast->get_bool())
                {
                    myhero->send_emote(emote_type::EMOTE_DANCE);
                    myhero->print_chat(1, "Fast E");
                }
            }
            else if (hash == spell_hash("YasuoQE1") || hash == spell_hash("YasuoQE2") || hash == spell_hash("YasuoQE3"))
            {
                last_eq_time = gametime->get_time();
            }
        }
    }

    void on_object_dead(game_object_script target)
    {
        if (misc::show_mastery_after_kill->get_bool() && target->is_valid() && target->is_enemy() && target->is_ai_hero() && myhero->get_distance(target->get_position()) < r->range())
        {
            myhero->display_champ_mastery_badge();
        }
    }

    vector get_e_dash_position(game_object_script target)
    {
        return utils::to_2d(myhero->get_position().extend(target->get_position(), myhero->get_distance(target) + 475 - myhero->get_distance(target)));
    }

    void draw_dash_positions(std::vector<std::vector<game_object_script>> lists)
    {
        for (auto& list : lists)
        {
            for (auto& enemy : list)
            {
                if (enemy->is_valid() && enemy->is_valid_target(e->range()) && !enemy->is_dead() && !enemy->has_buff(buff_hash("YasuoE")))
                {
                    auto pos = get_e_dash_position(enemy);
                    auto color = pos.is_under_enemy_turret() ? MAKE_COLOR(255, 69, 0, 255) : MAKE_COLOR(124, 255, 0, 255);
                    draw_manager->add_line(enemy->get_position(), pos, color, 0.5f);
                    draw_manager->add_circle(pos, 40.0f, color);
                }
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
};