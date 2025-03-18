#include "Hooks.h"
#include "Settings.h"
#include "Utility.h"


namespace Hooks
{
    void Install()
    {        
        logger::info("--------------------------------");
        logger::info("Installing hooks");

        AdjustActiveEffect::Install();
        CombatHit::Install();
        JumpHeight::Install();
        MainUpdate::InstallUpdate();
        DealtMeleeDamage::InstallMeleeDamageHook();

        logger::debug("installed all hooks");
        logger::info("--------------------------------");
    }
    void CombatHit::Install()
    {
        //1.6.1170: 1406BAD18 - 38627 //VR offsets are just SE offsets because i don't VR and didn't want to leave them blank. 
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0) };
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
        logger::info("Installed damage taken hook");
    }
    void CombatHit::RandomiseDamage(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        if (Settings::enable_damage_ranges) {
            float remaining = a_hitData->totalDamage;
            if (a_hitData->weapon && !a_hitData->weapon->IsHandToHandMelee()) {
                logger::debug("----------------------------------------------------");
                logger::debug("started hooked damage calc for {} you got hit by: {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName());
                //float rand_mult = Utility::GetRandomFloat(Settings::CalcPerc(Settings::damage_range_weapon, false), Settings::CalcPerc(Settings::damage_range_weapon, true));
                float rand_mult = Utility::GetRandomFloat(Settings::CalcPerc(Settings::weapon_lower_range, false), Settings::CalcPerc(Settings::weapon_upper_range, true));
                logger::debug("multiplier is {}", rand_mult);
                remaining *= rand_mult;
                logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
                a_hitData->totalDamage = remaining;
                return _originalCall(a_this, a_hitData);
                
            }

        }
                
    }
    void CombatHit::CHit(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        RandomiseDamage(a_this, a_hitData);
        logger::debug("hit data damage is {}", a_hitData->totalDamage);
        logger::debug("----------------------------------------------------");
        _originalCall(a_this, a_hitData);
    }


    void MainUpdate::PlayerUpdate(RE::PlayerCharacter* player, float a_delta)
    {
        if (frameCount > 10.0f) {
            frameCount = 0;
            if (Settings::enable_sneak_stamina) {
                if (player->IsGodMode()) {
                    if (Settings::sneak_stamina_spell)
                        player->RemoveSpell(Settings::sneak_stamina_spell);
                }
                else {
                    RE::TESObjectWEAP* weap = Utility::getWieldingWeapon(player);
                    if (player->IsSneaking() && Utility::IsMoving(player) || player->IsSneaking() && weap && weap->IsBow() && player->AsActorState()->IsWeaponDrawn() || player->IsSneaking() && weap && weap->IsCrossbow() && player->AsActorState()->IsWeaponDrawn()) {
                        if (!Utility::HasSpell(player, Settings::sneak_stamina_spell)) {
                            player->AddSpell(Settings::sneak_stamina_spell);                       
                        }
                        if (player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) <= 5) {
                            player->DrawWeaponMagicHands(false);
                            player->AsActorState()->actorState2.weaponState = RE::WEAPON_STATE::kWantToSheathe;
                        }                        

                    } else if (Utility::HasSpell(player, Settings::sneak_stamina_spell)) {
                        player->RemoveSpell(Settings::sneak_stamina_spell);
                    }
                }
            }
            //logger::debug("player has {} teammates", player->GetPlayerRuntimeData().teammateCount);
        }
        frameCount++;
        func(player, a_delta);
    }
    void MainUpdate::InstallUpdate()
    {
        REL::Relocation<std::uintptr_t> PlayerVTBL{ RE::VTABLE_PlayerCharacter[0] };
        func = PlayerVTBL.write_vfunc(0xAD, PlayerUpdate);
        logger::info("Installed player update hook");
    }

    // https://github.com/powerof3/MagicSneakAttacks/blob/275255b26492115557c7bfa3cb7c4a79e83f2f3d/src/Hooks.cpp#L29 
    void AdjustActiveEffect::thunk(RE::ActiveEffect* a_this, float a_power, bool a_onlyHostile)
    {
        const auto attacker = a_this->GetCasterActor();
        const auto target = a_this->GetTargetActor();
        const auto effect = a_this->GetBaseObject();
        const auto spell = a_this->spell;

        if (attacker && target && spell && effect && effect->IsHostile()) {
            if (const auto projectile = effect->data.projectileBase; projectile) {
                // if (auto damage_ranges = Utility::GetRandomFloat(Settings::CalcPerc(Settings::damage_range_magic, false), Settings::CalcPerc(Settings::damage_range_magic, true))) {
                if (auto damage_ranges = Utility::GetRandomFloat(Settings::CalcPerc(Settings::magic_lower_range, false), Settings::CalcPerc(Settings::magic_upper_range, true))) {
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
        logger::info("Installed active effect hook");
    }

    float JumpHeight::JumpHeightGetScale(RE::TESObjectREFR* refr)
    {
        float scale = refr->GetScale();
        RE::Actor* actor = refr->As<RE::Actor>();
        
        if (!actor) {
            return scale;
        }
        float mass = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMass);
        logger::debug("{}'s mass is {}", actor->GetName(), mass);
        if (actor->IsSneaking()) {
            scale *= Settings::sneak_height_modifier;
        }
        float ju_modifier = sqrt(1.0 / mass);

        return scale *= ju_modifier;
    }
    void JumpHeight::Install()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36271, 37257), REL::Relocate(0x190, 0x17f, 0x190) };
        func = trampoline.write_call<5>(target.address(), JumpHeightGetScale);
        logger::info("Installed jump height hook");
    }

    void DealtMeleeDamage::InstallMeleeDamageHook()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(42832, 44001), REL::VariantOffset(0x1a5, 0x1a4, 0x1a5) };
        _MeleeDamageCall = trampoline.write_call<5>(target.address(), &MeleeDamage);
        logger::info("Hooked melee damage");
    }
    float DealtMeleeDamage::MeleeDamage(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow)
    {
        RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
        RE::Actor* actor = skyrim_cast<RE::Actor*>(a);
        auto dam = _MeleeDamageCall(_weap, a, DamageMult, isbow);
        logger::debug("before if in melee damage hook for actor: {}", actor->GetDisplayFullName());
        if (Settings::enable_foll_change) {
            if (actor && actor->IsPlayerTeammate() && !actor->IsCommandedActor() || actor && actor == player) {
                logger::debug("melee damage active for actor: {}", actor->GetDisplayFullName());
                if (player->GetPlayerRuntimeData().teammateCount > 0) {
                    // formula:
                    // x = 1 - (N - 1) / (5 + S/20)
                    // N = teammateCount
                    // S = ActorValue::kSpeech
                    float adj_mod1 = 5.0f;
                    float adj_divider = 20.0f;
                    uint32_t foll_count = player->GetPlayerRuntimeData().teammateCount;
                    float speech_level = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeech);
                    logger::debug("pre calculation. foll count = {}, speech_level = {}", foll_count, speech_level);
                    float modifier = std::clamp(1.0f - (foll_count) / (adj_mod1 + speech_level/adj_divider), 0.10f, 0.90f);

                    if (modifier < 0.10f) {
                        modifier = 0.10f;
                    }
                    if (modifier > 0.9f) {
                        modifier = 0.9f;
                    }
                    dam *= modifier;
                    logger::debug("\n final modifier is {} \n", modifier);
                }
            }    
        }            
        return dam;
    }
} // namespace Hooks
