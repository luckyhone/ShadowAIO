#include "../plugin_sdk/plugin_sdk.hpp"

namespace spell_database
{

    struct spell_data
    {
        std::string menuslot;
        std::vector<std::string> spell_names;
        spellslot slot;
        bool targeted;
        bool missile;
        bool off_by_default;
        std::vector<buff_type> cc_types = {};
        std::vector<damage_type> damage_types = {};
        skillshot_type spellType;
        float spellDelay;
    };

    // Unfinished
    //
    std::map<champion_id, std::vector<spell_data>> spells
    {
        {
            champion_id::Ahri,
            {
                spell_data{"Q", {"AhriQ"}, spellslot::q, false, true, false, {}, {damage_type::magical}, skillshot_type::skillshot_line, 250},
                spell_data{"E", {"AhriE"}, spellslot::e, false, true, false, {buff_type::Charm}, {damage_type::magical}, skillshot_type::skillshot_line, 250}
            }
        },
        {
            champion_id::Amumu,
            {
                spell_data{"Q", {"AmumuQ"}, spellslot::q, false, true, false, {buff_type::Stun}, {damage_type::magical}, skillshot_type::skillshot_line, 250}
            }
        }
    };
}