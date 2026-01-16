#include "encounterzone.h"
#include <utility.h>

namespace ENCOUNTERZONE
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;


		extractData(line, "filterByEncounterZones\\s*=([^:]+)", l.object);

		extractData(line, "filterByEncounterZonesExcluded\\s*=([^:]+)", l.objectExcluded);

		extractValueString(line, "minLevel\\s*=([^:]+)", l.minLevel);
		extractValueString(line, "minLevelAdd\\s*=([^:]+)", l.minLevelAdd);
		extractValueString(line, "minLevelMult\\s*=([^:]+)", l.minLevelMult);
		extractValueString(line, "maxLevel\\s*=([^:]+)", l.maxLevel);
		extractValueString(line, "maxLevelAdd\\s*=([^:]+)", l.maxLevelAdd);
		extractValueString(line, "maxLevelMult\\s*=([^:]+)", l.maxLevelMult);

		extractValueString(line, "location\\s*=([^:]+)", l.location);

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::BGSEncounterZone*> ObjectArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSEncounterZone* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::EncounterZone) {
						curobj = (RE::BGSEncounterZone*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::debug("{:08X} editorID {}", curobj->formID, clib_util::editorID::get_editorID(curobj));
				//logger::debug("{:08X} editorID {}", RE::TESForm::LookupByEditorID("Saw01")->formID, "Saw01");
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

				
				if (!found && !line.filterByEditorIdContains.empty() ) {
					for (const auto& editorString : line.filterByEditorIdContains) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							contains = true;
						} else {
							contains = false;
							break;
						}
					}
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByEditorIdContainsOr) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							containsOr = true;
							break;
						}
					}
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				

				if (line.object.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}



				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::BGSEncounterZone* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::EncounterZone) {
							npc = (RE::BGSEncounterZone*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								break;
							}
						}
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found ) {
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

	void* patch(ENCOUNTERZONE::patch_instruction line, RE::BGSEncounterZone* curobj)
	{


		if (!line.minLevel.empty() && line.minLevel != "none") {
			try {
				int val = std::stoi(line.minLevel);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevel out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					curobj->data.minLevel = static_cast<int8_t>(val);
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed minLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.minLevel);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minLevelAdd.empty() && line.minLevelAdd != "none") {
			try {
				int val = std::stoi(line.minLevelAdd);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevelAdd value out of bounds"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.minLevel + val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.minLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(added) minLevel {}"),
						curobj->formID,
						clib_util::editorID::get_editorID(curobj),
						curobj->data.minLevel);
				}
			} catch (const std::invalid_argument&) {
				// Ungültige Eingabe – wird ignoriert
			}
		}


		if (!line.minLevelMult.empty() && line.minLevelMult != "none") {
			try {
				int val = std::stoi(line.minLevelMult);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevelMult out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.minLevel * val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.minLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(mult) minLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.minLevel);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxLevel.empty() && line.maxLevel != "none") {
			try {
				int val = std::stoi(line.maxLevel);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevel out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					curobj->data.maxLevel = static_cast<int8_t>(val);
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed maxLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.maxLevel);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxLevelAdd.empty() && line.maxLevelAdd != "none") {
			try {
				int val = std::stoi(line.maxLevelAdd);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevelAdd value out of bounds"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.maxLevel + val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.maxLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(added) maxLevel {}"),
						curobj->formID,
						clib_util::editorID::get_editorID(curobj),
						curobj->data.maxLevel);
				}
			} catch (const std::invalid_argument&) {
				// Ungültige Eingabe – wird ignoriert
			}
		}

		if (!line.maxLevelMult.empty() && line.maxLevelMult != "none") {
			try {
				int val = std::stoi(line.maxLevelMult);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevelMult out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.maxLevel * val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.maxLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(mult) maxLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.maxLevel);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.location.empty() && line.location != "none") {
			RE::TESForm*		currentform = nullptr;
			std::string			string_form = line.location;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Location) {
				curobj->data.location = (RE::BGSLocation*)currentform;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} set location {:08X} {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.location->formID, curobj->formID, clib_util::editorID::get_editorID(curobj->data.location));
			} else if (line.location != "null") {
				curobj->data.location = nullptr;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} removed location"), curobj->formID, clib_util::editorID::get_editorID(curobj) );
			}
		}

		if (!line.faction.empty() && line.faction != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.faction;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Faction) {
				curobj->data.zoneOwner = (RE::TESFaction*)currentform;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed zone owner to "), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->formID, clib_util::editorID::get_editorID(currentform));
			} else if (line.faction != "null") {
				curobj->data.zoneOwner = nullptr;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} removed zone owner"), curobj->formID, clib_util::editorID::get_editorID(curobj));
			}
		}

		if (!line.rank.empty() && line.rank != "none") {
			int val = std::stoi(line.rank);
			if (val < INT8_MIN || val > INT8_MAX) {
				curobj->data.ownerRank = static_cast<int8_t>(val);
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed rank {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.ownerRank);
			}
		}
		

	}

}
