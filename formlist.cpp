#include "formlist.h"
namespace FORMLIST
{

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		//// extract objects
		//std::regex  objects_regex("filterByFormLists\\s*=([^:]+)", regex::icase);
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
		//	l.objects = objects;
		//}

		extractData(line, "filterByFormLists\\s*=([^:]+)", l.objects);

		//// extract formsToAdd
		//std::regex  formsToAdd_regex("formsToAdd\\s*=([^:]+)", regex::icase);
		//std::smatch formsToAdd_match;
		//std::regex_search(line, formsToAdd_match, formsToAdd_regex);
		//std::vector<std::string> formsToAdd;
		//if (formsToAdd_match.empty() || formsToAdd_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          formsToAdd_str = formsToAdd_match[1];
		//	std::regex           formsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator formsToAdd_iterator(formsToAdd_str.begin(), formsToAdd_str.end(), formsToAdd_list_regex);
		//	std::sregex_iterator formsToAdd_end;
		//	while (formsToAdd_iterator != formsToAdd_end) {
		//		std::string keywordToAdd = (*formsToAdd_iterator)[0].str();
		//		keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
		//		keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
		//		if (keywordToAdd != "none") {
		//			//logger::info(FMT_STRING("formsToAdd: {}"), keywordToAdd);
		//			formsToAdd.push_back(keywordToAdd);
		//		}
		//		++formsToAdd_iterator;
		//	}
		//	l.objectsAdd = formsToAdd;
		//}

		extractData(line, "formsToAdd\\s*=([^:]+)", l.objectsAdd);

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

		//// extract formsToRemove
		//std::regex  formsToRemove_regex("formsToRemove\\s*=([^:]+)", regex::icase);
		//std::smatch formsToRemove_match;
		//std::regex_search(line, formsToRemove_match, formsToRemove_regex);
		//std::vector<std::string> formsToRemove;
		//if (formsToRemove_match.empty() || formsToRemove_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          formsToRemove_str = formsToRemove_match[1];
		//	std::regex           formsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator formsToRemove_iterator(formsToRemove_str.begin(), formsToRemove_str.end(), formsToRemove_list_regex);
		//	std::sregex_iterator formsToRemove_end;
		//	while (formsToRemove_iterator != formsToRemove_end) {
		//		std::string keywordToRemove = (*formsToRemove_iterator)[0].str();
		//		keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
		//		keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
		//		if (keywordToRemove != "none") {
		//			//logger::info(FMT_STRING("formsToRemove: {}"), keywordToRemove);
		//			formsToRemove.push_back(keywordToRemove);
		//		}
		//		++formsToRemove_iterator;
		//	}
		//	l.objectsRemove = formsToRemove;
		//}

		extractData(line, "formsToRemove\\s*=([^:]+)", l.objectsRemove);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                     dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::BGSListForm*> objectArray = dataHandler->GetFormArray<RE::BGSListForm>();
		for (const auto& line : tokens) {
			for (const auto& curobj : objectArray) {
				bool found = false;
				//logger::debug(FMT_STRING("formlist found. {:08X}"), curobj->formID);
				if (!line.objects.empty()) {
					//logger::info("npc not empty");
					for (const auto& objectstring : line.objects) {
						RE::TESForm*     currentform = nullptr;
						RE::BGSListForm* object = nullptr;

						std::string string_form = objectstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::FormList) {
							object = (RE::BGSListForm*)currentform;

							if (curobj->formID == object->formID) {
								found = true;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}
				if (line.objects.empty()) {
					found = true;
				}

				if (found && !line.objectsRemove.empty()) {
					for (size_t i = 0; i < line.objectsRemove.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = line.objectsRemove[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							//logger::debug(FMT_STRING("formlist formid: {:08X} with count({}) "), curobj->formID, curobj->arrayOfForms.size());
							curobj->forms.erase(std::remove_if(curobj->forms.begin(), curobj->forms.end(), [&](const RE::TESForm* x) {
								return x->formID == currentform->formID;
							}));
							logger::debug(FMT_STRING("formlist formid: {:08X} with count({}) removed form {:08X} "), curobj->formID, curobj->forms.size(), currentform->formID);
						}
					}
				}

				if (found && !line.objectsAdd.empty()) {
					for (size_t i = 0; i < line.objectsAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = line.objectsAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							//logger::debug(FMT_STRING("formlist formid: {:08X} with count({}) added form {:08X} "), curobj->formID, curobj->arrayOfForms.size(), currentform->formID);
							curobj->forms.push_back(currentform);
							logger::debug(FMT_STRING("formlist formid: {:08X} with count({}) added form {:08X} "), curobj->formID, curobj->forms.size(), currentform->formID);
						}
					}
				}

				if (found && !line.formsToReplace.empty()) {
					for (uint32_t i = 0; i < line.formsToReplace.size(); i++) {
						RE::TESForm* currentform = nullptr;
						RE::TESForm* currentform2 = nullptr;
						std::string  string_form = line.formsToReplace[i];
						std::string  string_form2 = line.formsToReplaceWith[i];
						currentform = GetFormFromIdentifier(string_form);
						currentform2 = GetFormFromIdentifier(string_form2);
						if (currentform && currentform2) {
							// Loop through all elements in arrayOfForms and replace matches
							for (auto& form : curobj->forms) {
								if (form && form->formID == currentform->formID) {
									form = currentform2;  // Replace the matching form
								}
							}
						}
					}
				}

				if (found) {
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
								std::string             line;
								std::ifstream           infile;
								std::list<line_content> tokens;
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
