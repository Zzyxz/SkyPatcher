#include "npc.h"
#include <algorithm>
#include <unordered_map>
//#include <execution>
//#include <memory>

namespace NPC
{

	std::list<patch_instruction> hooktokens;
	ActorData aData;
	bool      bEnableSetLevelDirectlyByPCMult = false;

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;

		extractData(line, "filterByNpcs\\s*=([^:]+)", l.object);
		extractData(line, "filterByNpcsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractData(line, "filterByDefaultOutfits\\s*=([^:]+)", l.filterByOutfit);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		extractData(line, "filterByRaces\\s*=([^:]+)", l.races);

		extractData(line, "restrictToRaces\\s*=([^:]+)", l.restrictToRaces);

		extractData(line, "filterByClass\\s*=([^:]+)", l.filterClass);

		extractData(line, "filterByCombatStyle\\s*=([^:]+)", l.filterCombatStyle);

		extractData(line, "filterByClassExclude\\s*=([^:]+)", l.filterClassExclude);

		extractData(line, "filterByFactions\\s*=([^:]+)", l.filterFactions);

		extractData(line, "filterByFactionsOr\\s*=([^:]+)", l.filterFactionsOr);

		extractData(line, "filterByFactionsExcluded\\s*=([^:]+)", l.filterFactionsExcluded);

		extractDataStrings(line, "restrictToMaleModelContains\\s*=([^:]+)", l.restrictToMaleModelContains);

		extractValueString(line, "setAutoCalcStats\\s*=([^:]+)", l.calcStats);

		extractMultiDataInt(line, "setPcLevelMult\\s*=([^:]+)", l.pcMultFlag, l.PCvalues1, l.PCvalues2);

		extractValueString(line, "setEssential\\s*=([^:]+)", l.kEssential);

		extractValueString(line, "setProtected\\s*=([^:]+)", l.kProtected);

		// extract getLevelRange
		std::regex  levelRange_regex("levelRange\\s*=\\s*(\\d+)\\s*~\\s*(\\d+)", regex::icase);
		std::smatch levelRange;
		std::regex_search(line, levelRange, levelRange_regex);
		if (levelRange.empty() || levelRange.size() < 3) {
			l.level_min = "none";
			l.level_max = "none";
		}
		else {
			l.level_min = levelRange[1].str();
			l.level_max = levelRange[2].str();
		}

		extractMultiDataFloat(line, "changeStats\\s*=([^:]+)", l.avifs, l.values1, l.values2);

		extractMultiDataInt(line, "changeSkills\\s*=([^:]+)", l.skills, l.skillvalues1, l.skillvalues2);

		extractToArr2D(line, "restrictToSkill\\s*=([^:]+)", l.skillRestrict);
		// extract Faction
		std::regex  Faction_regex("factionsToAdd\\s*=([^:]+)", std::regex::icase);
		std::smatch Faction_match;
		std::regex_search(line, Faction_match, Faction_regex);
		if (!Faction_match.empty() && !Faction_match[1].str().empty()) {
			std::string              Faction_str = Faction_match[1];
			std::regex               Faction_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);
			std::vector<std::string> temp_factionsToAdd;
			std::vector<float>       temp_factionsToAddRank1;
			std::vector<float>       temp_factionsToAddRank2;
			std::sregex_iterator     Faction_iterator(Faction_str.begin(), Faction_str.end(), Faction_list_regex);
			std::sregex_iterator     Faction_end;
			while (Faction_iterator != Faction_end) {
				std::string avif = (*Faction_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());
				if (avif == "none") {
					break;
				}
				temp_factionsToAdd.push_back(avif);
				temp_factionsToAddRank1.push_back(std::stof((*Faction_iterator)[2]));
				temp_factionsToAddRank2.push_back((*Faction_iterator)[3] != "" ? std::stof((*Faction_iterator)[3]) : std::stof((*Faction_iterator)[2]));
				++Faction_iterator;
			}
			l.factionsToAdd = std::move(temp_factionsToAdd);
			l.factionsToAddRank1 = std::move(temp_factionsToAddRank1);
			l.factionsToAddRank2 = std::move(temp_factionsToAddRank2);
		}

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractData(line, "factionsToRemove\\s*=([^:]+)", l.factionsToRemove);

		extractData(line, "perksToAdd\\s*=([^:]+)", l.perksToAdd);

		std::regex  objectsToAdd_regex("objectsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch objectsToAdd_match;
		std::regex_search(line, objectsToAdd_match, objectsToAdd_regex);
		std::vector<std::string> objectsToAdd;
		if (objectsToAdd_match.empty() || objectsToAdd_match[1].str().empty()) {
			//empty
		}
		else {
			std::string objectsToAdd_str = objectsToAdd_match[1];
			std::regex  pattern("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+)", regex::icase);
			std::smatch match;

			auto begin = std::sregex_iterator(objectsToAdd_str.begin(), objectsToAdd_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.objectsToAdd.push_back(match[1]);
				l.objectsToAddValue.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		//extractMultiDataString(line, "objectsToAdd\\s*=([^:]+)", );

		extractData(line, "ObjectsToRemove\\s*=([^:]+)", l.objectsToRemove);
		extractData(line, "objectsToRemove\\s*=([^:]+)", l.objectsToRemoveNew);

		extractValueString(line, "outfitDefault\\s*=([^:]+)", l.outfitDefault);

		extractValueString(line, "outfitSleep\\s*=([^:]+)", l.outfitSleep);

		extractValueString(line, "deathItem\\s*=([^:]+)", l.deathItem);

		extractValueString(line, "skin\\s*=([^:]+)", l.skin);

		extractValueString(line, "race\\s*=([^:]+)", l.race);

		extractValueString(line, "class\\s*=([^:]+)", l.Class);

		extractValueString(line, "level\\s*=([^:]+)", l.level);

		extractValueString(line, "calcLevelMin\\s*=([^:]+)", l.calcLevelMin);

		extractValueString(line, "calcLevelMax\\s*=([^:]+)", l.calcLevelMax);

		extractValueString(line, "filterByGender\\s*=([^:]+)", l.isFemale);
		extractValueString(line, "restrictToGender\\s*=([^:]+)", l.restrictGender);
		extractDataStrings(line, "restrictToCombatStyle\\s*=([^:]+)", l.restrictToCombatStyle);

		extractDataStrings(line, "restrictToKeywords\\s*=([^:]+)", l.restrictToKeyword);

		extractValueString(line, "healthBonus\\s*=([^:]+)", l.healthBonus);
		extractValueString(line, "staminaBonus\\s*=([^:]+)", l.staminaBonus);
		extractValueString(line, "magickaBonus\\s*=([^:]+)", l.magickaBonus);

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		extractData(line, "shoutsToAdd\\s*=([^:]+)", l.shoutsToAdd);
		extractData(line, "shoutsToRemove\\s*=([^:]+)", l.shoutsToRemove);
		extractData(line, "levSpellsToAdd\\s*=([^:]+)", l.levSpellsToAdd);
		extractData(line, "levSpellsToRemove\\s*=([^:]+)", l.levSpellsToRemove);
		extractData(line, "spellsToAdd\\s*=([^:]+)", l.spellsToAdd);
		extractData(line, "spellsToRemove\\s*=([^:]+)", l.spellsToRemove);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractValueString(line, "speedMultiplier\\s*=([^:]+)", l.speedMultiplier);

		extractValueString(line, "raceAttack\\s*=([^:]+)", l.attackRace);

		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		//extractDataStrings(line, "filterByEditorIdContainsExcludedForRVS\\s*=([^:]+)", l.filterByEditorIdContainsExcludedForRVS);
		extractValueString(line, "clearClassFromAttributes\\s*=([^:]+)", l.clearClassFromAttributes);

		extractValueString(line, "filterByPCLevelMult\\s*=([^:]+)", l.filterPCMult);
		extractValueString(line, "filterByAutoCalc\\s*=([^:]+)", l.filterAutoCalc);
		extractValueString(line, "filterByEssential\\s*=([^:]+)", l.filterEssential);
		extractValueString(line, "filterByProtected\\s*=([^:]+)", l.filterProtected);
		extractValueString(line, "voiceType\\s*=([^:]+)", l.voiceType);
		extractData(line, "restrictToVoiceType\\s*=([^:]+)", l.restrictToVoiceType);
		extractDataStrings(line, "setTemplateFlags\\s*=([^:]+)", l.setTemplateFlags);
		extractDataStrings(line, "removeTemplateFlags\\s*=([^:]+)", l.removeTemplateFlags);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractValueString(line, "clearInventory\\s*=([^:]+)", l.clearInventory);
		extractToArr2D(line, "removeInventoryObjectsByCount\\s*=([^:]+)", l.removeContainerObjectsByCount);
		extractData(line, "removeInventoryObjectsByKeywords\\s*=([^:]+)", l.removeContainerObjectsByKeyword);
		extractToArr2D(line, "addOnceToInventory\\s*=([^:]+)", l.addedContainerObjectsOnce);
		extractToArr2D(line, "setRandomCombatStyle\\s*=([^:]+)", l.rcslist);
		extractValueString(line, "copyVisualStyle\\s*=([^:]+)", l.copyVisualStyle);
		extractValueString(line, "rvsRestrictToTraits\\s*=([^:]+)", l.rvsRestrictToTraits);
		extractToArr2D(line, "setRandomVisualStyle\\s*=([^:]+)", l.setRandomVisualStyle);
		extractToArr2D(line, "aggressionRadiusRanges\\s*=([^:]+)", l.aggressionRadiusRanges);
		extractToArr2D(line, "addRandomVisualStyle\\s*=([^:]+)", l.addRandomVisualStyle);


		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "height\\s*=([^:]+)", l.height);

		extractValueString(line, "setAggression\\s*=([^:]+)", l.setAggression);
		extractValueString(line, "setAssistance\\s*=([^:]+)", l.setAssistance);
		extractValueString(line, "setConfidence\\s*=([^:]+)", l.setConfidence);
		extractValueString(line, "setMood\\s*=([^:]+)", l.setMood);
		extractValueString(line, "setMorality\\s*=([^:]+)", l.setMorality);
		extractValueString(line, "aggressionRadiusBehavior\\s*=([^:]+)", l.aggressionRadiusBehavior);



		extractDataStrings(line, "attackDataToRemove\\s*=([^:]+)", l.removeAttackData);
		extractDataStrings(line, "restrictToFlags\\s*=([^:]+)", l.restrictToFlags);
		extractDataStrings(line, "restrictToTemplateFlags\\s*=([^:]+)", l.restrictToTemplateFlags);
		extractValueString(line, "haircolor\\s*=([^:]+)", l.haircolor);

		extractToArr2D(line, "objectsToReplace\\s*=([^:]+)", l.objectsToReplace);

		//extractToArr2D(line, "overlayToAdd\\s*=([^:]+)", l.overlayToAdd);
		std::regex  add_regex("attackDataToAdd\\s*=([^:]+)", regex::icase);
		std::smatch add_match;
		std::regex_search(line, add_match, add_regex);
		std::vector<std::string> add;
		if (add_match.empty() || add_match[1].str().empty()) {
			// empty
		}
		else {
			std::string                                        valueLine = add_match[1].str();
			std::vector<std::vector<std::vector<std::string>>> arr3D;

			// Split the input string by commas
			std::vector<std::string> sections;
			size_t                   startPos = 0;
			size_t                   pos = 0;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				sections.push_back(valueLine.substr(startPos, pos - startPos));
				startPos = pos + 1;
			}
			sections.push_back(valueLine.substr(startPos));

			for (const auto& section : sections) {
				std::vector<std::string> arr;

				// exclude the addToLL= part from the first string
				startPos = 0;
				pos = 0;
				std::string token;
				while ((pos = section.find("~", startPos)) != std::string::npos) {
					token = section.substr(startPos, pos - startPos);
					token = trim(token);  // remove leading and trailing white spaces
					arr.push_back(token);
					startPos = pos + 1;
				}
				token = section.substr(startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);

				std::vector<std::vector<std::string>> arr2D(arr.size());

				for (int i = 0; i < arr.size(); i++) {
					std::vector<std::string> splitArr;
					size_t                   innerPos = 0;
					std::string              innerToken;
					while ((innerPos = arr[i].find("=")) != std::string::npos) {
						innerToken = arr[i].substr(0, innerPos);
						innerToken = trim(innerToken);  // remove leading and trailing white spaces
						splitArr.push_back(innerToken);
						arr[i].erase(0, innerPos + 1);
					}
					innerToken = arr[i];
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr2D[i] = splitArr;
				}

				arr3D.push_back(arr2D);
			}

			l.addedAttackData = arr3D;
		}

		std::regex  change_regex("attackDataToChange\\s*=([^:]+)", regex::icase);
		std::smatch change_match;
		std::regex_search(line, change_match, change_regex);
		std::vector<std::string> change;
		if (change_match.empty() || change_match[1].str().empty()) {
			// empty
		}
		else {
			std::string                                        valueLine = change_match[1].str();
			std::vector<std::vector<std::vector<std::string>>> arr3D;

			// Split the input string by commas
			std::vector<std::string> sections;
			size_t                   startPos = 0;
			size_t                   pos = 0;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				sections.push_back(valueLine.substr(startPos, pos - startPos));
				startPos = pos + 1;
			}
			sections.push_back(valueLine.substr(startPos));

			for (const auto& section : sections) {
				std::vector<std::string> arr;

				// exclude the changeToLL= part from the first string
				startPos = 0;
				pos = 0;
				std::string token;
				while ((pos = section.find("~", startPos)) != std::string::npos) {
					token = section.substr(startPos, pos - startPos);
					token = trim(token);  // remove leading and trailing white spaces
					arr.push_back(token);
					startPos = pos + 1;
				}
				token = section.substr(startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);

				std::vector<std::vector<std::string>> arr2D(arr.size());

				for (int i = 0; i < arr.size(); i++) {
					std::vector<std::string> splitArr;
					size_t                   innerPos = 0;
					std::string              innerToken;
					while ((innerPos = arr[i].find("=")) != std::string::npos) {
						innerToken = arr[i].substr(0, innerPos);
						innerToken = trim(innerToken);  // remove leading and trailing white spaces
						splitArr.push_back(innerToken);
						arr[i].erase(0, innerPos + 1);
					}
					innerToken = arr[i];
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr2D[i] = splitArr;
				}

				arr3D.push_back(arr2D);
			}

			l.attackDataToChange = arr3D;
		}

		extractValueString(line, "changeBaseObject\\s*=([^:]+)", l.baseObject);

		return l;
	}

	//void process_patch_instructions(const patch_instruction& line)
	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		const auto start_time{ std::chrono::system_clock::now() };
		logger::debug("processing patch instructions");
		const auto                dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESNPC*> NPCArray = dataHandler->GetFormArray<RE::TESNPC>();

		logger::debug("NPC count: {}", NPCArray.size());

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESNPC* curobj = nullptr;
					RE::Character* curActObj = nullptr;
					

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					//logger::debug("NPC Patcher - before if {:08X}", currentform->formID);
					if (currentform && currentform->formType == RE::FormType::NPC) {
						//logger::debug("NPC Patcher");
						curobj = (RE::TESNPC*)currentform;
						patch(line, curobj);
					}


				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterClass.empty() && line.filterCombatStyle.empty() && line.filterFactions.empty() && line.filterFactionsOr.empty() && line.filterByOutfit.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			for (const auto& curobj : NPCArray) {
				if (!curobj) {
					logger::debug("Skipping nullptr NPC in NPCArray.");
					continue;
				}
				
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				if (findObject(line, curobj)) {
					patch(line, curobj);
				}
			}
		}

		const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
		double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
		logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> NPC Patching (segment) finished in {} seconds", seconds);
	}


	void* patch(NPC::patch_instruction line, RE::TESNPC* curobj)
	{




		if (!line.calcStats.empty() && line.calcStats != "none" && curobj->formID != 0x000007) {
			if (line.calcStats == "yes" || line.calcStats == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("Enabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (curobj->IsAutoCalc() && (curobj->IsAutoCalc() && (line.calcStats == "no" || line.calcStats == "false"))) {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("Disabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.pcMultFlag.empty() && curobj->formID != 0x000007) {
			if (toLowerCase(line.pcMultFlag[0]) == "yes" || toLowerCase(line.pcMultFlag[0]) == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				logger::debug(FMT_STRING("Enabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (curobj->HasPCLevelMult() && (toLowerCase(line.pcMultFlag[0]) == "no" || toLowerCase(line.pcMultFlag[0]) == "false")) {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				if (!line.PCvalues1.empty()) {
					if (!bEnableSetLevelDirectlyByPCMult) {
						float tempLevel = static_cast<float>(curobj->actorData.calcLevelMin + curobj->actorData.calcLevelMax);
						if (tempLevel >= 1) {
							tempLevel = (std::ceil(tempLevel / 2));

							if (tempLevel >= line.PCvalues1[0]) {
								logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {} , set to Fallback {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, tempLevel, line.PCvalues1[0]);
								curobj->actorData.level = line.PCvalues1[0];

							}
							else {
								curobj->actorData.level = static_cast<uint16_t>(tempLevel);
								logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {}, Fallback is {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, curobj->actorData.level, line.PCvalues1[0]);
							}

						}
						else {
							curobj->actorData.level = line.PCvalues1[0];
							logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {} (Fallback)"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
						}
					}
					else {
						curobj->actorData.level = line.PCvalues1[0];
						logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {}"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
					}

				}
				else {
					logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
				}
			}
		}

		if (!line.kEssential.empty() && line.kEssential != "none" && curobj->formID != 0x000007) {
			if (line.kEssential == "yes" || line.kEssential == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("Enabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (line.kEssential == "no" || line.kEssential == "false") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("Disabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.kProtected.empty() && line.kProtected != "none" && curobj->formID != 0x000007) {
			if (line.kProtected == "yes" || line.kProtected == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("Enabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (line.kProtected == "no" || line.kProtected == "false") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("Disabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.rcslist.empty()) {
			for (auto tempCs : line.rcslist) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = tempCs[0];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::FormList) {
					aData.npcData[curobj->formID].npcCombatStyleList = ((RE::BGSListForm*)currentform);

					if (tempCs.size() > 1) {
						aData.npcData[curobj->formID].npcCombatStyleChance = std::stoi(tempCs[1]);
						logger::debug(FMT_STRING("NPC {:08X} {} will get random combat style from {:08X} with a chance of {}%"), curobj->formID, curobj->fullName, currentform->formID, tempCs[0]);
					}
					else {
						logger::debug(FMT_STRING("NPC {:08X} {} will get random combat style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				}
			}
		}

		if (!line.setAggression.empty() && line.setAggression != "none") {
			std::string lowerFlag = toLowerCase(line.setAggression);
			static const std::unordered_map<std::string, RE::ACTOR_AGGRESSION> aggressionMap = {
				{"calmed", RE::ACTOR_AGGRESSION::kCalmed},
				{"unaggressive", RE::ACTOR_AGGRESSION::kUnaggressive},
				{"aggressive", RE::ACTOR_AGGRESSION::kAggressive},
				{"veryaggressive", RE::ACTOR_AGGRESSION::kVeryAggressive},
				{"frenzied", RE::ACTOR_AGGRESSION::kFrenzied}
			};
			auto it = aggressionMap.find(lowerFlag);
			if (it != aggressionMap.end()) {
				// Set the aggression level on the NPC
				curobj->SetAggressionLevel(it->second);
				logger::debug(FMT_STRING("npc formid: {:08X} {} aggression level set to: {}"), curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown aggression level '{}' provided in restrictToTemplateFlags"), lowerFlag);
			}
		}

		if (!line.setConfidence.empty() && line.setConfidence != "none") {
			std::string lowerFlag = toLowerCase(line.setConfidence);
			static const std::unordered_map<std::string, RE::ACTOR_CONFIDENCE> confidenceMap = {
				{"cowardly", RE::ACTOR_CONFIDENCE::kCowardly},
				{"cautious", RE::ACTOR_CONFIDENCE::kCautious},
				{"average", RE::ACTOR_CONFIDENCE::kAverage},
				{"brave", RE::ACTOR_CONFIDENCE::kBrave},
				{"foolhardy", RE::ACTOR_CONFIDENCE::kFoolhardy}
			};
			auto it = confidenceMap.find(lowerFlag);
			if (it != confidenceMap.end()) {
				// Set the confidence level on the NPC
				curobj->SetConfidenceLevel(it->second);
				logger::debug(FMT_STRING("npc formid: {:08X} {} confidence level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown confidence level '{}' provided in setConfidence"), lowerFlag);
			}
		}

		if (!line.setAssistance.empty() && line.setAssistance != "none") {
			std::string lowerFlag = toLowerCase(line.setAssistance);
			static const std::unordered_map<std::string, RE::ACTOR_ASSISTANCE> assistanceMap = {
				{"helpsnobody", RE::ACTOR_ASSISTANCE::kHelpsNobody},
				{"helpsallies", RE::ACTOR_ASSISTANCE::kHelpsAllies},
				{"helpsfriends", RE::ACTOR_ASSISTANCE::kHelpsFriends}
			};
			auto it = assistanceMap.find(lowerFlag);
			if (it != assistanceMap.end()) {
				curobj->SetAssistanceLevel(it->second);
				logger::debug(FMT_STRING("npc formid: {:08X} {} assistance level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown assistance level '{}' provided in setAssistance"), lowerFlag);
			}
		}

		if (!line.setMorality.empty() && line.setMorality != "none") {
			std::string lowerFlag = toLowerCase(line.setMorality);
			static const std::unordered_map<std::string, RE::ACTOR_MORALITY> moralityMap = {
				{"anycrime", RE::ACTOR_MORALITY::kAnyCrime},
				{"violenceagainstenemy", RE::ACTOR_MORALITY::kViolenceAgainstEnemy},
				{"propertycrimeonly", RE::ACTOR_MORALITY::kPropertyCrimeOnly},
				{"nocrime", RE::ACTOR_MORALITY::kNoCrime}
			};
			auto it = moralityMap.find(lowerFlag);
			if (it != moralityMap.end()) {
				curobj->SetMoralityLevel(it->second);
				logger::debug(FMT_STRING("npc formid: {:08X} {} morality level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown morality level '{}' provided in setMorality"), lowerFlag);
			}
		}

		if (!line.setMood.empty() && line.setMood != "none") {
			std::string lowerFlag = toLowerCase(line.setMood);
			static const std::unordered_map<std::string, RE::ACTOR_MOOD> moodMap = {
				{"neutral", RE::ACTOR_MOOD::kNeutral},
				{"angry", RE::ACTOR_MOOD::kAngry},
				{"fear", RE::ACTOR_MOOD::kFear},
				{"happy", RE::ACTOR_MOOD::kHappy},
				{"sad", RE::ACTOR_MOOD::kSad},
				{"surprised", RE::ACTOR_MOOD::kSurprised},
				{"puzzled", RE::ACTOR_MOOD::kPuzzled},
				{"disgusted", RE::ACTOR_MOOD::kDisgusted}
			};
			auto it = moodMap.find(lowerFlag);
			if (it != moodMap.end()) {
				curobj->SetMoodLevel(it->second);
				logger::debug(FMT_STRING("npc formid: {:08X} {} mood level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown mood level '{}' provided in setMood"), lowerFlag);
			}
		}

		if (!line.aggressionRadiusBehavior.empty() && line.aggressionRadiusBehavior != "none") {
			std::string lowerString = toLowerCase(line.aggressionRadiusBehavior);
			if (lowerString == "true") {
				curobj->aiData.aggroRadiusBehaviour = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} enabled aggression radius"), curobj->formID, curobj->fullName);
			}
			else if (lowerString == "false") {
				logger::debug(FMT_STRING("npc formid: {:08X} {} disabled aggression radius"), curobj->formID, curobj->fullName);
			}
			else {
				logger::debug(FMT_STRING("npc formid: {:08X} {} wrong parameter for aggressionRadiusBehavior allowed are true / false"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.aggressionRadiusRanges.empty()) {
			for (const auto& aRanges : line.aggressionRadiusRanges) {
				if (aRanges.size() < 2) {
					logger::warn("Skipping invalid aggression radius entry, expected at least 2 elements");
					continue;
				}

				std::string lowerString = toLowerCase(aRanges[0]);
				int radius = 0;

				try {
					radius = std::stoi(aRanges[1]);
				}
				catch (const std::exception& e) {
					logger::error("Invalid aggression radius value '{}': {}", aRanges[1], e.what());
					continue;
				}

				if (lowerString == "attack") {
					curobj->aiData.aggroRadius[RE::ACTOR_AGGRO_RADIUS::kAttack] = radius;
					logger::debug(FMT_STRING("npc formid: {:08X} {} set attack radius to {}"), curobj->formID, curobj->fullName, radius);
				}
				else if (lowerString == "warn") {
					curobj->aiData.aggroRadius[RE::ACTOR_AGGRO_RADIUS::kWarn] = radius;
					logger::debug(FMT_STRING("npc formid: {:08X} {} set warn radius to {}"), curobj->formID, curobj->fullName, radius);
				}
				else if (lowerString == "attackandwarn") {
					curobj->aiData.aggroRadius[RE::ACTOR_AGGRO_RADIUS::kWarnAndAttack] = radius;
					logger::debug(FMT_STRING("npc formid: {:08X} {} set attack and warn radius to {}"), curobj->formID, curobj->fullName, radius);
				}
			}
		}



		if (!line.setRandomVisualStyle.empty()) {
			//logger::debug(" line.setRandomVisualStyle {}", line.setRandomVisualStyle);
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.setRandomVisualStyle[0][0];
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::FormList) {
				aData.npcData[curobj->formID].npcVisualStyleList = ((RE::BGSListForm*)currentform);

				if (line.setRandomVisualStyle[0].size() > 1) {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X} with a chance of {}%"), curobj->formID, curobj->fullName, currentform->formID, line.setRandomVisualStyle[0][1]);
					auto tempkywd = (RE::BGSKeyword*)GetFormFromIdentifier(line.setRandomVisualStyle[0][1]);
					aData.npcData[curobj->formID].styleChance = std::stoi(line.setRandomVisualStyle[0][1]);
				}
				else {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
				}
				if (line.setRandomVisualStyle[0].size() > 2) {
					aData.npcData[curobj->formID].changeVoice = true;
				}
				if (!line.rvsRestrictToTraits.empty() && line.rvsRestrictToTraits != "none") {
					aData.npcData[curobj->formID].restrictToTraits = true;
				}

			}
		}

		if (!line.addRandomVisualStyle.empty()) {
			//logger::debug(" line.addRandomVisualStyle {}", line.addRandomVisualStyle);
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.addRandomVisualStyle[0][0];
			currentform = GetFormFromIdentifier(string_form);
			int added = 0;
			if (currentform && currentform->formType == RE::FormType::FormList) {

				if (aData.npcData[curobj->formID].npcVisualStyleList == nullptr)
				{
					aData.npcData[curobj->formID].npcVisualStyleList = ((RE::BGSListForm*)currentform);
					added = ((RE::BGSListForm*)currentform)->forms.size();
				}
				else {
					for (auto tForm : ((RE::BGSListForm*)currentform)->forms) {
						if (tForm) {
							// Check if the form already exists before adding
							auto& targetForms = aData.npcData[curobj->formID].npcVisualStyleList->forms;
							if (std::find(targetForms.begin(), targetForms.end(), tForm) == targetForms.end()) {
								added++;
								//logger::debug("add to formlist {:08X} count {} size {} from list {:08X} - Form {:08X}",
								//	aData.npcData[curobj->formID].npcVisualStyleList->formID, added,
								//	aData.npcData[curobj->formID].npcVisualStyleList->forms.size(), currentform->formID, tForm->formID);
								targetForms.push_back(tForm);
							}
						}
					}
				}

				if (line.addRandomVisualStyle[0].size() > 1) {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X} with a chance of {}%  (added random visual styles {})"), curobj->formID, curobj->fullName, currentform->formID, line.addRandomVisualStyle[0][1], added);
					auto tempkywd = (RE::BGSKeyword*)GetFormFromIdentifier(line.addRandomVisualStyle[0][1]);
					aData.npcData[curobj->formID].styleChance = std::stoi(line.addRandomVisualStyle[0][1]);
				}
				else {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X}   (added random visual styles {})"), curobj->formID, curobj->fullName, currentform->formID, added);
				}
				if (line.addRandomVisualStyle[0].size() > 2) {
					aData.npcData[curobj->formID].changeVoice = true;
				}
				if (!line.rvsRestrictToTraits.empty() && line.rvsRestrictToTraits != "none") {
					aData.npcData[curobj->formID].restrictToTraits = true;
				}

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
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
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

				}
				else if (line.level_min != "none" && line.level_max != "none" && line.level_min != line.level_max && line.values1[i] != line.values2[i]) {
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
						}
						else if (target_level < level_min) {
							finalValue = avif_min;
						}
						else {
							finalValue = avif_max;
						}
					}
					else {
						finalValue = line.values1[i];
						logger::info(FMT_STRING("Error: Level Min > Level Max  using default value for av {}"), finalValue);
					}
					//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} target_level {} healthinc {} levelinc {} propIncrease {} finalValue {}"), level_min, level_max, line.values1[i], line.values2[i], target_level,health_increase, level_increase, proportional_increase, finalValue);

				}
				else {
					finalValue = line.values1[i];
				}
				//logger::info(FMT_STRING("Stringform {}"), string_form);
				StatWeights sw;
				if (curobj->npcClass) {
					sw.healthWeight = curobj->npcClass->data.attributeWeights.health;
					sw.staminaWeight = curobj->npcClass->data.attributeWeights.stamina;
					sw.magickaWeight = curobj->npcClass->data.attributeWeights.magicka;
				}
				else {
					sw.healthWeight = 1;
					sw.staminaWeight = 1;
					sw.magickaWeight = 1;
				}

				auto it2 = aData.npcData.find(curobj->formID);

				if (toLowerCase(string_form) == "health") {
					curobj->actorData.healthOffset = finalValue;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].health = finalValue;
					}
					else {
						it2->second.health = finalValue;
					}
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.healthOffset);
					curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
					curobj->playerSkills.health = curobj->actorData.healthOffset;
				}
				else if (toLowerCase(string_form) == "healthmult") {
					logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied health to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.healthOffset, line.values1[i], curobj->actorData.healthOffset * line.values1[i]);
					curobj->actorData.healthOffset = curobj->actorData.healthOffset * line.values1[i];
					curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
					curobj->playerSkills.health = curobj->actorData.healthOffset;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].health = curobj->actorData.healthOffset;
					}
					else {
						it2->second.health = curobj->actorData.healthOffset;
					}

				}
				else if (toLowerCase(string_form) == "stamina") {
					curobj->actorData.staminaOffset = finalValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.staminaOffset);
					curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
					curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
					}
					else {
						it2->second.stamina = curobj->actorData.staminaOffset;
					}
				}
				else if (toLowerCase(string_form) == "staminamult") {
					logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied stamina to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.staminaOffset, line.values1[i], curobj->actorData.staminaOffset * line.values1[i]);
					curobj->actorData.staminaOffset = curobj->actorData.staminaOffset * line.values1[i];
					curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
					curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
					}
					else {
						it2->second.stamina = curobj->actorData.staminaOffset;
					}
				}
				else if (toLowerCase(string_form) == "magicka") {
					curobj->actorData.magickaOffset = finalValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), curobj->actorData.magickaOffset);
					curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
					curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
					}
					else {
						it2->second.magicka = curobj->actorData.magickaOffset;
					}
				}
				else if (toLowerCase(string_form) == "magickamult") {
					logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied magicka to {} * {} = {}"), curobj->formID, curobj->fullName, curobj->actorData.magickaOffset, line.values1[i], curobj->actorData.magickaOffset * line.values1[i]);
					curobj->actorData.magickaOffset = curobj->actorData.magickaOffset * line.values1[i];
					curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
					curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
					}
					else {
						it2->second.magicka = curobj->actorData.magickaOffset;
					}
				}
				else if (toLowerCase(string_form) == "calchealth") {
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
					}
					else {
						it2->second.health = curobj->actorData.healthOffset;
					}
				}
				else if (toLowerCase(string_form) == "calcstamina") {
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
					}
					else {
						it2->second.stamina = curobj->actorData.staminaOffset;
					}
				}
				else if (toLowerCase(string_form) == "calcmagicka") {
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
					}
					else {
						it2->second.magicka = curobj->actorData.magickaOffset;
					}
				}
				else if (toLowerCase(string_form) == "byclassstamina") {
					AttributePoints tempResult;

					if (sw.staminaWeight > 0) {
						tempResult.stamina = finalValue;
					}
					else {
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
					}
					else {
						it2->second.stamina = curobj->actorData.staminaOffset;
					}
				}
				else if (toLowerCase(string_form) == "byclassmagicka") {
					AttributePoints tempResult;

					if (sw.magickaWeight > 0) {
						tempResult.magicka = finalValue;
					}
					else {
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
					}
					else {
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
					}
					else {
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
			}
			else if (line.deathItem == "null") {
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
			}
			else if (line.skin == "null") {
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

		if (!line.haircolor.empty() && line.haircolor != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.haircolor;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::ColorForm) {
				curobj->SetHairColor((RE::BGSColorForm*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} changed hair color to {:08X} "), curobj->formID, currentform->formID);
				aData.npcData[curobj->formID].haircolor = (RE::BGSColorForm*)currentform;
			}
		}

		if (!line.voiceType.empty() && line.voiceType != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.voiceType;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::VoiceType) {
				curobj->voiceType = ((RE::BGSVoiceType*)currentform);
				aData.npcData[curobj->formID].voiceType = (RE::BGSVoiceType*)currentform;
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

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed weight to {}"), curobj->formID, curobj->fullName, curobj->weight);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.height.empty() && line.height != "none") {
			try {
				curobj->height = std::stof(line.height);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed height to {}"), curobj->formID, curobj->fullName, curobj->height);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.level.empty() && line.level != "none") {
			try {
				curobj->actorData.level = std::stof(line.level);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed level to {}"), curobj->formID, curobj->fullName, curobj->actorData.level);
			}
			catch (const std::invalid_argument& e) {
			}
		}
		if (!line.calcLevelMin.empty() && line.calcLevelMin != "none") {
			try {
				curobj->actorData.calcLevelMin = std::stoi(line.calcLevelMin);
				aData.npcData[curobj->formID].minLevel = std::stoi(line.calcLevelMin);
				aData.npcData[curobj->formID].minLevelWasSet = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMin to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin);
			}
			catch (const std::invalid_argument& e) {
			}
		}
		if (!line.calcLevelMax.empty() && line.calcLevelMax != "none") {
			try {
				curobj->actorData.calcLevelMax = std::stoi(line.calcLevelMax);
				aData.npcData[curobj->formID].maxLevel = std::stoi(line.calcLevelMax);
				aData.npcData[curobj->formID].maxLevelWasSet = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMax to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMax);
			}
			catch (const std::invalid_argument& e) {
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
					}
					else {
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
				}
				else {
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
				}
				else {
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
					}
					else {
						// addForm already exists in the container, log a message or handle it accordingly
						logger::debug(FMT_STRING("npc {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
					}
				}
				else {
					logger::critical(FMT_STRING("npc {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.objectsToReplace.empty()) {
			for (const auto& objectToAdd : line.objectsToReplace) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         object1 = objectToAdd[0];
				std::string         object2 = objectToAdd[1];
				RE::TESBoundObject* oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(object1);
				RE::TESBoundObject* newObject = (RE::TESBoundObject*)GetFormFromIdentifier(object2);

				if (oldObject && newObject) {
					curobj->ReplaceObjectInContainer(oldObject, newObject);
					logger::debug(FMT_STRING("npc formid: {:08X} replaced Form {:08X} with Form {:08X} "), curobj->formID, oldObject->formID, newObject->formID);
				}
			}
		}


		if (!line.speedMultiplier.empty() && line.speedMultiplier != "none") {
			try {
				curobj->actorData.speedMult = std::stof(line.speedMultiplier);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed speedMultiplier to {}"), curobj->formID, curobj->fullName, curobj->actorData.speedMult);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.shortName.empty() && line.shortName != "none") {
			try {
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed shortname to {}"), curobj->formID, curobj->shortName, line.shortName);
				curobj->shortName = line.shortName;
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.skills.empty()) {
			for (size_t i = 0; i < line.skills.size(); i++) {
				std::string skill = line.skills[i];
				int         value = line.skillvalues1[i];

				if (toLowerCase(skill) == "onehanded") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kOneHanded] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "twohanded") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kTwoHanded] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "marksman") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kMarksman] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "block") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kBlock] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "smithing") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kSmithing] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "heavyarmor") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kHeavyArmor] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "lightarmor") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kLightArmor] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "pickpocket") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kPickpocket] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "lockpicking") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kLockpicking] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "sneak") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kSneak] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "alchemy") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kAlchemy] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "speechcraft") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kSpeechcraft] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "alteration") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kAlteration] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "conjuration") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kConjuration] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "destruction") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kDestruction] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "illusion") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kIllusion] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "restoration") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kRestoration] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else if (toLowerCase(skill) == "enchanting") {
					curobj->playerSkills.values[RE::TESNPC::Skills::kEnchanting] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else {
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
				}
				else {
					it2->second.npcClass = newClass;
				}
			}
		}

		if (!line.setTemplateFlags.empty()) {
			static const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG> flagMap = {
				{"traits", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits},
				{"stats", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats},
				{"factions", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions},
				{"spells", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells},
				{"aidata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData},
				{"aipackages", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages},
				{"unused", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused},
				{"basedata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData},
				{"inventory", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory},
				{"script", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript},
				{"aidefpacklist", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList},
				{"attackdata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData},
				{"keywords", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords},
				{"copiedtemplate", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate}
			};

			for (const auto& setFlag : line.setTemplateFlags) {
				auto it = flagMap.find(toLowerCase(setFlag));
				if (it != flagMap.end()) {
					curobj->actorData.templateUseFlags.set(it->second);
					//aData.npcData[curobj->formID].updateFlags = true;
					logger::debug(FMT_STRING("npc formid: {:08X} {} template flag enabled {}"), curobj->formID, curobj->fullName, setFlag);
					aData.npcData[curobj->formID].updateTemplateFlags = true;
				}
			}
		}


		if (!line.removeTemplateFlags.empty()) {
			std::map<std::string, RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG> flagMap = {
				{"traits", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits},
				{"stats", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats},
				{"factions", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions},
				{"spells", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells},
				{"aidata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData},
				{"aipackages", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages},
				{"unused", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused},
				{"basedata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData},
				{"inventory", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory},
				{"script", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript},
				{"aidefpacklist", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList},
				{"attackdata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData},
				{"keywords", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords},
				{"copiedtemplate", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate}
			};

			for (const auto& removeFlag : line.removeTemplateFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);

				auto it = flagMap.find(lowerFlag);
				if (it != flagMap.end()) {
					curobj->actorData.templateUseFlags.reset(it->second);
					logger::debug(FMT_STRING("npc formid: {:08X} {} template flag disabled {}"), curobj->formID, curobj->fullName, lowerFlag);
					aData.npcData[curobj->formID].updateTemplateFlags = true;
				}
				else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (!line.removeFlags.empty()) {
			// Define a map to associate flag names with their corresponding Flag values
			static const std::map<std::string, RE::ACTOR_BASE_DATA::Flag> flagMap = {
				{"female", RE::ACTOR_BASE_DATA::Flag::kFemale},
				{"essential", RE::ACTOR_BASE_DATA::Flag::kEssential},
				{"ischargenfacepreset", RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset},
				{"respawn", RE::ACTOR_BASE_DATA::Flag::kRespawn},
				{"autocalcstats", RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats},
				{"unique", RE::ACTOR_BASE_DATA::Flag::kUnique},
				{"doesntaffectstealthmeter", RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter},
				{"pclevelmult", RE::ACTOR_BASE_DATA::Flag::kPCLevelMult},
				{"usestemplate", RE::ACTOR_BASE_DATA::Flag::kUsesTemplate},
				{"calcfalltemplates", RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates},
				{"protected", RE::ACTOR_BASE_DATA::Flag::kProtected},
				{"norumors", RE::ACTOR_BASE_DATA::Flag::kNoRumors},
				{"summonable", RE::ACTOR_BASE_DATA::Flag::kSummonable},
				{"doesntbleed", RE::ACTOR_BASE_DATA::Flag::kDoesntBleed},
				{"bleedoutoverride", RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride},
				{"oppositegenderanims", RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims},
				{"simpleactor", RE::ACTOR_BASE_DATA::Flag::kSimpleActor},
				{"loopedscript", RE::ACTOR_BASE_DATA::Flag::kLoopedScript},
				{"noactivation", RE::ACTOR_BASE_DATA::Flag::kNoActivation},
				{"loopedaudio", RE::ACTOR_BASE_DATA::Flag::kLoopedAudio},
				{"isghost", RE::ACTOR_BASE_DATA::Flag::kIsGhost},
				{"invulnerable", RE::ACTOR_BASE_DATA::Flag::kInvulnerable}
			};

			// Iterate through the removeFlags and apply the respective changes
			for (const auto& removeFlag : line.removeFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);
				auto it = flagMap.find(lowerFlag);
				if (it != flagMap.end()) {
					curobj->actorData.actorBaseFlags.reset(it->second);
					logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled {}"), curobj->formID, curobj->fullName, lowerFlag);
					aData.npcData[curobj->formID].updateFlags = true;
				}
				else {
					// Handle the case where an invalid flag is provided
					logger::warn(FMT_STRING("npc formid: {:08X} {} invalid flag: {}"), curobj->formID, curobj->fullName, lowerFlag);
				}
			}
		}

		if (!line.setFlags.empty()) {
			static const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::Flag> flagMap = {
				{"female", RE::ACTOR_BASE_DATA::Flag::kFemale},
				{"essential", RE::ACTOR_BASE_DATA::Flag::kEssential},
				{"ischargenfacepreset", RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset},
				{"respawn", RE::ACTOR_BASE_DATA::Flag::kRespawn},
				{"autocalcstats", RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats},
				{"unique", RE::ACTOR_BASE_DATA::Flag::kUnique},
				{"doesntaffectstealthmeter", RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter},
				{"pclevelmult", RE::ACTOR_BASE_DATA::Flag::kPCLevelMult},
				{"usestemplate", RE::ACTOR_BASE_DATA::Flag::kUsesTemplate},
				{"calcfalltemplates", RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates},
				{"protected", RE::ACTOR_BASE_DATA::Flag::kProtected},
				{"norumors", RE::ACTOR_BASE_DATA::Flag::kNoRumors},
				{"summonable", RE::ACTOR_BASE_DATA::Flag::kSummonable},
				{"doesntbleed", RE::ACTOR_BASE_DATA::Flag::kDoesntBleed},
				{"bleedoutoverride", RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride},
				{"oppositegenderanims", RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims},
				{"simpleactor", RE::ACTOR_BASE_DATA::Flag::kSimpleActor},
				{"loopedscript", RE::ACTOR_BASE_DATA::Flag::kLoopedScript},
				{"noactivation", RE::ACTOR_BASE_DATA::Flag::kNoActivation},
				{"loopedaudio", RE::ACTOR_BASE_DATA::Flag::kLoopedAudio},
				{"isghost", RE::ACTOR_BASE_DATA::Flag::kIsGhost},
				{"invulnerable", RE::ACTOR_BASE_DATA::Flag::kInvulnerable}
			};

			for (const auto& setFlag : line.setFlags) {
				std::string flag = toLowerCase(setFlag);
				auto it = flagMap.find(flag);
				if (it != flagMap.end()) {
					curobj->actorData.actorBaseFlags.set(it->second);
					logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled {}"), curobj->formID, curobj->fullName, flag);
					aData.npcData[curobj->formID].updateFlags = true;
				}
				else {
					logger::warn(FMT_STRING("npc formid: {:08X} {} unrecognized flag: {}"), curobj->formID, curobj->fullName, flag);
				}
			}
		}

		if (!line.copyVisualStyle.empty() && line.copyVisualStyle != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.copyVisualStyle;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform) {
				RE::TESNPC* bo = (RE::TESNPC*)currentform;
				if (bo) {
					aData.npcData[curobj->formID].npcVisual = bo;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed visual to {:08X} Start"), curobj->formID, curobj->fullName, bo->formID);
					curobj->height = bo->height;
					curobj->weight = bo->weight;
					curobj->faceNPC = bo;
					curobj->tintLayers = bo->tintLayers;
					curobj->bodyTintColor = bo->bodyTintColor;
					curobj->headRelatedData = bo->headRelatedData;
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

		if (!line.baseObject.empty() && line.baseObject != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.baseObject;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::NPC) {
				aData.npcData[curobj->formID].baseObject = (RE::TESNPC*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} Replace with {:08X}"), curobj->formID, currentform->formID);
			}
		}

		/*if (!line.overlayToAdd.empty()) {
			for (const auto& objectToAdd : line.overlayToAdd) {
				if (objectToAdd.size() < 6) {
					logger::error("Invalid overlay entry. Expected 6 elements (Index, Alpha, R, G, B, Preset) but got {}", objectToAdd.size());
					continue;
				}

				const uint16_t tintIndex = static_cast<uint16_t>(std::stoi(objectToAdd[0]));
				const uint8_t  alpha = static_cast<uint8_t>(std::stoi(objectToAdd[1]));
				const uint8_t  red = static_cast<uint8_t>(std::stoi(objectToAdd[2]));
				const uint8_t  green = static_cast<uint8_t>(std::stoi(objectToAdd[3]));
				const uint8_t  blue = static_cast<uint8_t>(std::stoi(objectToAdd[4]));
				const uint16_t preset = static_cast<uint16_t>(std::stoi(objectToAdd[5]));

				const RE::Color newColor(red, green, blue, alpha);

				TintLayerOverlay newOverlay{
					.tintIndex = tintIndex,
					.color = newColor,
					.interpolationValue = static_cast<uint16_t>(alpha),
					.preset = preset
				};

				aData.npcData[curobj->formID].overlaysToAdd.push_back(newOverlay);

				logger::debug("NPC {:08X} - Queued overlay index {} with color {} preset {} for later",
					curobj->formID, tintIndex, newColor.ToHex(), preset);
			}
		}*/




		////////////////////////////////////////////////////


			//logger::debug("start Patch race {:08X}", curobj->formID);
		if (!line.addedAttackData.empty()) {
			//bool               key = true;
			RE::BGSAttackData* newAttackData = RE::BGSAttackData::Create();
			newAttackData->IncRefCount();
			//newAttackData->event = "attackStart";
			newAttackData->data.damageMult = 1;
			newAttackData->data.attackChance = 1;
			newAttackData->data.attackSpell = nullptr;
			newAttackData->data.attackAngle = 0;
			newAttackData->data.strikeAngle = 50;
			newAttackData->data.staggerOffset = 0;
			newAttackData->data.attackType = nullptr;
			newAttackData->data.knockDown = 0;
			newAttackData->data.recoveryTime = 0;
			newAttackData->data.staminaMult = 1;
			newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
			//logger::debug("attackData size before {}", curobj->attackDataMap.get()->attackDataMap.size());
			curobj->attackDataMap->attackDataMap.insert({ newAttackData->event, RE::NiPointer(newAttackData) });
			//logger::debug("attackData size after {}", curobj->attackDataMap.get()->attackDataMap.size());

			for (auto objectArr : line.addedAttackData) {
				for (auto object : objectArr) {
					if (toLowerCase(object[0]) == "key") {
						newAttackData->event = object[1];
						logger::debug("added event: {}", object[1]);
					}
					else if (toLowerCase(object[0]) == "damagemult") {
						newAttackData->data.damageMult = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
					}
					else if (toLowerCase(object[0]) == "attackchance") {
						newAttackData->data.attackChance = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
					}
					else if (toLowerCase(object[0]) == "attackspell") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackSpell = (RE::SpellItem*)form;
								logger::debug("npc formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
							}
						}
						else {
							newAttackData->data.attackSpell = nullptr;
							logger::debug("npc formid: {:08X} attackData - attackspell nullptr", curobj->formID);
						}
					}
					else if (toLowerCase(object[0]) == "attackangle") {
						newAttackData->data.attackAngle = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
					}
					else if (toLowerCase(object[0]) == "strikeangle") {
						newAttackData->data.strikeAngle = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
					}
					else if (toLowerCase(object[0]) == "staggeroffset") {
						newAttackData->data.staggerOffset = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
					}
					else if (toLowerCase(object[0]) == "attacktype") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackType = (RE::BGSKeyword*)form;
								logger::debug("npc formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
							}
						}
						else {
							newAttackData->data.attackType = nullptr;
							logger::debug("npc formid: {:08X} attackData - attacktype nullptr", curobj->formID);
						}
					}
					else if (toLowerCase(object[0]) == "knockdown") {
						newAttackData->data.knockDown = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
					}
					else if (toLowerCase(object[0]) == "recoverytime") {
						newAttackData->data.recoveryTime = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
					}
					else if (toLowerCase(object[0]) == "staminamult") {
						newAttackData->data.staminaMult = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
					}
					else if (toLowerCase(object[0]) == "resetflags") {
						newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
						logger::debug(FMT_STRING("npc formid: {:08X} attackData - all flags reset"), curobj->formID);
					}
					else if (toLowerCase(object[0]) == "removeflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerRemoveFlag = toLowerCase(object[i]);
							if (lowerRemoveFlag == "ignoreweapon") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
							}
							else if (lowerRemoveFlag == "bashattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
							}
							else if (lowerRemoveFlag == "powerattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
							}
							else if (lowerRemoveFlag == "chargeattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
							}
							else if (lowerRemoveFlag == "rotatingattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
							}
							else if (lowerRemoveFlag == "continuousattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
							}
							else if (lowerRemoveFlag == "overridedata") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
							}
							else {
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
							}
						}

					}
					else if (toLowerCase(object[0]) == "setflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerSetFlag = toLowerCase(object[i]);
							if (lowerSetFlag == "ignoreweapon") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
							}
							else if (lowerSetFlag == "bashattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
							}
							else if (lowerSetFlag == "powerattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
							}
							else if (lowerSetFlag == "chargeattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
							}
							else if (lowerSetFlag == "rotatingattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
							}
							else if (lowerSetFlag == "continuousattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
							}
							else if (lowerSetFlag == "overridedata") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
							}
							else {
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
							}
						}
					}
					else {
						// Handle the case where the keyword is not recognized
						// You may want to log an error or take appropriate action
						logger::debug("Unrecognized keyword: {}", object[0]);
					}
				}
			}
		}

		if (!line.removeAttackData.empty()) {
			for (auto attackData : line.removeAttackData) {
				//logger::debug("string {}", attackData);
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					//logger::debug("for {}", it->second.get()->event.c_str());
					if (toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
						logger::debug("npc formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					}
					else {
						// Move to the next iterator
						++it;
					}
				}
			}
		}

		if (!line.attackDataToChange.empty()) {
			//logger::debug("attackDataMap size {}", curobj->attackDataMap->attackDataMap.size());
			for (auto& attackData : curobj->attackDataMap->attackDataMap) {

				if (!attackData.second)
					continue;

				auto newAttackData = attackData.second.get();
				if (!newAttackData)
					continue;
				//logger::debug("processing: {}", newAttackData->event.c_str());
				//logger::debug("attackDataToChange size {}", line.attackDataToChange.size());
				for (auto& objectArr : line.attackDataToChange) {
					//attackData loop from race


					bool key = true;
					//line from ini
					//logger::debug("processing attackMap {} objectArr {} ", newAttackData->event.c_str(), objectArr.size());
					for (auto& object : objectArr) {
						//logger::debug("object-> 0 {} 1 {}", object[0], object[1]);
						if (object[1].empty()) {
							continue;
						}

						if (key) {
							if (!(newAttackData->event.c_str() == object[1])) {
								//logger::debug("race formid: {:08X} attackData change - entry not found ", curobj->formID);
								break;
							}
							else {
								logger::debug("npc formid: {:08X} attackData change - entry found {}", curobj->formID, newAttackData->event.c_str());
								key = false;
								continue;
							}
						}

						if (toLowerCase(object[0]) == "damagemult") {
							newAttackData->data.damageMult = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - damagemult {}", curobj->formID, newAttackData->data.damageMult);
						}
						else if (toLowerCase(object[0]) == "attackchance") {
							newAttackData->data.attackChance = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - attackchance {}", curobj->formID, newAttackData->data.attackChance);
						}
						else if (toLowerCase(object[0]) == "attackspell") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackSpell = (RE::SpellItem*)form;
									logger::debug("npc formid: {:08X} attackData changed - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
								}
							}
							else {
								newAttackData->data.attackSpell = nullptr;
								logger::debug("npc formid: {:08X} attackData changed - attackspell nullptr", curobj->formID);
							}
						}
						else if (toLowerCase(object[0]) == "attackangle") {
							newAttackData->data.attackAngle = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
						}
						else if (toLowerCase(object[0]) == "strikeangle") {
							newAttackData->data.strikeAngle = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
						}
						else if (toLowerCase(object[0]) == "staggeroffset") {
							newAttackData->data.staggerOffset = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
						}
						else if (toLowerCase(object[0]) == "attacktype") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackType = (RE::BGSKeyword*)form;
									logger::debug("npc formid: {:08X} attackData changed - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
								}
							}
							else {
								newAttackData->data.attackType = nullptr;
								logger::debug("npc formid: {:08X} attackData changed - attacktype nullptr", curobj->formID);
							}
						}
						else if (toLowerCase(object[0]) == "knockdown") {
							newAttackData->data.knockDown = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - knockdown {}", curobj->formID, newAttackData->data.knockDown);
						}
						else if (toLowerCase(object[0]) == "recoverytime") {
							newAttackData->data.recoveryTime = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
						}
						else if (toLowerCase(object[0]) == "staminamult") {
							newAttackData->data.staminaMult = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
						}
						else if (toLowerCase(object[0]) == "resetflags") {
							newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
							logger::debug(FMT_STRING("npc formid: {:08X} attackData - all flags reset"), curobj->formID);
						}
						else if (toLowerCase(object[0]) == "removeflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerRemoveFlag = toLowerCase(object[i]);
								if (lowerRemoveFlag == "ignoreweapon") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
								}
								else if (lowerRemoveFlag == "bashattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
								}
								else if (lowerRemoveFlag == "powerattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
								}
								else if (lowerRemoveFlag == "chargeattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
								}
								else if (lowerRemoveFlag == "rotatingattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
								}
								else if (lowerRemoveFlag == "continuousattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
								}
								else if (lowerRemoveFlag == "overridedata") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
								}
								else {
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
								}
							}

						}
						else if (toLowerCase(object[0]) == "setflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerSetFlag = toLowerCase(object[i]);
								if (lowerSetFlag == "ignoreweapon") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
								}
								else if (lowerSetFlag == "bashattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
								}
								else if (lowerSetFlag == "powerattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
								}
								else if (lowerSetFlag == "chargeattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
								}
								else if (lowerSetFlag == "rotatingattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
								}
								else if (lowerSetFlag == "continuousattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
								}
								else if (lowerSetFlag == "overridedata") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
								}
								else {
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
								}
							}
						}
						else {
							// Handle the case where the keyword is not recognized
							// You may want to log an error or take appropriate action
							logger::debug("Unrecognized keyword: {}", object[0]);
						}
					}
				}


				//

				for (auto attackData : line.removeAttackData) {
					//logger::debug("string {}", attackData);
					for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
						//logger::debug("for {}", it->second.get()->event.c_str());
						if (toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
							logger::debug("npc formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
							it = curobj->attackDataMap->attackDataMap.erase(it);
						}
						else {
							// Move to the next iterator
							++it;
						}
					}
				}






			}
		}

		///////////////////////////////////////////////////

		//curobj->outfit
	}

	ActorData readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string            currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string  fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						}
						else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();

								if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr)) {
									if (!IsPluginInstalled(modname)) {
										logger::info("************************************************************");
										logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), modname, fullPath);
										logger::info("************************************************************");
										continue;
									}
								}
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string                  line;
								std::ifstream                infile;
								std::list<patch_instruction> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									tokens.push_back(create_patch_instruction(line));
								}
								infile.close();

								// Convert std::list to std::vector
								std::vector<patch_instruction> tokensVector(tokens.begin(), tokens.end());
								process_patch_instructions(tokens);
								//std::for_each(std::execution::par, tokens.begin(), tokens.end(), process_patch_instructions);
							}
						}
					}
				}
				closedir(dir);
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return aData;
	}

	void* readConfigHook(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string            currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string  fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						}
						else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string   line;
								std::ifstream infile;

								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction(line));
								}
								infile.close();
								//process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

	void setbEnableSetLevelDirectlyByPCMult()
	{
		bEnableSetLevelDirectlyByPCMult = true;
		//return;
	}

	bool findObject(patch_instruction line, RE::TESNPC* curobj)
	{
		bool found = false;
		bool keywordAnd = false;
		bool keywordOr = false;
		bool contains = false;
		bool containsOr = false;
		bool factionAnd = false;
		bool factionOr = false;

		if (!found && !line.keywords.empty() && curobj->formID != 0x000007) {
			for (const auto& keywordstring : line.keywords) {
				RE::TESForm* currentform = nullptr;
				RE::BGSKeyword* keyword = nullptr;

				std::string string_form = keywordstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					keyword = (RE::BGSKeyword*)currentform;

					if (curobj->HasKeyword(keyword) || curobj->race && curobj->race->HasKeyword(keyword)) {
						keywordAnd = true;
					}
					else {
						keywordAnd = false;
						break;
					}
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			keywordAnd = true;
		}

		if (!found && !line.keywordsOr.empty() && curobj->formID != 0x000007) {
			for (const auto& keywordstring : line.keywordsOr) {
				RE::TESForm* currentform = nullptr;
				RE::BGSKeyword* keyword = nullptr;

				std::string string_form = keywordstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					keyword = (RE::BGSKeyword*)currentform;

					if (curobj->HasKeyword(keyword) || curobj->race && curobj->race->HasKeyword(keyword)) {
						keywordOr = true;
						break;
					}
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
			keywordOr = true;
		}

		if (!found && (!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
			//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
			found = true;
		}



		if (!found && !line.races.empty() && curobj->formID != 0x000007) {
			//logger::info("race not empty");
			for (const auto& racestring : line.races) {
				RE::TESForm* currentform = nullptr;
				RE::TESRace* race = nullptr;

				std::string string_form = racestring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Race) {
					race = (RE::TESRace*)currentform;

					if (curobj->race && curobj->race == race) {
						found = true;

						break;
					}
					else if (!curobj->race) {
						logger::debug(FMT_STRING("npc has no race id({:08X} )."), curobj->formID);
					}
				}
			}
		}

		if (!found && !line.filterByOutfit.empty()) {
			//logger::info("npc not empty");
			for (const auto& npcstring : line.filterByOutfit) {
				RE::TESForm* currentform = nullptr;
				RE::BGSOutfit* npc = nullptr;

				std::string string_form = npcstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Outfit) {
					npc = (RE::BGSOutfit*)currentform;

					if (curobj->defaultOutfit && curobj->defaultOutfit->formID == npc->formID) {
						found = true;
						//logger::info("NPC found.");
						break;
					}
				}
			}
		}



		if (!found && !line.filterClass.empty()) {
			//logger::info("npc not empty");
			for (const auto& npcstring : line.filterClass) {
				RE::TESForm* currentform = nullptr;
				RE::TESClass* npc = nullptr;

				std::string string_form = npcstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Class) {
					npc = (RE::TESClass*)currentform;

					if (curobj->npcClass && curobj->npcClass->formID == npc->formID) {
						found = true;
						//logger::info("NPC found.");
						break;
					}
				}
			}
		}

		if (!found && !line.filterCombatStyle.empty()) {
			//logger::info("npc not empty");
			for (const auto& npcstring : line.filterCombatStyle) {
				RE::TESForm* currentform = nullptr;
				RE::TESCombatStyle* npc = nullptr;

				std::string string_form = npcstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::CombatStyle) {
					npc = (RE::TESCombatStyle*)currentform;

					if (curobj->combatStyle && curobj->combatStyle->formID == npc->formID) {
						found = true;
						//logger::info("NPC found.");
						break;
					}
				}
			}
		}

		if (!line.filterPCMult.empty() && line.filterPCMult != "none") {
			std::string check = toLowerCase(line.filterPCMult);

			if ((check == "true")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kPCLevelMult) {
					found = true;
				}
				else {
					found = false;
				}
			}
			else if ((check == "false")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kPCLevelMult)) {
					found = true;
				}
				else {
					found = false;
				}
			}
		}

		if (!line.filterAutoCalc.empty() && line.filterAutoCalc != "none") {
			std::string check = toLowerCase(line.filterAutoCalc);
			if ((check == "true")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats) {
					found = true;
				}
				else {
					found = false;
				}
			}
			else if ((check == "false")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats)) {
					found = true;
				}
				else {
					found = false;
				}
			}
		}

		if (!line.filterEssential.empty() && line.filterEssential != "none") {
			std::string check = toLowerCase(line.filterEssential);
			if ((check == "true")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kEssential) {
					found = true;
				}
				else {
					found = false;
				}
			}
			else if ((check == "false")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kEssential)) {
					found = true;
				}
				else {
					found = false;
				}
			}
		}
		if (!line.filterProtected.empty() && line.filterProtected != "none") {
			std::string check = toLowerCase(line.filterProtected);
			if ((check == "true")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kProtected) {
					found = true;
				}
				else {
					found = false;
				}
			}
			else if ((check == "false")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kProtected)) {
					found = true;
				}
				else {
					found = false;
				}
			}
		}


		if (!found && !line.filterFactions.empty() && curobj->formID != 0x000007) {
			//logger::info("filterByFaction not empty");
			for (const auto& filterByFactiontring : line.filterFactions) {
				RE::TESForm* currentform = nullptr;
				RE::TESFaction* keyword = nullptr;

				std::string string_form = filterByFactiontring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Faction) {
					keyword = (RE::TESFaction*)currentform;

					if (curobj->HasFaction(keyword)) {
						factionAnd = true;
					}
					else {
						factionAnd = false;
						//logger::debug(FMT_STRING("KeywordAnd npc does not have all filterByFaction"));
						break;
					}
					//logger::debug(FMT_STRING("KeywordAnd npc true"));
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
			factionAnd = true;
		}

		if (!found && !line.filterFactionsOr.empty() && curobj->formID != 0x000007) {
			//logger::info("keywords not empty");
			for (const auto& keywordstring : line.filterFactionsOr) {
				RE::TESForm* currentform = nullptr;
				RE::TESFaction* keyword = nullptr;

				std::string string_form = keywordstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Faction) {
					keyword = (RE::TESFaction*)currentform;

					if (curobj->HasFaction(keyword)) {
						factionOr = true;
						//logger::debug(FMT_STRING("NPC has faction {:08X}"), curobj->formID, curobj->fullName);
						//logger::info("Keyword found.");
						break;
					}
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
			factionOr = true;
		}

		if (!found && !line.filterByEditorIdContains.empty() && curobj->formID != 0x000007) {
			for (const auto& editorString : line.filterByEditorIdContains) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					contains = true;
				}
				else {
					contains = false;
					break;
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			contains = true;
		}

		if (!found && !line.filterByEditorIdContainsOr.empty() && curobj->formID != 0x000007) {
			//logger::info("keywords not empty");
			for (const auto& editorString : line.filterByEditorIdContainsOr) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					containsOr = true;
					break;
				}
			}
		}
		else if (curobj->formID != 0x000007) {
			containsOr = true;
		}

		if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
			//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
			found = true;
		}

		if (!found && (!line.filterFactions.empty() || !line.filterFactionsOr.empty()) && factionAnd && factionOr) {
			//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
			found = true;
		}

		if (!found && line.object.empty() && line.races.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterClass.empty() && line.filterCombatStyle.empty() && line.filterFactions.empty() && line.filterFactionsOr.empty() && line.filterByOutfit.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && curobj->formID != 0x000007) {
			found = true;
			//logger::debug(FMT_STRING("Patch Everything but skip Player {:08X}"), curobj->formID);
		}

		if (!line.isFemale.empty() && found && line.isFemale != "none") {
			std::string lowercaseIsFemale = line.isFemale;
			std::transform(lowercaseIsFemale.begin(), lowercaseIsFemale.end(), lowercaseIsFemale.begin(), [](unsigned char c) { return std::tolower(c); });

			if ((lowercaseIsFemale == "female")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale) {
					found = true;
					//logger::debug(FMT_STRING("is female {:08X} {}"), curobj->formID, curobj->fullName);
				}
				else {
					found = false;
					//logger::debug(FMT_STRING("is not female {:08X} {}"), curobj->formID, curobj->fullName);
				}
			}
			else if ((lowercaseIsFemale == "male")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale)) {
					//logger::debug(FMT_STRING("is male {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}
				else {
					//logger::debug(FMT_STRING("is not male {:08X} {}"), curobj->formID, curobj->fullName);
					found = false;
				}
			}
		}

		if (!line.keywordsExcluded.empty() && found && curobj->formID != 0x000007) {
			//logger::info("keywords not empty");
			for (const auto& keywordstring : line.keywordsExcluded) {
				RE::TESForm* currentform = nullptr;
				RE::BGSKeyword* keyword = nullptr;
				std::string     string_form = keywordstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					keyword = (RE::BGSKeyword*)currentform;
					if (curobj->HasKeyword(keyword) || curobj->race && curobj->race->HasKeyword(keyword)) {
						found = false;
						break;
					}
				}
			}
		}

		if (!line.filterFactionsExcluded.empty() && found && curobj->formID != 0x000007) {
			//logger::info("factions not empty");
			for (const auto& factionstring : line.filterFactionsExcluded) {
				RE::TESForm* currentform = nullptr;
				RE::TESFaction* keyword = nullptr;

				std::string string_form = factionstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Faction) {
					keyword = (RE::TESFaction*)currentform;

					if (curobj->HasFaction(keyword)) {
						found = false;
						//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
						//logger::info("Keyword found.");
						break;
					}
				}
			}
		}

		if (!line.filterByEditorIdContainsExcluded.empty() && found && curobj->formID != 0x000007) {
			//logger::info("factions not empty");
			for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					found = false;
					break;
				}
			}
		}


		if (!line.filterClassExclude.empty() && found) {
			//logger::info("npc not empty");
			for (const auto& npcstring : line.filterClassExclude) {
				RE::TESForm* currentform = nullptr;
				RE::TESClass* npc = nullptr;

				std::string string_form = npcstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Class) {
					npc = (RE::TESClass*)currentform;

					if (curobj->npcClass->formID == npc->formID) {
						found = false;
						//logger::info("NPC found.");
						break;
					}
				}
			}
		}

		if (!line.objectExcluded.empty() && found) {
			//logger::info("npc not empty");
			for (const auto& npcstring : line.objectExcluded) {
				RE::TESForm* currentform = nullptr;
				RE::TESNPC* npc = nullptr;

				std::string string_form = npcstring;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::NPC) {
					npc = (RE::TESNPC*)currentform;

					if (curobj->formID == npc->formID) {
						found = false;
						//logger::info("NPC found.");
						break;
					}
				}
			}
		}

		if (!line.skillRestrict.empty()) {
			for (size_t i = 0; i < line.skillRestrict.size(); i++) {
				auto        skillArr = line.skillRestrict[i];
				std::string skill = skillArr[0];
				int         value = -99;
				int         value1 = -99;
				int         value2 = -99;
				value1 = std::stoi(skillArr[1]);
				value2 = std::stoi(skillArr[2]);

				if (toLowerCase(skill) == "onehanded") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kOneHanded];
				}
				else if (toLowerCase(skill) == "twohanded") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kTwoHanded];
				}
				else if (toLowerCase(skill) == "marksman") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kMarksman];
				}
				else if (toLowerCase(skill) == "block") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kBlock];
				}
				else if (toLowerCase(skill) == "smithing") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kSmithing];
				}
				else if (toLowerCase(skill) == "heavyarmor") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kHeavyArmor];
				}
				else if (toLowerCase(skill) == "lightarmor") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kLightArmor];
				}
				else if (toLowerCase(skill) == "pickpocket") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kPickpocket];
				}
				else if (toLowerCase(skill) == "lockpicking") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kLockpicking];
				}
				else if (toLowerCase(skill) == "sneak") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kSneak];
				}
				else if (toLowerCase(skill) == "alchemy") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kAlchemy];
				}
				else if (toLowerCase(skill) == "speechcraft") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kSpeechcraft];
				}
				else if (toLowerCase(skill) == "alteration") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kAlteration];
				}
				else if (toLowerCase(skill) == "conjuration") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kConjuration];
				}
				else if (toLowerCase(skill) == "destruction") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kDestruction];
				}
				else if (toLowerCase(skill) == "illusion") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kIllusion];
				}
				else if (toLowerCase(skill) == "restoration") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kRestoration];
				}
				else if (toLowerCase(skill) == "enchanting") {
					value = curobj->playerSkills.values[RE::TESNPC::Skills::kEnchanting];
				}
				else {
					//std::cerr << "Unknown skill: " << skill << std::endl;
				}

				if (value >= 0 && value1 >= 0 && value2 >= 0) {
					if (value >= value1 && value <= value2) {
						//logger::debug(FMT_STRING("npc formid: {:08X} {} invalid value found: {} {} {}"), curobj->formID, curobj->fullName, value, value1, value2);
					}
					else {
						found = false;
					}

				}
				else {
					logger::debug(FMT_STRING("npc formid: {:08X} {} invalid value found: {} {} {}"), curobj->formID, curobj->fullName, value, value1, value2);
				}
			}
		}

		if (found && !line.restrictToMaleModelContains.empty()) {

			for (const auto& modelString : line.restrictToMaleModelContains) {
				if (curobj->race) {
					std::string maleModel = normalizePath(trim(toLowerCase(curobj->race->skeletonModels[RE::SEXES::kMale].model.c_str())));
					std::string searchModel = normalizePath(trim(toLowerCase(modelString)));

					if (maleModel.find(searchModel) != std::string::npos) {

					}
					else {
						found = false;
						//break;
					}
				}

			}
		}

		if (!line.restrictGender.empty() && found && line.restrictGender != "none") {
			std::string lowercaseIsFemale = line.restrictGender;
			std::transform(lowercaseIsFemale.begin(), lowercaseIsFemale.end(), lowercaseIsFemale.begin(), [](unsigned char c) { return std::tolower(c); });

			if ((lowercaseIsFemale == "female")) {
				if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale) {
					//found = true;
					//logger::debug(FMT_STRING("is female {:08X} {}"), curobj->formID, curobj->fullName);
				}
				else {
					found = false;
					//logger::debug(FMT_STRING("is not female {:08X} {}"), curobj->formID, curobj->fullName);
				}
			}
			else if ((lowercaseIsFemale == "male")) {
				if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale)) {
					//logger::debug(FMT_STRING("is male {:08X} {}"), curobj->formID, curobj->fullName);
					//found = true;
				}
				else {
					//logger::debug(FMT_STRING("is not male {:08X} {}"), curobj->formID, curobj->fullName);
					found = false;
				}
			}
		}

		if (!line.restrictToTemplateFlags.empty() && found) {
			for (const auto& setFlag : line.restrictToTemplateFlags) {
				std::string lowerFlag = toLowerCase(setFlag);

				static const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG> flagMap = {
					{"traits", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits},
					{"stats", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats},
					{"factions", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions},
					{"spells", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells},
					{"aidata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData},
					{"aipackages", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages},
					{"unused", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused},
					{"basedata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData},
					{"inventory", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory},
					{"script", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript},
					{"aidefpacklist", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList},
					{"attackdata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData},
					{"keywords", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords},
					{"copiedtemplate", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate}
				};


				auto it = flagMap.find(lowerFlag);
				if (it != flagMap.end()) {
					// Check if the flag is set on the NPC
					if (!curobj->actorData.templateUseFlags.any(it->second)) {
						found = false;
						logger::debug(FMT_STRING("npc formid: {:08X} {} is missing required flag: {}"),
							curobj->formID, curobj->fullName, lowerFlag);
						break; // Stop checking once we find a missing flag
					}
				}
				else {
					logger::warn(FMT_STRING("Unknown flag '{}' provided in restrictToTemplateFlags"), lowerFlag);
				}
			}
		}


		if (!line.restrictToFlags.empty() && found) {
			for (const auto& setFlag : line.restrictToFlags) {
				std::string lowerFlag = toLowerCase(setFlag);

				const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::Flag> flagMap = {
					{"female", RE::ACTOR_BASE_DATA::Flag::kFemale},
					{"essential", RE::ACTOR_BASE_DATA::Flag::kEssential},
					{"ischargenfacepreset", RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset},
					{"respawn", RE::ACTOR_BASE_DATA::Flag::kRespawn},
					{"autocalcstats", RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats},
					{"unique", RE::ACTOR_BASE_DATA::Flag::kUnique},
					{"doesntaffectstealthmeter", RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter},
					{"pclevelmult", RE::ACTOR_BASE_DATA::Flag::kPCLevelMult},
					{"usestemplate", RE::ACTOR_BASE_DATA::Flag::kUsesTemplate},
					{"calcfalltemplates", RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates},
					{"protected", RE::ACTOR_BASE_DATA::Flag::kProtected},
					{"norumors", RE::ACTOR_BASE_DATA::Flag::kNoRumors},
					{"summonable", RE::ACTOR_BASE_DATA::Flag::kSummonable},
					{"doesntbleed", RE::ACTOR_BASE_DATA::Flag::kDoesntBleed},
					{"bleedoutoverride", RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride},
					{"oppositegenderanims", RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims},
					{"simpleactor", RE::ACTOR_BASE_DATA::Flag::kSimpleActor},
					{"loopedscript", RE::ACTOR_BASE_DATA::Flag::kLoopedScript},
					{"noactivation", RE::ACTOR_BASE_DATA::Flag::kNoActivation},
					{"loopedaudio", RE::ACTOR_BASE_DATA::Flag::kLoopedAudio},
					{"isghost", RE::ACTOR_BASE_DATA::Flag::kIsGhost},
					{"invulnerable", RE::ACTOR_BASE_DATA::Flag::kInvulnerable}
				};


				auto it = flagMap.find(lowerFlag);
				if (it != flagMap.end()) {
					// Check if the flag is set on the NPC
					if (!curobj->actorData.actorBaseFlags.any(it->second)) {
						found = false;
						logger::debug(FMT_STRING("npc formid: {:08X} {} is missing required flag: {}"),
							curobj->formID, curobj->fullName, lowerFlag);
						break; // Stop checking once we find a missing flag
					}
				}
				else {
					logger::warn(FMT_STRING("Unknown flag '{}' provided in restrictToFlags"), lowerFlag);
				}
			}
		}

		if (found && !line.restrictToCombatStyle.empty()) {
			bool hasObject = false;
			for (const auto& restrictObject : line.restrictToCombatStyle) {
				RE::TESForm* currentform = nullptr;
				RE::TESCombatStyle* tempForm = nullptr;
				std::string         string_form = restrictObject;

				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::CombatStyle) {
					tempForm = (RE::TESCombatStyle*)currentform;
					if (curobj->combatStyle && curobj->combatStyle->formID == tempForm->formID) {
						hasObject = true;
					}
				}
			}
			if (!hasObject) {
				found = false;
			}
		}

		if (found && !line.restrictToRaces.empty()) {
			bool hasObject = false;
			for (const auto& restrictObject : line.restrictToRaces) {
				RE::TESForm* currentform = nullptr;
				RE::TESRace* tempForm = nullptr;
				std::string         string_form = restrictObject;

				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Race) {
					tempForm = (RE::TESRace*)currentform;
					if (curobj->race && curobj->race->formID == tempForm->formID) {
						hasObject = true;
					}
				}
			}
			if (!hasObject) {
				found = false;
			}
		}

		if (found && !line.restrictToVoiceType.empty()) {
			bool hasObject = false;
			for (const auto& restrictObject : line.restrictToVoiceType) {
				RE::TESForm* currentform = nullptr;
				RE::BGSVoiceType* tempForm = nullptr;
				std::string         string_form = restrictObject;

				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::VoiceType) {
					tempForm = (RE::BGSVoiceType*)currentform;
					if (curobj->voiceType && curobj->voiceType->formID == tempForm->formID) {
						hasObject = true;
					}
				}
			}
			if (!hasObject) {
				found = false;
			}
		}



		if (found && !line.restrictToKeyword.empty()) {
			bool hasObject = false;
			for (const auto& restrictObject : line.restrictToKeyword) {
				RE::TESForm* currentform = nullptr;
				RE::BGSKeyword* tempForm = nullptr;
				std::string         string_form = restrictObject;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					tempForm = (RE::BGSKeyword*)currentform;
					if (tempForm && curobj->HasKeyword(tempForm) || curobj->race && curobj->race->HasKeyword(tempForm)) {
						hasObject = true;
					}
				}
			}
			if (!hasObject) {
				found = false;
				//logger::debug("not found");
			}
			//logger::debug("found");
		}


		if (found)
			return true;
		return false;

	}

	REL::Relocation<decltype(&LoadRaceDataHook::Hook_Load)> func;

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x6, &Hook_Load);
		logger::debug("{} Done!", __FUNCTION__);
	}

	bool LoadRaceDataHook::Hook_Load(RE::TESNPC* curobj, RE::TESFile* a_mod)
	{
		auto result = func(curobj, a_mod);
		if (result) {

			//logger::debug("NPC {:08X} {} {}", a_race->formID, a_race->fullName, clib_util::editorID::get_editorID(a_race));

			for (const auto& line : hooktokens) {
				bool found = false;
				if (curobj->IsDeleted()) {
					return result;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const& modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				if (!line.object.empty()) {
					//logger::debug("objects size {}", line.object.size());
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = npcstring;
						//logger::info("looking up npc {:08X} {} {}", curobj->formID, clib_util::editorID::get_editorID(curobj), string_form);
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							if (curobj->formID == currentform->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (found || findObject(line, curobj))
					patch(line, curobj);
			}

		}
		return result;
	}

}
