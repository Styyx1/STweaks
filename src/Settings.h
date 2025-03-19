#pragma once

namespace Settings
{
    namespace Values
    {
        static REX::INI::Bool debug_logging{"DebugLogging", "bDebugLoggingEnable", false};
        static REX::INI::Bool enable_damage_ranges{"Toggles", "bEnableDamageRanges", true};
        static REX::INI::Bool enable_sneak_jump_limit{"Toggles", "bEnableSneakJumpLimit", true};
        static REX::INI::Bool enable_mass_based_jump_height{"Toggles", "bMassBasedJump", true};
        static REX::INI::Bool enable_sneak_stamina{"Toggles", "bEnableSneakStamina", true};
        static REX::INI::Bool enable_foll_change{"Toggles", "bEnableFollowerDamageChange", true};
        static REX::INI::Bool use_requiem_stamina{"Toggles", "bUseRequiem", true};

        static REX::INI::F32 sneak_height_modifier{"SettingValues", "fSneakJumpModifier", 0.25f};
        static REX::INI::I32 weapon_upper_range{"SettingValues", "iUpperRangeWeapons", 15};
        static REX::INI::I32 weapon_lower_range{"SettingValues", "iLowerRangeWeapons", 15};
        static REX::INI::I32 magic_upper_range{"SettingValues", "iUpperRangeMagic", 15};
        static REX::INI::I32 magic_lower_range{"SettingValues", "iLowerRangeMagic", 15};

        inline static void Update()
        {
            logger::info("Loading settings...");
            const auto ini = REX::INI::SettingStore::GetSingleton();
            ini->Init(R"(.\Data\SKSE\Plugins\STweaks.ini)", R"(.\Data\SKSE\Plugins\STweaks.ini)");
            ini->Load();

            // change log type
            if (debug_logging.GetValue())
            {
                spdlog::set_level(spdlog::level::debug);
                logger::debug("Debug logging enabled");
            }
            // set range limits
            weapon_upper_range.SetValue(std::clamp(weapon_upper_range.GetValue(), 1, 99));
            logger::debug("weapon upper range is {}", weapon_upper_range.GetValue());
            weapon_lower_range.SetValue(std::clamp(weapon_lower_range.GetValue(), 1, 99));
            logger::debug("weapon lower range is {}", weapon_lower_range.GetValue());
            magic_upper_range.SetValue(std::clamp(magic_upper_range.GetValue(), 1, 99));
            logger::debug("magic upper range is {}", magic_upper_range.GetValue());
            magic_lower_range.SetValue(std::clamp(magic_lower_range.GetValue(), 1, 99));
            logger::debug("magic lower range is {}", magic_lower_range.GetValue());
        }
    }
    struct Forms
    {
        static void LoadForms() noexcept
        {
            logger::info("Loading Forms");

            // Stweaks specific forms
            const int sneak_stamina_spell_ID = 0x804;
            const char *the_mod = "STweaks.esp";
            // Requiem specific forms
            const int stamina_spell_attack_playerID = 0x0;
            const int stamina_spell_jump_playerID = 0x0;
            const int stamina_spell_attack_npcID = 0x0;
            const int stamina_spell_jump_npcID = 0x0;
            const int stamina_spell_walk_playerID = 0x0;
            const int stamina_spell_sprint_playerID = 0x0;
            const char *requiem = "Requiem.esp";

            auto dh = RE::TESDataHandler::GetSingleton();

            sneak_stamina_spell = dh->LookupForm<RE::SpellItem>(sneak_stamina_spell_ID, the_mod);

            if (Settings::Values::use_requiem_stamina.GetValue())
            {
                stamina_attack_player = dh->LookupForm<RE::SpellItem>(stamina_spell_attack_playerID, requiem);
                stamina_jump_player = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_playerID, requiem);
                stamina_walk_player = dh->LookupForm<RE::SpellItem>(stamina_spell_walk_playerID, requiem);
                stamina_sprint_player = dh->LookupForm<RE::SpellItem>(stamina_spell_sprint_playerID, requiem);
                stamina_attack_npc = dh->LookupForm<RE::SpellItem>(stamina_spell_attack_npcID, requiem);
                stamina_jump_npc = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_npcID, requiem);
            }

            logger::info("Loaded Forms");
        }

        static float CalcPerc(int a_input, bool a_high)
        {
            float result;
            if (a_high)
            {
                return result = (100.00f + a_input) / 100.00f;
            }
            else
                return result = (100.00f - a_input) / 100.00f;
        }

        static inline RE::SpellItem *stamina_spell_jump{nullptr};
        static inline RE::SpellItem *sneak_stamina_spell{nullptr};
        static inline RE::SpellItem *stamina_attack_player{nullptr};
        static inline RE::SpellItem *stamina_jump_player{nullptr};
        static inline RE::SpellItem *stamina_walk_player{nullptr};
        static inline RE::SpellItem *stamina_sprint_player{nullptr};
        static inline RE::SpellItem *stamina_attack_npc{nullptr};
        static inline RE::SpellItem *stamina_jump_npc{nullptr};
    };
}
