#include "../plugin_sdk/plugin_sdk.hpp"

// Declare plugin name & supported champions
//
PLUGIN_NAME("ShadowAIO");
SUPPORTED_CHAMPIONS(champion_id::Tryndamere, champion_id::Kindred, champion_id::Trundle, champion_id::Jax, champion_id::Kayle, champion_id::Vex);

// Include champion file
//
#include "tryndamere.h"
#include "kindred.h"
#include "trundle.h"
#include "jax.h"
#include "kayle.h"
#include "vex.h"

// Entry point of plugin
//
PLUGIN_API bool on_sdk_load(plugin_sdk_core* plugin_sdk_good)
{
    // Global declaretion macro
    //
    DECLARE_GLOBALS(plugin_sdk_good);

    //Switch by myhero champion id
    //
    switch (myhero->get_champion())
    {
        case champion_id::Tryndamere:
            // Load tryndamere script
            //
            tryndamere::load();
            break;
        case champion_id::Kindred:
            // Load kindred script
            //
            kindred::load();
            break;
        case champion_id::Trundle:
            // Load trundle script
            //
            trundle::load();
            break;
        case champion_id::Jax:
            // Load jax script
            //
            jax::load();
            break;
        case champion_id::Kayle:
            // Load kayle script
            //
            kayle::load();
            break;
        case champion_id::Vex:
            // Load vex script
            //
            vex::load();
            break;
        default:
            // We don't support this champ, print message and return false (core will not load this plugin and on_sdk_unload will be never called)
            //
            console->print("[ShadowAIO] [ERROR] Champion %s is not supported in ShadowAIO!", myhero->get_model_cstr());
            return false;
    }

    // Return success, our plugin will be loaded now
    //
    console->print("[ShadowAIO] [INFO] Champion %s loaded successfully.", myhero->get_model_cstr());
    return true;
}

// Unload function, only when on_sdk_load returned true
//
PLUGIN_API void on_sdk_unload()
{
    switch (myhero->get_champion())
    {
        case champion_id::Tryndamere:
            // Unload tryndamere script
            //
            tryndamere::unload();
            break;
        case champion_id::Kindred:
            // Unload kindred script
            //
            kindred::unload();
            break;
        case champion_id::Trundle:
            // Unload trundle script
            //
            trundle::unload();
            break;
        case champion_id::Jax:
            // Unload jax script
            //
            jax::unload();
            break;
        case champion_id::Kayle:
            // Unload kayle script
            //
            kayle::unload();
            break;
        case champion_id::Vex:
            // Unload vex script
            //
            vex::unload();
            break;
        default:
            break;
    }
}