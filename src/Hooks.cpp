#include "Hooks.h"
#include "Settings.h"
#include "Utility.h"
namespace Hooks
{
    void Install()
    {
        SKSE::AllocTrampoline(1 << 10);
        CombatHit::Install();
        MainUpdate::InstallUpdate();
    }
    void CombatHit::Install()
    {
        //SE: 140628C20 - 37673 // AE: 1406BAD18 - 38627
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0) };
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
    }
    void CombatHit::RandomiseDamage(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        const Settings* settings = Settings::GetSingleton();

        if (settings->enable_damage_ranges) {
            float remaining = a_hitData->totalDamage;
            Utility* util = Utility::GetSingleton();
            if (a_hitData->weapon && !a_hitData->weapon->IsHandToHandMelee()) {
                logger::debug("----------------------------------------------------");
                logger::debug("started hooked damage calc for {} you got hit by: {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName());

                remaining *= util->GetRandomFloat(0.85, 1.15);
                logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
                _originalCall(a_this, a_hitData);
                a_hitData->totalDamage = remaining;
            }
        }

        
    }
    void CombatHit::CHit(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        RandomiseDamage(a_this, a_hitData);
        logger::info("hit data damage is {}", a_hitData->totalDamage);
        logger::debug("----------------------------------------------------");
        _originalCall(a_this, a_hitData);
    }
    i32 MainUpdate::Thunk() noexcept
    {
        return func();
    }
    void MainUpdate::InstallUpdate()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(35565, 36564), REL::Relocate(0x748, 0xc26, 0x7ee) };
        func = trampoline.write_call<5>(target.address(), Thunk);
    }
} // namespace Hooks
