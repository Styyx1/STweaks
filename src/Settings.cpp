#include "Settings.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\STweaks.ini)");

    debug_logging = ini.GetBoolValue("Log", "Debug");
    enable_damage_ranges = ini.GetBoolValue("General", "bEnableDamageRanges");
    damage_range_weapon = std::stoi(ini.GetValue("General", "iDamageRangeWeapons", "15"));
    damage_range_magic = std::stoi(ini.GetValue("General", "iDamageRangeMagic", "25"));

    if (debug_logging) {
        spdlog::set_level(spdlog::level::debug);
        logger::debug("Debug logging enabled");
    }
    int max_range = 99;
    int min_range = 1;
    damage_range_weapon = std::min(std::max(damage_range_weapon, min_range), max_range);
    logger::debug("weapon damage range is {}", damage_range_weapon);
    damage_range_magic = std::min(std::max(damage_range_magic, min_range), max_range);
    logger::debug("magic damage range is {}", damage_range_magic);

    // Load settings

    logger::info("Loaded settings");
    logger::info("");
}

void Settings::LoadForms() noexcept
{
    const char* the_mod = "STweaks.esp";
    auto dh = RE::TESDataHandler::GetSingleton();
    stamina_spell_jump = dh->LookupForm<RE::SpellItem>(0x801, the_mod);
}

float Settings::CalcPerc(int a_input, bool a_high)
{
    float result;
    if (a_high) {
        return result = (100.00f + a_input) / 100.00f;
    }
    else
    return result = (100.00f - a_input) / 100.00f;
}
