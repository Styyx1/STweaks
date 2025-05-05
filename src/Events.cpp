#include "Events.h"

namespace Events
{
    EventResult HitEventHandler::ProcessEvent(const RE::TESHitEvent *a_event, RE::BSTEventSource<RE::TESHitEvent> *a_eventSource)
    {
        using HitFlag = RE::TESHitEvent::Flag;
        if (!a_event || !a_event->target || !a_event->cause)
        {
            return EventResult::kContinue;
        }
        auto defender = a_event->target ? a_event->target->As<RE::Actor>() : nullptr;
        if (!defender)
        {
            return EventResult::kContinue;
        }
        auto aggressor = a_event->cause ? a_event->cause->As<RE::Actor>() : nullptr;
        if (!aggressor)
        {
            return EventResult::kContinue;
        }
        

        if (!a_event->flags.any(HitFlag::kBashAttack))
        {
            if (defender->IsPlayerRef()) {
                auto spellItem = RE::TESForm::LookupByID<RE::SpellItem>(a_event->source);
                if (spellItem && Settings::Forms::disease_mod_active) {
                    logger::debug("Spell item is {}", spellItem->GetName());
                    auto chance = Settings::Values::curse_chance.GetValue();
					if (spellItem->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
						chance *= 0.10f;
					}
                    if(Utility::Curses::ShouldApplyCurse(chance)){
                        Utility::Curses::ApplyRandomCurse(defender, Settings::Forms::curse_list);
                    }
                }
            }     

            auto& tick = disease_timers[defender];
			if (!tick.IsRunning() || tick.ElapsedSeconds() > 3.0f)
			{
                if (Utility::ActiveEffectHasNewDiseaseKeyword(defender, Settings::Values::diseases[0]))
                {
                    // storedHealth_disease = std::min(std::max(storedHealth_disease + 1.0f, 0.0f), 99.0f);
                    float decrease_value = CalculatePenaltyAndStoreIt(defender, RE::ActorValue::kHealth, storedHealth_disease);
                    logger::debug("stored health disease is {}", storedHealth_disease);
                    defender->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth, -decrease_value);
                }
                if (Utility::ActiveEffectHasNewDiseaseKeyword(defender, Settings::Values::diseases[1]))
                {
                    // storedStamina_disease = std::min(std::max(storedStamina_disease + 1.0f, 0.0f), 99.0f);
                    float decrease_value_stam = CalculatePenaltyAndStoreIt(defender, RE::ActorValue::kStamina, storedStamina_disease);
                    logger::debug("stored stamina disease is {}", storedStamina_disease);
                    defender->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kStamina, -decrease_value_stam);
                }
                if (Utility::ActiveEffectHasNewDiseaseKeyword(defender, Settings::Values::diseases[2]))
                {

                    // storedMagicka_disease = std::min(std::max(storedMagicka_disease + 1.0f, 0.0f), 99.0f);
                    float decrease_value_mag = CalculatePenaltyAndStoreIt(defender, RE::ActorValue::kMagicka, storedMagicka_disease);
                    logger::debug("stored magicka disease is {}", storedMagicka_disease);
                    defender->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kMagicka, -decrease_value_mag);
                }
                tick.Reset();
				tick.Start(); 
			}
			else
			{
				logger::debug("{} tried to apply disease too soon. {:.1f}s remaining", defender->GetName(), 3.0f - tick.ElapsedSeconds());
				return EventResult::kContinue;
			}            
        }
        return EventResult::kContinue;
    }

    float HitEventHandler::CalculatePenaltyAndStoreIt(RE::Actor *a_actor, RE::ActorValue a_av, float &penalty_storage)
    {
        float maxPenAv = GetMaxActorValue(a_actor, a_av) + penalty_storage;
        float newPenaltyMag = std::roundf(maxPenAv * 0.01f);
        float maxAllowedPenalty = std::roundf(maxPenAv * 0.9f);
		logger::debug("penalty_storage is {}", penalty_storage);
		logger::debug("maxPenAv is {}", maxPenAv);
		logger::debug("newPenaltyMag is {}", newPenaltyMag);

        if (penalty_storage + newPenaltyMag > maxAllowedPenalty) {
            newPenaltyMag = maxAllowedPenalty - penalty_storage;
            newPenaltyMag = std::max(0.0f, newPenaltyMag);  // Prevent going negative
        }

        penalty_storage += newPenaltyMag;
        return newPenaltyMag;
    }

    void HitEventHandler::RegisterHitEvent()
    {
        RE::ScriptEventSourceHolder *eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventHolder->AddEventSink(HitEventHandler::GetSingleton());
        logger::info("Registered for {}", typeid(RE::TESHitEvent).name());
    }
    float HitEventHandler::GetMaxActorValue(RE::Actor *a_actor, RE::ActorValue a_av)
    {
        return a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_av) + a_actor->AsActorValueOwner()->GetPermanentActorValue(a_av);
    }
    void RegisterEvents()
    {
        if (Settings::Values::enable_diseases.GetValue())
        {
            auto hitEventHandler = HitEventHandler::GetSingleton();
            hitEventHandler->RegisterHitEvent();
            auto effectHandler = ApplyEffectEvent::GetSingleton();
            effectHandler->RegisterApplyEffect();
        }
    }
    RE::BSEventNotifyControl ApplyEffectEvent::ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource)
    {
        if (!a_event)
            return EventResult::kContinue;
        if (!a_event->magicEffect)
            return EventResult::kContinue;
        auto effect = RE::TESForm::LookupByID<RE::EffectSetting>(a_event->magicEffect);
        if (!effect) 
            return EventResult::kContinue;
        if (!effect->HasKeywordByEditorID(Settings::Constants::cure_keyword))
            return EventResult::kContinue;
        auto target = a_event->target.get();
        auto targetActor = target ? target->As<RE::Actor>() : nullptr;
        if (!targetActor)
            return EventResult::kContinue;
        Utility::Curses::CleanseCurse(targetActor);
        logger::debug("Effect '{}' triggered. Target is '{}'.", effect->GetName(), targetActor->GetName());

        return EventResult::kContinue;
    }
    void ApplyEffectEvent::RegisterApplyEffect()
    {
        auto* eventSink = ApplyEffectEvent::GetSingleton();
        auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventSourceHolder->AddEventSink<RE::TESMagicEffectApplyEvent>(eventSink);

        logger::info("registered Apply Effect Event");
    }
}
