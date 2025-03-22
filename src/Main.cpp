#include "Events.h"
#include "Hooks.h"
#include "Logging.h"
#include "Settings.h"
#include "cache.h"
#include "inputhandler.h"

void Listener(SKSE::MessagingInterface::Message *message) noexcept
{
    if (message->type == SKSE::MessagingInterface::kDataLoaded)
    {
        Hooks::Install();
        Settings::Forms::LoadForms();
        Input::RegisterEvents();
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

    if (const auto messaging{SKSE::GetMessagingInterface()}; !messaging->RegisterListener(Listener))
    {
        return false;
    }

    logger::info("{} has finished loading.", name);
    logger::info("");
    Settings::Values::Update();

    return true;
}
