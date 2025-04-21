#pragma once
#include "cache.h"
class Utility
{
public:

    enum class GameDifficulty : std::int32_t {
        Novice = 0,
        Apprentice = 1,
        Adept = 2,
        Expert = 3,
        Master = 4,
        Legendary = 5
    };

    static RE::Setting* get_gmst(const char* a_setting)
    {
        return RE::GameSettingCollection::GetSingleton()->GetSetting(a_setting);
    }

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

    static bool ActorHasActiveEffect(RE::Actor *a_actor, RE::EffectSetting *a_effect)
    {
        auto activeEffects = a_actor->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting *setting = nullptr;
        if (!activeEffects->empty())
        {
            for (RE::ActiveEffect *effect : *activeEffects)
            {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive))
                {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting)
                    {
                        if (setting == a_effect)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    static bool ActiveEffectHasNewDiseaseKeyword(RE::Actor *a_actor, std::string a_keyword)
    {
        auto activeEffects = a_actor->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting *setting = nullptr;
        if (!activeEffects->empty())
        {
            for (RE::ActiveEffect *effect : *activeEffects)
            {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive))
                {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting)
                    {
                        if (setting->HasKeywordString(a_keyword))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    inline static bool IsQuestItem(const RE::TESObjectREFR* a_ref)
    {
        if (const auto xAliases = a_ref->extraList.GetByType<RE::ExtraAliasInstanceArray>(); xAliases) {
            RE::BSReadLockGuard locker(xAliases->lock);

            return std::ranges::any_of(xAliases->aliases, [](const auto& aliasData) {
                const auto alias = aliasData ? aliasData->alias : nullptr;
                return alias && alias->IsQuestObject();
                });
        }

        return a_ref->HasQuestObject();
    }

    static bool ActorHasEffectWithArchetype(RE::Actor *a_actor, RE::EffectArchetype a_archetype)
    {
        auto activeEffects = a_actor->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting *setting = nullptr;
        if (!activeEffects->empty())
        {
            for (RE::ActiveEffect *effect : *activeEffects)
            {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive))
                {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting)
                    {
                        if (setting->GetArchetype() == a_archetype)
                        {
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

    

    class Timer
    {
    public:
        void Start()
        {
            if (!running) {
                startTime = std::chrono::steady_clock::now();
                running = true;
            }
        }

        void Stop()
        {
            running = false;
        }

        void Reset()
        {
            startTime = std::chrono::steady_clock::now();
        }

        double ElapsedSeconds() const
        {
            if (!running) {
                return 0.0;
            }
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<double>(now - startTime).count();
        }

        bool IsRunning() const
        {
            return running;
        }

    private:
        std::chrono::steady_clock::time_point startTime{};
        bool running{false};
    };

    struct Curses
    {
        static inline std::unordered_map<RE::Actor*, RE::SpellItem*> active_curses;
        static inline std::unordered_map<RE::Actor*, Timer> curse_swap_timers;

        static RE::SpellItem* GetRandomSpell(const std::vector<RE::SpellItem*>& spells)
        {
            if (spells.empty()) {
                return nullptr;
            }

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<std::size_t> dist(0, spells.size() - 1);

            return spells[dist(gen)];
        }

        static bool ShouldApplyCurse(float chancePercent)
        {
            if (chancePercent <= 0.0f)
                return false;
            if (chancePercent >= 100.0f)
                return true;

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0.0f, 100.0f);

            return dist(gen) < chancePercent;
        }

        static void ApplyRandomCurse(RE::Actor* a_actor, const std::vector<RE::SpellItem*>& curses)
        {
            double CURSE_SWAP_COOLDOWN = Settings::Values::curse_swap_cooldown.GetValue(); // 60 seconds
            logger::debug("curse vector size is: {}", curses.size());
            if (!a_actor || curses.empty())
                return;

            // If actor already has any curse, swap instead of reapplying
            for (auto curse : curses) {
                if (curse && a_actor->HasSpell(curse)) {
                    auto& timer = curse_swap_timers[a_actor];
                    if (!timer.IsRunning() || timer.ElapsedSeconds() >= CURSE_SWAP_COOLDOWN) {
                        RE::SpellItem* newCurse = GetRandomSpell(curses);
                        if (newCurse && newCurse != curse) {
                            SwapCurse(a_actor, newCurse);
                            timer.Reset();
                            timer.Start();
                            return;
                        }
                        else {
                            logger::debug("{} already has this curse: {}", a_actor->GetName(), curse->GetName());
                        }
                        return;
                    }
                    else {
                        logger::debug("{} tried to swap a curse too soon. {:.1f}s remaining", a_actor->GetName(), CURSE_SWAP_COOLDOWN - timer.ElapsedSeconds());
                    }
                    return;
                }
            }

            RE::SpellItem* curse_to_add = GetRandomSpell(curses);
            if (curse_to_add) {
                ApplySpell(a_actor, a_actor, curse_to_add);
                Curses::active_curses[a_actor] = curse_to_add;
                curse_swap_timers[a_actor].Start(); 
                logger::info("Applied curse {} to {}", curse_to_add->GetName(), a_actor->GetName());
            } else {
                logger::warn("Failed to apply curse: selected spell was null");
            }

        }

        static void CleanseCurse(RE::Actor* a_actor)
        {
            auto it = active_curses.find(a_actor);
            if (it != active_curses.end() && it->second) {
                a_actor->RemoveSpell(it->second);
                active_curses.erase(it);
                logger::info("{}'s curse has been cleansed", a_actor->GetName());
            }
        }
        static void SwapCurse(RE::Actor* actor, RE::SpellItem* newCurse)
        {
            if (!actor || !newCurse)
                return;

            // If there's already a curse, dispel it � effectEnd will handle restoration
            for (auto& curse : Settings::Forms::curse_list) {
                if (curse && actor->HasSpell(curse)) {
                    CleanseCurse(actor);
                    break;
                }
            }
            // Apply the new curse
            Utility::ApplySpell(actor, actor, newCurse);
            active_curses[actor] = newCurse;
            logger::info("{} has been cursed with {}", actor->GetName(), newCurse->GetName());
        }

        inline static void PopulateActiveCursesAfterLoad(RE::Actor* a_actor)
        {
            class Visitor : public RE::Actor::ForEachSpellVisitor
            {
            public:
                RE::BSContainer::ForEachResult Visit(RE::SpellItem* a_spell) override
                {
                    for (auto& curse : Settings::Forms::curse_list) {
                        if (a_spell == curse) {
                            foundCurse = curse;
                            return RE::BSContainer::ForEachResult::kStop; // found one, done
                        }
                    }
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                RE::SpellItem* foundCurse{ nullptr };
            } visitor;

            a_actor->VisitSpells(visitor);

            if (visitor.foundCurse) {
                active_curses[a_actor] = visitor.foundCurse;
                logger::debug("Restored active curse {} on {}", visitor.foundCurse->GetName(), a_actor->GetName());
            }
            else {
                Utility::Curses::CleanseCurse(a_actor);
            }
        }
    };

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
