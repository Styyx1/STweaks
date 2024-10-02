#include "Settings.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\STweaks.ini)");

    debug_logging = ini.GetBoolValue("Log", "Debug");

    if (debug_logging) {
        spdlog::set_level(spdlog::level::debug);
        logger::debug("Debug logging enabled");
    }

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
