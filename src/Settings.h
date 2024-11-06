#pragma once

class Settings 
{
public:
    static void LoadSettings() noexcept;
    static void LoadForms() noexcept;
    static float CalcPerc(int a_input, bool a_high);
    inline static bool debug_logging{};
    inline static bool enable_damage_ranges{};
    inline static int damage_range_magic{ 25 };
    inline static int damage_range_weapon{ 15 };

    static inline RE::SpellItem* stamina_spell_jump;

};
