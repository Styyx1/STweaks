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
        static int32_t Thunk() noexcept;
        static void InstallUpdate();
    private: 
        inline static REL::Relocation<decltype(&Thunk)> func;
        
    };

    class AdjustActiveEffect
    {
    public:
        static void thunk(RE::ActiveEffect* a_this, float a_power, bool a_onlyHostile);
        
        
        static void Install();
        
    private:
        static inline REL::Relocation<decltype(thunk)> func;
    };


} // namespace Hooks


