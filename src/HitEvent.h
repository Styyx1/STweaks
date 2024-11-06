#pragma once

namespace HitHandler
{
    class HitEventH : public RE::BSTEventSink<RE::TESHitEvent>
    {
    public:
        static HitEventH* GetSingleton();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) noexcept override;
        static void Register();
    private:
        HitEventH() = default;
        HitEventH(const HitEventH&) = delete;
        HitEventH(HitEventH&&) = delete;
        HitEventH& operator=(const HitEventH&) = delete;
        HitEventH& operator=(HitEventH&&) = delete;
    };
}
