#pragma once

class Settings : public Singleton<Settings>
{
public:
    static void LoadSettings() noexcept;
    void LoadForms() noexcept;
    inline static bool debug_logging{};
    inline static bool enable_damage_ranges{};

    RE::SpellItem* stamina_spell_jump;

};
