
#include "WeaponParry_Hooks.h"
#include "WeaponParry_Functions.h"

#include "PrecisionAPI.h"
PRECISION_API::IVPrecision2* g_Precision = nullptr;

#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\MaxsuWeaponParry.ini"

int precisionParryFlag = -1; // With Precision, all hits are queued on the main thread, so this is thread-safe

PRECISION_API::PreHitCallbackReturn PrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData)
{
	if (a_precisionHitData.attacker && a_precisionHitData.target) {
		auto hit_causer = a_precisionHitData.attacker;
		if (auto hit_target = a_precisionHitData.target->As<RE::Actor>()) {
			if (MaxsuWeaponParry::ParryCheck::ShouldParry(hit_causer, hit_target)) {
				logger::debug("Weapon Swing Parry Trigger! Attacker ID is {:x},  target ID is {:x}", hit_causer->GetFormID(), hit_target->GetFormID());
				hit_target->SetGraphVariableBool("IsBlocking", true);
				MaxsuWeaponParry::ParryCheck::SetVariables(hit_causer, hit_target);
				precisionParryFlag = 1;	 // set
			}
		}
	}
	PRECISION_API::PreHitCallbackReturn toReturn;
	return toReturn;
}

void PrecisionPostHit(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData&)
{
	if (a_precisionHitData.attacker && a_precisionHitData.target) {
		auto hit_causer = a_precisionHitData.attacker;
		if (precisionParryFlag) {
			if (auto hit_target = a_precisionHitData.target->As<RE::Actor>()) {
				bool recoil = false;
				if (hit_causer->GetGraphVariableBool("IsRecoiling", recoil) && recoil) {
					logger::debug("Attacker is recoiling, ID is {:x}", hit_causer->GetFormID());
					MaxsuWeaponParry::MeleeHitHook::SendRecoil(hit_target);
				}
				hit_target->SetGraphVariableBool("IsBlocking", false);
				MaxsuWeaponParry::ParryCheck::ResetVariables(hit_causer, hit_target);
			}
		}
	}
	precisionParryFlag = -1; // clear
}


void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		g_Precision = reinterpret_cast<PRECISION_API::IVPrecision2*>(PRECISION_API::RequestPluginAPI());
		if (g_Precision) {
			logger::info("Obtained Precision API");
			g_Precision->AddPreHitCallback(SKSE::GetPluginHandle(), PrecisionPreHit);
			g_Precision->AddPostHitCallback(SKSE::GetPluginHandle(), PrecisionPostHit);
		} else {
			logger::info("Unable to acquire Precision API");
		}
		break;
	}
}

void Load()
{
	auto IsEnableDebugMessage = GetPrivateProfileIntA("Main", "EnableDebugMessage", 0, SETTINGFILE_PATH);

	if (IsEnableDebugMessage == 1) {
		logger::info("Enable Debug Message!");
	}

	MaxsuWeaponParry::MeleeHitHook::InstallHook();

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
}
