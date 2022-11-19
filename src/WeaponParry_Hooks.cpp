#include "WeaponParry_Hooks.h"
#include "WeaponParry_Functions.h"

namespace MaxsuWeaponParry
{
	void MeleeHitHook::OnMeleeHit(RE::Actor* hit_causer, RE::Actor* hit_target, std::int64_t a_int1, bool a_bool, void* a_unkptr)
	{
		logger::debug("MeleeHit Hook Trigger!");

		 if (ParryCheck::ShouldParry(hit_causer, hit_target)){
			 logger::debug(FMT_STRING("Weapon Swing Parry Trigger! Attacker ID is {:x},  target ID is {:x}"), hit_causer->GetFormID(), hit_target->GetFormID());

			 hit_target->SetGraphVariableBool("IsBlocking", true);
			 MaxsuWeaponParry::ParryCheck::SetVariables(hit_causer, hit_target);
			 _OnMeleeHit(hit_causer, hit_target, a_int1, a_bool, a_unkptr);

			 bool recoil = false;
			 if (hit_causer->GetGraphVariableBool("IsRecoiling", recoil) && recoil) {
				 logger::debug(FMT_STRING("Attacker is recoling, ID is {:x}"), hit_causer->GetFormID());
				 SendRecoil(hit_target);
			 }

			 hit_target->SetGraphVariableBool("IsBlocking", false);
			 MaxsuWeaponParry::ParryCheck::ResetVariables(hit_causer, hit_target);
			 return; 
		 }

		return _OnMeleeHit(hit_causer, hit_target, a_int1, a_bool, a_unkptr);
	}
}
