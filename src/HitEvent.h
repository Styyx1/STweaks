#pragma once

namespace HitHandler
{
    class HitEventH : public EventSingleton<HitEventH, RE::TESHitEvent>
    {
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) noexcept override;
    };
}
