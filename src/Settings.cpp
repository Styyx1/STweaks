#include "Settings.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\STweaks.ini)");

    
    //int
    damage_range_weapon = (int)ini.GetDoubleValue("SettingValues", "iDamageRangeWeapons", 15);
    damage_range_magic = (int)ini.GetDoubleValue("SettingValues", "iDamageRangeMagic", 15);

    weapon_upper_range = (int)ini.GetDoubleValue("SettingValues", "iUpperRangeWeapons", 15);
    weapon_lower_range = (int)ini.GetDoubleValue("SettingValues", "iLowerRangeWeapons", 15);
    magic_upper_range = (int)ini.GetDoubleValue("SettingValues", "iUpperRangeMagic", 15);
    magic_lower_range = (int)ini.GetDoubleValue("SettingValues", "iLowerRangeMagic", 15);

    //float
    sneak_height_modifier = (float)ini.GetDoubleValue("SettingValues", "fSneakJumpModifier", 0.25);    
    jump_modifier_heavy = (float)ini.GetDoubleValue("SettingValues", "fJumpModifierHeavy", 0.4f);
    jump_modifier_light = (float)ini.GetDoubleValue("SettingValues", "fJumpModifierLight", 0.85f);
    //bool
    enable_mass_based_jump_height = ini.GetBoolValue("Toggles", "bMassBasedJump", true);
    enable_damage_ranges = ini.GetBoolValue("Toggles", "bEnableDamageRanges", true);
    enable_sneak_jump_limit = ini.GetBoolValue("Toggles", "bEnableSneakJumpLimit", true);
    enable_carry_weight_debuffs = ini.GetBoolValue("Toggles", "bToggleCarryWeightDebuff", true);
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
    damage_range_weapon = std::clamp(damage_range_weapon, 1, 99);
    logger::debug("weapon damage range is {}", damage_range_weapon);
    damage_range_magic = std::clamp(damage_range_magic, 1, 99);
    logger::debug("magic damage range is {}", damage_range_magic);

    weapon_upper_range = std::clamp(weapon_upper_range, 1, 99);
    weapon_lower_range = std::clamp(weapon_lower_range, 1, 99);
    magic_upper_range = std::clamp(magic_upper_range, 1, 99);
    magic_lower_range = std::clamp(magic_lower_range, 1, 99);


    //set jump height limits (unused)
    jump_modifier_heavy = std::clamp(jump_modifier_heavy, 0.1f, 1.0f);
    jump_modifier_light = std::clamp(jump_modifier_light, 0.1f, 1.0f);

    logger::info("Loaded settings");
    logger::info("");
}

void Settings::LoadForms() noexcept
{
    //const int stamina_spell_jump_ID = 0x801;
    const int sneak_stamina_spell_ID = 0x804;
    const char* the_mod = "STweaks.esp";
    auto dh = RE::TESDataHandler::GetSingleton();
    //stamina_spell_jump = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_ID, the_mod);
    sneak_stamina_spell = dh->LookupForm<RE::SpellItem>(sneak_stamina_spell_ID, the_mod);
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
