#pragma once

class Utility : public Singleton<Utility>
{
public:

    static void AddItem(RE::Actor* a, RE::TESBoundObject* item, RE::ExtraDataList* extraList, int count, RE::TESObjectREFR* fromRefr)
    {
        using func_t = decltype(AddItem);
        REL::Relocation<func_t> func{ REL::RelocationID(36525, 37525)};
        return func(a, item, extraList, count, fromRefr);
    }

    static void AddItemPlayer(RE::TESBoundObject* item, int count)
    {
        return AddItem(RE::PlayerCharacter::GetSingleton(), item, nullptr, count, nullptr);
    }

    static int RemoveItemPlayer(RE::TESBoundObject* item, int count)
    {
        using func_t = decltype(RemoveItemPlayer);
        REL::Relocation<func_t> func{ REL::RelocationID(16564, 16919)};
        return func(item, count);
    }

    int get_item_count(RE::Actor* a, RE::TESBoundObject* item)
    {
        if (auto changes = a->GetInventoryChanges()) {
            using func_t = int(RE::InventoryChanges*, RE::TESBoundObject*);
            REL::Relocation<func_t> func{ REL::RelocationID(15868, 16047)};
            return func(changes, item);
        }
        return 0;
    }

    void LogBool(bool bLog) {
        if (bLog) {
            logger::debug("true");
        }
        else
            logger::debug("false");
    }
    void LogItemCountMiscItem(RE::TESObjectMISC* item, int count) {
        logger::debug("player has {} of {} in the inventory", count, item->GetName());
    }
    void LogGlobal(RE::TESGlobal* global)
    {
        logger::debug("lookup successfull global {} with the value {} found", global->GetFormEditorID(), global->value);
    }

    // Credit: D7ry for getWieldingWeapon in ValhallaCombat
    // https://github.com/D7ry/valhallaCombat/blob/48fb4c3b9bb6bbaa691ce41dbd33f096b74c07e3/src/include/Utils.cpp#L10
    inline static RE::TESObjectWEAP* getWieldingWeapon(RE::Actor* a_actor)
    {
        bool dual_wielding = false;
        auto weapon        = a_actor->GetAttackingWeapon();
        if (weapon) {
            dual_wielding = false;
            return weapon->object->As<RE::TESObjectWEAP>();
        }
        auto rhs = a_actor->GetEquippedObject(false);
        if (rhs && rhs->IsWeapon()) {
            dual_wielding = false;
            return rhs->As<RE::TESObjectWEAP>();
        }
        auto lhs = a_actor->GetEquippedObject(true);
        if (lhs && lhs->IsWeapon()) {
            dual_wielding = false;
            return lhs->As<RE::TESObjectWEAP>();
        }

        return nullptr;
    }

    float GetRandomFloat(float a_min, float a_max)
    {
        static std::random_device        rd;
        static std::mt19937              gen(rd());
        std::uniform_real_distribution<float> distrib(a_min, a_max);
        return std::roundf((distrib(gen) * 100)) / 100;
    }

    inline static bool IsDualWielding(RE::Actor* a_actor)
    {
        auto weapon = a_actor->GetAttackingWeapon();
        auto rhs    = a_actor->GetEquippedObject(false);
        auto lhs    = a_actor->GetEquippedObject(true);
        if (weapon && rhs && lhs && lhs->IsWeapon() && rhs->IsWeapon()) {
            logger::debug("dual wielding is active");
            return true;
        }
        else
            return false;
    }

    static std::vector<RE::Actor*> GetNearbyActors(RE::TESObjectREFR* a_ref, float a_radius, bool a_ignorePlayer)
    {
        {
            std::vector<RE::Actor*> result;
            if (const auto processLists = RE::ProcessLists::GetSingleton(); processLists) {
                if (a_ignorePlayer && processLists->numberHighActors == 0) {
                    logger::debug("no process list");
                    return result;
                }

                const auto squaredRadius = a_radius * a_radius;
                const auto originPos     = a_ref->GetPosition();

                result.reserve(processLists->numberHighActors);

                const auto get_actor_within_radius = [&](RE::Actor* a_actor) {
                    if (a_actor && a_actor != a_ref && originPos.GetSquaredDistance(a_actor->GetPosition()) <= squaredRadius) {
                        result.emplace_back(a_actor);
                    }
                    };
                for (auto& actorHandle : processLists->highActorHandles) {
                    const auto actor = actorHandle.get();
                    get_actor_within_radius(actor.get());
                }

                if (!a_ignorePlayer) {
                    get_actor_within_radius(RE::PlayerCharacter::GetSingleton());
                }

                if (!result.empty()) {
                    logger::debug("vector is not empty");
                    return result;
                }
            }
            return result;
        }
    }

    // Credit: KernalsEgg for ApplySpell and IsPermanent
    // extensions
    static bool IsPermanent(RE::MagicItem* item)
    {
        switch (item->GetSpellType()) {
        case RE::MagicSystem::SpellType::kDisease:
        case RE::MagicSystem::SpellType::kAbility:
        case RE::MagicSystem::SpellType::kAddiction: {
            return true;
        }
        default: {
            return false;
        }
        }
    }

    inline static void ApplySpell(RE::Actor* caster, RE::Actor* target, RE::SpellItem* spell)
    {
        if (IsPermanent(spell)) {
            target->AddSpell(spell);
        }
        else {
            caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, false, target, 1.0F, false, 0.0F, nullptr);
        }
    }


};
