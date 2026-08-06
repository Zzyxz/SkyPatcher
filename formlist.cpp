#include "formlist.h"
#include "parsetable.h"
namespace FORMLIST
{
	// ==================================================================
	// PHASE 2 -- FORMLIST's key table. One row per upstream extract*()
	// call, in the same order. See parsetable.h.
	//
	// "formsToReplace" stays hand-written below: its sub-scan pattern
	// `([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\s*=\s*([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})`
	// requires the SECOND capture group to also be `name|editorid`-shaped,
	// which does not match FieldKind::ObjectsToAdd's second group
	// (plain `[^,]+`) -- those two kinds diverge on inputs where the
	// replacement value is not itself `name|editorid` shaped, so it cannot
	// be represented by an existing FieldKind.
	// ==================================================================
	namespace
	{
#define FORMLIST_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kFormlistKeys[] = {
			{ "filterByFormLists", sp::FieldKind::List, FORMLIST_F(objects) },
			{ "formsToAdd", sp::FieldKind::List, FORMLIST_F(objectsAdd) },
			{ "formsToRemove", sp::FieldKind::List, FORMLIST_F(objectsRemove) },
			{ "filterByModNames", sp::FieldKind::ListAssign, FORMLIST_F(modNames) },
			{ "clear", sp::FieldKind::Value, FORMLIST_F(clear) },
		};

#undef FORMLIST_F

		const sp::KeyTable& formlistKeyTable()
		{
			static const sp::KeyTable table(kFormlistKeys, std::size(kFormlistKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		formlistKeyTable().parse(line, &l);

		// NOT convertible: see the comment on kFormlistKeys above.
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
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                     dataHandler = RE::TESDataHandler::GetSingleton();
<<<<<<< HEAD
		auto& objectArray = dataHandler->GetFormArray<RE::BGSListForm>();
=======
		const auto& objectArray = dataHandler->GetFormArray<RE::BGSListForm>();
>>>>>>> origin/new-main
		for (const auto& line : tokens) {

			if (!line.objects.empty()) {
				//logger::info("npc not empty");
				for (const auto& objectstring : line.objects) {
					RE::TESForm*     currentform = nullptr;
					RE::BGSListForm* object = nullptr;

					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::FormList) {
						object = (RE::BGSListForm*)currentform;
						if (!object || object->IsDeleted()) {
							continue;
						}
						if (object->formID) {
							patch(line, object);
							//logger::info("NPC found.");
							break;
						}
					}
				}
			}

			

			//for (const auto& curobj : objectArray) {
			//	bool found = false;
			//	
			//	if (!line.modNames.empty()) {
			//		bool modFound = false;
			//		for (auto const modName : line.modNames) {
			//			if (modName == curobj->GetFile(0)->fileName) {
			//				modFound = true;
			//			}
			//		}
			//		if (modFound == false) {
			//			continue;
			//		}
			//	}
			//}
		}
	}

	void patch(const FORMLIST::line_content& line, RE::BGSListForm* curobj)
	{
		if (!line.clear.empty() && line.clear != "none") {
			curobj->forms.clear();
		}

		if (!line.objectsRemove.empty()) {
			for (const auto& objectToRemove : line.objectsRemove) {
				std::string removeFormStr = objectToRemove;
				auto        removeForm = GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = std::remove_if(curobj->forms.begin(), curobj->forms.end(), [&](const RE::TESForm* x) {
						return x->formID == removeForm->formID;
					});
					while (it != curobj->forms.end()) {
						curobj->forms.erase(it);
						it = std::remove_if(it, curobj->forms.end(), [&](const RE::TESForm* x) {
							return x->formID == removeForm->formID;
						});
					}
					logger::debug(FMT_STRING("formlist formid: {:08X} removed form {:08X} "), curobj->formID, removeForm->formID);
				}
			}
		}

		if (!line.objectsAdd.empty()) {
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
					for (auto& form : curobj->forms) {
						if (form && form->formID == currentform->formID) {
							form = currentform2;  // Replace the matching form
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
								std::string             line;
								std::ifstream           infile;
								std::list<line_content> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
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
