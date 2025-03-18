#include "Settings.h"

void Settings::LoadSettings() noexcept
{
    logger::info("--------------------------------");
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\STweaks.ini)");

    
    //int
    weapon_upper_range = (int)ini.GetDoubleValue("SettingValues", "iUpperRangeWeapons", 15);
    weapon_lower_range = (int)ini.GetDoubleValue("SettingValues", "iLowerRangeWeapons", 15);
    magic_upper_range = (int)ini.GetDoubleValue("SettingValues", "iUpperRangeMagic", 15);
    magic_lower_range = (int)ini.GetDoubleValue("SettingValues", "iLowerRangeMagic", 15);

    //float
    sneak_height_modifier = (float)ini.GetDoubleValue("SettingValues", "fSneakJumpModifier", 0.25);    

    //bool
    enable_mass_based_jump_height = ini.GetBoolValue("Toggles", "bMassBasedJump", true);
    enable_damage_ranges = ini.GetBoolValue("Toggles", "bEnableDamageRanges", true);
    enable_sneak_jump_limit = ini.GetBoolValue("Toggles", "bEnableSneakJumpLimit", true);
    //enable_carry_weight_debuffs = ini.GetBoolValue("Toggles", "bToggleCarryWeightDebuff", true);
    enable_sneak_stamina = ini.GetBoolValue("Toggles", "bEnableSneakStamina", true);
    enable_foll_change = ini.GetBoolValue("Toggles", "bEnableFollowerDamageChange", true);

    //debug logging
    debug_logging = ini.GetBoolValue("DebugLogging", "bDebugLoggingEnable");

    //after reading the ini

    //change log type
    if (debug_logging) {
        spdlog::set_level(spdlog::level::debug);
        logger::debug("Debug logging enabled");
    }
    //set range limits

    weapon_upper_range = std::clamp(weapon_upper_range, 1, 99);
    logger::debug("weapon upper range is {}", weapon_upper_range);
    weapon_lower_range = std::clamp(weapon_lower_range, 1, 99);
    logger::debug("weapon lower range is {}", weapon_lower_range);
    magic_upper_range = std::clamp(magic_upper_range, 1, 99);
    logger::debug("magic upper range is {}", magic_upper_range);
    magic_lower_range = std::clamp(magic_lower_range, 1, 99);
    logger::debug("magic lower range is {}", magic_lower_range);

    logger::info("Loaded settings");
    logger::info("--------------------------------");
}

void Settings::LoadForms() noexcept
{
    logger::info("--------------------------------");
    logger::info("Loading Forms");
    const int sneak_stamina_spell_ID = 0x804;
    const char* the_mod = "STweaks.esp";
    auto dh = RE::TESDataHandler::GetSingleton();
    sneak_stamina_spell = dh->LookupForm<RE::SpellItem>(sneak_stamina_spell_ID, the_mod);
    logger::info("Loaded Forms");
    logger::info("--------------------------------");
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
