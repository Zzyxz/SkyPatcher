#include "race.h"

namespace RACE
{
	
	std::list<patch_instruction> hooktokens;

	struct patch_instruction create_patch_instruction_hook(const std::string& line)
	{
		patch_instruction hookl;
		extractData(line, "filterByRaces\\s*=([^:]+)", hookl.object);

		extractData(line, "filterByRacesExcluded\\s*=([^:]+)", hookl.objectExcluded);

		extractData(line, "filterByKeywords\\s*=([^:]+)", hookl.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", hookl.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", hookl.keywordsExcluded);

		std::regex  add_regex("attackDataToAdd\\s*=([^:]+)", regex::icase);
		std::smatch add_match;
		std::regex_search(line, add_match, add_regex);
		std::vector<std::string> add;
		if (add_match.empty() || add_match[1].str().empty()) {
			// empty
		} else {
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

			hookl.addedObjects = arr3D;
		}

		std::regex  change_regex("attackDataToChange\\s*=([^:]+)", regex::icase);
		std::smatch change_match;
		std::regex_search(line, change_match, change_regex);
		std::vector<std::string> change;
		if (change_match.empty() || change_match[1].str().empty()) {
			// empty
		} else {
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

			hookl.attackDataToChange = arr3D;
		}

		extractDataStrings(line, "attackDataToRemove\\s*=([^:]+)", hookl.removeAttackData);
		//extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		//logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		//logger::info("returning patch instructions");
		return hookl;
	}
	//
	//
	//
	//
	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;

		//// extract objects
		//std::regex  objects_regex("filterByRaces\\s*=([^:]+)", regex::icase);
		//std::smatch objects_match;
		//std::regex_search(line, objects_match, objects_regex);
		//std::vector<std::string> objects;
		//if (objects_match.empty() || objects_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objects_str = objects_match[1];
		//	std::regex           objects_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
		//	std::sregex_iterator objects_end;
		//	while (objects_iterator != objects_end) {
		//		std::string tempVar = (*objects_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			objects.push_back(tempVar);
		//		}
		//		++objects_iterator;
		//	}
		//	l.object = objects;
		//}

		extractData(line, "filterByRaces\\s*=([^:]+)", l.object);

		//// extract objectsExcluded
		//std::regex  objectsExcluded_regex("filterByRacesExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch objectsExcluded_match;
		//std::regex_search(line, objectsExcluded_match, objectsExcluded_regex);
		//std::vector<std::string> objectsExcluded;
		//if (objectsExcluded_match.empty() || objectsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objectsExcluded_str = objectsExcluded_match[1];
		//	std::regex           objectsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objectsExcluded_iterator(objectsExcluded_str.begin(), objectsExcluded_str.end(), objectsExcluded_list_regex);
		//	std::sregex_iterator objectsExcluded_end;
		//	while (objectsExcluded_iterator != objectsExcluded_end) {
		//		std::string tempVar = (*objectsExcluded_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			objectsExcluded.push_back(tempVar);
		//		}
		//		++objectsExcluded_iterator;
		//	}
		//	l.objectExcluded = objectsExcluded;
		//}

		extractData(line, "filterByRacesExcluded\\s*=([^:]+)", l.objectExcluded);

		//// extract keywords
		//std::regex  keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
		//std::smatch keywords_match;
		//std::regex_search(line, keywords_match, keywords_regex);
		//std::vector<std::string> keywords;
		//if (keywords_match.empty() || keywords_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywords_str = keywords_match[1];
		//	std::regex           keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
		//	std::sregex_iterator keywords_end;
		//	while (keywords_iterator != keywords_end) {
		//		std::string keyword = (*keywords_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywords.push_back(keyword);
		//		}
		//		++keywords_iterator;
		//	}
		//	l.keywords = keywords;
		//}

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		//// extract keywords
		//std::regex  keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsOr_match;
		//std::regex_search(line, keywordsOr_match, keywordsOr_regex);
		//std::vector<std::string> keywordsOr;
		//if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywordsOr_str = keywordsOr_match[1];
		//	std::regex           keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
		//	std::sregex_iterator keywordsOr_end;
		//	while (keywordsOr_iterator != keywordsOr_end) {
		//		std::string keyword = (*keywordsOr_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsOr.push_back(keyword);
		//		}
		//		++keywordsOr_iterator;
		//	}
		//	l.keywordsOr = keywordsOr;
		//}

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);
		extractData(line, "filterByVoiceTypeOr\\s*=([^:]+)", l.filterByVoiceTypeOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		// extract avifs
		std::regex  avifs_regex("changeStats\\s*=([^:]+)", regex::icase);
		std::smatch avifs_match;
		std::regex_search(line, avifs_match, avifs_regex);
		std::vector<std::string> avifs_before_eq;
		std::vector<float>       avifs_min_values;
		std::vector<float>       avifs_max_values;
		if (avifs_match.empty() || avifs_match[1].str().empty()) {
			//empty
		} else {
			std::string          avifs_str = avifs_match[1];
			std::regex           avifs_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
			std::sregex_iterator avifs_iterator(avifs_str.begin(), avifs_str.end(), avifs_list_regex);
			std::sregex_iterator avifs_end;
			while (avifs_iterator != avifs_end) {
				std::string avif = (*avifs_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				avifs_before_eq.push_back(avif);
				avifs_min_values.push_back(std::stof((*avifs_iterator)[2]));
				if ((*avifs_iterator)[3] != "") {
					avifs_max_values.push_back(std::stof((*avifs_iterator)[3]));
				} else {
					avifs_max_values.push_back(std::stof((*avifs_iterator)[2]));
				}
				std::string val1 = ((*avifs_iterator)[2]);
				std::string val2 = ((*avifs_iterator)[3] != "") ? ((*avifs_iterator)[3]) : ((*avifs_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++avifs_iterator;
			}
			l.avifs = avifs_before_eq;
			l.values1 = avifs_min_values;
			l.values2 = avifs_max_values;
		}


		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractDataStrings(line, "filterByMaleModelContains\\s*=([^:]+)", l.filterByMaleModelContains);
		extractDataStrings(line, "filterByFemaleModelContains\\s*=([^:]+)", l.filterByFemaleModelContains);
		extractToArr2D(line, "keywordToAddIfKeywordsEIDDoesNotContain\\s*=([^:]+)", l.keywordsToAddIfKeywordsEIDDoNotContain);
		
		extractData(line, "spellsToAdd\\s*=([^:]+)", l.spellsToAdd);
		extractData(line, "spellsToRemove\\s*=([^:]+)", l.spellsToRemove);
		extractData(line, "shoutsToAdd\\s*=([^:]+)", l.shoutsToAdd);
		extractData(line, "shoutsToRemove\\s*=([^:]+)", l.shoutsToRemove);
		extractData(line, "levSpellsToAdd\\s*=([^:]+)", l.levSpellsToAdd);
		extractData(line, "levSpellsToRemove\\s*=([^:]+)", l.levSpellsToRemove);
		extractValueString(line, "baseMass\\s*=([^:]+)", l.baseMass);
		extractValueString(line, "baseCarryweight\\s*=([^:]+)", l.baseCarryweight);
		extractValueString(line, "skin\\s*=([^:]+)", l.skin);
		extractValueString(line, "startingHealth\\s*=([^:]+)", l.startingHealth);
		extractValueString(line, "startingStamina\\s*=([^:]+)", l.startingStamina);
		extractValueString(line, "startingMagicka\\s*=([^:]+)", l.startingMagicka);
		extractValueString(line, "regenHealth\\s*=([^:]+)", l.regenHealth);
		extractValueString(line, "regenStamina\\s*=([^:]+)", l.regenStamina);
		extractValueString(line, "regenMagicka\\s*=([^:]+)", l.regenMagicka);
		extractValueString(line, "damageUnarmed\\s*=([^:]+)", l.damageUnarmed);
		extractValueString(line, "reachUnarmed\\s*=([^:]+)", l.reachUnarmed);

		extractValueString(line, "baseMassMult\\s*=([^:]+)", l.baseMassMult);
		extractValueString(line, "baseCarryweightMult\\s*=([^:]+)", l.baseCarryweightMult);
		extractValueString(line, "startingHealthMult\\s*= ([^:]+)", l.startingHealthMult);
		extractValueString(line, "startingStaminaMult\\s*= ([^:]+)", l.startingStaminaMult);
		extractValueString(line, "startingMagickaMult\\s*= ([^:]+)", l.startingMagickaMult);
		extractValueString(line, "regenHealthMult\\s*=([^:]+)", l.regenHealthMult);
		extractValueString(line, "regenStaminaMult\\s*=([^:]+)", l.regenStaminaMult);
		extractValueString(line, "regenMagickaMult\\s*=([^:]+)", l.regenMagickaMult);
		extractValueString(line, "damageUnarmedMult\\s*=([^:]+)", l.damageUnarmedMult);
		extractValueString(line, "reachUnarmedMult\\s*=([^:]+)", l.reachUnarmedMult);

		extractValueString(line, "weightMale\\s*=([^:]+)", l.weightMale);
		extractValueString(line, "weightFemale\\s*=([^:]+)", l.weightFemale);
		extractValueString(line, "heightMale\\s*=([^:]+)", l.heightMale);
		extractValueString(line, "heightFemale\\s*=([^:]+)", l.heightFemale);
		extractValueString(line, "heightFemale\\s*=([^:]+)", l.heightFemale);
		extractValueString(line, "accelerationRate\\s*=([^:]+)", l.rateAcceleration);
		extractValueString(line, "decelerationRate\\s*=([^:]+)", l.rateDeceleration);
		extractValueString(line, "accelerationAngularRate\\s*=([^:]+)", l.angularAccelerationRate);
		extractValueString(line, "aimAngleTolerance\\s*=([^:]+)", l.aimAngleTolerance);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		//logger::info("returning patch instructions");
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                 dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESRace*> RaceArray = dataHandler->GetFormArray<RE::TESRace>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESRace* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Race) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESRace*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByVoiceTypeOr.empty() && line.filterByMaleModelContains.empty() && line.filterByFemaleModelContains.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			//logger::info("processing config line");
			for (const auto& curobj : RaceArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool contains = false;
				bool containsOr = false;

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

				if (!line.object.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;
						RE::TESRace* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Race) {
							npc = (RE::TESRace*)currentform;

							if (curobj->formID == npc->formID) {
								found = true;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (!found && !line.filterByMaleModelContains.empty()) {
					//logger::info("npc not empty");
					for (const auto& modelString : line.filterByMaleModelContains) {
						//(toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(modelString)) != std::string::npos)
						//std::string femaleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kFemale].model.c_str())));
						std::string maleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kMale].model.c_str())));
						std::string searchModel = normalizePath(trim(toLowerCase(modelString)));

						//logger::info("ModelFemale: {}", femaleModel);
						//logger::info("ModelMale: {}", maleModel);
						//logger::info("Searchedfor: {} {:08X}", searchModel, curobj->formID);

						        // Additional logging to debug
						//logger::info("Lengths - FemaleModel: {}, MaleModel: {}, SearchModel: {}",
						//	femaleModel.length(), maleModel.length(), searchModel.length());

						//// Hexadecimal representation for debugging
						//logger::info("Hex - FemaleModel: {}", toHex(femaleModel));
						//logger::info("Hex - MaleModel: {}", toHex(maleModel));
						//logger::info("Hex - SearchModel: {}", toHex(searchModel));

						if (maleModel.find(searchModel) != std::string::npos) {
							found = true;
							//logger::info("found model");
							break;
						}
					}
				}

				if (!found && !line.filterByFemaleModelContains.empty()) {
					//logger::info("npc not empty");
					for (const auto& modelString : line.filterByFemaleModelContains) {
						//(toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(modelString)) != std::string::npos)
						std::string femaleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kFemale].model.c_str())));
						//std::string maleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kMale].model.c_str())));
						std::string searchModel = normalizePath(trim(toLowerCase(modelString)));

						//logger::info("ModelFemale: {}", femaleModel);
						//logger::info("ModelMale: {}", maleModel);
						//logger::info("Searchedfor: {} {:08X}", searchModel, curobj->formID);

						// Additional logging to debug
						//logger::info("Lengths - FemaleModel: {}, MaleModel: {}, SearchModel: {}",
						//	femaleModel.length(), maleModel.length(), searchModel.length());

						//// Hexadecimal representation for debugging
						//logger::info("Hex - FemaleModel: {}", toHex(femaleModel));
						//logger::info("Hex - MaleModel: {}", toHex(maleModel));
						//logger::info("Hex - SearchModel: {}", toHex(searchModel));

						if (femaleModel.find(searchModel) != std::string::npos ) {
							found = true;
							//logger::info("found model");
							break;
						}
					}
				}

				if (!found && !line.filterByEditorIdContains.empty()) {
					for (const auto& editorString : line.filterByEditorIdContains) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							contains = true;
						} else {
							contains = false;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByEditorIdContainsOr) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							containsOr = true;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd race does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd race true"));
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X}"), curobj->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching race by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.filterByVoiceTypeOr.empty()) {
					for (const auto& keywordstring : line.filterByVoiceTypeOr) {
						//logger::info("filterByVoiceTypeOr {}", keywordstring);
						RE::TESForm*      currentform = nullptr;
						RE::BGSVoiceType* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::VoiceType) {
							keyword = (RE::BGSVoiceType*)currentform;

							if (curobj->defaultVoiceTypes[RE::SEXES::kMale] && curobj->defaultVoiceTypes[RE::SEXES::kMale]->formID == keyword->formID || curobj->defaultVoiceTypes[RE::SEXES::kFemale] && curobj->defaultVoiceTypes[RE::SEXES::kFemale]->formID == keyword->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByVoiceTypeOr.empty() && line.filterByMaleModelContains.empty() && line.filterByFemaleModelContains.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}



				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::TESRace* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							npc = (RE::TESRace*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (found) {
					patch(line, curobj);
				}

				//
			}
		}
	}

	//
	void hookpatch(RACE::patch_instruction line, RE::TESRace* curobj)
	{
		//logger::debug("start Patch race {:08X}", curobj->formID);
		if (!line.addedObjects.empty()) {
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

			for (auto objectArr : line.addedObjects) {
				for (auto object : objectArr) {
					if (toLowerCase(object[0]) == "key") {
						newAttackData->event = object[1];
						logger::debug("added event: {}", object[1]);
					} else if (toLowerCase(object[0]) == "damagemult") {
						newAttackData->data.damageMult = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
					} else if (toLowerCase(object[0]) == "attackchance") {
						newAttackData->data.attackChance = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
					} else if (toLowerCase(object[0]) == "attackspell") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackSpell = (RE::SpellItem*)form;
								logger::debug("race formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
							}
						} else {
							newAttackData->data.attackSpell = nullptr;
							logger::debug("race formid: {:08X} attackData - attackspell nullptr", curobj->formID);
						}
					} else if (toLowerCase(object[0]) == "attackangle") {
						newAttackData->data.attackAngle = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
					} else if (toLowerCase(object[0]) == "strikeangle") {
						newAttackData->data.strikeAngle = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
					} else if (toLowerCase(object[0]) == "staggeroffset") {
						newAttackData->data.staggerOffset = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
					} else if (toLowerCase(object[0]) == "attacktype") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackType = (RE::BGSKeyword*)form;
								logger::debug("race formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
							}
						} else {
							newAttackData->data.attackType = nullptr;
							logger::debug("race formid: {:08X} attackData - attacktype nullptr", curobj->formID);
						}
					} else if (toLowerCase(object[0]) == "knockdown") {
						newAttackData->data.knockDown = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
					} else if (toLowerCase(object[0]) == "recoverytime") {
						newAttackData->data.recoveryTime = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
					} else if (toLowerCase(object[0]) == "staminamult") {
						newAttackData->data.staminaMult = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
					} else if (toLowerCase(object[0]) == "resetflags") {
						newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
						logger::debug(FMT_STRING("race formid: {:08X} attackData - all flags reset"), curobj->formID);
					} else if (toLowerCase(object[0]) == "removeflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerRemoveFlag = toLowerCase(object[i]);
							if (lowerRemoveFlag == "ignoreweapon") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
							} else if (lowerRemoveFlag == "bashattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "powerattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "chargeattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "rotatingattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "continuousattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "overridedata") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
							} else {
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
							}
						}

					} else if (toLowerCase(object[0]) == "setflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerSetFlag = toLowerCase(object[i]);
							if (lowerSetFlag == "ignoreweapon") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
							} else if (lowerSetFlag == "bashattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
							} else if (lowerSetFlag == "powerattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
							} else if (lowerSetFlag == "chargeattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
							} else if (lowerSetFlag == "rotatingattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
							} else if (lowerSetFlag == "continuousattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
							} else if (lowerSetFlag == "overridedata") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
							} else {
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
							}
						}
					} else {
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
						logger::debug("race formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					} else {
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
							} else {
								logger::debug("race formid: {:08X} attackData change - entry found {}", curobj->formID, newAttackData->event.c_str());
								key = false;
								continue;
							}
						}

						if (toLowerCase(object[0]) == "damagemult") {
							newAttackData->data.damageMult = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - damagemult {}", curobj->formID, newAttackData->data.damageMult);
						} else if (toLowerCase(object[0]) == "attackchance") {
							newAttackData->data.attackChance = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - attackchance {}", curobj->formID, newAttackData->data.attackChance);
						} else if (toLowerCase(object[0]) == "attackspell") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackSpell = (RE::SpellItem*)form;
									logger::debug("race formid: {:08X} attackData changed - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
								}
							} else {
								newAttackData->data.attackSpell = nullptr;
								logger::debug("race formid: {:08X} attackData changed - attackspell nullptr", curobj->formID);
							}
						} else if (toLowerCase(object[0]) == "attackangle") {
							newAttackData->data.attackAngle = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
						} else if (toLowerCase(object[0]) == "strikeangle") {
							newAttackData->data.strikeAngle = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
						} else if (toLowerCase(object[0]) == "staggeroffset") {
							newAttackData->data.staggerOffset = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
						} else if (toLowerCase(object[0]) == "attacktype") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackType = (RE::BGSKeyword*)form;
									logger::debug("race formid: {:08X} attackData changed - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
								}
							} else {
								newAttackData->data.attackType = nullptr;
								logger::debug("race formid: {:08X} attackData changed - attacktype nullptr", curobj->formID);
							}
						} else if (toLowerCase(object[0]) == "knockdown") {
							newAttackData->data.knockDown = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - knockdown {}", curobj->formID, newAttackData->data.knockDown);
						} else if (toLowerCase(object[0]) == "recoverytime") {
							newAttackData->data.recoveryTime = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
						} else if (toLowerCase(object[0]) == "staminamult") {
							newAttackData->data.staminaMult = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
						} else if (toLowerCase(object[0]) == "resetflags") {
							newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
							logger::debug(FMT_STRING("race formid: {:08X} attackData - all flags reset"), curobj->formID);
						} else if (toLowerCase(object[0]) == "removeflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerRemoveFlag = toLowerCase(object[i]);
								if (lowerRemoveFlag == "ignoreweapon") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
								} else if (lowerRemoveFlag == "bashattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "powerattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "chargeattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "rotatingattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "continuousattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "overridedata") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
								} else {
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
								}
							}

						} else if (toLowerCase(object[0]) == "setflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerSetFlag = toLowerCase(object[i]);
								if (lowerSetFlag == "ignoreweapon") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
								} else if (lowerSetFlag == "bashattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
								} else if (lowerSetFlag == "powerattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
								} else if (lowerSetFlag == "chargeattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
								} else if (lowerSetFlag == "rotatingattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
								} else if (lowerSetFlag == "continuousattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
								} else if (lowerSetFlag == "overridedata") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
								} else {
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
								}
							}
						} else {
							// Handle the case where the keyword is not recognized
							// You may want to log an error or take appropriate action
							logger::debug("Unrecognized keyword: {}", object[0]);
						}
					}
				}
			}

			//

			for (auto attackData : line.removeAttackData) {
				//logger::debug("string {}", attackData);
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					//logger::debug("for {}", it->second.get()->event.c_str());
					if (toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
						logger::debug("race formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					} else {
						// Move to the next iterator
						++it;
					}
				}
			}
		}
	}
	//

	void patch(RACE::patch_instruction line, RE::TESRace* curobj)
	{
		if (!line.avifs.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.avifs.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.avifs[i];

				//logger::info("avif valid!");
				float randomValue = 0;
				if (!line.values1.empty() && !line.values2.empty()) {
					//std::srand(std::time(nullptr));
					randomValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
				}

				if (toLowerCase(string_form) == "health") {
					curobj->data.startingHealth = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (toLowerCase(string_form) == "stamina") {
					curobj->data.startingStamina = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (toLowerCase(string_form) == "magicka") {
					curobj->data.startingMagicka = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				}
				logger::debug(FMT_STRING("race formid: {:08X} {} changed {:08X} {} {}"), curobj->formID, curobj->formEditorID, ((RE::ActorValueInfo*)currentform)->formID, ((RE::ActorValueInfo*)currentform)->fullName, randomValue);
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
						logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToAddIfKeywordsEIDDoNotContain.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (const auto& objectToAdd : line.keywordsToAddIfKeywordsEIDDoNotContain) {
				std::string        eidkeyword = (objectToAdd[0]);
				std::string  keywordString = (objectToAdd[1]);
				RE::TESForm* currentform = nullptr;
				bool               addKeyword = true;
				currentform = GetFormFromIdentifier(keywordString);
				//logger::info("keywordstring {}", keywordString);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					//logger::info("keyword found ");
					for (std::uint32_t i = 0; i < curobj->numKeywords; ++i) {
						auto keyword = curobj->keywords[i];
						//logger::info("loop keywords {} keyword {} search for {} ", i, keyword->formEditorID, eidkeyword);
						if (keyword && toLowerCase(keyword->formEditorID.c_str()).find(toLowerCase(eidkeyword)) != std::string::npos) {
							addKeyword = false;
							//logger::info("keywords has keyword with editor ID ");
						}

					}

					if (addKeyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} (keywordToAddIfKeywordsEIDDoNotContain)"), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
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
					logger::debug(FMT_STRING("race formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.spellsToAdd.empty()) {
			for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddSpell((RE::SpellItem*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} added spell {:08X} "), curobj->formID, ((RE::SpellItem*)currentform)->formID);
				}
			}
		}

		if (!line.spellsToRemove.empty()) {
			for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					curobj->actorEffects->RemoveSpell((RE::SpellItem*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} removed spell {:08X} "), curobj->formID, ((RE::SpellItem*)currentform)->formID);
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
					logger::debug(FMT_STRING("race formid: {:08X} added levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
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
					logger::debug(FMT_STRING("race formid: {:08X} removed levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
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
					logger::debug(FMT_STRING("race formid: {:08X} added shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
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
					logger::debug(FMT_STRING("race formid: {:08X} removed shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
				}
			}
		}

		if (!line.baseMass.empty() && line.baseMass != "none") {
			try {
				curobj->data.baseMass = std::stof(line.baseMass);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed baseMass {}"), curobj->formID, curobj->fullName, curobj->data.baseMass);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCarryweight.empty() && line.baseCarryweight != "none") {
			try {
				curobj->data.baseCarryWeight = std::stof(line.baseCarryweight);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingHealth.empty() && line.startingHealth != "none") {
			try {
				curobj->data.startingHealth = std::stof(line.startingHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingStamina.empty() && line.startingStamina != "none") {
			try {
				curobj->data.startingStamina = std::stof(line.startingStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingMagicka.empty() && line.startingMagicka != "none") {
			try {
				curobj->data.startingMagicka = std::stof(line.startingMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenHealth.empty() && line.regenHealth != "none") {
			try {
				curobj->data.healthRegen = std::stof(line.regenHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenStamina.empty() && line.regenStamina != "none") {
			try {
				curobj->data.staminaRegen = std::stof(line.regenStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenMagicka.empty() && line.regenMagicka != "none") {
			try {
				curobj->data.magickaRegen = std::stof(line.regenMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageUnarmed.empty() && line.damageUnarmed != "none") {
			try {
				curobj->data.unarmedDamage = std::stof(line.damageUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reachUnarmed.empty() && line.reachUnarmed != "none") {
			try {
				curobj->data.unarmedReach = std::stof(line.reachUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseMassMult.empty() && line.baseMassMult != "none") {
			try {
				curobj->data.baseMass = curobj->data.baseMass * std::stof(line.baseMassMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseMass {}"), curobj->formID, curobj->fullName, curobj->data.baseMass);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCarryweightMult.empty() && line.baseCarryweightMult != "none") {
			try {
				curobj->data.baseCarryWeight = curobj->data.baseCarryWeight * std::stof(line.baseCarryweightMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingHealthMult.empty() && line.startingHealthMult != "none") {
			try {
				curobj->data.startingHealth = curobj->data.startingHealth * std::stof(line.startingHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingStaminaMult.empty() && line.startingStaminaMult != "none") {
			try {
				curobj->data.startingStamina = curobj->data.startingStamina * std::stof(line.startingStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingMagickaMult.empty() && line.startingMagickaMult != "none") {
			try {
				curobj->data.startingMagicka = curobj->data.startingMagicka * std::stof(line.startingMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenHealthMult.empty() && line.regenHealthMult != "none") {
			try {
				curobj->data.healthRegen = curobj->data.healthRegen * std::stof(line.regenHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenStaminaMult.empty() && line.regenStaminaMult != "none") {
			try {
				curobj->data.staminaRegen = curobj->data.staminaRegen * std::stof(line.regenStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenMagickaMult.empty() && line.regenMagickaMult != "none") {
			try {
				curobj->data.magickaRegen = curobj->data.magickaRegen * std::stof(line.regenMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageUnarmedMult.empty() && line.damageUnarmedMult != "none") {
			try {
				curobj->data.unarmedDamage = curobj->data.unarmedDamage * std::stof(line.damageUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reachUnarmedMult.empty() && line.reachUnarmedMult != "none") {
			try {
				curobj->data.unarmedReach = curobj->data.unarmedReach * std::stof(line.reachUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.skin.empty() && line.skin != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.skin;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					curobj->skin = (RE::TESObjectARMO*)currentform;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed skin {:08X}"), curobj->formID, curobj->fullName, curobj->skin->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.heightMale.empty() && line.heightMale != "none") {
			try {
				curobj->data.height[RE::SEXES::kMale] = std::stof(line.heightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightMale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kMale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.heightFemale.empty() && line.heightFemale != "none") {
			try {
				curobj->data.height[RE::SEXES::kFemale] = std::stof(line.heightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightFemale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kFemale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightMale.empty() && line.weightMale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kMale] = std::stof(line.weightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightMale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kMale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightFemale.empty() && line.weightFemale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kFemale] = std::stof(line.weightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightFemale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kFemale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rateAcceleration.empty() && line.rateAcceleration != "none") {
			try {
				curobj->data.accelerate = std::stof(line.rateAcceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateAcceleration {}"), curobj->formID, curobj->fullName, curobj->data.accelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rateDeceleration.empty() && line.rateDeceleration != "none") {
			try {
				curobj->data.decelerate = std::stof(line.rateDeceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateDeceleration {}"), curobj->formID, curobj->fullName, curobj->data.decelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.angularAccelerationRate.empty() && line.angularAccelerationRate != "none") {
			try {
				curobj->data.angleAccelerate = std::stof(line.angularAccelerationRate);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed angularAccelerationRate {}"), curobj->formID, curobj->fullName, curobj->data.angleAccelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.aimAngleTolerance.empty() && line.aimAngleTolerance != "none") {
			try {
				curobj->data.aimAngleTolerance = std::stof(line.aimAngleTolerance);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed aimAngleTolerance {}"), curobj->formID, curobj->fullName, curobj->data.aimAngleTolerance);
			} catch (const std::invalid_argument& e) {
			}
		}

		//if (!line.addedObjects.empty()) {
		//	bool               key = true;
		//	RE::BGSAttackData* newAttackData = RE::BGSAttackData::Create();
		//	newAttackData->IncRefCount();
		//	//newAttackData->event = "attackStart";
		//	newAttackData->data.damageMult = 1;
		//	newAttackData->data.attackChance = 1;
		//	newAttackData->data.attackSpell = nullptr;
		//	newAttackData->data.attackAngle = 0;
		//	newAttackData->data.strikeAngle = 50;
		//	newAttackData->data.staggerOffset = 0;
		//	newAttackData->data.attackType = nullptr;
		//	newAttackData->data.knockDown = 0;
		//	newAttackData->data.recoveryTime = 0;
		//	newAttackData->data.staminaMult = 1;
		//	newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
		//	//logger::debug("attackData size before {}", curobj->attackDataMap.get()->attackDataMap.size());
		//	curobj->attackDataMap->attackDataMap.insert({ newAttackData->event, RE::NiPointer(newAttackData) });
		//	//logger::debug("attackData size after {}", curobj->attackDataMap.get()->attackDataMap.size());

		//	for (auto object : line.addedObjects) {
		//		if (key) {
		//			newAttackData->event = object[0];
		//			logger::debug("added event: {}", object[0]);
		//			key = false;
		//			continue;
		//		}

		//		if (toLowerCase(object[0]) == "damagemult") {
		//			newAttackData->data.damageMult = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
		//		} else if (toLowerCase(object[0]) == "attackchance") {
		//			newAttackData->data.attackChance = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
		//		} else if (toLowerCase(object[0]) == "attackspell") {
		//			if (object[1] != "null") {
		//				auto form = GetFormFromIdentifier(object[1]);
		//				if (form) {
		//					newAttackData->data.attackSpell = (RE::SpellItem*)form;
		//					logger::debug("race formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
		//				}
		//			} else {
		//				newAttackData->data.attackSpell = nullptr;
		//				logger::debug("race formid: {:08X} attackData - attackspell nullptr", curobj->formID);
		//			}
		//		} else if (toLowerCase(object[0]) == "attackangle") {
		//			newAttackData->data.attackAngle = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
		//		} else if (toLowerCase(object[0]) == "strikeangle") {
		//			newAttackData->data.strikeAngle = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
		//		} else if (toLowerCase(object[0]) == "staggeroffset") {
		//			newAttackData->data.staggerOffset = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
		//		} else if (toLowerCase(object[0]) == "attacktype") {
		//			if (object[1] != "null") {
		//				auto form = GetFormFromIdentifier(object[1]);
		//				if (form) {
		//					newAttackData->data.attackType = (RE::BGSKeyword*)form;
		//					logger::debug("race formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
		//				}
		//			} else {
		//				newAttackData->data.attackType = nullptr;
		//				logger::debug("race formid: {:08X} attackData - attacktype nullptr", curobj->formID);
		//			}
		//		} else if (toLowerCase(object[0]) == "knockdown") {
		//			newAttackData->data.knockDown = std::stoi(object[1]);
		//			logger::debug("race formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
		//		} else if (toLowerCase(object[0]) == "recoverytime") {
		//			newAttackData->data.recoveryTime = std::stoi(object[1]);
		//			logger::debug("race formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
		//		} else if (toLowerCase(object[0]) == "staminamult") {
		//			newAttackData->data.staminaMult = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
		//		} else if (toLowerCase(object[0]) == "flags") {
		//			newAttackData->data.flags = static_cast<RE::AttackData::AttackFlag>(std::stoi(object[1]));
		//			logger::debug("race formid: {:08X} attackData - flags ", curobj->formID);
		//		} else {
		//			// Handle the case where the keyword is not recognized
		//			// You may want to log an error or take appropriate action
		//			logger::debug("Unrecognized keyword: {}", toLowerCase(object[0]));
		//		}
		//	}
		//}
	}

	void* readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR*                   dir;
		struct dirent*         ent;
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
						} else {
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
								process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

	void* readConfigHook(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR*                   dir;
		struct dirent*         ent;
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
						} else {
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

									hooktokens.push_back(create_patch_instruction_hook(line));
								}
								infile.close();
								//process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

	REL::Relocation<decltype(&LoadRaceDataHook::Hook_InitItemImpl)> func;

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x13, &Hook_InitItemImpl);
		logger::debug("{} Done!", __FUNCTION__);
	}

	bool LoadRaceDataHook::Hook_InitItemImpl(RE::TESRace* curobj)
	{

		auto result = func(curobj);
		if (result) {

			//logger::info("processing npc {:08X} {}", curobj->formID, clib_util::editorID::get_editorID(curobj));
			for (const auto& line : hooktokens) {
				//logger::info("processing config line");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

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
								//logger::debug("found objects curobj {:08X} currentform {:08X} string {}", curobj->formID, currentform->formID, string_form);
								found = true;
								break;
							}
								
								//hookpatch(line, curobj);
							//continue;
						}
					}
				}

				//if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
				//	//logger::info("continue");
				//	continue;
				//}

				//if (!line.object.empty()) {
				//	//logger::info("npc not empty");
				//	for (const auto& npcstring : line.object) {
				//		RE::TESForm* currentform = nullptr;
				//		RE::TESRace* npc = nullptr;

				//		std::string string_form = npcstring;
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::FormType::Race) {
				//			npc = (RE::TESRace*)currentform;

				//			if (curobj->formID == npc->formID) {
				//				found = true;
				//				//logger::info("NPC found.");
				//				break;
				//			}
				//		}
				//	}
				//}
				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd race does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd race true"));
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X}"), curobj->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching race by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
					//logger::debug("found keywords");
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
					//logger::debug("found line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()");
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::TESRace* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							npc = (RE::TESRace*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (found) {
					//logger::debug("hookpatch");
					hookpatch(line, curobj);
				}
			}

			return result;
		}
	}
}
