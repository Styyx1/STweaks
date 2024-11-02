#include "Hooks.h"
#include "Settings.h"
#include "Utility.h"
namespace Hooks
{
    void Install()
    {        
        AdjustActiveEffect::Install();
        CombatHit::Install();
        MainUpdate::InstallUpdate();        
        logger::debug("installed hooks");
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
                float rand_mult = util->GetRandomFloat(Settings::CalcPerc(Settings::damage_range_weapon, false), Settings::CalcPerc(Settings::damage_range_weapon, true));
                logger::debug("multiplier is {}", rand_mult);
                remaining *= rand_mult;
                logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
                a_hitData->totalDamage = remaining;
                return _originalCall(a_this, a_hitData);
                
            }

        }
        ///constexpr std::array<REL::VariantID, 1>  VTABLE_HealthDamageFunctor{ REL::VariantID(264062, 210014, 0x1700028) };

        
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
        RE::BGSSaveLoadManager* save_manager = RE::BGSSaveLoadManager::GetSingleton();
        save_manager->PopulateSaveList();


        return func();
    }
    void MainUpdate::InstallUpdate()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(35565, 36564), REL::Relocate(0x748, 0xc26, 0x7ee) };
        func = trampoline.write_call<5>(target.address(), Thunk);
    }

    // https://github.com/powerof3/MagicSneakAttacks/blob/275255b26492115557c7bfa3cb7c4a79e83f2f3d/src/Hooks.cpp#L29 
    void AdjustActiveEffect::thunk(RE::ActiveEffect* a_this, float a_power, bool a_onlyHostile)
    {
        
        Utility* util = Utility::GetSingleton();
        const auto attacker = a_this->GetCasterActor();
        const auto target = a_this->GetTargetActor();
        const auto effect = a_this->GetBaseObject();
        const auto spell = a_this->spell;

        if (attacker && target && spell && effect && effect->IsHostile()) {
            if (const auto projectile = effect->data.projectileBase; projectile) {
                if (auto damage_ranges = util->GetRandomFloat(Settings::CalcPerc(Settings::damage_range_magic, false), Settings::CalcPerc(Settings::damage_range_magic, true))) {
                    logger::debug("original damag is: {}", a_this->magnitude);
                    logger::debug("damage_ranges is {}", damage_ranges);
                    a_this->magnitude *= damage_ranges;
                    logger::debug("spell damage is {}", a_this->magnitude);
                }
            }
        }
        func(a_this, a_power, a_onlyHostile);
    }
    void AdjustActiveEffect::Install()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(33763, 34547), REL::Relocate(0x4A3, 0x656, 0x427)  }; // MagicTarget::CheckAddEffect
        func = trampoline.write_call<5>(target.address(), thunk);
    }
} // namespace Hooks
