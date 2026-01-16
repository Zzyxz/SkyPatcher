#include "location.h"
#include <utility.h>

namespace LOCATION
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;


		extractData(line, "filterByLocations\\s*=([^:]+)", l.object);

		extractData(line, "filterByLocationsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);
		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);
		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		extractDataStrings(line, "filterByMusicType\\s*=([^:]+)", l.filterByMusicType);
		extractDataStrings(line, "filterByParentLocation\\s*=([^:]+)", l.filterByParentLoc);
		extractDataStrings(line, "filterByUnreportedCrimeFaction\\s*=([^:]+)", l.filterByUnreportedCrimeFaction);

		extractValueString(line, "musicType\\s*=([^:]+)", l.musicType);
		extractValueString(line, "unreportedCrimeFaction\\s*=([^:]+)", l.unreportedCrimeFaction);
		extractValueString(line, "parentLocation\\s*=([^:]+)", l.parentLoc);

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::BGSLocation*> ObjectArray = dataHandler->GetFormArray<RE::BGSLocation>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSLocation* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Location) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::BGSLocation*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByParentLoc.empty()) {
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

				if (!found && !line.filterByUnreportedCrimeFaction.empty()) {
					for (const auto& factionString : line.filterByUnreportedCrimeFaction) {
						RE::TESForm* currentform = GetFormFromIdentifier(factionString);
						if (currentform && currentform->formType == RE::FormType::Faction) {
							auto* faction = static_cast<RE::TESFaction*>(currentform);
							if (curobj->unreportedCrimeFaction && curobj->unreportedCrimeFaction->formID == faction->formID) {
								found = true;
								break;
							}
						}
					}
				}


				if (!found && !line.filterByParentLoc.empty()) {
					for (const auto& locString : line.filterByParentLoc) {
						RE::TESForm* currentform = GetFormFromIdentifier(locString);
						if (currentform && currentform->formType == RE::FormType::Location) {
							auto* parent = static_cast<RE::BGSLocation*>(currentform);
							if (curobj->parentLoc && curobj->parentLoc->formID == parent->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByMusicType.empty()) {
					for (const auto& musicString : line.filterByMusicType) {
						RE::TESForm* currentform = GetFormFromIdentifier(musicString);
						if (currentform && currentform->formType == RE::FormType::MusicType) {
							auto* music = static_cast<RE::BGSMusicType*>(currentform);
							if (curobj->musicType && curobj->musicType->formID == music->formID) {
								found = true;
								break;
							}
						}
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

				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByParentLoc.empty()) {
					found = true;
				}



				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::BGSLocation* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Misc) {
							npc = (RE::BGSLocation*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::info("Misc Excluded.");
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

	void* patch(LOCATION::patch_instruction line, RE::BGSLocation* curobj)
	{

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("location formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("location formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}



		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("location formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.musicType.empty() && line.musicType != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.musicType);
			if (currentform && currentform->formType == RE::FormType::MusicType) {
				curobj->musicType = static_cast<RE::BGSMusicType*>(currentform);
				logger::debug(FMT_STRING("location formid: {:08X} changed musicType to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.musicType == "null") {
				curobj->musicType = nullptr;
				logger::debug(FMT_STRING("location {:08X} changed musicType to null"), curobj->formID);
			}
		}

		if (!line.parentLoc.empty() && line.parentLoc != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.parentLoc);
			if (currentform && currentform->formType == RE::FormType::Location) {
				curobj->parentLoc = static_cast<RE::BGSLocation*>(currentform);
				logger::debug(FMT_STRING("location formid: {:08X} changed parentLoc to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.parentLoc == "null") {
				curobj->parentLoc = nullptr;
				logger::debug(FMT_STRING("location formid: {:08X} changed parentLoc to null"), curobj->formID);
			}
		}

		if (!line.unreportedCrimeFaction.empty() && line.unreportedCrimeFaction != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.unreportedCrimeFaction);
			if (currentform && currentform->formType == RE::FormType::Faction) {
				curobj->unreportedCrimeFaction = static_cast<RE::TESFaction*>(currentform);
				logger::debug(FMT_STRING("location {:08X} changed unreportedCrimeFaction to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.unreportedCrimeFaction == "null") {
				curobj->unreportedCrimeFaction = nullptr;
				logger::debug(FMT_STRING("location formid: {:08X} cleared unreportedCrimeFaction (set to null)"), curobj->formID);
			}
		}




	}

}
