#include "../plugin_sdk/plugin_sdk.hpp"

// Declare plugin name & supported champions
//
PLUGIN_NAME("ShadowAIO");
PLUGIN_TYPE(plugin_type::champion);
SUPPORTED_CHAMPIONS(champion_id::Tryndamere, champion_id::Kindred, champion_id::Trundle, champion_id::Jax, champion_id::Kayle, champion_id::Vex, champion_id::MasterYi, champion_id::Chogath, champion_id::Twitch, champion_id::Kalista, champion_id::Malzahar, champion_id::MissFortune, champion_id::Teemo, champion_id::Gwen, champion_id::Ivern, champion_id::Rengar, champion_id::Draven, champion_id::Belveth, champion_id::Viego, champion_id::Thresh, champion_id::Nasus/*, champion_id::Yasuo*/);

// Include champion file
//
#include "tryndamere.h"
#include "kindred.h"
#include "trundle.h"
#include "jax.h"
#include "kayle.h"
#include "vex.h"
#include "masteryi.h"
#include "chogath.h"
#include "twitch.h"
#include "kalista.h"
#include "malzahar.h"
#include "missfortune.h"
#include "teemo.h"
#include "gwen.h"
#include "ivern.h"
#include "rengar.h"
#include "draven.h"
#include "belveth.h"
#include "viego.h"
#include "thresh.h"
#include "nasus.h"
//#include "yasuo.h"

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
        case champion_id::MasterYi:
            // Load masteryi script
            //
            masteryi::load();
            break;
        case champion_id::Chogath:
            // Load chogath script
            //
            chogath::load();
            break;
        case champion_id::Twitch:
            // Load twitch script
            //
            twitch::load();
            break;
        case champion_id::Kalista:
            // Load kalista script
            //
            kalista::load();
            break;
        case champion_id::Malzahar:
            // Load malzahar script
            //
            malzahar::load();
            break;
        case champion_id::MissFortune:
            // Load missfortune script
            //
            missfortune::load();
            break;
        case champion_id::Teemo:
            // Load teemo script
            //
            teemo::load();
            break;
        case champion_id::Gwen:
            // Load gwen script
            //
            gwen::load();
            break;
        case champion_id::Ivern:
            // Load ivern script
            //
            ivern::load();
            break;
        case champion_id::Rengar:
            // Load rengar script
            //
            rengar::load();
            break;
        case champion_id::Draven:
            // Load draven script
            //
            draven::load();
            break;
        case champion_id::Belveth:
            // Load belveth script
            //
            belveth::load();
            break;
        case champion_id::Viego:
            // Load viego script
            //
            viego::load();
            break;
        case champion_id::Thresh:
            // Load thresh script
            //
            thresh::load();
            break;
        case champion_id::Nasus:
            // Load nasus script
            //
            nasus::load();
            break;
        /*case champion_id::Yasuo:
            // Load yasuo script
            //
            yasuo::load();
            break;*/
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
        case champion_id::MasterYi:
            // Unload masteryi script
            //
            masteryi::unload();
            break;
        case champion_id::Chogath:
            // Unload chogath script
            //
            chogath::unload();
            break;
        case champion_id::Twitch:
            // Unload twitch script
            //
            twitch::unload();
            break;
        case champion_id::Kalista:
            // Unload kalista script
            //
            kalista::unload();
            break;
        case champion_id::Malzahar:
            // Unload malzahar script
            //
            malzahar::unload();
            break;
        case champion_id::MissFortune:
            // Unload missfortune script
            //
            missfortune::unload();
            break;
        case champion_id::Teemo:
            // Unload teemo script
            //
            teemo::unload();
            break;
        case champion_id::Gwen:
            // Unload gwen script
            //
            gwen::unload();
            break;
        case champion_id::Ivern:
            // Unload ivern script
            //
            ivern::unload();
            break;
        case champion_id::Rengar:
            // Unload rengar script
            //
            rengar::unload();
            break;
        case champion_id::Draven:
            // Unload draven script
            //
            draven::unload();
            break;
        case champion_id::Belveth:
            // Unload belveth script
            //
            belveth::unload();
            break;
        case champion_id::Viego:
            // Unload viego script
            //
            viego::unload();
            break;
        case champion_id::Thresh:
            // Unload thresh script
            //
            thresh::unload();
            break;
        case champion_id::Nasus:
            // Unload nasus script
            //
            nasus::unload();
            break;
        /*case champion_id::Yasuo:
            // Unload yasuo script
            //
            yasuo::unload();
            break;*/
        default:
            break;
    }
}