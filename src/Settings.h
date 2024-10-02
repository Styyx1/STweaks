#pragma once

class Settings : public Singleton<Settings>
{
public:
    static void LoadSettings() noexcept;
    void LoadForms() noexcept;
    inline static bool debug_logging{};

    RE::SpellItem* stamina_spell_jump;

};
