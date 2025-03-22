#pragma once
#include "cache.h"
class Utility
{
public:
    static void AddItem(RE::Actor *a, RE::TESBoundObject *item, RE::ExtraDataList *extraList, int count, RE::TESObjectREFR *fromRefr)
    {
        using func_t = decltype(AddItem);
        REL::Relocation<func_t> func{REL::RelocationID(36525, 37525)};
        return func(a, item, extraList, count, fromRefr);
    }

    static void AddItemPlayer(RE::TESBoundObject *item, int count)
    {
        return AddItem(RE::PlayerCharacter::GetSingleton(), item, nullptr, count, nullptr);
    }

    static int RemoveItemPlayer(RE::TESBoundObject *item, int count)
    {
        using func_t = decltype(RemoveItemPlayer);
        REL::Relocation<func_t> func{REL::RelocationID(16564, 16919)};
        return func(item, count);
    }

    static int get_item_count(RE::Actor *a, RE::TESBoundObject *item)
    {
        if (auto changes = a->GetInventoryChanges())
        {
            using func_t = int(RE::InventoryChanges *, RE::TESBoundObject *);
            REL::Relocation<func_t> func{REL::RelocationID(15868, 16047)};
            return func(changes, item);
        }
        return 0;
    }

    static void LogBool(bool bLog)
    {
        if (bLog)
        {
            logger::debug("true");
        }
        else
            logger::debug("false");
    }
    static void LogItemCountMiscItem(RE::TESObjectMISC *item, int count)
    {
        logger::debug("player has {} of {} in the inventory", count, item->GetName());
    }
    static void LogGlobal(RE::TESGlobal *global)
    {
        logger::debug("lookup successfull global {} with the value {} found", global->GetFormEditorID(), global->value);
    }

    // Credit: D7ry for getWieldingWeapon in ValhallaCombat
    // https://github.com/D7ry/valhallaCombat/blob/48fb4c3b9bb6bbaa691ce41dbd33f096b74c07e3/src/include/Utils.cpp#L10
    inline static RE::TESObjectWEAP *getWieldingWeapon(RE::Actor *a_actor)
    {
        bool dual_wielding = false;
        auto weapon = a_actor->GetAttackingWeapon();
        if (weapon)
        {
            dual_wielding = false;
            return weapon->object->As<RE::TESObjectWEAP>();
        }
        auto rhs = a_actor->GetEquippedObject(false);
        if (rhs && rhs->IsWeapon())
        {
            dual_wielding = false;
            return rhs->As<RE::TESObjectWEAP>();
        }
        auto lhs = a_actor->GetEquippedObject(true);
        if (lhs && lhs->IsWeapon())
        {
            dual_wielding = false;
            return lhs->As<RE::TESObjectWEAP>();
        }

        return nullptr;
    }

    static float GetRandomFloat(float a_min, float a_max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distrib(a_min, a_max);
        return std::roundf((distrib(gen) * 100)) / 100;
    }

    inline static bool IsDualWielding(RE::Actor *a_actor)
    {
        auto weapon = a_actor->GetAttackingWeapon();
        auto rhs = a_actor->GetEquippedObject(false);
        auto lhs = a_actor->GetEquippedObject(true);
        if (weapon && rhs && lhs && lhs->IsWeapon() && rhs->IsWeapon())
        {
            logger::debug("dual wielding is active");
            return true;
        }
        else
            return false;
    }

    // https://github.com/powerof3/PapyrusExtenderSSE/blob/640b79d554da4a5392a05107560685621825568e/include/Papyrus/Functions/ObjectReference.h#L662
    static std::vector<RE::Actor *> GetNearbyActors(RE::TESObjectREFR *a_ref, float a_radius, bool a_ignorePlayer)
    {
        {
            std::vector<RE::Actor *> result;
            if (const auto processLists = RE::ProcessLists::GetSingleton(); processLists)
            {
                if (a_ignorePlayer && processLists->numberHighActors == 0)
                {
                    logger::debug("no process list");
                    return result;
                }

                const auto squaredRadius = a_radius * a_radius;
                const auto originPos = a_ref->GetPosition();

                result.reserve(processLists->numberHighActors);

                const auto get_actor_within_radius = [&](RE::Actor *a_actor)
                {
                    if (a_actor && a_actor != a_ref && originPos.GetSquaredDistance(a_actor->GetPosition()) <= squaredRadius)
                    {
                        result.emplace_back(a_actor);
                    }
                };
                for (auto &actorHandle : processLists->highActorHandles)
                {
                    const auto actor = actorHandle.get();
                    get_actor_within_radius(actor.get());
                }

                if (!a_ignorePlayer)
                {
                    get_actor_within_radius(RE::PlayerCharacter::GetSingleton());
                }

                if (!result.empty())
                {
                    logger::debug("vector is not empty");
                    return result;
                }
            }
            return result;
        }
    }


     static bool ActorHasActiveEffect(RE::Actor* a_actor, RE::EffectSetting* a_effect)
    {
        auto activeEffects = a_actor->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting* setting       = nullptr;
        if (!activeEffects->empty()) {
            for (RE::ActiveEffect* effect : *activeEffects) {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive)) {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting) {
                        if (setting == a_effect) {
                            return true;
                        }
                    }
                }
            }
        } 
        return false;
    }

    // Credit: KernalsEgg for ApplySpell and IsPermanent
    // extensions
    static bool IsPermanent(RE::MagicItem *item)
    {
        switch (item->GetSpellType())
        {
        case RE::MagicSystem::SpellType::kDisease:
        case RE::MagicSystem::SpellType::kAbility:
        case RE::MagicSystem::SpellType::kAddiction:
        {
            return true;
        }
        default:
        {
            return false;
        }
        }
    }

    inline static void ApplySpell(RE::Actor *caster, RE::Actor *target, RE::SpellItem *spell)
    {
        if (IsPermanent(spell))
        {
            target->AddSpell(spell);
        }
        else
        {
            caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, false, target, 1.0F, false, 0.0F, nullptr);
        }
    }

    // https://github.com/colinswrath/BladeAndBlunt/blob/main/include/Conditions.h
    static bool HasSpell(RE::Actor *actor, RE::SpellItem *spell)
    {
        using func_t = decltype(&Utility::HasSpell);

        REL::Relocation<func_t> func{Cache::HasSpellAddress};

        return func(actor, spell);
    }
    inline static REL::Relocation<decltype(HasSpell)> _HasSpell;

    static bool IsMoving(RE::Actor *a_actor)
    {
        auto actorState = a_actor->AsActorState();
        return (static_cast<bool>(actorState->actorState1.movingForward) || static_cast<bool>(actorState->actorState1.movingBack) || static_cast<bool>(actorState->actorState1.movingLeft) || static_cast<bool>(actorState->actorState1.movingRight));
    }

    static bool IsAttacking(RE::Actor *actor)
    {
        using func_t = decltype(&IsAttacking);
        REL::Relocation<func_t> func{Cache::IsAttackingAddress};
        return func(actor);
    }

    inline static REL::Relocation<decltype(IsAttacking)> _IsAttacking;

    static bool IsBlocking(RE::Actor *actor)
    {
        using func_t = decltype(&IsBlocking);
        REL::Relocation<func_t> func{Cache::IsBlockingAddress};
        return func(actor);
    }
    inline static REL::Relocation<decltype(IsBlocking)> _IsBlocking;

    static bool GetMount(RE::Actor *a_actor, RE::ActorPtr *a_mountOut)
    {
        using func_t = decltype(&GetMount);
        REL::Relocation<func_t> func{REL::RelocationID(37757, 38702)};
        return func(a_actor, a_mountOut);
    }

    static inline bool IsPowerAttacking(RE::Actor *actor)
    {
        if (auto high = actor->GetHighProcess())
        {
            if (const auto attackData = high->attackData)
            {
                auto flags = attackData->data.flags;

                if (flags && flags.any(RE::AttackData::AttackFlag::kPowerAttack))
                {
                    return true;
                }
            }
        }
        return false;
    }

    struct Actor
    {
        static inline bool jumpHeightPlayer(RE::PlayerCharacter *player, float height_mod)
        {
            if (!player->IsInMidair())
            {
                player->GetCharController()->jumpHeight *= height_mod;
                return true;
            }
            return false;
        }
        static inline bool applyDebuffs(RE::PlayerCharacter *player, RE::SpellItem *a_debuff, bool a_check)
        {
            if (a_check)
            {
                ApplySpell(player, player, a_debuff);
                return true;
            }
            else
                return false;
        }
    };
};
