#include "../plugin_sdk/plugin_sdk.hpp"
#include "twitch.h"
#include "permashow.hpp"

namespace twitch
{

    // To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;
    script_spell* b = nullptr;

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
        TreeEntry* draw_damage_e = nullptr;
        TreeEntry* draw_q_timeleft = nullptr;
        TreeEntry* draw_r_timeleft = nullptr;
        TreeEntry* draw_e_stacks = nullptr;
        TreeEntry* q_timeleft_color = nullptr;
        TreeEntry* r_timeleft_color = nullptr;
        TreeEntry* e_stacks_color = nullptr;
        TreeEntry* draw_e_stacks_time = nullptr;
        TreeEntry* e_stacks_time_color = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_dont_use_on_q = nullptr;
        TreeEntry* w_dont_use_on_r = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_if_target_leaving_range = nullptr;
        TreeEntry* e_leaving_range_minimum_stacks = nullptr;
        TreeEntry* e_use_before_death = nullptr;
        TreeEntry* e_before_death_use_on_x_stacks = nullptr;
        TreeEntry* e_before_death_myhero_under_hp = nullptr;
        TreeEntry* e_before_death_calculate_incoming_damage = nullptr;
        TreeEntry* e_before_death_damage_time = nullptr;
        TreeEntry* e_before_death_over_my_hp_in_percent = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_use_if_enemies_more_than = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_use_on_x_stacks = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* use_q_on_turret = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_minimum_minions = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* e_use_if_killable_minions = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
    }

    namespace antigapclose
    {
        TreeEntry* use_w = nullptr;
    }

    namespace misc
    {
        TreeEntry* stealth_recall = nullptr;
        TreeEntry* stealth_recall_key = nullptr;
    }

    namespace hitchance
    {
        TreeEntry* w_hitchance = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_after_attack_orbwalker(game_object_script target);
    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args);

    // Declaring functions responsible for spell-logic
    //
    void w_logic();
    void e_logic();
    void r_logic();


    // Utils
    //
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);
    int get_twitch_e_stacks(game_object_script target);

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 500);
        q->set_spell_lock(false);
        w = plugin_sdk->register_spell(spellslot::w, 950);
        w->set_skillshot(0.25f, 200.f, 1400.0f, { }, skillshot_type::skillshot_circle);
        e = plugin_sdk->register_spell(spellslot::e, 1200);
        r = plugin_sdk->register_spell(spellslot::r, 1100);
        b = plugin_sdk->register_spell(spellslot::recall, 0);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("twitch", "Twitch");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            // Info
            //
            main_tab->add_separator(myhero->get_model() + ".aio", "ShadowAIO : " + myhero->get_model());

            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q after AA", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + "combo.w.config", "W Config");
                {
                    combo::w_dont_use_on_q = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_on_q", "Dont use W on Q", true);
                    combo::w_dont_use_on_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                    combo::w_dont_use_on_r = w_config->add_checkbox(myhero->get_model() + ".combo.w.dont_use_on_r", "Dont use W on R", true);
                    combo::w_dont_use_on_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E on Killable", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
                {
                    combo::e_if_target_leaving_range = e_config->add_checkbox(myhero->get_model() + ".combo.e.target_leaving_range", "Use if target leaving E range", true);
                    combo::e_leaving_range_minimum_stacks = e_config->add_slider(myhero->get_model() + ".combo.e.leaving_range_minimum_stacks", "Enemy leaving range use on x stacks", 6, 1, 6);
                	combo::e_use_before_death = e_config->add_checkbox(myhero->get_model() + ".combo.e.use_before_death", "Use before death", true);
                    auto before_death_config = e_config->add_tab(myhero->get_model() + "combo.e.before_death.config", "Use before death Config");
                    {
                        combo::e_before_death_use_on_x_stacks = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_use_on_x_stacks", "Use on x stacks", 6, 1, 6);
                        combo::e_before_death_myhero_under_hp = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_myhero_under_hp", "Myhero HP is under (in %)", 10, 0, 100);
                        combo::e_before_death_calculate_incoming_damage = before_death_config->add_checkbox(myhero->get_model() + ".combo.e.before_death_calculate_incoming_damage", "Calculate incoming damage", true);
                        combo::e_before_death_damage_time = before_death_config->add_slider(myhero->get_model() + ".combo.e.before_death_damage_time", "Incoming damage time (in ms)", 600, 0, 1000);
                        combo::e_before_death_over_my_hp_in_percent = before_death_config->add_slider(myhero->get_model() + ".combo.w.before_death_over_my_hp_in_percent", "Coming damage is over my HP (in %)", 90, 0, 100);
                    }
                }
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_use_if_enemies_more_than = r_config->add_slider(myhero->get_model() + ".combo.r.use_if_enemies_more_than", "Use if enemies more than", 3, 0, 5);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", false);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = harass->add_tab(myhero->get_model() + ".harass.e.config", "E Config");
                {
                    harass::e_use_on_x_stacks = e_config->add_slider(myhero->get_model() + ".harass.e.use_on_x_stacks", "Use on x stacks", 6, 1, 6);
                }
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", false);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_q_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q.on_turret", "Use Q On Turret", false);
                laneclear::use_q_on_turret->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_w = laneclear->add_checkbox(myhero->get_model() + ".laneclear.w", "Use W", true);
                laneclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = laneclear->add_tab(myhero->get_model() + ".laneclear.w.config", "W Config");
                {
                    laneclear::w_minimum_minions = w_config->add_slider(myhero->get_model() + ".laneclear.w.minimum_minions", "Minimum minions", 3, 0, 5);
                }
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                    laneclear::e_use_if_killable_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.use_if_killable_minions", "Use only when killable minions more than", 3, 0, 5);
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
                fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".flee.q", "I WAS HIDING HAHAHAHA", true);
                fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W to slow enemies", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto antigapclose = main_tab->add_tab(myhero->get_model() + ".antigapclose", "Anti Gapclose");
            {
                antigapclose::use_w = antigapclose->add_checkbox(myhero->get_model() + ".antigapclose.w", "Use W", true);
                antigapclose::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Miscellaneous Settings");
            {
                misc::stealth_recall = misc->add_checkbox(myhero->get_model() + ".misc.stealth_recall", "Stealth Recall", true);
                misc::stealth_recall->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                misc::stealth_recall_key = misc->add_hotkey(myhero->get_model() + ".misc.stealth_recall.key", "Stealth Recall Key", TreeHotkeyMode::Hold, 'S', true);
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::w_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.w", "Hitchance W", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
            {
                float color[] = { 0.0f, 1.0f, 1.0f, 1.0f };

                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);

                draw_settings->add_separator(myhero->get_model() + ".draw.separator1", "");
                draw_settings::draw_e_stacks = draw_settings->add_checkbox(myhero->get_model() + ".draw.e.stacks", "Draw E Stacks", true);
                draw_settings::draw_e_stacks->set_texture(myhero->get_passive_icon_texture());
                draw_settings::draw_damage_e = draw_settings->add_checkbox(myhero->get_model() + "draw.e.damage", "Draw E Damage", true);
                draw_settings::draw_damage_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

                draw_settings->add_separator(myhero->get_model() + ".draw.separator2", "");
                draw_settings::draw_q_timeleft = draw_settings->add_checkbox(myhero->get_model() + "draw.q.time", "Draw Q Time Left", true);
                draw_settings::draw_q_timeleft->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());

                draw_settings::draw_r_timeleft = draw_settings->add_checkbox(myhero->get_model() + "draw.r.time", "Draw R Time Left", true);
                draw_settings::draw_r_timeleft->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                draw_settings::draw_e_stacks_time = draw_settings->add_checkbox(myhero->get_model() + ".draw.e.stacks.time", "Draw E Stacks Time Left", true);
                draw_settings::draw_e_stacks_time->set_texture(myhero->get_passive_icon_texture());


                draw_settings->add_separator(myhero->get_model() + ".draw.separator3", "");
                float color1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                draw_settings::q_timeleft_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.time.color", "Q Time Left Color", color1);
                draw_settings::r_timeleft_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.time.color", "R Time Left Color", color1);
                draw_settings::e_stacks_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.stacks.color", "E Stacks Color", color1);
                draw_settings::e_stacks_time_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.stacks.time.color", "E Stacks Time Left Color", color1);
            }
        }

        // Permashow initialization
        //
        {
	        Permashow::Instance.Init(main_tab);
	        Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
	        Permashow::Instance.AddElement("Stealth Recall", misc::stealth_recall_key);
        }

        // Add anti gapcloser handler
        //
        antigapcloser::add_event_handler(on_gapcloser);

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
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
        event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack_orbwalker);
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
            if (q->is_ready() && b->is_ready() && misc::stealth_recall->get_bool() && misc::stealth_recall_key->get_bool())
            {
                if (q->cast() && b->cast())
                {
                    return;
                }
            }

            if (e->is_ready())
            {
                e_logic();
            }

            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (w->is_ready() && combo::use_w->get_bool())
                {
                    w_logic();
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
                    if (w->is_ready() && harass::use_w->get_bool())
                    {
                        w_logic();
                    }
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (q->is_ready() && fleemode::use_q->get_bool())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
                if (w->is_ready() && fleemode::use_w->get_bool())
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
                    if (w->is_ready() && laneclear::use_w->get_bool())
                    {
                        if (w->cast_on_best_farm_position(laneclear::w_minimum_minions->get_int()))
                        {
                            return;
                        }
                    }
                }

                if (!monsters.empty())
                {
                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->get_damage(monsters.front()) > monsters.front()->get_health())
                        {
                            if (e->cast())
                            {
                                return;
                            }
                        }
                    }
                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->cast_on_best_farm_position(1, true))
                        {
                            return;
                        }
                    }
                }
            }
        }
    }   

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(w->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if ((!combo::w_dont_use_on_q->get_bool() || !myhero->has_buff(buff_hash("TwitchHideInShadows"))) && (!combo::w_dont_use_on_r->get_bool() || !myhero->has_buff(buff_hash("TwitchFullAutomatic"))))
            {
                w->cast(target, get_hitchance(hitchance::w_hitchance));
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

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return x->has_buff({ buff_hash("UndyingRage"), buff_hash("ChronoShift"), buff_hash("KayleR"), buff_hash("KindredRNoDeathBuff") });
            }), enemies.end());

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](game_object_script x)
            {
                return x->is_zombie();
            }), enemies.end());

        if ((orbwalker->harass() || orbwalker->lane_clear_mode()) && harass::use_e->get_bool())
        {
            for (auto& enemy : enemies)
            {
                if (get_twitch_e_stacks(enemy) >= harass::e_use_on_x_stacks->get_int())
                {
                    if (e->cast())
                    {
                        return;
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
                            myhero->get_max_health() > myhero->get_health_percent() * (combo::e_before_death_over_my_hp_in_percent->get_int() / 100.f))) && get_twitch_e_stacks(enemy) >= combo::e_before_death_use_on_x_stacks->get_int())
                {
                    e->cast();
                }

                else if (combo::e_if_target_leaving_range->get_bool())
                {
                    if (get_twitch_e_stacks(enemy) >= combo::e_leaving_range_minimum_stacks->get_int() && myhero->count_enemies_in_range(e->range() - 50) == 0)
                    {
	                    e->cast();
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        auto enemies = myhero->count_enemies_in_range(r->range());

        if (enemies >= combo::r_use_if_enemies_more_than->get_int())
        {
            r->cast();
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

        if (draw_settings::draw_q_timeleft->get_bool())
        {
            auto q_buff = myhero->get_buff(buff_hash("TwitchHideInShadows"));

            if (q_buff != nullptr && q_buff->is_valid() && q_buff->is_alive())
            {
                auto pos = myhero->get_position() + vector(100, 100);
                renderer->world_to_screen(pos, pos);
                draw_manager->add_text_on_screen(pos, draw_settings::q_timeleft_color->get_color(), 18, "Q Time: [%.1fs]", q_buff->get_remaining_time());
            }
        }

        if (draw_settings::draw_r_timeleft->get_bool())
        {
            auto r_buff = myhero->get_buff(buff_hash("TwitchFullAutomatic"));

            if (r_buff != nullptr && r_buff->is_valid() && r_buff->is_alive())
            {
                auto pos = myhero->get_position() + vector(100, 70);
                renderer->world_to_screen(pos, pos);
                draw_manager->add_text_on_screen(pos, draw_settings::r_timeleft_color->get_color(), 18, "R Time: [%.1fs]", r_buff->get_remaining_time());
            }
        }

        if (draw_settings::draw_e_stacks->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    int stacks = get_twitch_e_stacks(enemy);

                    if (stacks != 0)
                    {
                        auto pos = enemy->get_position() + vector(-60, 20);
                        renderer->world_to_screen(pos, pos);

                        if (draw_settings::draw_e_stacks_time->get_bool())
                        {
                            draw_manager->add_text_on_screen(pos, draw_settings::e_stacks_color->get_color(), 18, "Stacks: %d [%.1fs]", stacks, enemy->get_buff_time_left(buff_hash("TwitchDeadlyVenom")));
                        }
                        else
                        {
                            draw_manager->add_text_on_screen(pos, draw_settings::e_stacks_color->get_color(), 18, "Stacks: %d", stacks);
                        }
                    }
                }
            }
        }
    }

    void on_after_attack_orbwalker(game_object_script target)
    {
        if (target->is_ai_hero() && (orbwalker->combo_mode() && combo::use_q->get_bool() || orbwalker->harass() && harass::use_q->get_bool()))
        {
            if (q->cast())
            {
                return;
            }
        }

        // Use q after autoattack on lane minions
        if (target->is_minion() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && laneclear::use_q->get_bool())) {
            if (q->cast())
            {
                return;
            }
        }

        // Use q after autoattack on monsters
        if (target->is_monster() && (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool())) {
            if (q->cast())
            {
                return;
            }
        }

        // Use q after autoattack on turrets
        if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_q_on_turret->get_bool() && target->is_ai_turret())
        {
            if (q->cast())
            {
                return;
            }
        }
    }

    void on_gapcloser(game_object_script sender, antigapcloser::antigapcloser_args* args)
    {
        if (antigapclose::use_w->get_bool() && w->is_ready())
        {
            if (sender->is_valid_target(w->range() + sender->get_bounding_radius()))
            {
                w->cast(sender, get_hitchance(hitchance::w_hitchance));
            }
        }
    }

    int get_twitch_e_stacks(game_object_script target)
    {
        if (target->is_valid())
        {
            auto buff = target->get_buff(buff_hash("TwitchDeadlyVenom"));
            if (buff != nullptr && buff->is_valid() && buff->is_alive())
            {
                return buff->get_count();
            }
        }
        return 0;
    }
};