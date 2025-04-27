#include "Events.h"
#include "Hooks.h"
#include "Logging.h"
#include "Settings.h"
#include "cache.h"
#include "serialisation.h"
#include "papyrus.h"

void Listener(SKSE::MessagingInterface::Message *message) noexcept
{
    if (message->type == SKSE::MessagingInterface::kDataLoaded)
    {
        Hooks::Install();
        Settings::Forms::LoadForms();
        Events::RegisterEvents();
        Settings::Exceptions::LoadJson();
    }
    if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        Utility::Curses::PopulateActiveCursesAfterLoad(Cache::GetPlayerSingleton());
        if (!Settings::Values::enable_diseases.GetValue()) {
            Utility::Curses::CleanseCurse(Cache::GetPlayerSingleton());
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    // while (!IsDebuggerPresent()) Sleep(1000);
    InitLogging();

    const auto plugin{SKSE::PluginDeclaration::GetSingleton()};
    const auto name{plugin->GetName()};
    const auto version{plugin->GetVersion()};

    logger::info("{} {} is loading...", name, version);

    Init(skse);

    SKSE::AllocTrampoline(1 << 10);
    Cache::CacheAddLibAddresses();

    const auto papyrus = SKSE::GetPapyrusInterface();
    papyrus->Register(Papyrus::BindAll);

    if (auto serialization = SKSE::GetSerializationInterface())
    {
        serialization->SetUniqueID(Serialisation::ID);
        serialization->SetSaveCallback(&Serialisation::SaveCallback);
        serialization->SetLoadCallback(&Serialisation::LoadCallback);
        serialization->SetRevertCallback(&Serialisation::RevertCallback);
    }

    if (const auto messaging{SKSE::GetMessagingInterface()}; !messaging->RegisterListener(Listener))
    {
        return false;
    }

    logger::info("{} has finished loading.", name);
    logger::info("");
    Settings::Values::Update();

    return true;
}
