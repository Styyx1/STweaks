#include "HitEvent.h"

namespace HitHandler {
    RE::BSEventNotifyControl HitEventH::ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) noexcept
    {
        using HitFlag = RE::TESHitEvent::Flag;
        if (!a_event || !a_event->target || !a_event->cause || a_event->projectile) {
            return RE::BSEventNotifyControl::kContinue;
        }
        auto defender = a_event->target ? a_event->target->As<RE::Actor>() : nullptr;
        if (!defender) {
            return RE::BSEventNotifyControl::kContinue;
        }
        auto aggressor = a_event->cause ? a_event->cause->As<RE::Actor>() : nullptr;
        if (!aggressor) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Do stuff
        return RE::BSEventNotifyControl();
    }
}

