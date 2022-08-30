#include "HitEventHandler.h"

#include "WeaponParry_Functions.h"


bool HitEventHandler::SendRecoil(RE::Actor* a_actor)
{
	bool result = false;

	if (a_actor && a_actor->GetGraphVariableBool("IsRecoiling", result) && result == false) {
		a_actor->NotifyAnimationGraph("recoilStart");
		return true;
	}

	return false;
}

void HitEventHandler::PreProcessHit(RE::Actor* a_target, RE::HitData* a_hitData)
{
	auto hit_causer = a_hitData->aggressor ? a_hitData->aggressor.get().get() : nullptr;
	auto hit_target = a_target;

	logger::debug("MeleeHit Hook Trigger!");

	if (hit_causer && hit_target && MaxsuWeaponParry::ParryCheck::ShouldParry(hit_causer, hit_target)) {
		logger::debug(FMT_STRING("Weapon Swing Parry Trigger! Attacker ID is {:x},  target ID is {:x}"), hit_causer->GetFormID(), hit_target->GetFormID());

		hit_target->SetGraphVariableBool("Isblocking", true);
		HitEventHandler::Hooks::ProcessHitEvent::func(a_target, a_hitData);

		bool recoil = false;
		if (hit_causer->GetGraphVariableBool("IsRecoiling", recoil) && recoil) {
			logger::debug(FMT_STRING("Attacker is recoling, ID is {:x}"), hit_causer->GetFormID());
			SendRecoil(hit_target);
		}

		hit_target->SetGraphVariableBool("Isblocking", false);
	} else {
		HitEventHandler::Hooks::ProcessHitEvent::func(a_target, a_hitData);
	}
}
