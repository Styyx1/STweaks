#pragma once

namespace Hooks
{
    void Install();

    class CombatHit
    {
    public:
        static void Install();

    private:
        static void RandomiseDamage(RE::Actor* a_this, RE::HitData* a_hitData);
        static void CHit(RE::Actor* a_this, RE::HitData* a_hitData);
        static inline REL::Relocation<decltype(&CHit)> _originalCall;
    };

    class MainUpdate
    {
    public:

        static void PlayerUpdate(RE::PlayerCharacter* p, float a_delta);

        static void InstallUpdate();
        static inline float frameCount = 0;
        static inline bool can_debuff = true;
        static inline std::uint32_t lightLevelCount;
    private: 
        inline static REL::Relocation<decltype(&PlayerUpdate)> func;
        
    };

    class AdjustActiveEffect
    {
    public:
        static void thunk(RE::ActiveEffect* a_this, float a_power, bool a_onlyHostile);
        
        
        static void Install();
        
    private:
        static inline REL::Relocation<decltype(thunk)> func;
    };

    class JumpHeight
    {
    public:
        static float JumpHeightGetScale(RE::TESObjectREFR* refr);
        static void Install();

    private:
        static inline REL::Relocation<decltype(JumpHeightGetScale)> func;
    };

    class ActorUpdateHook
    {
    public: 
        static void InstallUpdateActor();
    private:
        static bool done;
        static bool once;
        static void ActorUpdate(RE::Character* a_this, float a_delta);
        static inline REL::Relocation<decltype(&ActorUpdate)> _ActorUpdate;
    };

    class DealtMeleeDamage
    {
    public:
        static void InstallMeleeDamageHook();
    private:
        static float MeleeDamage(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow);
        static inline REL::Relocation<decltype(&MeleeDamage)> _MeleeDamageCall;
    };

    // unused, might revisit later

    class OverEncumbered
    {
    public:        
        static void Install();
    private:
        static bool IsOverEncumberedEX(RE::Actor* a_actor);
        static inline REL::Relocation<decltype(IsOverEncumberedEX)> func;        
    };


    static RE::ActorValue LookupActorValueByName(const char* av_name)
    {
        // SE: 0x3E1450, AE: 0x3FC5A0, VR: ---
        using func_t = decltype(&LookupActorValueByName);
        REL::Relocation<func_t> func{ REL::RelocationID(26570, 27203) };
        return func(av_name);
    }

    //returns from 0 - 1.0 1 == 100%
    static float GetActorValuePercentage(RE::Actor* a_actor, RE::ActorValue a_av)
    {
        auto& trampoline = SKSE::GetTrampoline();
        using func_t = decltype(&GetActorValuePercentage);
        REL::Relocation<func_t> target{ RELOCATION_ID(36347, 37337)};
        return target(a_actor, a_av);
    }  
} // namespace Hooks


