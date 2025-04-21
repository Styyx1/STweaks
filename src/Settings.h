#pragma once

namespace Settings
{
    namespace Values
    {
        static REX::INI::Bool debug_logging{ "DebugLogging", "bDebugLoggingEnable", false };
        static REX::INI::Bool enable_damage_ranges{ "Toggles", "bEnableDamageRanges", true };
        static REX::INI::Bool enable_sneak_jump_limit{ "Toggles", "bEnableSneakJumpLimit", true };
        static REX::INI::Bool enable_mass_based_jump_height{ "Toggles", "bMassBasedJump", true };
        static REX::INI::Bool enable_sneak_stamina{ "Toggles", "bEnableSneakStamina", true };
        static REX::INI::Bool enable_foll_change{ "Toggles", "bEnableFollowerDamageChange", true };
        static REX::INI::Bool enable_etheral_change{ "Toggles", "bEnableEtherealChange", true };
        static REX::INI::Bool enable_normal_sneak_attack{ "Toggles", "bEnableNormalSneakAttack", true };
        static REX::INI::Bool enable_diseases{ "Toggles", "bEnableDiseases", true };
        static REX::INI::Bool enable_fading_actors{ "Toggles", "bEnableFadingActors", true };
        static REX::INI::Bool enable_quest_item_nerf{ "Toggles", "bEnableQuestItemNerf", true };

        static REX::INI::F32 sneak_height_modifier{ "SettingValues", "fSneakJumpModifier", 0.25f };
        static REX::INI::I32 weapon_upper_range{ "SettingValues", "iUpperRangeWeapons", 15 };
        static REX::INI::I32 weapon_lower_range{ "SettingValues", "iLowerRangeWeapons", 15 };
        static REX::INI::I32 magic_upper_range{ "SettingValues", "iUpperRangeMagic", 15 };
        static REX::INI::I32 magic_lower_range{ "SettingValues", "iLowerRangeMagic", 15 };
        static REX::INI::F32 curse_chance{ "SettingValues", "fCurseChance", 1.0f };
        static REX::INI::F64 curse_swap_cooldown{ "SettingValues", "fCurseSwapCooldown", 60.0 };

        static void LogBool(const char* a_boolName, bool a_setting)
        {
            logger::debug("bool {} is {}", a_boolName, a_setting ? "true" : "false");
        }

        static inline std::vector<std::string> diseases{
            "stweaksDisease_Health",
            "stweaksDisease_Stamina",
            "stweaksDisease_Magicka"
        };



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
        }
    }

    namespace Exceptions {

        static inline std::set<RE::TESObjectWEAP*> weapon_exceptions;

        static inline RE::TESForm *GetFormFromString(const std::string &spellName)
        {
            std::istringstream ss{spellName};
            std::string plugin, id;
            std::getline(ss, id, '|');
            std::getline(ss, plugin);

            RE::FormID rawFormID;
            std::istringstream(id) >> std::hex >> rawFormID;

            auto dataHandler = RE::TESDataHandler::GetSingleton();
            return dataHandler->LookupForm(rawFormID, plugin);
        }

        static inline void LoadExceptionWeapons(const std::string &configFilePath)
        {
            std::ifstream file(configFilePath);
            if (!file.is_open())
            {
                logger::error("Failed to open the file: {}", configFilePath);
                return;
            }

            nlohmann::json j;
            file >> j;

            //load Exceptions
            if (j.contains("quest_weapon_exceptions") && j["quest_weapon_exceptions"].is_array()) {
                for (const auto& str : j["quest_weapon_exceptions"])
                {
                    const std::string& formStr = str.get<std::string>();
                    logger::debug("Loading exception weapon: {}", formStr);

                    RE::TESForm* form = GetFormFromString(formStr);
                    if (form && form->GetFormType() == RE::FormType::Weapon)
                    {
                        weapon_exceptions.insert(form->As<RE::TESObjectWEAP>());                     
                        logger::info("loaded {} as exception", form->GetName());
                    } 
                    else 
                    {
                        logger::warn("Invalid or non-weapon form: {}", formStr);
                    }
                }
            }

            logger::info("Loaded {} weapons from {}.",
                weapon_exceptions.size(),
                configFilePath);
        }

        static void LoadJson() {
            const auto path = R"(.\Data\SKSE\Plugins\stweaks_exceptions.json)";
            LoadExceptionWeapons(path);
        }

        inline static bool IsQuestWeaponException(RE::TESObjectWEAP* form)
        {
            return form && weapon_exceptions.contains(form);
        }
    }

    namespace Constants {
        constexpr const char* mod_name = "STweaks.esp";
        constexpr const char* diseases_name = "Stweaks - Diseases.esp";
        const int sneak_stamina_spell_ID = 0x804;
        const int health_curse_ID = 0x3;
        const int stamina_curse_ID = 0x6;
        const int magicka_curse_ID = 0x9;
        const int silence_curse_ID = 0xD;
        const int melee_weakness_curse_ID = 0x10;
        const int bow_weakness_curse_ID = 0x13;
        const int jump_curse_ID = 0x16;

        constexpr const char* cure_keyword = "cleanse_curse";
        constexpr const char* curse_keyword = "stweaks_curse";
        constexpr const char* silence_key = "curse_silence";
        constexpr const char* bow_curse_key = "curse_bow";
        constexpr const char* jump_curse_key = "curse_jump";
    }

    struct Forms
    {
        static inline bool disease_mod_active = false;

        static void LoadForms() noexcept
        {
            logger::info("Loading Forms");

            auto dh = RE::TESDataHandler::GetSingleton();
            if (auto main_file = dh->LookupModByName(Constants::mod_name); main_file && main_file->compileIndex != 0xFF) {
                sneak_stamina_spell = dh->LookupForm<RE::SpellItem>(Constants::sneak_stamina_spell_ID, Constants::mod_name);
            }
            else
                SKSE::stl::report_and_fail(std::format("{} not found, please enable it.", Constants::mod_name));
            
            if (Settings::Values::enable_diseases.GetValue()) {
                if (auto file = dh->LookupModByName(Constants::diseases_name); file && file->compileIndex != 0xFF){
                    health_curse = dh->LookupForm<RE::SpellItem>(Constants::health_curse_ID, Constants::diseases_name);
                    stamina_curse = dh->LookupForm<RE::SpellItem>(Constants::stamina_curse_ID, Constants::diseases_name);
                    magicka_curse = dh->LookupForm<RE::SpellItem>(Constants::magicka_curse_ID, Constants::diseases_name);
                    silence_curse = dh->LookupForm<RE::SpellItem>(Constants::silence_curse_ID, Constants::diseases_name);
                    melee_damage_curse = dh->LookupForm<RE::SpellItem>(Constants::melee_weakness_curse_ID, Constants::diseases_name);
                    bow_damage_curse = dh->LookupForm<RE::SpellItem>(Constants::bow_weakness_curse_ID, Constants::diseases_name);
                    jump_curse = dh->LookupForm<RE::SpellItem>(Constants::jump_curse_ID, Constants::diseases_name);

                    curse_list = {
                        health_curse, stamina_curse, magicka_curse, silence_curse, melee_damage_curse, bow_damage_curse, jump_curse
                    };

                    disease_mod_active = true;
                }
                else {
                    SKSE::stl::report_and_fail(std::format("{} not found, please either enable the esp or disable the diseases in the ini file of this mod.", Constants::diseases_name));
                }
            }
            LogAllFormsLoaded();
            logger::info("Loaded Forms");
        }

       

        static void LogForm(std::string a_name, RE::TESForm *a_form)
        {
            logger::debug("{} is {}", a_name, a_form->GetName());
        }

        static void LogAllFormsLoaded() {
			LogForm("Sneak Stamina Spell", sneak_stamina_spell);
            if (disease_mod_active) {
                LogForm("Health Curse", health_curse);
                LogForm("Stamina Curse", stamina_curse);
                LogForm("Magicka Curse", magicka_curse);
                LogForm("Silence Curse", silence_curse);
                LogForm("Melee Weakness Curse", melee_damage_curse);
                LogForm("Bow Weakness Curse", bow_damage_curse);
                LogForm("Jump Curse", jump_curse);
            }			
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

        static inline RE::SpellItem *sneak_stamina_spell{nullptr};
		static inline RE::SpellItem* health_curse{ nullptr };
		static inline RE::SpellItem* stamina_curse{ nullptr };
		static inline RE::SpellItem* magicka_curse{ nullptr };
        static inline RE::SpellItem* silence_curse{ nullptr };
        static inline RE::SpellItem* melee_damage_curse{ nullptr };
        static inline RE::SpellItem* bow_damage_curse{ nullptr };
        static inline RE::SpellItem* jump_curse{ nullptr };

        static inline std::vector<RE::SpellItem*> curse_list{};

    };
}
