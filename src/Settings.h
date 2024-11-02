#pragma once

class Settings : public Singleton<Settings>
{
public:
    static void LoadSettings() noexcept;
    void LoadForms() noexcept;
    static float CalcPerc(int a_input, bool a_high);
    inline static bool debug_logging{};
    inline static bool enable_damage_ranges{};
    inline static int damage_range_magic{ 25 };
    inline static int damage_range_weapon{ 15 };

    RE::SpellItem* stamina_spell_jump;

};
