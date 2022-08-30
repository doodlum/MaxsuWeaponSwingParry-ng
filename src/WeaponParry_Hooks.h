#pragma once

#include "PrecisionAPI.h"

namespace MaxsuWeaponParry
{
	class MeleeHitHook
	{

	public:

		[[nodiscard]] static MeleeHitHook* GetSingleton()
		{
			static MeleeHitHook singleton;
			return std::addressof(singleton);
		}

		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);

			REL::Relocation<std::uintptr_t> OnMeleeHitBase{ REL::RelocationID(37650, 38603) };
			auto& trampoline = SKSE::GetTrampoline();
			_OnMeleeHit = trampoline.write_call<5>(OnMeleeHitBase.address() + REL::Relocate(0x38B, 0x45A), OnMeleeHit);

			logger::info("OnMeleeHit Hook Install!");
		}

		static bool SendRecoil(RE::Actor* thisactor)
		{
			bool result = false;

			if (thisactor && thisactor->GetGraphVariableBool("IsRecoiling", result) && result == false) {
				thisactor->NotifyAnimationGraph("recoilStart");
				return true;
			}

			return false;
		}


	private:
		static void OnMeleeHit(RE::Actor* attacker, RE::Actor* target, std::int64_t a_int1, bool a_bool, void* a_unkptr);

		static inline REL::Relocation<decltype(OnMeleeHit)> _OnMeleeHit;

	};

}
