#include "outfit.h"
#include <utility.h>

namespace OUTFIT
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;


		extractData(line, "filterByOutfits\\s*=([^:]+)", l.object);

		extractData(line, "filterByForms\\s*=([^:]+)", l.formAnd);
		extractData(line, "filterByFormsOr\\s*=([^:]+)", l.formOr);
		extractData(line, "filterByFormsExclude\\s*=([^:]+)", l.formExcluded);

		std::regex  formListReplace_regex("formsToReplace\\s*=([^:]+)", regex::icase);
		std::smatch formListReplace_match;
		std::regex_search(line, formListReplace_match, formListReplace_regex);
		std::vector<std::string> formListReplace;
		if (formListReplace_match.empty() || formListReplace_match[1].str().empty()) {
			//empty
		} else {
			std::string formListReplace_str = formListReplace_match[1];
			std::regex  pattern("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})", regex::icase);
			std::smatch match;

			auto begin = std::sregex_iterator(formListReplace_str.begin(), formListReplace_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.formsToReplace.push_back(match[1]);
				l.formsToReplaceWith.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		extractData(line, "formsToAdd\\s*=([^:]+)", l.addedObjects);
		extractData(line, "formsToRemove\\s*=([^:]+)", l.objectsRemove);
		extractValueString(line, "clear\\s*=([^:]+)", l.clear);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::BGSOutfit*> ObjectArray = dataHandler->GetFormArray<RE::BGSOutfit>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSOutfit* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Outfit) {
						curobj = (RE::BGSOutfit*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && !line.formAnd.empty() && !line.formOr.empty()) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool mgefAnd = false;
				bool mgefOr = false;

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

				if (!line.formAnd.empty()) {
					bool foundInList = false;
					for (const auto& mgefstring : line.formAnd) {
						foundInList = false;
						RE::TESForm*       currentform = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform ) {
							
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit->formID == currentform->formID) {
									foundInList = true;
									break;
								}
							}
							if (!foundInList) {
								mgefAnd = false;
								break;
							}
						}
					}
					if (foundInList) {
						mgefAnd = true;
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					mgefAnd = true;
				}
				if (!line.formOr.empty()) {
					for (const auto& mgefstring : line.formOr) {
						RE::TESForm*       currentform = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform ) {
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit->formID == currentform->formID) {
									mgefOr = true;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
						if (mgefOr) {
							break;
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					mgefOr = true;
				}

				if ((!line.formAnd.empty() || !line.formOr.empty()) && mgefAnd && mgefOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by mgefs. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.object.empty() && line.formAnd.empty() && line.formOr.empty()) {
					found = true;
				}


				if (!line.formExcluded.empty() && found) {
					for (const auto& mgefstring : line.formExcluded) {
						RE::TESForm*       currentform = nullptr;
						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit->formID == currentform->formID) {
									found = false;
									logger::debug(FMT_STRING("outfit exlucded {:08X} by form {:08X}"), curobj->formID, currentform->formID);
									break;
								}
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

	void patch(OUTFIT::patch_instruction line, RE::BGSOutfit* curobj)
	{
		

		//logger::debug(FMT_STRING("outfit"));


		if (!line.clear.empty() && line.clear != "none") {
			try {
				curobj->outfitItems.clear();
				logger::debug(FMT_STRING("outfit formid: {:08X} cleared"), curobj->formID);
			} catch (const std::invalid_argument& e) {
			}
		}


		if (!line.objectsRemove.empty()) {
			for (const auto& objectToRemove : line.objectsRemove) {
				std::string        removeFormStr = objectToRemove;
				RE::EffectSetting* removeForm = (RE::EffectSetting*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->outfitItems.begin();
					while (it != curobj->outfitItems.end()) {
						//logger::debug(FMT_STRING("spell {:08X} {}, effect {:08X} cost {} {}"), curobj->formID, curobj->fullName, (*it)->baseEffect->formID, (*it)->cost, curobj->CalculateMagickaCost(RE::PlayerCharacter::GetSingleton()));

						if ((*it) && (*it)->formID == removeForm->formID) {
							it = curobj->outfitItems.erase(it);
							logger::debug(FMT_STRING("outfit {:08X}, {:08X} object removed, objects left {}"), curobj->formID, removeForm->formID, curobj->outfitItems.size());
						} else {
							++it;
						}
					}
				}
			}
		}


		if ( !line.addedObjects.empty()) {
			for (size_t i = 0; i < line.addedObjects.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.addedObjects[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform) {
					//logger::debug(FMT_STRING("formlist formid: {:08X} with count({}) added form {:08X} "), curobj->formID, curobj->arrayOfForms.size(), currentform->formID);
					curobj->outfitItems.push_back(currentform);
					logger::debug(FMT_STRING("outfit formid: {:08X} added form {:08X} "), curobj->formID,  currentform->formID);
				}
			}
		}

		if (!line.formsToReplace.empty()) {
			for (uint32_t i = 0; i < line.formsToReplace.size(); i++) {
				RE::TESForm* currentform = nullptr;
				RE::TESForm* currentform2 = nullptr;
				std::string  string_form = line.formsToReplace[i];
				std::string  string_form2 = line.formsToReplaceWith[i];
				currentform = GetFormFromIdentifier(string_form);
				currentform2 = GetFormFromIdentifier(string_form2);
				if (currentform && currentform2) {
					// Loop through all elements in arrayOfForms and replace matches
					for (auto& form : curobj->outfitItems) {
						if (form && form->formID == currentform->formID) {
							form = currentform2;  // Replace the matching form
							logger::debug(FMT_STRING("outfit formid: {:08X} replaced {:08X} with {:08X} "), curobj->formID, currentform->formID, currentform2->formID);
						}
					}
				}
			}
		}







		


	}

}
