#include "../plugin_sdk/plugin_sdk.hpp"

#pragma once
namespace utils
{

	// AIO utilities
	//
	void on_load();

	// Buff checks
	//
	bool has_unkillable_buff(game_object_script target);
	bool has_untargetable_buff(game_object_script target);
	bool has_crowd_control_buff(game_object_script target);

	// Spell casting
	//
	bool fast_cast(script_spell* spell);
	bool cast(spellslot slot, bool is_charged_spell);
	bool cast(spellslot slot, game_object_script unit, bool is_charged_spell);
	bool fast_cast(script_spell* spell, vector position);
	bool cast(spellslot slot, vector position, bool is_charged_spell);
	bool fast_cast(script_spell* spell, game_object_script unit, hit_chance minimum, bool aoe, int min_targets);
	bool fast_cast(script_spell* spell, int minMinions, bool is_jugnle_mobs);

	// Spell utilities
	//
	bool is_ready(spellslot slot);
	
	// Vector utilities
	//
	vector to_2d(vector vec);
	vector to_3d(vector vec);
	vector to_3d2(vector vec);
	vector add(vector source, float add);

	// Damage utilities
	//
	float get_damage(game_object_script target, std::vector<script_spell*> spells, int include_aa);

	// Minion utilities
	//
	int32_t count_minions_in_range(game_object_script target, float range);
	int32_t count_minions_in_range(vector vec, float range);
	int32_t count_monsters_in_range(game_object_script target, float range);
	int32_t count_monsters_in_range(vector vec, float range);

	// Other
	//
	bool enabled_in_map(std::map<std::uint32_t, TreeEntry*> map, game_object_script target);
	hit_chance get_hitchance(TreeEntry* entry);
};

