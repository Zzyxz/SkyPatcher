#include "npc_patcher.h"
#include "npc.h"
#include <cmath>
#include <iostream>
#include <regex>
#include <unordered_map>


void* patch(NPC::patch_instruction line, RE::TESNPC* curobj, bool bEnableSetLevelDirectlyByPCMult, NPC::ActorData aData)
{
	if (!line.calcStats.empty() && line.calcStats != "none" && curobj->formID != 0x000007) {
		if (line.calcStats == "yes" || line.calcStats == "true") {
			curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
			logger::debug(FMT_STRING("Enabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		} else if (curobj->IsAutoCalc() && (curobj->IsAutoCalc() && (line.calcStats == "no" || line.calcStats == "false"))) {
			curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
			logger::debug(FMT_STRING("Disabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		}
	}

	if (!line.pcMultFlag.empty() && curobj->formID != 0x000007) {
		if (toLowerCase(line.pcMultFlag[0]) == "yes" || toLowerCase(line.pcMultFlag[0]) == "true") {
			curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
			logger::debug(FMT_STRING("Enabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		} else if (curobj->HasPCLevelMult() && (toLowerCase(line.pcMultFlag[0]) == "no" || toLowerCase(line.pcMultFlag[0]) == "false")) {
			curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
			if (!line.PCvalues1.empty()) {
				if (!bEnableSetLevelDirectlyByPCMult) {
					float tempLevel = static_cast<float>(curobj->actorData.calcLevelMin + curobj->actorData.calcLevelMax);
					if (tempLevel >= 1) {
						tempLevel = (std::ceil(tempLevel / 2));

						if (tempLevel >= line.PCvalues1[0]) {
							logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {} , set to Fallback {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, tempLevel, line.PCvalues1[0]);
							curobj->actorData.level = line.PCvalues1[0];

						} else {
							curobj->actorData.level = static_cast<uint16_t>(tempLevel);
							logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {}, Fallback is {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, curobj->actorData.level, line.PCvalues1[0]);
						}

					} else {
						curobj->actorData.level = line.PCvalues1[0];
						logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {} (Fallback)"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
					}
				} else {
					curobj->actorData.level = line.PCvalues1[0];
					logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {}"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
				}

			} else {
				logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}
	}

	if (!line.kEssential.empty() && line.kEssential != "none" && curobj->formID != 0x000007) {
		if (line.kEssential == "yes" || line.kEssential == "true") {
			curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kEssential);
			logger::debug(FMT_STRING("Enabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		} else if (line.kEssential == "no" || line.kEssential == "false") {
			curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kEssential);
			logger::debug(FMT_STRING("Disabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		}
	}

	if (!line.kProtected.empty() && line.kProtected != "none" && curobj->formID != 0x000007) {
		if (line.kProtected == "yes" || line.kProtected == "true") {
			curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kProtected);
			logger::debug(FMT_STRING("Enabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		} else if (line.kProtected == "no" || line.kProtected == "false") {
			curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kProtected);
			logger::debug(FMT_STRING("Disabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
		}
	}

	if (!line.rcslist.empty() && line.rcslist != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.rcslist;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::FormList) {
			aData.npcData[curobj->formID].npcCombatStyleList = ((RE::BGSListForm*)currentform);
			logger::debug(FMT_STRING("NPC {:08X} {} will get random combat style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
		}
	}

	if (!line.setRandomVisualStyle.empty() && line.setRandomVisualStyle != "none") {
		//logger::debug(" line.setRandomVisualStyle {}", line.setRandomVisualStyle);
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.setRandomVisualStyle;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::FormList) {
			aData.npcData[curobj->formID].npcVisualStyleList = ((RE::BGSListForm*)currentform);
			logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
		}
	}

	if (!line.keywordsToAdd.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.keywordsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				curobj->AddKeyword((RE::BGSKeyword*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			}
		}
	}

	if (!line.keywordsToRemove.empty()) {
		for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.keywordsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			}
		}
	}

	if (!line.factionsToRemove.empty()) {
		for (size_t i = 0; i < line.factionsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.factionsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Faction) {
				for (auto& faction : curobj->factions) {
					if (faction.faction && faction.faction->formID && faction.faction->formID == currentform->formID) {
						curobj->factions.erase(&faction);
						logger::debug(FMT_STRING("npc formid: {:08X} removed faction {:08X} {} "), curobj->formID, ((RE::TESFaction*)currentform)->formID, ((RE::TESFaction*)currentform)->fullName);
						break;
					}
				}
			}
		}
	}

	//curobj->InitItem

	if (!line.factionsToAdd.empty()) {
		//logger::debug("factionsToAdd 1");
		for (size_t i = 0; i < line.factionsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.factionsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			//logger::debug("factionsToAdd 2");
			if (currentform && currentform->formType == RE::FormType::Faction) {
				//logger::debug("factionsToAdd 3");
				float finalValue = 0;
				if (!line.values1.empty() && !line.values2.empty()) {
					//std::srand(std::time(nullptr));
					finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.factionsToAddRank2[i] - line.factionsToAddRank1[i] + 1) + line.factionsToAddRank1[i]);
				}
				bool factionFound = false;
				for (auto& faction : curobj->factions) {
					//logger::debug("factionsToAdd 4");
					if (faction.faction && faction.faction->formID && faction.faction->formID == currentform->formID) {
						factionFound = true;
						//logger::debug("factionsToAdd 5");
						break;
					}
				}
				if (!factionFound) {
					RE::FACTION_RANK newFactionRank;
					newFactionRank.faction = (RE::TESFaction*)currentform;
					newFactionRank.rank = finalValue;
					curobj->factions.push_back(newFactionRank);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added faction {:08X} with rank {}"), curobj->formID, curobj->fullName, ((RE::TESFaction*)currentform)->formID, finalValue);
				}
			}
		}
	}

	if (!line.avifs.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.avifs.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.avifs[i];

			//logger::info("avif valid!");
			float finalValue = 0;

			if (line.level_min == "none") {
				if (!line.values1.empty() && !line.values2.empty()) {
					unsigned seed = static_cast<unsigned>(std::time(nullptr)) + static_cast<unsigned>(std::clock());
					std::srand(seed);
					//int random_value = std::rand() % 501 + 500;
					finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
				}

			} else if (line.level_min != "none" && line.level_max != "none" && line.level_min != line.level_max && line.values1[i] != line.values2[i]) {
				int level_min = std::stoi(line.level_min);  // minimum level
				int level_max = std::stoi(line.level_max);  // maximum level
				if (level_min < level_max) {
					int avif_min = line.values1[i];  // minimum health
					int avif_max = line.values2[i];  // maximum health
					//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} "), level_min, level_max, line.values1[i], line.values2[i]);
					// Calculate proportional increase
					float health_increase = avif_max - avif_min;
					float level_increase = level_max - level_min;
					float proportional_increase = health_increase / level_increase;

					// Calculate health at level 50
					int target_level = curobj->actorData.level;
					finalValue = avif_min;

					//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} "), level_min, level_max, line.values1[i], line.values2[i]);

					if (target_level >= level_min && target_level <= level_max) {
						float level_difference = target_level - level_min;
						finalValue = avif_min + (level_difference * proportional_increase);
					} else if (target_level < level_min) {
						finalValue = avif_min;
					} else {
						finalValue = avif_max;
					}
				} else {
					finalValue = line.values1[i];
					logger::info(FMT_STRING("Error: Level Min > Level Max  using default value for av {}"), finalValue);
				}
				//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} target_level {} healthinc {} levelinc {} propIncrease {} finalValue {}"), level_min, level_max, line.values1[i], line.values2[i], target_level,health_increase, level_increase, proportional_increase, finalValue);

			} else {
				finalValue = line.values1[i];
			}
			//logger::info(FMT_STRING("Stringform {}"), string_form);
			StatWeights sw;
			if (curobj->npcClass) {
				sw.healthWeight = curobj->npcClass->data.attributeWeights.health;
				sw.staminaWeight = curobj->npcClass->data.attributeWeights.stamina;
				sw.magickaWeight = curobj->npcClass->data.attributeWeights.magicka;
			} else {
				sw.healthWeight = 1;
				sw.staminaWeight = 1;
				sw.magickaWeight = 1;
			}

			auto it2 = aData.npcData.find(curobj->formID);

			if (toLowerCase(string_form) == "health") {
				curobj->actorData.healthOffset = finalValue;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].health = finalValue;
				} else {
					it2->second.health = finalValue;
				}
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.healthOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
				curobj->playerSkills.health = curobj->actorData.healthOffset;
			} else if (toLowerCase(string_form) == "healthmult") {
				logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied health to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.healthOffset, line.values1[i], curobj->actorData.healthOffset * line.values1[i]);
				curobj->actorData.healthOffset = curobj->actorData.healthOffset * line.values1[i];
				curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
				curobj->playerSkills.health = curobj->actorData.healthOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].health = curobj->actorData.healthOffset;
				} else {
					it2->second.health = curobj->actorData.healthOffset;
				}

			} else if (toLowerCase(string_form) == "stamina") {
				curobj->actorData.staminaOffset = finalValue;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.staminaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
				curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
				} else {
					it2->second.stamina = curobj->actorData.staminaOffset;
				}
			} else if (toLowerCase(string_form) == "staminamult") {
				logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied stamina to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.staminaOffset, line.values1[i], curobj->actorData.staminaOffset * line.values1[i]);
				curobj->actorData.staminaOffset = curobj->actorData.staminaOffset * line.values1[i];
				curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
				curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
				} else {
					it2->second.stamina = curobj->actorData.staminaOffset;
				}
			} else if (toLowerCase(string_form) == "magicka") {
				curobj->actorData.magickaOffset = finalValue;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.magickaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
				curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
				} else {
					it2->second.magicka = curobj->actorData.magickaOffset;
				}
			} else if (toLowerCase(string_form) == "magickamult") {
				logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied magicka to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.magickaOffset, line.values1[i], curobj->actorData.magickaOffset * line.values1[i]);
				curobj->actorData.magickaOffset = curobj->actorData.magickaOffset * line.values1[i];
				curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
				curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
				} else {
					it2->second.magicka = curobj->actorData.magickaOffset;
				}
			} else if (toLowerCase(string_form) == "calchealth") {
				auto tempResult = calculateAttributePoints(curobj->actorData.level, sw, line.values1[i]);

				if (!line.healthBonus.empty() && line.healthBonus != "none") {
					tempResult.health = tempResult.health + std::stoi(line.healthBonus);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added health bonus of {}"), curobj->formID, curobj->fullName, line.healthBonus);
				}

				curobj->actorData.healthOffset = tempResult.health;
				logger::debug(FMT_STRING("npc formid: {:08X} {} calculated {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.healthOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
				curobj->playerSkills.health = curobj->actorData.healthOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].health = curobj->actorData.healthOffset;
				} else {
					it2->second.health = curobj->actorData.healthOffset;
				}
			} else if (toLowerCase(string_form) == "calcstamina") {
				auto tempResult = calculateAttributePoints(curobj->actorData.level, sw, line.values1[i]);

				if (!line.staminaBonus.empty() && line.staminaBonus != "none") {
					tempResult.stamina = tempResult.stamina + std::stoi(line.staminaBonus);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added stamina bonus of {}"), curobj->formID, curobj->fullName, line.staminaBonus);
				}

				curobj->actorData.staminaOffset = tempResult.stamina;
				logger::debug(FMT_STRING("npc formid: {:08X} {} calculated {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.staminaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
				curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
				} else {
					it2->second.stamina = curobj->actorData.staminaOffset;
				}
			} else if (toLowerCase(string_form) == "calcmagicka") {
				auto tempResult = calculateAttributePoints(curobj->actorData.level, sw, line.values1[i]);

				if (!line.magickaBonus.empty() && line.magickaBonus != "none") {
					tempResult.magicka = tempResult.magicka + std::stoi(line.magickaBonus);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added magicka bonus of {}"), curobj->formID, curobj->fullName, line.magickaBonus);
				}

				curobj->actorData.magickaOffset = tempResult.magicka;
				logger::debug(FMT_STRING("npc formid: {:08X} {} calculated {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.magickaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
				curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
				} else {
					it2->second.magicka = curobj->actorData.magickaOffset;
				}
			} else if (toLowerCase(string_form) == "byclassstamina") {
				AttributePoints tempResult;

				if (sw.staminaWeight > 0) {
					tempResult.stamina = finalValue;
				} else {
					tempResult.stamina = 0;
				}

				if (!line.staminaBonus.empty() && line.staminaBonus != "none") {
					tempResult.stamina = tempResult.stamina + std::stoi(line.staminaBonus);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added stamina bonus of {}"), curobj->formID, curobj->fullName, line.staminaBonus);
				}

				curobj->actorData.staminaOffset = tempResult.stamina;
				logger::debug(FMT_STRING("npc formid: {:08X} {} setByClass {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.staminaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
				curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
				} else {
					it2->second.stamina = curobj->actorData.staminaOffset;
				}
			} else if (toLowerCase(string_form) == "byclassmagicka") {
				AttributePoints tempResult;

				if (sw.magickaWeight > 0) {
					tempResult.magicka = finalValue;
				} else {
					tempResult.magicka = 0;
				}

				if (!line.magickaBonus.empty() && line.magickaBonus != "none") {
					tempResult.magicka = tempResult.magicka + std::stoi(line.magickaBonus);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added magicka bonus of {}"), curobj->formID, curobj->fullName, line.magickaBonus);
				}

				curobj->actorData.magickaOffset = tempResult.magicka;
				logger::debug(FMT_STRING("npc formid: {:08X} {} setByClass {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.magickaOffset);
				curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
				curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
				} else {
					it2->second.magicka = curobj->actorData.magickaOffset;
				}
			}
		}
	}

	if (!line.perksToAdd.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.perksToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.perksToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Perk) {
				curobj->AddPerk((RE::BGSPerk*)currentform, 1);
				logger::debug(FMT_STRING("npc formid: {:08X} added perk {:08X} {} "), curobj->formID, ((RE::BGSPerk*)currentform)->formID, ((RE::BGSPerk*)currentform)->fullName);
				auto it2 = aData.npcData.find(curobj->formID);
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].perks.push_back(((RE::BGSPerk*)currentform));
				} else {
					it2->second.perks.push_back(((RE::BGSPerk*)currentform));
				}
			}
		}
	}

	if (!line.outfitDefault.empty() && line.outfitDefault != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.outfitDefault;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::Outfit) {
			curobj->defaultOutfit = (RE::BGSOutfit*)currentform;
			//curobj->defOutfit->outfitItems.add
			logger::debug(FMT_STRING("npc formid: {:08X} changed outfit default to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
		}
	}

	if (!line.outfitSleep.empty() && line.outfitSleep != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.outfitSleep;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::Outfit) {
			curobj->sleepOutfit = (RE::BGSOutfit*)currentform;
			logger::debug(FMT_STRING("npc formid: {:08X} changed outfit sleep to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
		}
	}

	if (!line.deathItem.empty() && line.deathItem != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.deathItem;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::LeveledItem) {
			curobj->deathItem = (RE::TESLevItem*)currentform;
			logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to {:08X} "), curobj->formID, ((RE::TESLevItem*)currentform)->formID);
		} else if (line.deathItem == "null") {
			curobj->deathItem = nullptr;
			logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to null (none) "), curobj->formID);
		}
	}

	if (!line.skin.empty() && line.skin != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.skin;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::Armor) {
			curobj->skin = (RE::TESObjectARMO*)currentform;
			logger::debug(FMT_STRING("npc formid: {:08X} changed skin to {:08X} "), curobj->formID, ((RE::TESObjectARMO*)currentform)->formID);
			aData.npcData[curobj->formID].skin = (RE::TESObjectARMO*)currentform;
			aData.npcData[curobj->formID].skinWasSet = true;
		} else if (line.skin == "null") {
			curobj->skin = nullptr;
			logger::debug(FMT_STRING("npc formid: {:08X} changed skin to null (none) "), curobj->formID);
			aData.npcData[curobj->formID].skin = nullptr;
			aData.npcData[curobj->formID].skinWasSet = true;
		}
	}

	if (!line.race.empty() && line.race != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.race;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::Race) {
			curobj->race = (RE::TESRace*)currentform;
			logger::debug(FMT_STRING("npc formid: {:08X} changed race to {:08X} "), curobj->formID, ((RE::TESRace*)currentform)->formID);
		}
	}

	if (!line.Class.empty() && line.Class != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.Class;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::Class) {
			curobj->npcClass = (RE::TESClass*)currentform;
			logger::debug(FMT_STRING("npc formid: {:08X} changed class to {:08X} "), curobj->formID, ((RE::TESClass*)currentform)->formID);
		}
	}

	if (!line.voiceType.empty() && line.voiceType != "none") {
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.voiceType;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform && currentform->formType == RE::FormType::VoiceType) {
			curobj->voiceType = ((RE::BGSVoiceType*)currentform);
			logger::debug(FMT_STRING("npc formid: {:08X} changed voiceType to {:08X} "), curobj->formID, (currentform)->formID);
		}
	}

	//if (found && !line.xpValueOffset.empty() && line.xpValueOffset != "none") {
	//	try {
	//		curobj->actorData. = std::stof(line.xpValueOffset);
	//		logger::debug(FMT_STRING("npc formid: {:08X} {} changed xpValueOffset to {}"), curobj->formID, curobj->fullName, curobj->actorData.xpValueOffset);
	//	} catch (const std::invalid_argument& e) {
	//	}
	//}

	if (!line.level.empty() && line.level != "none") {
		try {
			curobj->actorData.level = std::stof(line.level);
			logger::debug(FMT_STRING("npc formid: {:08X} {} changed level to {}"), curobj->formID, curobj->fullName, curobj->actorData.level);
		} catch (const std::invalid_argument& e) {
		}
	}
	if (!line.calcLevelMin.empty() && line.calcLevelMin != "none") {
		try {
			curobj->actorData.calcLevelMin = std::stof(line.calcLevelMin);
			logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMin to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin);
		} catch (const std::invalid_argument& e) {
		}
	}
	if (!line.calcLevelMax.empty() && line.calcLevelMax != "none") {
		try {
			curobj->actorData.calcLevelMax = std::stof(line.calcLevelMax);
			logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMax to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMax);
		} catch (const std::invalid_argument& e) {
		}
	}

	if (!line.spellsToAdd.empty()) {
		//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Spell) {
				curobj->GetSpellList()->AddSpell((RE::SpellItem*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} {} added spell {:08X} {} "), curobj->formID, curobj->fullName, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
				auto it2 = aData.npcData.find(curobj->formID);
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].spellItems.push_back(((RE::SpellItem*)currentform));
				} else {
					it2->second.spellItems.push_back(((RE::SpellItem*)currentform));
				}
			}
		}
	}

	if (!line.spellsToRemove.empty()) {
		//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Spell) {
				curobj->GetSpellList()->RemoveSpell((RE::SpellItem*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} {} remove spell {:08X} {} "), curobj->formID, curobj->fullName, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
			}
		}
	}

	if (!line.levSpellsToAdd.empty()) {
		for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
				if (!curobj->actorEffects) {
					curobj->actorEffects = new RE::TESSpellList::SpellData();
				}
				curobj->actorEffects->AddLevSpell((RE::TESLevSpell*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} added levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
			}
		}
	}

	if (!line.levSpellsToRemove.empty()) {
		for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
				curobj->actorEffects->RemoveLevSpell((RE::TESLevSpell*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} removed levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
			}
		}
	}

	if (!line.shoutsToAdd.empty()) {
		for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Shout) {
				if (!curobj->actorEffects) {
					curobj->actorEffects = new RE::TESSpellList::SpellData();
				}
				curobj->actorEffects->AddShout((RE::TESShout*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} added shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
			}
		}
	}

	if (!line.shoutsToRemove.empty()) {
		for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.spellsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Shout) {
				curobj->actorEffects->RemoveShout((RE::TESShout*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} removed shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
			}
		}
	}

	if (!line.clearInventory.empty() && line.clearInventory != "none") {
		//logger::info("found! patching values");
		curobj->RemoveAllObjectFromContainer();
		logger::debug(FMT_STRING("npc formid: {:08X} removed all objects "), curobj->formID);
	}

	if (!line.objectsToRemove.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.objectsToRemove.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.objectsToRemove[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform) {
				RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
				if (bo) {
					curobj->RemoveObjectInstancesFromContainer(bo);
					logger::debug(FMT_STRING("npc formid: {:08X} {} removed all object instances {:08X} "), curobj->formID, curobj->fullName, bo->formID);
				}
			}
		}
	}

	if (!line.objectsToRemoveNew.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.objectsToRemoveNew.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.objectsToRemoveNew[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform) {
				RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
				if (bo) {
					curobj->RemoveObjectInstancesFromContainer(bo);
					logger::debug(FMT_STRING("npc formid: {:08X} {} removed all object instances {:08X} "), curobj->formID, curobj->fullName, bo->formID);
				}
			}
		}
	}

	if (!line.removeContainerObjectsByCount.empty()) {
		for (const auto& objectToAdd : line.removeContainerObjectsByCount) {
			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
			std::string         addFormStr = objectToAdd[0];
			uint32_t            count = std::stoi(objectToAdd[1]);
			RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

			if (addForm) {
				curobj->RemoveObjectFromContainer(addForm, count);
				logger::debug(FMT_STRING("npc formid: {:08X} removed by count Form {:08X}"), curobj->formID, addForm->formID);
			} else {
				logger::critical(FMT_STRING("npc formid: {:08X} Form not found: {} (removeContainerObjectsByCount)"), curobj->formID, objectToAdd[0]);
			}
		}
	}

	if (!line.removeContainerObjectsByKeyword.empty()) {
		for (const auto& objectToAdd : line.removeContainerObjectsByKeyword) {
			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
			std::string     addFormStr = objectToAdd;
			RE::BGSKeyword* addForm = (RE::BGSKeyword*)GetFormFromIdentifier(addFormStr);
			if (addForm) {
				curobj->RemoveObjectFromContainerByKeyword(addForm);
				logger::debug(FMT_STRING("npc formid: {:08X} removed by keyword Form {:08X}"), curobj->formID, addForm->formID);
			} else {
				logger::critical(FMT_STRING("npc formid: {:08X} Form not found: {} (remove by keyword)"), curobj->formID, objectToAdd[0]);
			}
		}
	}

	if (!line.objectsToAdd.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		for (size_t i = 0; i < line.objectsToAdd.size(); i++) {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.objectsToAdd[i];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform) {
				RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
				if (bo) {
					curobj->AddObjectToContainer(bo, std::stoi(line.objectsToAddValue[i]), nullptr);
					logger::debug(FMT_STRING("npc formid: {:08X} added object {:08X} {} "), curobj->formID, ((RE::TESBoundObject*)currentform)->formID, line.objectsToAddValue[i]);
				}
			}
		}
	}

	if (!line.addedContainerObjectsOnce.empty()) {
		for (const auto& objectToAdd : line.addedContainerObjectsOnce) {
			std::string         addFormStr = objectToAdd[0];
			uint32_t            count = std::stoi(objectToAdd[1]);
			RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);
			if (addForm) {
				// Check if addForm already exists in the container
				bool formExists = curobj->CountObjectsInContainer(addForm) != 0;
				if (!formExists) {
					// addForm not found in the container, add it
					curobj->AddObjectToContainer(addForm, count, nullptr);
					logger::debug(FMT_STRING("npc {:08X} added once Form  {:08X}"), curobj->formID, addForm->formID);
				} else {
					// addForm already exists in the container, log a message or handle it accordingly
					logger::debug(FMT_STRING("npc {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
				}
			} else {
				logger::critical(FMT_STRING("npc {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
			}
		}
	}

	if (!line.speedMultiplier.empty() && line.speedMultiplier != "none") {
		try {
			curobj->actorData.speedMult = std::stof(line.speedMultiplier);
			logger::debug(FMT_STRING("npc formid: {:08X} {} changed speedMultiplier to {}"), curobj->formID, curobj->fullName, curobj->actorData.speedMult);
		} catch (const std::invalid_argument& e) {
		}
	}

	if (!line.fullName.empty() && line.fullName != "none") {
		try {
			logger::debug(FMT_STRING("npc formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
			curobj->fullName = line.fullName;
		} catch (const std::invalid_argument& e) {
		}
	}

	if (!line.skills.empty()) {
		for (size_t i = 0; i < line.skills.size(); i++) {
			std::string skill = line.skills[i];
			int         value = line.skillvalues1[i];

			if (toLowerCase(skill) == "onehanded") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kOneHanded] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "twohanded") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kTwoHanded] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "marksman") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kMarksman] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "block") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kBlock] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "smithing") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kSmithing] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "heavyarmor") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kHeavyArmor] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "lightarmor") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kLightArmor] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "pickpocket") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kPickpocket] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "lockpicking") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kLockpicking] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "sneak") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kSneak] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "alchemy") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kAlchemy] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "speechcraft") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kSpeechcraft] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "alteration") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kAlteration] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "conjuration") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kConjuration] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "destruction") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kDestruction] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "illusion") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kIllusion] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "restoration") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kRestoration] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else if (toLowerCase(skill) == "enchanting") {
				curobj->playerSkills.values[RE::TESNPC::Skills::kEnchanting] = value;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
			} else {
				//std::cerr << "Unknown skill: " << skill << std::endl;
			}
		}
	}

	if (line.clearClassFromAttributes != "none") {
		if (curobj->npcClass) {
			auto newClass = copyClass(curobj->npcClass)->As<RE::TESClass>();
			newClass->data.attributeWeights.health = 0;
			newClass->data.attributeWeights.stamina = 0;
			newClass->data.attributeWeights.magicka = 0;
			curobj->npcClass = newClass;
			logger::debug(FMT_STRING("npc formid: {:08X} {} copied class and cleared attributes"), curobj->formID, curobj->fullName);
			auto it2 = aData.npcData.find(curobj->formID);
			if (it2 == aData.npcData.end()) {
				aData.npcData[curobj->formID].npcClass = newClass;
			} else {
				it2->second.npcClass = newClass;
			}
		}
	}

	if (!line.setTemplateFlags.empty()) {
		for (const auto& setFlag : line.setTemplateFlags) {
			if (toLowerCase(setFlag) == "traits") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled traits"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "stats") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled stats"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "factions") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled factions"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "spells") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled spells"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "aidata") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled aiData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "aipackages") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled aiPackages"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "unused") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled unused"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "basedata") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled baseData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "inventory") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled inventory"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "script") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled script"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "aidefpacklist") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled aiDefPackList"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "attackdata") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled attackData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "keywords") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled keywords"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "copiedtemplate") {
				curobj->actorData.templateUseFlags.set(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled copiedTemplate"), curobj->formID, curobj->fullName);
			} else {
				// Handle the case where an invalid flag is provided
				// You might want to add error handling or logging here
			}
		}
	}

	if (!line.removeTemplateFlags.empty()) {
		for (const auto& removeFlag : line.removeTemplateFlags) {
			if (toLowerCase(removeFlag) == "traits") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled traits"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "stats") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled stats"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "factions") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled factions"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "spells") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled spells"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "aidata") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled aiData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "aipackages") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled aiPackages"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "unused") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled unused"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "basedata") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled baseData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "inventory") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled inventory"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "script") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled script"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "aidefpacklist") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled aiDefPackList"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "attackdata") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled attackData"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "keywords") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled keywords"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "copiedtemplate") {
				curobj->actorData.templateUseFlags.reset(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled copiedTemplate"), curobj->formID, curobj->fullName);
			} else {
				// Handle the case where an invalid flag is provided
				// You might want to add error handling or logging here
			}
		}
	}

	if (!line.removeFlags.empty()) {
		for (const auto& removeFlag : line.removeFlags) {
			if (toLowerCase(removeFlag) == "female") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled female"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "essential") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled essential"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "ischargenfacepreset") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled ischargenfacepreset"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "respawn") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kRespawn);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled respawn"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "autocalcstats") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled autocalcstats"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "unique") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kUnique);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled unique"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "doesntaffectstealthmeter") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled doesntaffectstealthmeter"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "pclevelmult") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled pclevelmult"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "usestemplate") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kUsesTemplate);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled usestemplate"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "calcfalltemplates") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled calcfalltemplates"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "protected") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled protected"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "norumors") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kNoRumors);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled norumors"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "summonable") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kSummonable);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled summonable"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "doesntbleed") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kDoesntBleed);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled doesntbleed"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "bleedoutoverride") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled bleedoutoverride"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "oppositegenderanims") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled oppositegenderanims"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "simpleactor") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kSimpleActor);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled simpleactor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "loopedscript") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kLoopedScript);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled loopedscript"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "noactivation") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kNoActivation);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled noactivation"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "loopedaudio") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kLoopedAudio);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled loopedaudio"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "isghost") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kIsGhost);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled isghost"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(removeFlag) == "invulnerable") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kInvulnerable);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled invulnerable"), curobj->formID, curobj->fullName);
			} else {
				// Handle the case where an invalid flag is provided
				// You might want to add error handling or logging here
			}
		}
	}

	if (!line.setFlags.empty()) {
		for (const auto& setFlag : line.setFlags) {
			if (toLowerCase(setFlag) == "female") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled female"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "essential") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled essential"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "ischargenfacepreset") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled ischargenfacepreset"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "respawn") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kRespawn);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled respawn"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "autocalcstats") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled autocalcstats"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "unique") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kUnique);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled unique"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "doesntaffectstealthmeter") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled doesntaffectstealthmeter"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "pclevelmult") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled pclevelmult"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "usestemplate") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kUsesTemplate);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled usestemplate"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "calcfalltemplates") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled calcfalltemplates"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "protected") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled protected"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "norumors") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kNoRumors);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled norumors"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "summonable") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kSummonable);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled summonable"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "doesntbleed") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kDoesntBleed);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled doesntbleed"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "bleedoutoverride") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled bleedoutoverride"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "oppositegenderanims") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled oppositegenderanims"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "simpleactor") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kSimpleActor);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled simpleactor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "loopedscript") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kLoopedScript);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled loopedscript"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "noactivation") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kNoActivation);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled noactivation"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "loopedaudio") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kLoopedAudio);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled loopedaudio"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "isghost") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kIsGhost);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled isghost"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(setFlag) == "invulnerable") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kInvulnerable);
				logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled invulnerable"), curobj->formID, curobj->fullName);
			} else {
				// Handle the case where an invalid flag is provided
				// You might want to add error handling or logging here
			}
		}
	}

	if (!line.copyVisualStyle.empty()) {
		//logger::info("found! patching values");
		//for (const auto& avifstring : line.avifs)
		RE::TESForm* currentform = nullptr;
		std::string  string_form = line.copyVisualStyle;
		currentform = GetFormFromIdentifier(string_form);
		if (currentform) {
			RE::TESNPC* bo = (RE::TESNPC*)currentform;
			if (bo) {
				aData.npcData[curobj->formID].npcVisual = bo;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed visual to {:08X} Start"), curobj->formID, curobj->fullName, bo->formID);
				//curobj->race = bo->race;
				curobj->height = bo->height;
				curobj->weight = bo->weight;
				//curobj->faceData = bo->faceData;
				curobj->faceNPC = bo;
				curobj->tintLayers = bo->tintLayers;
				curobj->bodyTintColor = bo->bodyTintColor;
				curobj->headRelatedData = bo->headRelatedData;
				//curobj->headParts = bo->headParts;
				//curobj->numHeadParts = bo->numHeadParts;
				/*for (int j = 0; j < curobj->numHeadParts; j++) {
						logger::debug(" Head Part {:08X} {}", curobj->headParts[j]->formID, curobj->headParts[j]->fullName);
					}*/
				for (int i = 0; i < bo->numHeadParts; i++) {
					if (bo->headParts && bo->headParts[i]) {
						logger::debug("-> npc formid: {:08X} {} Change Head Part {:08X} {}", curobj->formID, curobj->fullName, bo->headParts[i]->formID, bo->headParts[i]->fullName);
						curobj->ChangeHeadPart(bo->headParts[i]);
					}
				}
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed visual to {:08X} Completed"), curobj->formID, curobj->fullName, bo->formID);
			}
		}
	}

	//curobj->outfit
}
