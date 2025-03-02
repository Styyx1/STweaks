#pragma once

class Settings 
{
public:
    static void LoadSettings() noexcept;
    static void LoadForms() noexcept;
    static float CalcPerc(int a_input, bool a_high);
    inline static bool debug_logging{false};
    inline static bool enable_damage_ranges{true};
    inline static int damage_range_magic{ 25 };
    inline static int damage_range_weapon{ 15 };
    inline static bool enable_sneak_jump_limit{true};
    inline static float sneak_height_modifier{ 0.25 };
    inline static bool enable_carry_weight_debuffs{true};
    inline static bool enable_mass_based_jump_height{ true };
    inline static bool enable_sneak_stamina{ true };
    inline static float jump_modifier_heavy{ 0.5 };
    inline static float jump_modifier_light{ 0.85 };
    inline static bool enable_foll_change{ true };

    inline static int weapon_upper_range{ 15 };
    inline static int weapon_lower_range{ 15 };
    inline static int magic_upper_range{ 15 };
    inline static int magic_lower_range{ 15 };


    static inline RE::SpellItem* stamina_spell_jump;
    static inline RE::SpellItem* sneak_stamina_spell;

};
