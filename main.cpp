#include "dirent.h"
#include <leveledlist.h>
#include <alch.h>
#include <ingredient.h>
#include <npc.h>
#include <race.h>
#include <unordered_map>
#include <weapon.h>
#include <armor.h>
#include <ammo.h>
#include <spell.h>
#include <enchant.h>

const char* configFile = ".\\Data\\SKSE\\Plugins\\SkyPatcher.ini";
const char* settingsSection = "Patcher";
const char* settingsSectionLog = "Log";
const char* settingsSectionFeatures = "Features";

float playerHealth;
float playerStamina;
float playerMagicka;

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kNewGame:
		{
			auto player = RE::PlayerCharacter::GetSingleton();
			auto npc = player->data.objectReference->As<RE::TESNPC>();
			npc->actorData.healthOffset = playerHealth;
			npc->actorData.staminaOffset = playerStamina;
			npc->actorData.magickaOffset = playerMagicka;
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
			break;
		}
		break;

	case SKSE::MessagingInterface::kDataLoaded:
		{
			if (GetPrivateProfileInt(settingsSection, "iEnableAmmoPatching", 0, configFile)) {
				AMMO::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ammo\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableWeaponPatching", 0, configFile)) {
				WEAPON::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\weapon\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableArmorPatching", 0, configFile)) {
				ARMOR::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\armor\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableRacePatching", 0, configFile)) {
				RACE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\race\\");
			}
			NPC::ActorData b;
			if (GetPrivateProfileInt(settingsSection, "iEnableNPCPatching", 0, configFile)) {
				b = NPC::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\npc");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableLeveledListPatching", 0, configFile)) {
				LEVELEDLIST::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\leveledList\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableIngestiblePatching", 0, configFile)) {
				ALCH::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ingestible\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableIngestiblePatching", 0, configFile)) {
				INGR::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ingredient\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableSpellPatching", 0, configFile)) {
				SPELL::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\spell\\");
			}

			if (GetPrivateProfileInt(settingsSection, "iEnableEnchantmentPatching", 0, configFile)) {
				ENCH::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\enchantment\\");
			}
			

			//std::pair<RE::BSTHashMap<uint32_t, RE::TESForm*>*, std::reference_wrapper<RE::BSReadWriteLock>> allForms = RE::TESForm::GetAllForms();
			//for (auto [key, form] : *allForms.first) {

			//processLists->ForAllActors();

			auto player = RE::PlayerCharacter::GetSingleton();
			auto npc = player->data.objectReference->As<RE::TESNPC>();
			playerHealth = npc->actorData.healthOffset;
			playerStamina = npc->actorData.staminaOffset;
			playerMagicka = npc->actorData.magickaOffset;
			player->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
			player->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
			player->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);

			const auto  processLists = RE::ProcessLists::GetSingleton();
			for (auto actorHandles : processLists->allProcesses) {
				
				for (const auto actorHandle : *actorHandles) {
					const auto&      actorPtr = actorHandle;
					const auto actorHand = actorPtr.get();
					auto             actor = actorHand.get();
					if (actor && actor->data.objectReference) {
						//logger::debug(FMT_STRING("Actor from ProcessList {:08X} "), actor->formID);
						if (actor->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
							//refr->extraList.GetByType(RE::ExtraDataType::kLevCreaModifier);
							auto data = (RE::ExtraLeveledCreature*)actor->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
							if (data) {
								logger::debug(FMT_STRING("Actor {:08X}  orginalBase {:08X} templateBAse {:08X}"), actor->formID, data->originalBase->formID, data->templateBase->formID);
								if (data->templateBase) {
									actor->data.objectReference->As<RE::TESNPC>()->actorData = data->templateBase->actorData;
								}
							}
						}
						auto npc = actor->data.objectReference->As<RE::TESNPC>();
						if (actor->formID == 0x000014) {
							playerHealth = npc->actorData.healthOffset;
							playerStamina = npc->actorData.staminaOffset;
							playerMagicka = npc->actorData.magickaOffset;
						}
						actor->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
						actor->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
						actor->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
						logger::debug(FMT_STRING("Actor NEW {:08X}  health {} mag {} stam {}"), actor->formID, npc->actorData.healthOffset+npc->race->data.startingHealth,npc->actorData.magickaOffset + npc->race->data.startingMagicka,npc->actorData.staminaOffset + npc->race->data.startingStamina);
					}
				}
			}
				
			//logger::debug(FMT_STRING("--------------"));

				if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableUnlevelNPCs", 0, configFile)) {
					
					const auto               dataHandler = RE::TESDataHandler::GetSingleton();
					RE::BSTArray<RE::BGSEncounterZone*> EncounterZoneArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();
					RE::BSTArray<RE::TESLevCharacter*>  LevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();

					std::for_each(std::execution::par, EncounterZoneArray.begin(), EncounterZoneArray.end(), [&](auto& encounterZone) {
						encounterZone->data.maxLevel = 127;
						logger::debug(FMT_STRING("Encounterzone {:08X} unleveled."), encounterZone->formID);
					});

					std::for_each(std::execution::par, LevCharArray.begin(), LevCharArray.end(), [&](auto& levChar) {
						levChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
						logger::debug(FMT_STRING("Changed TESLevCharacterList {:08X}"), levChar->formID);
						for (auto& entry : levChar->entries) {
								if (entry.form) {
									entry.level = 1;
									logger::debug(FMT_STRING("-> NPC {:08X} unleveled in LevCharList to level 1 "), entry.form->formID);
								}
							}
					});



					//if (form && form->formType == RE::FormType::LeveledNPC) {
					//	RE::TESLevCharacter* tempLLChar = (RE::TESLevCharacter*)form->As<RE::TESLevCharacter>();
					//	tempLLChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
					//	for (auto& entry : tempLLChar->entries) {
					//		if (entry.form) {
					//			entry.level = 1;
					//			logger::debug(FMT_STRING("NPC {:08X} unleveled to level 1 "), entry.form->formID);
					//		}
					//	}
					//}


				}

			//auto allForms = RE::TESForm::GetAllForms();
			//std::for_each(std::execution::par, allForms.first->begin(), allForms.first->end(), [&](auto& pair) {
			//	auto& key = pair.first;
			//	auto& form = pair.second;
			//	if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableUnlevelNPCs", 0, configFile)) {
			//		if (form && form->formType == RE::FormType::LeveledNPC) {
			//			RE::TESLevCharacter* tempLLChar = (RE::TESLevCharacter*)form->As<RE::TESLevCharacter>();
			//			tempLLChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
			//			for (auto& entry : tempLLChar->entries) {
			//				if (entry.form) {
			//					entry.level = 1;
			//					logger::debug(FMT_STRING("NPC {:08X} unleveled to level 1 "), entry.form->formID);
			//				}
			//			}
			//		}
			//		if (form && form->formType == RE::FormType::EncounterZone) {
			//			RE::BGSEncounterZone* tempEncounterZone = (RE::BGSEncounterZone*)form->As<RE::BGSEncounterZone>();
			//			//tempEncounterZone->data.minLevel = 1;
			//			tempEncounterZone->data.maxLevel = 127;
			//			logger::debug(FMT_STRING("Encounterzone {:08X} unleveled."), tempEncounterZone->formID);
			//		}
			//	}
			//	if (form && form->formType == RE::FormType::ActorCharacter) {
			//		//logger::debug(FMT_STRING("IS Actor  {} {:08X}"), form->formType.underlying(), form->formID);
			//		auto refr = (RE::Actor*)form;
			//		if (refr && refr->data.objectReference) {
			//			
			//			//refr->ShouldPerformRevert();
			//			//refr->extraList.RemoveByType(RE::ExtraDataType::kLevCreaModifier);
			//			//if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
			//			//	refr->data.objectReference = nullptr;
			//			//	logger::debug(FMT_STRING("Removed kLevCreaModifier"));
			//			//}
			//			//refr->InitValues();
			//			//auto aaa =  refr->GetTemplateActorBase();
			//			//logger::debug(FMT_STRING("IS NPC {:08X}"), npc->formID);
			//			//logger::debug(FMT_STRING("Stats Health {} {} {} Race {:08X}"), npc->actorData.healthOffset, npc->actorData.staminaOffset, npc->actorData.magickaOffset, npc->race->formID);
			//			//	auto player = refr->data.objectReference->As<RE::TESNPC>();
			//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, player->actorData.healthOffset + player->race->data.startingHealth);
			//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, player->actorData.magickaOffset + player->race->data.startingMagicka);
			//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, player->actorData.staminaOffset + player->race->data.startingStamina);
			//			//	//refr->InitValues();
			//			//	//refr->ApplyPerksFromBase();
			//			//}
			//			//auto cache = refr->As<RE::Actor>()->currentProcess->cachedValues->actorValueCache;
			//			//for (RE::CachedValueData* it = cache.begin(); it != cache.end(); ++it)
			//			//{
			//			//	// Access the extra data using 'it'
			//			//	logger::debug(FMT_STRING("v1 {} v2 {} v3 {}"), it->pad1, it->pad2, it->value);
			//			//	// Do something with 'extraData'
			//			//}
			//			//logger::debug(FMT_STRING("Map sizes {} {} {} "), b.healthMap.size(), b.staminaMap.size(), b.magickaMap.size());
			//			//if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
			//			//	auto healthIt = b.healthMap.find(npc->race->formID);
			//			//	if (healthIt != b.healthMap.end()) {
			//			//		npc->actorData.healthOffset = healthIt->second;
			//			//		//logger::debug(FMT_STRING("Setting Health."));
			//			//	}
			//			//	auto stamIt = b.staminaMap.find(npc->race->formID);
			//			//	if (stamIt != b.staminaMap.end()) {
			//			//		npc->actorData.staminaOffset = stamIt->second;
			//			//	}
			//			//	auto magIt = b.magickaMap.find(npc->race->formID);
			//			//	if (magIt != b.magickaMap.end()) {
			//			//		npc->actorData.magickaOffset = magIt->second;
			//			//	}
			//			//	//logger::debug(FMT_STRING("Stats Health {} {} {} Race {:08X}"), npc->actorData.healthOffset, npc->actorData.staminaOffset, npc->actorData.magickaOffset, npc->race->formID);
			//			//	StatWeights swActor;
			//			//	swActor.healthWeight = npc->npcClass->data.attributeWeights.health;
			//			//	swActor.staminaWeight = npc->npcClass->data.attributeWeights.stamina;
			//			//	swActor.magickaWeight = npc->npcClass->data.attributeWeights.magicka;
			//			//	
			//			//	auto autoHealthIt = b.autoHealthMap.find(npc->race->formID);
			//			//	if (autoHealthIt != b.autoHealthMap.end()) {
			//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
			//			//		npc->actorData.healthOffset = tempResult.health;
			//			//		//logger::debug(FMT_STRING("Setting Health."));
			//			//	}
			//			//	auto autoStamIt = b.autoStaminaMap.find(npc->race->formID);
			//			//	if (autoStamIt != b.autoStaminaMap.end()) {
			//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
			//			//		npc->actorData.staminaOffset = tempResult.stamina;
			//			//	}
			//			//	auto autoMagIt = b.autoMagickaMap.find(npc->race->formID);
			//			//	if (autoMagIt != b.autoMagickaMap.end()) {
			//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
			//			//		npc->actorData.magickaOffset = tempResult.magicka;
			//			//	}
			//			//}
			//			if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
			//				//refr->extraList.GetByType(RE::ExtraDataType::kLevCreaModifier);
			//				auto data = (RE::ExtraLeveledCreature*)refr->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
			//				if (data) {
			//					//logger::debug(FMT_STRING("Actor {:08X}  orginalBase {:08X} templateBAse {:08X}"), refr->formID, data->originalBase->formID, data->templateBase->formID);
			//					if (data->templateBase) {
			//						refr->data.objectReference->As<RE::TESNPC>()->actorData = data->templateBase->actorData;
			//						//refr->data.objectReference->As<RE::TESNPC>()->perks = data->templateBase->perks;
			//						//refr->data.objectReference->As<RE::TESNPC>()->perkCount = data->templateBase->perkCount;
			//					}
			//				}
			//			}
			//			
			//			auto npc = refr->data.objectReference->As<RE::TESNPC>();
			//			if (refr->formID == 0x000014) {
			//				playerHealth = npc->actorData.healthOffset;
			//				playerStamina = npc->actorData.staminaOffset;
			//				playerMagicka = npc->actorData.magickaOffset;
			//			}
			//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
			//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
			//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
			//			logger::debug(FMT_STRING("Actor OLD {:08X}  "), refr->formID);
			//			//logger::debug(FMT_STRING("Actor got patched {:08X}  "), refr->formID);
			//			//refr->InitValues();
			//			//refr->ApplyPerksFromBase();
			//		}
			//		//logger::debug(FMT_STRING("----------------------"));
			//	}
			//});

			break;
		}
	default:
		break;
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("Skypatcher");
	v.AuthorName("Zzyxzz");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "SkyPatcher";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver
#	ifndef SKYRIMVR
		< SKSE::RUNTIME_1_5_39
#	else
		> SKSE::RUNTIME_VR_1_4_15_1
#	endif
	) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	if (GetPrivateProfileInt(settingsSectionLog, "iEnablelog", 0, configFile)) {
		log->set_level(spdlog::level::debug);
		log->flush_on(spdlog::level::debug);
	} else {
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
	}

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e][%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());
	SKSE::Init(a_skse);
	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
