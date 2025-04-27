#include "papyrus.h"



namespace Papyrus {
	bool BindAll(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		logger::info("{:*^30}", "FUNCTIONS");

		Functions::Bind(*a_vm);

		return true;
	}

	namespace Functions {

		void Bind(VM& a_vm)
		{
			constexpr auto script = "STweaksFunctions"sv;

			a_vm.RegisterFunction("GetVersion", script, GetVersion, true);
			a_vm.RegisterFunction("CleanseCurseActor", script, CleanseCurseActor, true);
			logger::info("Registered STweaks functions");
		}

		std::int32_t GetVersion(STATIC_ARGS) {
			return kVersion;
		}

		void CleanseCurseActor(STATIC_ARGS, RE::Actor* a_actor) {

			if (!a_actor) {
				a_vm->TraceStack("Actor is None", a_stackID);
				return;
			}
			Utility::Curses::CleanseCurse(a_actor);		
		}
	}



}


