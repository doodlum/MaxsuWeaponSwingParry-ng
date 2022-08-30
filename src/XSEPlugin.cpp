
#include "WeaponParry_Hooks.h"
#include "WeaponParry_Functions.h"

#include "PrecisionAPI.h"
PRECISION_API::IVPrecision2* g_Precision = nullptr;

#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\MaxsuWeaponParry.ini"

int precisionParryFlag = -1;

PRECISION_API::PreHitCallbackReturn PrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData)
{
	if (a_precisionHitData.attacker && a_precisionHitData.target) {
		auto hit_causer = a_precisionHitData.attacker;
		if (auto hit_target = a_precisionHitData.target->As<RE::Actor>()) {
			if (MaxsuWeaponParry::ParryCheck::ShouldParry(hit_causer, hit_target)) {
				logger::debug(FMT_STRING("Weapon Swing Parry Trigger! Attacker ID is {:x},  target ID is {:x}"), hit_causer->GetFormID(), hit_target->GetFormID());
				hit_target->SetGraphVariableBool("IsBlocking", true);
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
					logger::debug(FMT_STRING("Attacker is recoling, ID is {:x}"), hit_causer->GetFormID());
					MaxsuWeaponParry::MeleeHitHook::SendRecoil(hit_target);
				}
				hit_target->SetGraphVariableBool("IsBlocking", false);
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

void Init()
{
	auto IsEnableDebugMessage = GetPrivateProfileIntA("Main", "EnableDebugMessage", 0, SETTINGFILE_PATH);

	if (IsEnableDebugMessage == 1) {
		logger::info("Enable Debug Message!");
	}

	MaxsuWeaponParry::MeleeHitHook::InstallHook();

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
}

void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = GetPrivateProfileIntA("Main", "EnableDebugMessage", 0, SETTINGFILE_PATH) == 1 ? spdlog::level::debug : spdlog ::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {};
#endif

	InitializeLog();

	logger::info("Loaded plugin");

	SKSE::Init(a_skse);

	Init();

	return true;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName("PluginName");
	v.PluginVersion({ 1, 0, 0, 0 });
	v.UsesAddressLibrary(true);
	return v;
}();

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
