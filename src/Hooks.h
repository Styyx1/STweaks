#pragma once

namespace Hooks
{
    void Install();

    class CombatHit : public Singleton<CombatHit>
    {
    public:
        static void Install();

    private:
        static void RandomiseDamage(RE::Actor* a_this, RE::HitData* a_hitData);
        static void CHit(RE::Actor* a_this, RE::HitData* a_hitData);
        static inline REL::Relocation<decltype(&CHit)> _originalCall;
    };

    class MainUpdate : public Singleton<MainUpdate>
    {
    public:
        static i32 Thunk() noexcept;
        static void InstallUpdate();
    private: 
        inline static REL::Relocation<decltype(&Thunk)> func;
        
    };

} // namespace Hooks


