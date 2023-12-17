#include "race.h"

namespace RACE
{

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;

		// extract objects
		std::regex  objects_regex("filterByRaces\\s*=([^:]+)", regex::icase);
		std::smatch objects_match;
		std::regex_search(line, objects_match, objects_regex);
		std::vector<std::string> objects;
		if (objects_match.empty() || objects_match[1].str().empty()) {
			//empty
		} else {
			std::string          objects_str = objects_match[1];
			std::regex           objects_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
			std::sregex_iterator objects_end;
			while (objects_iterator != objects_end) {
				std::string tempVar = (*objects_iterator)[0].str();
				tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
				tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
				//logger::info(FMT_STRING("Race: {}"), race);
				if (tempVar != "none") {
					objects.push_back(tempVar);
				}
				++objects_iterator;
			}
			l.object = objects;
		}

		// extract objectsExcluded
		std::regex  objectsExcluded_regex("filterByRacesExcluded\\s*=([^:]+)", regex::icase);
		std::smatch objectsExcluded_match;
		std::regex_search(line, objectsExcluded_match, objectsExcluded_regex);
		std::vector<std::string> objectsExcluded;
		if (objectsExcluded_match.empty() || objectsExcluded_match[1].str().empty()) {
			//empty
		} else {
			std::string          objectsExcluded_str = objectsExcluded_match[1];
			std::regex           objectsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator objectsExcluded_iterator(objectsExcluded_str.begin(), objectsExcluded_str.end(), objectsExcluded_list_regex);
			std::sregex_iterator objectsExcluded_end;
			while (objectsExcluded_iterator != objectsExcluded_end) {
				std::string tempVar = (*objectsExcluded_iterator)[0].str();
				tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
				tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
				//logger::info(FMT_STRING("Race: {}"), race);
				if (tempVar != "none") {
					objectsExcluded.push_back(tempVar);
				}
				++objectsExcluded_iterator;
			}
			l.objectExcluded = objectsExcluded;
		}

		// extract keywords
		std::regex  keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
		std::smatch keywords_match;
		std::regex_search(line, keywords_match, keywords_regex);
		std::vector<std::string> keywords;
		if (keywords_match.empty() || keywords_match[1].str().empty()) {
			//empty
		} else {
			std::string          keywords_str = keywords_match[1];
			std::regex           keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
			std::sregex_iterator keywords_end;
			while (keywords_iterator != keywords_end) {
				std::string keyword = (*keywords_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					keywords.push_back(keyword);
				}
				++keywords_iterator;
			}
			l.keywords = keywords;
		}

		// extract keywords
		std::regex  keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
		std::smatch keywordsOr_match;
		std::regex_search(line, keywordsOr_match, keywordsOr_regex);
		std::vector<std::string> keywordsOr;
		if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
			//empty
		} else {
			std::string          keywordsOr_str = keywordsOr_match[1];
			std::regex           keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
			std::sregex_iterator keywordsOr_end;
			while (keywordsOr_iterator != keywordsOr_end) {
				std::string keyword = (*keywordsOr_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					keywordsOr.push_back(keyword);
				}
				++keywordsOr_iterator;
			}
			l.keywordsOr = keywordsOr;
		}

		// extract keywords
		std::regex  keywordsExcluded_regex("filterByKeywordsExcluded\\s*=([^:]+)", regex::icase);
		std::smatch keywordsExcluded_match;
		std::regex_search(line, keywordsExcluded_match, keywordsExcluded_regex);
		std::vector<std::string> keywordsExcluded;
		if (keywordsExcluded_match.empty() || keywordsExcluded_match[1].str().empty()) {
			//empty
		} else {
			std::string          keywordsExcluded_str = keywordsExcluded_match[1];
			std::regex           keywordsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywordsExcluded_iterator(keywordsExcluded_str.begin(), keywordsExcluded_str.end(), keywordsExcluded_list_regex);
			std::sregex_iterator keywordsExcluded_end;
			while (keywordsExcluded_iterator != keywordsExcluded_end) {
				std::string keyword = (*keywordsExcluded_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					keywordsExcluded.push_back(keyword);
				}
				++keywordsExcluded_iterator;
			}
			l.keywordsExcluded = keywordsExcluded;
		}

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

		// extract keywordsToAdd
		std::regex  keywordsToAdd_regex("keywordsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch keywordsToAdd_match;
		std::regex_search(line, keywordsToAdd_match, keywordsToAdd_regex);
		std::vector<std::string> keywordsToAdd;
		if (keywordsToAdd_match.empty() || keywordsToAdd_match[1].str().empty()) {
			//empty
		} else {
			std::string          keywordsToAdd_str = keywordsToAdd_match[1];
			std::regex           keywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywordsToAdd_iterator(keywordsToAdd_str.begin(), keywordsToAdd_str.end(), keywordsToAdd_list_regex);
			std::sregex_iterator keywordsToAdd_end;
			while (keywordsToAdd_iterator != keywordsToAdd_end) {
				std::string keywordToAdd = (*keywordsToAdd_iterator)[0].str();
				keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
				keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
				if (keywordToAdd != "none") {
					//logger::info(FMT_STRING("keywordsToAdd: {}"), keywordToAdd);
					keywordsToAdd.push_back(keywordToAdd);
				}
				++keywordsToAdd_iterator;
			}
			l.keywordsToAdd = keywordsToAdd;
		}

		// extract keywordsToRemove
		std::regex  keywordsToRemove_regex("keywordsToRemove\\s*=([^:]+)", regex::icase);
		std::smatch keywordsToRemove_match;
		std::regex_search(line, keywordsToRemove_match, keywordsToRemove_regex);
		std::vector<std::string> keywordsToRemove;
		if (keywordsToRemove_match.empty() || keywordsToRemove_match[1].str().empty()) {
			//empty
		} else {
			std::string          keywordsToRemove_str = keywordsToRemove_match[1];
			std::regex           keywordsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywordsToRemove_iterator(keywordsToRemove_str.begin(), keywordsToRemove_str.end(), keywordsToRemove_list_regex);
			std::sregex_iterator keywordsToRemove_end;
			while (keywordsToRemove_iterator != keywordsToRemove_end) {
				std::string keywordToRemove = (*keywordsToRemove_iterator)[0].str();
				keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
				keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
				if (keywordToRemove != "none") {
					//logger::info(FMT_STRING("keywordsToRemove: {}"), keywordToRemove);
					keywordsToRemove.push_back(keywordToRemove);
				}
				++keywordsToRemove_iterator;
			}
			l.keywordsToRemove = keywordsToRemove;
		}

		// extract spellsToAdd
		std::regex  spellsToAdd_regex("spellsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch spellsToAdd_match;
		std::regex_search(line, spellsToAdd_match, spellsToAdd_regex);
		std::vector<std::string> spellsToAdd;
		if (spellsToAdd_match.empty() || spellsToAdd_match[1].str().empty()) {
			//empty
		} else {
			std::string          spellsToAdd_str = spellsToAdd_match[1];
			std::regex           spellsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator spellsToAdd_iterator(spellsToAdd_str.begin(), spellsToAdd_str.end(), spellsToAdd_list_regex);
			std::sregex_iterator spellsToAdd_end;
			while (spellsToAdd_iterator != spellsToAdd_end) {
				std::string spellToAdd = (*spellsToAdd_iterator)[0].str();
				spellToAdd.erase(spellToAdd.begin(), std::find_if_not(spellToAdd.begin(), spellToAdd.end(), ::isspace));
				spellToAdd.erase(std::find_if_not(spellToAdd.rbegin(), spellToAdd.rend(), ::isspace).base(), spellToAdd.end());
				if ((*spellsToAdd_iterator)[0].str() != "none") {
					//logger::info(FMT_STRING("spellsToAdd: {}"), spellToAdd);
					spellsToAdd.push_back(spellToAdd);
				}
				++spellsToAdd_iterator;
			}
			l.spellsToAdd = spellsToAdd;
		}

		std::regex  add_regex("attackDatasToAdd\\s*=([^:]+)", regex::icase);
		std::smatch add_match;
		std::regex_search(line, add_match, add_regex);
		std::vector<std::string> add;
		if (add_match.empty() || add_match[1].str().empty()) {
			//empty
		} else {
			std::string              valueLine = add_match[1].str();
			std::vector<std::string> arr;

			// exclude the addToLL= part from the first string
			size_t      startPos = valueLine.find("=") + 1;
			size_t      pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);
				startPos = pos + 1;
			}
			token = valueLine.substr(startPos);
			token = trim(token);  // remove leading and trailing white spaces
			arr.push_back(token);

			std::vector<std::vector<std::string>> arr2D(arr.size());

			for (int i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				size_t                   innerPos = 0;
				std::string              innerToken;
				while ((innerPos = arr[i].find("~")) != std::string::npos) {
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
			l.addedObjects = arr2D;
		}

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
			//logger::info("processing config line");
			for (const auto& curobj : RaceArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

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

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty()) {
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

				if (!line.objectExcluded.empty()) {
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

				if (found && !line.avifs.empty()) {
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
				if (found && !line.keywordsToAdd.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = line.keywordsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							curobj->AddKeyword((RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}

				if (found && !line.keywordsToRemove.empty()) {
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

				if (!line.addedObjects.empty()) {
					bool key = true;
					auto aa = new RE::BGSAttackData;
					//RE::BSFixedString myString("aa");
					
					RE::BSTTuple<RE::BSFixedString, RE::BGSAttackDataPtr> tmip;

					//auto newTuple = new RE::BSTTuple<const RE::BSFixedString, RE::BGSAttackDataPtr>("Test",);
					//newTuple->first.
					for (const auto& objectToAdd : line.addedObjects) {
						//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
						if (key) {
							//newTuple-> = objectToAdd[0]
						}
						std::string addFormStr = objectToAdd[0];
						uint16_t    level = std::stoi(objectToAdd[1]);
						uint16_t    count = std::stoi(objectToAdd[2]);
						//uint8_t      chanceNone = std::stoi(objectToAdd[3]);
						RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

						/*	if (addForm) {
							llVec.push_back({ addForm, count, level });
							logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
						} else {
							logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
						}*/
					}

					//auto newAttackData = new RE::BGSAttackData;

					    // Create a newTuple
					RE::BSFixedString                key("attackData");
					RE::NiPointer<RE::BGSAttackData> value;

					//setup for insert.
					auto& table = curobj->attackDataMap.get()->attackDataMap;
					auto  tMap = new RE::BSTHashMap<RE::BSFixedString, RE::BGSAttackDataPtr>;
					tMap->insert(tmip);
					table.insert(tMap->begin(), tMap->end());
					RE::BGSAttackData                                     aData;
					
					RE::BGSAttackData* bgsAttackData = new RE::BGSAttackData();
					RE::BGSAttackDataPtr bgsAttackDataPtr(bgsAttackData);
				

				}

				for (auto &tmap : curobj->attackDataMap.get()->attackDataMap) {
					//RE::BSTTuple<const RE::BSFixedString, RE::BGSAttackDataPtr> tmap;
					auto data = tmap.second.get();
					
					//tmap.first.
				}
					


				if (found && !line.spellsToAdd.empty()) {
					//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = line.spellsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Spell) {
							if (!curobj->actorEffects) {
								curobj->actorEffects = new RE::TESSpellList::SpellData();
							}
							curobj->actorEffects->AddSpell((RE::SpellItem*)currentform);
							logger::debug(FMT_STRING("race formid: {:08X} added spell {:08X} "), curobj->formID,  ((RE::SpellItem*)currentform)->formID);
						}
					}
				}
			}
		}
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

}
