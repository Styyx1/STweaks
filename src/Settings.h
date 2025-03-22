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

        static void LogBool(const char *a_boolName, bool a_setting)
        {
            logger::debug("bool {} is {}", a_boolName, a_setting ? "true" : "false");
        }

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

            LogBool("sneak stamina", Settings::Values::enable_sneak_stamina.GetValue());
            LogBool("use Requiem", use_requiem_stamina.GetValue());
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
            const int stamina_spell_attack_ID = 0x6AA964;
            const int stamina_spell_jump_ID = 0x6AA967;
            const int stamina_spell_swim_playerID = 0xADDDCB;
            const int exhaustion_swimID = 0xADDDCC;
            const int stamina_block_spell_ID = 0x2C6F32;
            const int bullrush_player_ID = 0x766B8B;
            const int bullrush_horse_ID = 0x77D000;
            const int dodge_spell_cost_ID = 0x6AA965;

            const int dodge_perk_effect_ID = 0x6C3574;

            //-----------------------------------------//

            const int stamina_spell_attack_npcID = 0x0;
            const int stamina_spell_jump_npcID = 0x0;
            const int stamina_spell_walk_playerID = 0x0;
            const int stamina_spell_sprint_playerID = 0x0;

            const char *requiem = "Requiem.esp";

            auto dh = RE::TESDataHandler::GetSingleton();

            sneak_stamina_spell = dh->LookupForm<RE::SpellItem>(sneak_stamina_spell_ID, the_mod);

            if (Settings::Values::use_requiem_stamina.GetValue())
            {
                /*
                stamina_jump_player = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_playerID, requiem);
                stamina_walk_player = dh->LookupForm<RE::SpellItem>(stamina_spell_walk_playerID, requiem);
                stamina_sprint_player = dh->LookupForm<RE::SpellItem>(stamina_spell_sprint_playerID, requiem);
                stamina_attack_npc = dh->LookupForm<RE::SpellItem>(stamina_spell_attack_npcID, requiem);
                stamina_jump_npc = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_npcID, requiem); */

                stamina_attack_player = dh->LookupForm<RE::SpellItem>(stamina_spell_attack_ID, requiem);
                stamina_swimm_player = dh->LookupForm<RE::SpellItem>(stamina_spell_swim_playerID, requiem);
                exhaustion_swim = dh->LookupForm<RE::SpellItem>(exhaustion_swimID, requiem);
                stamina_spell_jump = dh->LookupForm<RE::SpellItem>(stamina_spell_jump_ID, requiem);
                stamina_block_spell = dh->LookupForm<RE::SpellItem>(stamina_block_spell_ID, requiem);
                bullrush_player_spell = dh->LookupForm<RE::SpellItem>(bullrush_player_ID, requiem);
                bullrush_horse_spell = dh->LookupForm<RE::SpellItem>(bullrush_horse_ID, requiem);
                dodge_cost_spell = dh->LookupForm<RE::SpellItem>(dodge_spell_cost_ID, requiem);
                dodge_perk_effect = dh->LookupForm<RE::EffectSetting>(dodge_perk_effect_ID, requiem);

                LogForm("swim_stamina_player", stamina_swimm_player);
                LogForm("swim exhaustion", exhaustion_swim);
            }

            logger::info("Loaded Forms");
        }

        static void LogForm(std::string a_name, RE::TESForm *a_form)
        {
            logger::debug("{} is {}", a_name, a_form->GetName());
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
        static inline RE::SpellItem *stamina_swimm_player{nullptr};
        static inline RE::SpellItem *exhaustion_swim{nullptr};
        static inline RE::SpellItem *stamina_block_spell{nullptr};
        static inline RE::SpellItem *bullrush_horse_spell{nullptr};
        static inline RE::SpellItem *bullrush_player_spell{nullptr};
        static inline RE::SpellItem *dodge_cost_spell{nullptr};

        static inline RE::EffectSetting *dodge_perk_effect{nullptr};

        static inline RE::SpellItem *stamina_jump_player{nullptr};
        static inline RE::SpellItem *stamina_walk_player{nullptr};
        static inline RE::SpellItem *stamina_sprint_player{nullptr};
        static inline RE::SpellItem *stamina_attack_npc{nullptr};
        static inline RE::SpellItem *stamina_jump_npc{nullptr};
    };
}
