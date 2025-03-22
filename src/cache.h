#pragma once

namespace Cache
{
	inline uintptr_t IsAttackingAddress;
	inline uintptr_t IsBlockingAddress;
	inline uintptr_t HasSpellAddress;
	inline uintptr_t PlayerSingletonAddress;

	// https://github.com/colinswrath/BladeAndBlunt/blob/main/include/Cache.h
	inline void CacheAddLibAddresses()
	{
		//1.6 = 38590
		//1.5.97 = 37637
		IsAttackingAddress = REL::RelocationID(37637, 38590).address();

		//1.6 = 37952
		//1.5.97 = 36927
		IsBlockingAddress = REL::RelocationID(36927, 37952).address();
		// 1.6 = 38782
		// 1.5.97 = 37828
		HasSpellAddress = REL::RelocationID(37828, 38782).address();

		// 1.6 = 403521
		// 1.5 = 517014
		PlayerSingletonAddress = REL::RelocationID(517014, 403521).address();
	}

	inline RE::PlayerCharacter *GetPlayerSingleton()
	{
		REL::Relocation<RE::NiPointer<RE::PlayerCharacter> *> singleton{PlayerSingletonAddress};
		return singleton->get();
	}
}