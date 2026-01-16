#include "faction.h"
#include <utility.h>

namespace FACTION
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		
		//// extract objects
		//std::regex  objects_regex("filterByFactions\\s*=([^:]+)", regex::icase);
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
		//		//logger::info(FMT_STRING("Projectile String: {}"), tempVar);
		//		if (tempVar != "none") {
		//			objects.push_back(tempVar);
		//		}
		//		++objects_iterator;
		//	}
		//	l.object = objects;
		//}

		extractData(line, "filterByFactions\\s*=([^:]+)", l.object);


		//// extract objectsExcluded
		//std::regex  objectsExcluded_regex("filterByFactionsExcluded\\s*=([^:]+)", regex::icase);
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

		extractData(line, "filterByFactionsExcluded\\s*=([^:]+)", l.objectExcluded);


		


		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESFaction*> ObjectArray = dataHandler->GetFormArray<RE::TESFaction>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESFaction* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Faction) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESFaction*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty()  ) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;



				if (!curobj ||curobj->IsDeleted()) {
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



				if (!found && line.object.empty()  && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
				}



				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectWEAP* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Faction) {
							npc = (RE::TESObjectWEAP*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::info("Projectile Excluded.");
								break;
							}
						}
					}
				}
			
				


				

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

	void* patch(FACTION::patch_instruction line, RE::TESFaction* curobj)
	{


		// Remove Flags
		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				auto lowerRemoveFlag = toLowerCase(std::string(removeFlag));
				if (lowerRemoveFlag == "hiddenfromnpc") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kHiddenFromNPC);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled HiddenFromNPC"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "specialcombat") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kSpecialCombat);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled SpecialCombat"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "playerisexpelled") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kPlayerIsExpelled);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled PlayerIsExpelled"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "playerisenemy") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kPlayerIsEnemy);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled PlayerIsEnemy"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "trackcrime") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kTrackCrime);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled TrackCrime"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_murder") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Murder);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Murder"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_assult") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Assult);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Assult"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_stealing") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Stealing);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Stealing"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ingorescrimes_trespass") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IngoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "donotreportcrimesagainstmembers") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kDoNotReportCrimesAgainstMembers);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled DoNotReportCrimesAgainstMembers"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "crimegold_usedefaults") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kCrimeGold_UseDefaults);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled CrimeGold_UseDefaults"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_pickpocket") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Pickpocket);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Pickpocket"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "vendor") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kVendor);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled Vendor"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "canbeowner") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kCanBeOwner);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled CanBeOwner"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_werewolf") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Werewolf);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Werewolf"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		// Set Flags
		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				auto lowerSetFlag = toLowerCase(std::string(setFlag));
				if (lowerSetFlag == "hiddenfromnpc") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kHiddenFromNPC);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled HiddenFromNPC"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "specialcombat") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kSpecialCombat);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled SpecialCombat"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "playerisexpelled") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kPlayerIsExpelled);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled PlayerIsExpelled"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "playerisenemy") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kPlayerIsEnemy);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled PlayerIsEnemy"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "trackcrime") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kTrackCrime);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled TrackCrime"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_murder") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Murder);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Murder"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_assult") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Assult);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Assult"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_stealing") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Stealing);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Stealing"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ingorescrimes_trespass") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_trespass") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "donotreportcrimesagainstmembers") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kDoNotReportCrimesAgainstMembers);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled DoNotReportCrimesAgainstMembers"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "crimegold_usedefaults") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kCrimeGold_UseDefaults);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled CrimeGold_UseDefaults"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_pickpocket") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Pickpocket);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Pickpocket"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "vendor") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kVendor);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled Vendor"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "canbeowner") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kCanBeOwner);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled CanBeOwner"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_werewolf") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Werewolf);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Werewolf"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		
		//auto test = *curobj->reactions.begin();
		//test->fightReaction = RE::FIGHT_REACTION::kEnemy;
		

	}

}
