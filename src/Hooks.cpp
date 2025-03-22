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

        if (enable_foll_change.GetValue())
        {
            DealtMeleeDamage::InstallMeleeDamageHook();
        }

        if (use_requiem_stamina.GetValue())
        {
            EffectEndHooks::InstallEffectEndHooks();
        }

        logger::debug("installed all hooks");
        logger::info("--------------------------------");
    }
    void CombatHit::Install()
    {
        // 1.6.1170: 1406BAD18 - 38627 //VR offsets are just SE offsets because i don't VR and didn't want to leave them blank.
        auto &trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0)};
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
        logger::info("Installed damage taken hook");
    }
    void CombatHit::RandomiseDamage(RE::Actor *a_this, RE::HitData *a_hitData)
    {
        float remaining = a_hitData->totalDamage;
        if (a_hitData->weapon && !a_hitData->weapon->IsHandToHandMelee())
        {
            logger::debug("----------------------------------------------------");
            logger::debug("started hooked damage calc for {} you got hit by: {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName());
            // float rand_mult = Utility::GetRandomFloat(Settings::CalcPerc(Settings::damage_range_weapon, false), Settings::CalcPerc(Settings::damage_range_weapon, true));
            float rand_mult = Utility::GetRandomFloat(Settings::Forms::CalcPerc(weapon_lower_range.GetValue(), false), Settings::Forms::CalcPerc(weapon_upper_range.GetValue(), true));
            logger::debug("multiplier is {}", rand_mult);
            remaining *= rand_mult;
            logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
            a_hitData->totalDamage = remaining;
            return _originalCall(a_this, a_hitData);
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
                if (Settings::Forms::stamina_swimm_player)
                    player->RemoveSpell(Settings::Forms::stamina_swimm_player);
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
                case 2:
                    if (player->AsActorState()->IsSwimming())
                    {
                        if (!Utility::HasSpell(player, Settings::Forms::exhaustion_swim) && use_requiem_stamina.GetValue())
                        {
                            player->AddSpell(Settings::Forms::exhaustion_swim);
                        }
                        if (!Utility::HasSpell(player, Settings::Forms::stamina_swimm_player) && use_requiem_stamina.GetValue())
                        {
                            Utility::ApplySpell(player, player, Settings::Forms::stamina_swimm_player);
                        }
                    }
                    else
                    {
                        if (Utility::HasSpell(player, Settings::Forms::exhaustion_swim))
                        {
                            player->RemoveSpell(Settings::Forms::exhaustion_swim);
                        }
                        if (Utility::HasSpell(player, Settings::Forms::stamina_swimm_player))
                        {
                            player->RemoveSpell(Settings::Forms::stamina_swimm_player);
                        }
                    }
                    break;
                case 3:
                    if (Utility::IsAttacking(player) && !Utility::IsPowerAttacking(player))
                    {
                        if (!Utility::HasSpell(player, Settings::Forms::stamina_attack_player))
                        {
                            player->AddSpell(Settings::Forms::stamina_attack_player);
                        }
                    }
                    else
                    {
                        if (Utility::HasSpell(player, Settings::Forms::stamina_attack_player))
                        {
                            player->RemoveSpell(Settings::Forms::stamina_attack_player);
                        }

                        if (Utility::IsBlocking(player))
                        {
                            if (!Utility::HasSpell(player, Settings::Forms::stamina_block_spell))
                            {
                                player->AddSpell(Settings::Forms::stamina_block_spell);
                            }
                        }
                        else
                        {
                            if (Utility::HasSpell(player, Settings::Forms::stamina_block_spell))
                            {
                                player->RemoveSpell(Settings::Forms::stamina_block_spell);
                            }
                        }
                    }
                    break;
                case 4:
                    if (player->AsActorState()->IsSprinting())
                    {
                        if (!isSprinting)
                        {
                            // Player just started sprinting, record the time
                            sprintStartTime = std::chrono::steady_clock::now();
                            isSprinting = true;
                        }

                        // Check elapsed time
                        auto now = std::chrono::steady_clock::now();
                        double elapsedSeconds = std::chrono::duration<double>(now - sprintStartTime).count();

                        if (elapsedSeconds >= 2.0)
                        { // 2 seconds sprinting
                            DoBullrush(player);
                        }
                    }
                    else
                    {
                        // Reset when the player stops sprinting
                        isSprinting = false;
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
    inline void MainUpdate::LaunchArrow(RE::Actor *a_actor, RE::TESAmmo *a_ammo, RE::TESObjectWEAP *a_weapon, RE::BSFixedString a_nodeName, std::int32_t a_source, RE::TESObjectREFR *a_target, RE::AlchemyItem *a_poison)
    {

        SKSE::GetTaskInterface()->AddTask([a_actor, a_ammo, a_weapon, a_nodeName, a_source, a_target, a_poison]()
                                          {
			RE::NiAVObject* fireNode = nullptr;
			auto            root = a_actor->GetCurrent3D();
			switch (a_source) {
			case -1:
				{
					if (!a_nodeName.empty()) {
						if (root) {
							fireNode = root->GetObjectByName(a_nodeName);
						}
					} else {
						if (const auto currentProcess = a_actor->GetActorRuntimeData().currentProcess) {
							const auto& biped = a_actor->GetCurrentBiped();
							fireNode = a_weapon->IsCrossbow() ? currentProcess->GetMagicNode(biped) : currentProcess->GetWeaponNode(biped);
						} else {
							fireNode = a_weapon->GetFireNode(root);
						}
					}
				}
				break;
			case 0:
				fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcLMagicNode) : nullptr;
				break;
			case 1:
				fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcRMagicNode) : nullptr;
				break;
			case 2:
				fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcHeadMagicNode) : nullptr;
				break;
			default:
				break;
			}
			RE::NiPoint3                  origin;
			RE::Projectile::ProjectileRot angles{};
			if (fireNode) {
				if (a_actor->IsPlayerRef()) {
					angles.z = a_actor->GetHeading(false);

					float tiltUpAngle;
					if (a_ammo->IsBolt()) {
						tiltUpAngle = RE::INISettingCollection::GetSingleton()->GetSetting("f1PBoltTiltUpAngle:Combat")->GetFloat();
					} else {
						tiltUpAngle = RE::INISettingCollection::GetSingleton()->GetSetting(RE::PlayerCamera::GetSingleton()->IsInFirstPerson() ? "f1PArrowTiltUpAngle:Combat" : "f3PArrowTiltUpAngle:Combat")->GetFloat();
					}
					angles.x = a_actor->GetAngleX() - (RE::deg_to_rad(tiltUpAngle));

					origin = fireNode->world.translate;
					a_actor->Unk_117(origin);
				} else {
					origin = fireNode->world.translate;
					a_actor->Unk_A0(fireNode, angles.x, angles.z, origin);
				}
			} else {
				origin = a_actor->GetPosition();
				origin.z += 96.0f;

				angles.x = a_actor->GetAimAngle();
				angles.z = a_actor->GetAimHeading();
			}
			RE::ProjectileHandle       handle{};
			RE::Projectile::LaunchData launchData(a_actor, origin, angles, a_ammo, a_weapon);

			launchData.desiredTarget = a_target;
			launchData.poison = a_poison;
			launchData.enchantItem = a_weapon->formEnchanting;

			RE::Projectile::Launch(&handle, launchData);

			RE::BGSSoundDescriptorForm* sound = nullptr;
			std::uint32_t               flags = 0;
			if (a_actor->IsPlayerRef() && a_weapon->attackSound2D) {
				sound = a_weapon->attackSound2D;
				flags = 18;
			} else {
				sound = a_weapon->attackSound;
				flags = 16;
			}
			if (sound) {
				RE::BSSoundHandle soundHandle;
				RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(soundHandle, sound, flags);
				soundHandle.SetPosition(origin);
				soundHandle.Play();
			} });
    }
    void MainUpdate::DoBullrush(RE::PlayerCharacter *a_player)
    {
        if (a_player->IsOnMount())
        {
            RE::ActorPtr mount = nullptr;
            Utility::GetMount(a_player, &mount);
            if (mount)
            {
                Utility::ApplySpell(a_player, mount.get(), Settings::Forms::bullrush_horse_spell);
            }
        }
        else
        {
            Utility::ApplySpell(a_player, a_player, Settings::Forms::bullrush_player_spell);
        }
    }
    void MainUpdate::InstallUpdate()
    {
        REL::Relocation<std::uintptr_t> PlayerVTBL{RE::VTABLE_PlayerCharacter[0]};
        func = PlayerVTBL.write_vfunc(0xAD, PlayerUpdate);
        logger::info("Installed player update hook");
    }

    void MainUpdate::HandleSwimming(RE::Actor *a_actor)
    {
        if (a_actor->AsActorState()->IsSwimming())
        {
            if (!Utility::HasSpell(a_actor, Settings::Forms::stamina_swimm_player) && use_requiem_stamina.GetValue())
            {
                a_actor->AddSpell(Settings::Forms::stamina_swimm_player);
            }
        }
        else if (Utility::HasSpell(a_actor, Settings::Forms::stamina_swimm_player))
        {
            a_actor->RemoveSpell(Settings::Forms::stamina_swimm_player);
        }
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

        if (use_requiem_stamina.GetValue())
        {
            Utility::ApplySpell(actor, actor, Settings::Forms::stamina_spell_jump);
        }

        if (actor->IsSneaking())
        {
            scale *= sneak_height_modifier.GetValue();
        }
        float ju_modifier = (float)sqrt(1.0 / mass);

        return scale *= ju_modifier;
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
    float DealtMeleeDamage::MeleeDamage(void *_weap, RE::ActorValueOwner *a, float DamageMult, char isbow)
    {
        RE::PlayerCharacter *player = Cache::GetPlayerSingleton();
        RE::Actor *actor = skyrim_cast<RE::Actor *>(a);
        auto dam = _MeleeDamageCall(_weap, a, DamageMult, isbow);
        logger::debug("before if in melee damage hook for actor: {}", actor->GetDisplayFullName());

        if (actor && actor->IsPlayerTeammate() && !actor->IsCommandedActor() || actor && actor == player)
        {
            logger::debug("melee damage active for actor: {}", actor->GetDisplayFullName());
            if (player->GetPlayerRuntimeData().teammateCount > 0)
            {
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
        return dam;
    }
    void EffectEndHooks::InstallEffectEndHooks()
    {
        _valModEffEnd = REL::Relocation<uintptr_t>{RE::ValueModifierEffect::VTABLE[0]}.write_vfunc(21, ValueModifierEffectEnd);
        _dualValModEffEnd = REL::Relocation<uintptr_t>{RE::DualValueModifierEffect::VTABLE[0]}.write_vfunc(21, DualValueModEffectEnd);

        logger::info("Installed <{}>", typeid(EffectEndHooks).name());
    }
    void EffectEndHooks::ValueModifierEffectEnd(RE::ValueModifierEffect *a_this)
    {
        _valModEffEnd(a_this);
    }
    void EffectEndHooks::DualValueModEffectEnd(RE::DualValueModifierEffect *a_this)
    {
        _dualValModEffEnd(a_this);
        if (a_this->spell == Settings::Forms::exhaustion_swim)
        {
            logger::debug("swim exhaustion ended, start stamina drain spell");
            auto actor = skyrim_cast<RE::Actor *>(a_this->target);
            if (actor)
            {
                Utility::ApplySpell(actor, actor, Settings::Forms::stamina_swimm_player);
            }
        }
    }
} // namespace Hooks
