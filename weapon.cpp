#include "weapon.h"
#include <utility.h>

namespace WEAPON
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		
		// extract objects
		std::regex  objects_regex("filterByWeapons\\s*=([^:]+)", regex::icase);
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
				//logger::info(FMT_STRING("Weapon String: {}"), tempVar);
				if (tempVar != "none") {
					objects.push_back(tempVar);
				}
				++objects_iterator;
			}
			l.object = objects;
		}

		// extract skipFlags
		std::regex  skipFlagsExcluded_regex("filterByFlagsExcluded\\s*=([^:]+)", regex::icase);
		std::smatch skipFlagsExcluded_match;
		std::regex_search(line, skipFlagsExcluded_match, skipFlagsExcluded_regex);
		std::vector<std::string> skipFlagsExcluded;
		if (skipFlagsExcluded_match.empty() || skipFlagsExcluded_match[1].str().empty()) {
			//empty
		} else {
			std::string          skipFlagsExcluded_str = skipFlagsExcluded_match[1];
			std::regex           skipFlagsExcluded_list_regex("[^,]+(\\w+)", regex::icase);
			std::sregex_iterator skipFlagsExcluded_iterator(skipFlagsExcluded_str.begin(), skipFlagsExcluded_str.end(), skipFlagsExcluded_list_regex);
			std::sregex_iterator skipFlagsExcluded_end;
			while (skipFlagsExcluded_iterator != skipFlagsExcluded_end) {
				std::string keyword = (*skipFlagsExcluded_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					skipFlagsExcluded.push_back(keyword);
				}
				++skipFlagsExcluded_iterator;
			}
			l.filterByFlagsExclude = skipFlagsExcluded;
		}

		// extract objectsExcluded
		std::regex  objectsExcluded_regex("filterByWeaponsExcluded\\s*=([^:]+)", regex::icase);
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


		// extract attackDamage
		std::regex  attackDamage_regex("attackDamage\\s*=([^:]+)", regex::icase);
		std::smatch match;
		std::regex_search(line, match, attackDamage_regex);
		// extract the value after the equals sign
		if (match.empty() || match[1].str().empty()) {
			l.attackDamage = "none";
		} else {
			std::string value = match[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.attackDamage = value;
		}

				// extract critDamage
		std::regex  critDamage_regex("critDamage\\s*=([^:]+)", regex::icase);
		std::smatch matchCrit;
		std::regex_search(line, matchCrit, critDamage_regex);
		// extract the value after the equals sign
		if (matchCrit.empty() || matchCrit[1].str().empty()) {
			l.critDamage = "none";
		} else {
			std::string value = matchCrit[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.critDamage = value;
		}

		// extract critPercentMult
		std::regex  critPercentMult_regex("critPercentMult\\s*=([^:]+)", regex::icase);
		std::smatch matchMult;
		std::regex_search(line, matchMult, critPercentMult_regex);
		// extract the value after the equals sign
		if (matchMult.empty() || matchMult[1].str().empty()) {
			l.critPercentMult = "none";
		} else {
			std::string value = matchMult[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.critPercentMult = value;
		}

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectWEAP*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectWEAP* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform) {
						logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESObjectWEAP*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() ) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				//logger::info(FMT_STRING("Its a form! {:08X}"), curobj->formID);


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
								//logger::debug(FMT_STRING("KeywordAnd Weapon does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd Weapon true"));
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
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
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
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}



				if (!found && line.object.empty()  && line.keywords.empty() && line.keywordsOr.empty()) {
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
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"),keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectWEAP* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Weapon) {
							npc = (RE::TESObjectWEAP*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::info("Weapon Excluded.");
								break;
							}
						}
					}
				}

				//if (!line.filterByFlagsExclude.empty()) {
				//	for (const auto& flag : line.filterByFlagsExclude) {
				//		//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude"), curobj->formID, curobj->fullName);
				//		if (toLowerCase(flag) == "notplayable" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kNotPlayable) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude nonplayable set to false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "cantdrop" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCantDrop) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "embeddedweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kEmbeddedWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingattack" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingAttack) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "criteffectondeath" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCritEffectOnDeath) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "boltaction" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kBoltAction) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "automatic" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kAutomatic) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "secondaryweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kSecondaryWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingreload" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingReload) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "holdinputtopower" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kHoldInputToPower) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		}
				//	}
				//}

				if (found) {
					patch(line, curobj);
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

									tokens.push_back(create_patch_instructions(line));
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

	void* patch(WEAPON::patch_instruction line, RE::TESObjectWEAP* curobj)
	{
		
		if (!line.attackDamage.empty() && line.attackDamage != "none") {
			try {
				curobj->attackDamage = std::stof(line.attackDamage);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->attackDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critDamage.empty() && line.critDamage != "none") {
			try {
				curobj->criticalData.damage = std::stof(line.critDamage);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->criticalData.damage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critPercentMult.empty() && line.critPercentMult != "none") {
			try {
				curobj->criticalData.prcntMult = std::stof(line.critPercentMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->criticalData.prcntMult);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.capsvalue.empty() && line.capsvalue != "none") {
			try {
				curobj->value = std::stof(line.capsvalue);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.hittype.empty() && line.hittype != "none") {
			try {
				std::string hittypeLower = line.hittype;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});
				if (hittypeLower == "normal") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "dismemberonly" || hittypeLower == "dismember") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kDismemberOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to dismember only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "explodeOnly" || hittypeLower == "explode") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kExplodeOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to explode only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "nodismemberorexplode" || hittypeLower == "no") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNoDismemberOrExplode;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to no dismember or explode"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.soundlevel.empty() && line.soundlevel != "none") {
			try {
				std::string hittypeLower = line.soundlevel;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});
				if (hittypeLower == "loud") {
					curobj->soundLevel = RE::SOUND_LEVEL::kLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to loud"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "normal") {
					curobj->soundLevel = RE::SOUND_LEVEL::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "silent") {
					curobj->soundLevel = RE::SOUND_LEVEL::kSilent;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to silent"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "veryload") {
					curobj->soundLevel = RE::SOUND_LEVEL::kVeryLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to very load"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "quiet") {
					curobj->soundLevel = RE::SOUND_LEVEL::kQuiet;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to quiet"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

	}

}
