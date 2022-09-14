#include "../plugin_sdk/plugin_sdk.hpp"
#include "viego.h"
#include "utils.h"
#include "permashow.hpp"

namespace viego
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
        TreeEntry* r_color = nullptr;
        TreeEntry* draw_damage_r = nullptr;
    }

    namespace combo
    {
        TreeEntry* allow_tower_dive = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* q_mode = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_semi_manual_cast = nullptr;
        TreeEntry* r_save = nullptr;
        TreeEntry* r_include_aa_in_damage_calculation = nullptr;
        TreeEntry* r_use_before_expire = nullptr;
        TreeEntry* r_use_if_all_spells_cooldown = nullptr;
        std::map<std::uint32_t, TreeEntry*> r_use_on;
        TreeEntry* auto_catch_soul = nullptr;
        TreeEntry* force_catch_soul = nullptr;
        TreeEntry* simple_spell_usage_on_soul = nullptr;
        TreeEntry* max_soul_distance = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* spell_farm = nullptr;
        TreeEntry* use_q = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_w;
        TreeEntry* use_e;
    }

    namespace hitchance
    {
        TreeEntry* q_hitchance = nullptr;
        TreeEntry* w_hitchance = nullptr;
        TreeEntry* r_hitchance = nullptr;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_after_attack(game_object_script target);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    void r_logic();
    void r_logic_semi();

    // Utils
    //
    bool can_use_r_on(game_object_script target);
    hit_chance get_hitchance(TreeEntry* entry);
    inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);
    int get_viego_r_damage(game_object_script target);

    // Champion data
    //
    float total_ad;
    float additional_ad;
    float crit;

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 625);
        q->set_skillshot(0.35f, 125.0f, FLT_MAX, { collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line); //cast time is based on attack speed (140% of viego attack windup)
        w = plugin_sdk->register_spell(spellslot::w, 900);
        w->set_skillshot(0.0f, 80.0f, 1300.0f, { collisionable_objects::heroes, collisionable_objects::minions, collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
        w->set_charged(500.0f, 900.0f, 1.0f);
        e = plugin_sdk->register_spell(spellslot::e, 775);
        r = plugin_sdk->register_spell(spellslot::r, 500);
        r->set_skillshot(0.50f, 150.0f, FLT_MAX, { }, skillshot_type::skillshot_circle);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("viego", "Viego");
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

                auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "Q Config");
                {
                    combo::q_mode = q_config->add_combobox(myhero->get_model() + ".combo.q.mode", "Q Mode", { {"If enemy above AA range or After AA", nullptr}, {"In Combo", nullptr}, {"After AA", nullptr } }, 0);
                }

                combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R on killable", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

                auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
                {
                    combo::r_semi_manual_cast = r_config->add_hotkey(myhero->get_model() + ".combo.r.semi_manual_cast", "Semi manual cast", TreeHotkeyMode::Hold, 'T', true);
                    combo::r_save = r_config->add_hotkey(myhero->get_model() + ".combo.r.save", "Save R", TreeHotkeyMode::Toggle, 'G', false);
                    combo::r_include_aa_in_damage_calculation = r_config->add_slider("combo.r.include_aa_in_damage_calculation", "Include x AA in R damage calculation", 1, 0, 3);
                    combo::r_use_before_expire = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_before_expire", "Use R before expire on soul", true);
                    combo::r_use_before_expire->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    combo::r_use_if_all_spells_cooldown = r_config->add_checkbox(myhero->get_model() + ".combo.r.use_if_all_spells_cooldown", "Use R on soul if all spells are on cooldown", true);
                    combo::r_use_if_all_spells_cooldown->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                    
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

                auto passive_config = combo->add_tab(myhero->get_model() + "combo.passive.config", "Passive Config");
                {
                    combo::auto_catch_soul = passive_config->add_checkbox(myhero->get_model() + ".combo.passive.auto_soul_catch", "Auto Soul Catch", true);
                    combo::auto_catch_soul->set_texture(myhero->get_passive_icon_texture());
                    combo::force_catch_soul = passive_config->add_checkbox(myhero->get_model() + ".combo.passive.force_catch_soul", "Force Soul Catch", true);
                    combo::force_catch_soul->set_texture(myhero->get_passive_icon_texture());
                    combo::simple_spell_usage_on_soul = passive_config->add_checkbox(myhero->get_model() + ".combo.passive.auto_soul_spell_usage", "Simple Soul Spell Usage", true);
                    combo::simple_spell_usage_on_soul->set_texture(myhero->get_passive_icon_texture());
                    combo::max_soul_distance = passive_config->add_slider("combo.passive.auto_soul_catch.max_distance", "Max distance to soul", 225, 100, 600);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harass.w", "Use W", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclear.q", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
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
                fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W", true);
                fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".flee.e", "Use E", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
            {
                hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 1);
                hitchance::w_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.w", "Hitchance W", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
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
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);

                draw_settings::draw_damage_r = draw_settings->add_checkbox(myhero->get_model() + "draw.R.damage", "Draw R Damage", true);
            }
        }

        // Permashow initialization
        //
        {
            Permashow::Instance.Init(main_tab);
            Permashow::Instance.AddElement("Spell Farm", laneclear::spell_farm);
            Permashow::Instance.AddElement("Semi Auto R", combo::r_semi_manual_cast);
            Permashow::Instance.AddElement("Save R", combo::r_save);
            Permashow::Instance.AddElement("Allow Tower Dive", combo::allow_tower_dive);
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_after_attack_orbwalker>::add_callback(on_after_attack);

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


        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_after_attack_orbwalker>::remove_handler(on_after_attack);
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
            if (combo::auto_catch_soul->get_bool())
            {
                for (auto& object : entitylist->get_all_minions())
                {
                    if (object->is_valid() && !object->is_dead() && object->is_attack_allowed_on_target() && myhero->get_distance(object) <= combo::max_soul_distance->get_int() && object->get_model() == "ViegoSoul")
                    {
                        orbwalker->set_movement(false);
                        if (combo::force_catch_soul->get_bool())
                            orbwalker->set_attack(false);
                        myhero->issue_order(object);
                        return;
                    }
                }

                orbwalker->set_movement(true);
                if (combo::force_catch_soul->get_bool())
                    orbwalker->set_attack(true);
            }

            if (r->is_ready() && combo::r_semi_manual_cast->get_bool())
            {
                r_logic_semi();
            }

            bool is_viego = myhero->get_character_data()->get_base_skin_name() == "Viego";

            if (is_viego)
            {
                total_ad = myhero->get_total_attack_damage();
                additional_ad = myhero->get_additional_attack_damage();
                crit = myhero->get_crit();
            }

            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (!is_viego)
                {
                    if (r->is_ready() && combo::use_r->get_bool())
                    {
                        r_logic();

                        auto buff = myhero->get_buff(buff_hash("viegopassivetransform"));
                        if ((!q->is_ready() && !w->is_ready() && !e->is_ready() && combo::r_use_if_all_spells_cooldown->get_bool()) || (combo::r_use_before_expire->get_bool() && buff != nullptr && buff->is_valid() && buff->is_alive() && buff->get_remaining_time() <= 2.0))
                        {
                            r_logic_semi();
                        }
                    }   

                    if (combo::simple_spell_usage_on_soul->get_bool() && !myhero->is_casting_interruptible_spell())
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            spellslot slot = static_cast<spellslot>(i);
                            auto spell = myhero->get_spell(slot);

                            if (spell != nullptr && utils::is_ready(slot))
                            {
                                bool channeling = spell->get_spell_data()->mUseChargeChanneling();
                                float* castrange = spell->get_spell_data()->CastRange();
                                float range = std::max(200.0f, std::min(900.0f, *castrange));
                                //console->print("Spell %d: %s Channeling: [%s]", i, spell->get_name().c_str(), channeling ? "true" : "false");

                                // Get a target from a given range
                                auto target = target_selector->get_target(range, damage_type::physical);

                                // Always check an object is not a nullptr!
                                if (target != nullptr)
                                {
                                    spell_targeting type = spell->get_spell_data()->get_targeting_type();
                                    //console->print("Spell Type: %d", type);

                                    if (type == spell_targeting::target)
                                    {
                                        utils::cast(slot, target, channeling);
                                        continue;
                                    }
                                    else if (type == spell_targeting::self || type == spell_targeting::self_aoe)
                                    {
                                        utils::cast(slot, myhero, channeling);
                                        continue;
                                    }
                                    else
                                    {
                                        float* castradius = spell->get_spell_data()->CastRadius();
                                        float delay = std::max(0.25f, spell->get_spell_data()->mCastTime());
                                        float radius = *castradius <= 50.0f ? 100.0f : *castradius;
                                        float speed = spell->get_spell_data()->MissileSpeed() < 400.0f ? FLT_MAX : spell->get_spell_data()->MissileSpeed();

                                        //console->print("Delay: [%.2f] Range: [%.2f] Radius: [%.2f] Speed: [%.2f] Type: [%d]", spell->get_spell_data()->mCastTime(), *castrange, spell->get_spell_data()->CastRadius(), spell->get_spell_data()->MissileSpeed(), (unsigned char)spell->get_spell_data()->get_targeting_type());

                                        prediction_input x;

                                        x._from = myhero->get_position();
                                        x.unit = target;
                                        x.delay = delay;
                                        x.radius = radius;
                                        x.speed = speed;
                                        x.collision_objects = { collisionable_objects::minions };
                                        x.range = range;
                                        x.type = (type == spell_targeting::cone) ? skillshot_type::skillshot_cone : skillshot_type::skillshot_circle;
                                        x.aoe = false;
                                        x.spell_slot = slot;
                                        x.use_bounding_radius = x.type != skillshot_type::skillshot_circle;

                                        auto output = prediction->get_prediction(&x);

                                        if (output.hitchance > hit_chance::high)
                                        {
                                            utils::cast(slot, output.get_cast_position(), channeling);
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    return;
                }

                if (w->is_ready() && combo::use_w->get_bool())
                {
                    w_logic();
                }
                
                if (!w->is_charging())
                {
                    if (q->is_ready() && combo::use_q->get_bool())
                    {
                        q_logic();
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
            }

            if (!is_viego)
            {
                return;
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

                    if (!w->is_charging() && q->is_ready() && harass::use_q->get_bool())
                    {
                        q_logic();
                    }
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (w->is_ready() && fleemode::use_w->get_bool())
                {
                    if (w->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                    {
                        return;
                    }
                }
                if (!w->is_charging() && e->is_ready() && fleemode::use_e->get_bool())
                {
                    if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                    {
                        return;
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

                if (!lane_minions.empty())
                {
                    if (!w->is_charging() && q->is_ready() && laneclear::use_q->get_bool())
                    {
                        q->cast_on_best_farm_position(1);
                    }
                }


                if (!monsters.empty())
                {
                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->is_charging())
                        {
                            w->cast(monsters.front());
                        }
                        else
                        {
                            w->cast();
                        }
                    }

                    if (!w->is_charging())
                    {
                        if (q->is_ready() && jungleclear::use_q->get_bool() && combo::q_mode->get_int() == 1)
                        {
                            q->cast_on_best_farm_position(1, true);
                        }

                        if (e->is_ready() && jungleclear::use_e->get_bool())
                        {
                            e->cast_on_best_farm_position(1, true);
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
            auto q_mode = combo::q_mode->get_int();
            if ((q_mode == 0 && myhero->get_distance(target) > myhero->get_attack_range()) || q_mode == 1 || q->get_damage(target) > target->get_real_health())
            {
                q->cast(target);
            }
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target != nullptr && (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool()))
        {
            if (w->is_charging())
            {
                auto pred = w->get_prediction(target);
                
                if (pred.hitchance >= get_hitchance(hitchance::w_hitchance))
                {
                    auto pos = w->get_prediction(target).get_cast_position();
                    if (!evade->is_dangerous(pos))
                    {
                        w->cast(pos);
                    }
                }
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
        auto target = target_selector->get_target(myhero->get_attack_range(), damage_type::physical);
        
        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            e->cast(target);
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && !combo::r_save->get_bool() && target->is_attack_allowed_on_target() && !utils::has_unkillable_buff(target) && !utils::has_untargetable_buff(target) && can_use_r_on(target) && get_viego_r_damage(target) > target->get_real_health())
        {
            r->cast(target, get_hitchance(hitchance::r_hitchance));
        }
    }
#pragma endregion

#pragma region r_logic_semi
    void r_logic_semi()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr && target->is_attack_allowed_on_target() && !utils::has_unkillable_buff(target) && !utils::has_untargetable_buff(target) && can_use_r_on(target) && (!target->is_under_ally_turret() || combo::allow_tower_dive->get_bool() || get_viego_r_damage(target) > target->get_real_health()))
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

        if (r->is_ready() && draw_settings::draw_damage_r->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && enemy->is_hpbar_recently_rendered())
                {
                    draw_dmg_rl(enemy, get_viego_r_damage(enemy), 0x8000ff00);
                }
            }
        }
    }

    void on_after_attack(game_object_script target)
    {
        // Use q after autoattack on enemies
        if (q->is_ready() && combo::q_mode->get_int() != 1 && target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
        {
            q->cast(target);
        }

        // Use q after autoattack on monsters
        if (q->is_ready() && combo::q_mode->get_int() != 1 && orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool() && jungleclear::use_q->get_bool() && target->is_valid() && !target->is_dead() && target->is_monster())
        {
            q->cast_on_best_farm_position(1, true);
        }
    }


    int get_viego_r_damage(game_object_script target)
    {
        float r_damage[] = { 12.0f, 16.0f, 20.0f };
        float base_dmg = (1.2f * total_ad) + (crit * (1.2f * total_ad));

        float base_percent_dmg = r_damage[r->level() - 1];
        base_percent_dmg += (additional_ad * 0.03);

        float health = target->get_health();
        float max_health = target->get_max_health();
        float missing_health = max_health - health;
        float missing_health_damage = (base_percent_dmg / 100) * missing_health;

        //if (target->is_visible_on_screen())
        //    myhero->print_chat(1, "My Total AD: [%.1f] | My Additional AD: [%.1f] | My Crit: [%.1f] | Base DMG: [%.1f] | Base percent DMG: [%.1f] | Missing Health DMG: [%.1f]", total_ad, additional_ad, crit, base_dmg, base_percent_dmg, missing_health_damage);

        damage_input input;
        input.raw_physical_damage = base_dmg + missing_health_damage;
        return damagelib->calculate_damage_on_unit(myhero, target, &input) + (myhero->get_auto_attack_damage(target) * combo::r_include_aa_in_damage_calculation->get_int());
    }
};