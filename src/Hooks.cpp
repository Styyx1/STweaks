#include "Hooks.h"
#include "Settings.h"
#include "Utility.h"


namespace Hooks
{
    void Install()
    {        
        AdjustActiveEffect::Install();
        CombatHit::Install();
        JumpHeight::Install();
        OverEncumbered::Install();
        MainUpdate::InstallUpdate();
        //ActorUpdateHook::InstallUpdateActor();
        DealtMeleeDamage::InstallMeleeDamageHook();
        logger::debug("installed hooks");
    }
    void CombatHit::Install()
    {
        //1.6.1170: 1406BAD18 - 38627 //VR offsets are just SE offsets because i don't VR and didn't want to leave them blank. 
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0) };
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
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
        logger::info("hook:Player Update");
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
        /*
SE ID: 36271 SE Offset: 0x190 (Heuristic)
AE ID: 37257 AE Offset: 0x17f
        */
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36271, 37257), REL::Relocate(0x190, 0x17f, 0x190) };
        func = trampoline.write_call<5>(target.address(), JumpHeightGetScale);
        logger::info("Installed jump height hook");

    }

    bool OverEncumbered::IsOverEncumberedEX(RE::Actor* a_actor)
    {
        //auto test = func(a_actor);
        //logger::info("test is {}", test ? "true" : "false");
        logger::info("inside overencumbered hook");
        return false;
    }
    void OverEncumbered::Install()
    {        
        /*
SE ID: 21017 SE Offset: 0x2e (Heuristic)
AE ID: 21467 AE Offset: 0x2e 

SE ID: 27811 SE Offset: 0x66 (Heuristic)
AE ID: 28536 AE Offset: 0x66

        SE ID: 38610 SE Offset: 0xa1 (Heuristic)
        AE ID: 39641 AE Offset: 0xa1

        SE ID: 38615 SE Offset: 0x3b (Heuristic)
        AE ID: 39646 AE Offset: 0x3b

        SE ID: Not Found SE Offset: Not Found
        AE ID: 40447 AE Offset: 0x1274

SE ID: 38046 SE Offset: 0x24e (Heuristic)
AE ID: 39002 AE Offset: 0x1f6

SE ID: 39460 SE Offset: 0x58 (Heuristic)
AE ID: 40537 AE Offset: 0x63

SE ID: 39372 SE Offset: Not Found
AE ID: 40444 AE Offset: 0x0 //fast travel bool

        */
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(27811, 40444), REL::Relocate(0x66, 0x0, 0x66)};
        func = trampoline.write_call<5>(target.address(), IsOverEncumberedEX);
        logger::info("Installed overencumbered hook");
    }

    void ActorUpdateHook::InstallUpdateActor()
    {
        REL::Relocation<std::uintptr_t> ActorVTABLE{ RE::VTABLE_Character[0] };

        _ActorUpdate = ActorVTABLE.write_vfunc(0xAD, ActorUpdate);
        logger::info("hook:NPC Update");
    }

    void ActorUpdateHook::ActorUpdate(RE::Character* a_this, float a_delta)
    {
        _ActorUpdate(a_this, a_delta);
    }

    void DealtMeleeDamage::InstallMeleeDamageHook()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(42832, 44001), REL::VariantOffset(0x1a5, 0x1a4, 0x1a5) };
        _MeleeDamageCall = trampoline.write_call<5>(target.address(), &MeleeDamage);
        logger::info("hooked melee damage");
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
