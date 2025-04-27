#include "Hooks.h"
#include "Settings.h"
#include "Utility.h"

using namespace Settings::Values;

namespace Hooks
{
    void Install()
    {
        logger::info("--------------------------------");
        logger::info("Installing hooks");

        if (enable_damage_ranges.GetValue())
        {
            AdjustActiveEffect::Install();
            CombatHit::Install();
        }

        if (enable_mass_based_jump_height.GetValue() || enable_sneak_jump_limit.GetValue())
        {
            JumpHeight::Install();
        }

        if (enable_sneak_stamina.GetValue())
        {
            MainUpdate::InstallUpdate();
        }

        if (enable_foll_change.GetValue() || enable_diseases.GetValue())
        {
            DealtMeleeDamage::InstallMeleeDamageHook();
        }

        if (enable_fading_actors.GetValue())
        {
            NPCFade::Install();
        }

        if (enable_diseases.GetValue()) {
            PreventCast::Install();
            PlayerPotionUsed::Install();
            HighGravityArrows::Install();
        }
        OnEffectEndHook::Install();
        
        logger::debug("installed all hooks");
        logger::info("--------------------------------");
    }

    bool wasEnraged = false;
           


    void CombatHit::Install()
    {
        // 1.6.1170: 1406BAD18 - 38627 //VR offsets are just SE offsets because i don't VR and didn't want to leave them blank.
        auto &trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0)};
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
        logger::info("Installed damage taken hook");
    }
    float CombatHit::WeaponTypeModifier(RE::TESObjectWEAP* a_weap, float f_in)
    {
        float f_out = f_in;

        if (a_weap->IsOneHandedDagger())
            f_out *= 2.0f;

        return f_out;
    }
    void CombatHit::RandomiseDamage(RE::Actor *a_this, RE::HitData *a_hitData)
    {
        float remaining = a_hitData->totalDamage;
        //RE::TESObjectWEAP* weap = a_hitData->weapon;

        if (a_hitData->weapon && !a_hitData->weapon->IsHandToHandMelee())
        {
            logger::debug("----------------------------------------------------");
            logger::debug("started hooked damage calc for {} you got hit by: {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName());
            float rand_mult = Utility::GetRandomFloat(Settings::Forms::CalcPerc(weapon_lower_range.GetValue(), false), Settings::Forms::CalcPerc(weapon_upper_range.GetValue(), true));
            logger::debug("multiplier is {}", rand_mult);
            remaining *= rand_mult;
            logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
            a_hitData->totalDamage = remaining;
        }
    }
    void CombatHit::CHit(RE::Actor *a_this, RE::HitData *a_hitData)
    {
        RandomiseDamage(a_this, a_hitData);
        _originalCall(a_this, a_hitData);
    }

    void MainUpdate::PlayerUpdate(RE::PlayerCharacter *player, float a_delta)
    {
        if (frameCount > 10)
        {
            frameCount = 0;
        }
        else
        {
            if (player->IsGodMode())
            {
                if (Settings::Forms::sneak_stamina_spell)
                    player->RemoveSpell(Settings::Forms::sneak_stamina_spell);
            }
            else
            {
                switch (frameCount)
                {
                case 1:
                    if (player->IsSneaking() && Utility::IsMoving(player) && enable_sneak_stamina.GetValue() || player->IsSneaking() && HasRangedWeaponDrawn(player) && enable_sneak_stamina.GetValue())
                    {
                        if (!Utility::HasSpell(player, Settings::Forms::sneak_stamina_spell))
                        {
                            player->AddSpell(Settings::Forms::sneak_stamina_spell);
                        }
                        if (player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) <= 5 && player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) > 0)
                        {
                            player->AsActorState()->actorState2.weaponState = RE::WEAPON_STATE::kWantToSheathe;
                            player->DrawWeaponMagicHands(false);
                        }
                    }
                    else if (Utility::HasSpell(player, Settings::Forms::sneak_stamina_spell))
                    {
                        player->RemoveSpell(Settings::Forms::sneak_stamina_spell);
                    }

                    break;
                default:
                    break;
                }
            }
        }
        frameCount++;
        return func(player, a_delta);
    }
    bool MainUpdate::HasRangedWeaponDrawn(RE::PlayerCharacter *player)
    {
        bool result = false;
        RE::TESObjectWEAP *weap = Utility::getWieldingWeapon(player);
        if (weap)
        {
            if (weap->IsBow() && player->AsActorState()->IsWeaponDrawn() || weap->IsCrossbow() && player->AsActorState()->IsWeaponDrawn())
                result = true;
        }

        logger::debug("check for ranged weapon, it is {}", result ? "true" : "false");
        return result;
    }
    void MainUpdate::InstallUpdate()
    {
        REL::Relocation<std::uintptr_t> PlayerVTBL{RE::VTABLE_PlayerCharacter[0]};
        func = PlayerVTBL.write_vfunc(0xAD, PlayerUpdate);
        logger::info("Installed player update hook");
    }

    // https://github.com/powerof3/MagicSneakAttacks/blob/275255b26492115557c7bfa3cb7c4a79e83f2f3d/src/Hooks.cpp#L29
    void AdjustActiveEffect::thunk(RE::ActiveEffect *a_this, float a_power, bool a_onlyHostile)
    {
        const auto attacker = a_this->GetCasterActor();
        const auto target = a_this->GetTargetActor();
        const auto effect = a_this->GetBaseObject();
        const auto spell = a_this->spell;

        if (attacker && target && spell && effect && effect->IsHostile())
        {
            if (const auto projectile = effect->data.projectileBase; projectile)
            {
                // if (auto damage_ranges = Utility::GetRandomFloat(Settings::CalcPerc(Settings::damage_range_magic, false), Settings::CalcPerc(Settings::damage_range_magic, true))) {
                if (auto damage_ranges = Utility::GetRandomFloat(Settings::Forms::CalcPerc(magic_lower_range.GetValue(), false), Settings::Forms::CalcPerc(magic_upper_range.GetValue(), true)))
                {
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
        auto &trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(33763, 34547), REL::Relocate(0x4A3, 0x656, 0x427)}; // MagicTarget::CheckAddEffect
        func = trampoline.write_call<5>(target.address(), thunk);
        logger::info("Installed active effect hook");
    }

    float JumpHeight::JumpHeightGetScale(RE::TESObjectREFR *refr)
    {
        float scale = refr->GetScale();
        RE::Actor *actor = refr->As<RE::Actor>();

        if (!actor)
        {
            return scale;
        }
        float mass = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMass);

        if (actor->IsSneaking())
        {
            scale *= sneak_height_modifier.GetValue();
        }
        float ju_modifier = (float)sqrt(1.0 / mass);

        float curse_modi = 1.0f;
        if (Utility::ActiveEffectHasNewDiseaseKeyword(actor, Settings::Constants::jump_curse_key)) {
            curse_modi = 0.5f;
        }

        return scale *= ju_modifier * curse_modi;
    }
    void JumpHeight::Install()
    {
        auto &trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(36271, 37257), REL::Relocate(0x190, 0x17f, 0x190)};
        func = trampoline.write_call<5>(target.address(), JumpHeightGetScale);
        logger::info("Installed jump height hook");
    }

    void DealtMeleeDamage::InstallMeleeDamageHook()
    {
        auto &trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(42832, 44001), REL::VariantOffset(0x1a5, 0x1a4, 0x1a5)};
        _MeleeDamageCall = trampoline.write_call<5>(target.address(), &MeleeDamage);
        logger::info("Hooked melee damage");
    }
    float DealtMeleeDamage::MeleeDamage(RE::TESObjectWEAP *_weap, RE::ActorValueOwner *a, float DamageMult, char isbow)
    {
        RE::PlayerCharacter *player = Cache::GetPlayerSingleton();
        RE::Actor *actor = skyrim_cast<RE::Actor *>(a);
        float dam = DamageMult;
        logger::debug("damage mult argument is {}", DamageMult);

        if (actor == player)
        {
            if (Settings::Values::enable_diseases.GetValue()) {
                const char* curse_word = "curse_weapons";
                if (Utility::ActiveEffectHasNewDiseaseKeyword(actor, curse_word)) {
                    float chance_for_effect = Utility::GetRandomFloat(0.0, 100.0);
                    if (chance_for_effect < 10.0f) {
                        
                        dam *= 0.0;
                        logger::debug(" 10% weakness curse is active, you deal {} damage", dam);
                    }
                    else if (chance_for_effect < 60.0f) {
                        dam *= 0.5f;
                        logger::debug("50% weakness curse is active, you deal {} damage", dam);
                    }
                }
            }
            if (Settings::Values::enable_quest_item_nerf.GetValue()) {
                if (ActorHasQuestObjectInHand(player)) {
                    if (_weap && !Settings::Exceptions::IsQuestWeaponException(_weap)) {
                        dam *= 0.005f;
                    }                    
                }
            }
        }

        if (Utility::ActorHasEffectWithArchetype(actor, RE::EffectArchetypes::ArchetypeID::kEtherealize) && Settings::Values::enable_etheral_change.GetValue())
        {
            dam *= 0.0f;
            logger::debug("[{}] is ethereal, damage is set to 0", actor->GetName());
        }        

        if (actor && actor->IsPlayerTeammate() && !actor->IsCommandedActor() || actor && actor == player)
        {         

            if (player->GetPlayerRuntimeData().teammateCount > 0 && Settings::Values::enable_foll_change.GetValue())
            {
                logger::debug("melee damage active for actor: {}", actor->GetDisplayFullName());
                // formula:
                // x = 1 - (N - 1) / (5 + S/20)
                // N = teammateCount
                // S = ActorValue::kSpeech
                float adj_mod1 = 5.0f;
                float adj_divider = 20.0f;
                uint32_t foll_count = player->GetPlayerRuntimeData().teammateCount;
                float speech_level = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeech);
                logger::debug("pre calculation. foll count = {}, speech_level = {}", foll_count, speech_level);
                float modifier = std::clamp(1.0f - (foll_count) / (adj_mod1 + speech_level / adj_divider), 0.10f, 0.90f);

                if (modifier < 0.10f)
                {
                    modifier = 0.10f;
                }
                if (modifier > 0.9f)
                {
                    modifier = 0.9f;
                }
                dam *= modifier;
                logger::debug("\n final modifier is {} \n", modifier);
            }            
        }
        return _MeleeDamageCall(_weap, a, dam, isbow);
    }
    bool DealtMeleeDamage::ActorHasQuestObjectInHand(RE::Actor* actor) {
        if (actor) {
            auto* rightHandItem = actor->GetEquippedEntryData(false); 
            if (rightHandItem) {
                if (rightHandItem->IsQuestObject()) {
                    return true;
                }
            }

            auto* leftHandItem = actor->GetEquippedEntryData(true);
            if (leftHandItem) {
                if (leftHandItem->IsQuestObject()) {
                    return true;
                }
            }
        }

        return false;
    }

    void OnEffectEndHook::Install()
    {
        REL::Relocation<std::uintptr_t> effectVtable{RE::VTABLE_ScriptEffect[0]};
        _effectEnd = effectVtable.write_vfunc(0x15, OnEffectEnd);
        logger::info("Installed effect end hook");
    }
    void OnEffectEndHook::OnEffectEnd(RE::ScriptEffect *a_this)
    {
        _effectEnd(a_this);
        auto hitEv = Events::HitEventHandler::GetSingleton();

            if (a_this->target && a_this->GetBaseObject()->HasAnyKeywordByEditorID(Settings::Values::diseases))
            {
				logger::debug("curse ended");
                RE::Actor *aff_actor = skyrim_cast<RE::Actor *>(a_this->target);
                if (a_this->GetBaseObject()->HasKeywordString(Settings::Values::diseases[0]))
                {
                    float currentPenaltyH = hitEv->storedHealth_disease;
                    if (currentPenaltyH > 0)
                    {
                        hitEv->storedHealth_disease = 0.0f;

                        aff_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth, currentPenaltyH);
                    }
                }
                if (a_this->GetBaseObject()->HasKeywordString(Settings::Values::diseases[1]))
                {
                    float currentPenaltyS = hitEv->storedStamina_disease;
                    if (currentPenaltyS > 0)
                    {
                        hitEv->storedStamina_disease = 0.0f;
                        aff_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kStamina, currentPenaltyS);
                    }
                }
                if (a_this->GetBaseObject()->HasKeywordString(Settings::Values::diseases[2]))
                {
                    float currentPenaltyM = hitEv->storedMagicka_disease;
                    if (currentPenaltyM > 0)
                    {
                        hitEv->storedMagicka_disease = 0.0f;
                        aff_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kMagicka, currentPenaltyM);
                    }
                }
            
        }
    }

    void NPCFade::Install()
    {
        REL::Relocation<std::uintptr_t> NPCVtable{RE::Character::VTABLE[0]};
        func = NPCVtable.write_vfunc(0xAD, ActorUpdate);
        logger::info("Installed character update hook");
    }
    void NPCFade::ActorUpdate(RE::Character *a_actor, float a_delta)
    {
        RE::PlayerCharacter *player = Cache::GetPlayerSingleton();
        float distance_full_fade = 2400.00;
        float distance_no_fade = 1200.00f;
        float light_level_threshold = 30.0f;
        float fade_range = distance_full_fade - distance_no_fade;
        float minAlpha = 0.05f;		

        auto difficulty_level = player->GetPlayerRuntimeData().difficulty;
        float difficultyMult = static_cast<float>(difficulty_level) / 5.0f;  // 0.0 - 1.0

        if (float distance = a_actor->GetDistance(player); distance >= distance_full_fade && a_actor->GetHighProcess() && a_actor->GetHighProcess()->lightLevel <= light_level_threshold)
        {
            float fade_progress = std::min(std::max((distance - distance_no_fade) / fade_range, 0.0f), 1.0f);
            float alpha = std::lerp(1.0f, minAlpha, fade_progress);
            //logger::debug("distance_full_fade: {}, distance_no_fade: {}, light_level_threshold: {}, fade_range: {}, minAlpha: {}", distance_full_fade, distance_no_fade, light_level_threshold, fade_range, minAlpha);

            a_actor->SetAlpha(alpha);
            a_actor->GetActorRuntimeData().dialogueItemTarget;
        }
        else
        {
            a_actor->SetAlpha();
        }
        func(a_actor, a_delta);
    }

    void PreventCast::Install()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_ActorMagicCaster[0] };
        func = vtbl.write_vfunc(0x0A, CheckCast);
        logger::info("Installed interupt cast hook");
    }

    bool PreventCast::CheckCast(RE::ActorMagicCaster* a_this, RE::MagicItem* a_spell, bool a_dualCast, float* a_effectStrength, RE::MagicSystem::CannotCastReason* a_reason, bool a_useBaseValueForCost)
    {
        auto actor = a_this->actor;
        const char* curse_word = "curse_silence";
        if (Utility::ActiveEffectHasNewDiseaseKeyword(actor, Settings::Constants::silence_key)) {
            InterruptActor(actor, a_this->GetCastingSource());
            return false;
        }
        return func(a_this, a_spell, a_dualCast, a_effectStrength, a_reason, a_useBaseValueForCost);
    }

    void PreventCast::InterruptActor(RE::Actor* a_actor, RE::MagicSystem::CastingSource a_castingSource)
    {
        switch (a_castingSource)
        {
        case RE::MagicSystem::CastingSource::kLeftHand:
            RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionLeftInterrupt);
            break;
        case RE::MagicSystem::CastingSource::kRightHand:
            RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionRightInterrupt);
            break;
        case RE::MagicSystem::CastingSource::kOther:
            RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionVoiceInterrupt);
            break;
        default:
            break;
        }
    }

    void PlayerPotionUsed::Install()
    {
        REL::Relocation<uintptr_t> target{REL::VariantID(39604, 40690, 0x6d7b00), REL::VariantOffset(0x15, 0x15, 0x15)};
        auto &trampoline = SKSE::GetTrampoline();
        _PlayerUsePotion = trampoline.write_call<5>(target.address(), PlayerUsePotion);
        logger::info("Installed <{}>", typeid(PlayerPotionUsed).name());
    }

    void PlayerPotionUsed::PlayerUsePotion(uint64_t self, RE::AlchemyItem *alch, uint64_t extralist)
    {
        if (alch->HasKeywordString(Settings::Constants::cure_keyword))
        {
            RE::PlayerCharacter *player = Cache::GetPlayerSingleton();
            Utility::Curses::CleanseCurse(player);
        }
        return _PlayerUsePotion(self, alch, extralist);
    }

    void HighGravityArrows::Install() {
        REL::Relocation<std::uintptr_t> projVTABLE{RE::VTABLE_ArrowProjectile[0]};
        func = projVTABLE.write_vfunc(0xB5, GetGravityArrow);
        logger::info("Installed arrow gravity hook");
    }

    float HighGravityArrows::GetGravityArrow(RE::Projectile* a_this)
    {
        auto shooterRef = a_this->GetProjectileRuntimeData().shooter.get().get();
        auto actorShooter = shooterRef ? shooterRef->As<RE::Actor>() : nullptr;
        if (actorShooter) {
            if (Utility::ActiveEffectHasNewDiseaseKeyword(actorShooter, Settings::Constants::bow_curse_key)) {
                return func(a_this) * 10.0f;
            }
        }        
        return func(a_this);
    }
}
