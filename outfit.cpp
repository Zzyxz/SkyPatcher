#include "outfit.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <unordered_set>

namespace OUTFIT
{

	namespace
	{
#define OUTFIT_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kOutfitKeys[] = {
			{ "filterByOutfits", sp::FieldKind::List, OUTFIT_F(object) },
			{ "filterByForms", sp::FieldKind::List, OUTFIT_F(formAnd) },
			{ "filterByFormsOr", sp::FieldKind::List, OUTFIT_F(formOr) },
			{ "filterByFormsExclude", sp::FieldKind::List, OUTFIT_F(formExcluded) },
			{ "formsToAdd", sp::FieldKind::List, OUTFIT_F(addedObjects) },
			{ "formsToRemove", sp::FieldKind::List, OUTFIT_F(objectsRemove) },
			{ "clear", sp::FieldKind::Value, OUTFIT_F(clear) },
			{ "filterByModNames", sp::FieldKind::ListAssign, OUTFIT_F(modNames) },
		};

#undef OUTFIT_F

		const sp::KeyTable& outfitKeyTable()
		{
			static const sp::KeyTable table(kOutfitKeys, std::size(kOutfitKeys));
			return table;
		}

		bool NamesPlugin(const std::string& a_name)
		{
			const auto lower = toLowerCase(a_name);
			return lower.find(".esp") != std::string::npos || lower.find(".esm") != std::string::npos || lower.find(".esl") != std::string::npos;
		}

		void splitReplacePairs(const std::string& a_value, std::vector<std::string>& a_old, std::vector<std::string>& a_new)
		{
			std::size_t start = 0;
			while (start <= a_value.size()) {
				const auto comma = a_value.find(',', start);
				const auto entry = a_value.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
				auto       separator = entry.find('=');
				if (separator == std::string::npos) {
					separator = entry.find('~');
				}
				if (separator != std::string::npos) {
					const auto oldForm = trim(entry.substr(0, separator));
					const auto newForm = trim(entry.substr(separator + 1));
					if (!oldForm.empty() && oldForm.front() != ' ' && !newForm.empty() && newForm.front() != ' ') {
						a_old.push_back(oldForm);
						a_new.push_back(newForm);
					}
				}
				if (comma == std::string::npos) {
					break;
				}
				start = comma + 1;
			}
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		outfitKeyTable().parse(line, &l);

		std::regex  formListReplace_regex("formsToReplace\\s*=([^:]+)", regex::icase);
		std::smatch formListReplace_match;
		std::regex_search(line, formListReplace_match, formListReplace_regex);
		if (!formListReplace_match.empty() && !formListReplace_match[1].str().empty()) {
			splitReplacePairs(formListReplace_match[1].str(), l.formsToReplace, l.formsToReplaceWith);
		}

		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSOutfit>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;

			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSOutfit* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Outfit) {
						curobj = (RE::BGSOutfit*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.object.empty() && line.formAnd.empty() && line.formOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings()
			};

			for (const auto& curobj : ObjectArray) {
				bool found = false;
				bool mgefAnd = false;
				bool mgefOr = false;

				if (!curobj || curobj->IsDeleted() || patched.contains(curobj->formID)) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!line.formAnd.empty()) {
					bool anyResolved = false;
					bool allInList = true;
					for (const auto& mgefstring : line.formAnd) {
						RE::TESForm*       currentform = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							anyResolved = true;
							bool foundInList = false;
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit && outfit->formID == currentform->formID) {
									foundInList = true;
									break;
								}
							}
							if (!foundInList) {
								allInList = false;
								break;
							}
						}
					}
					mgefAnd = anyResolved && allInList;
				} else {
					mgefAnd = true;
				}
				if (!line.formOr.empty()) {
					for (const auto& mgefstring : line.formOr) {
						RE::TESForm*       currentform = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform ) {
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit && outfit->formID == currentform->formID) {
									mgefOr = true;
									break;
								}
							}
						}
						if (mgefOr) {
							break;
						}
					}
				} else {
					mgefOr = true;
				}

				if ((!line.formAnd.empty() || !line.formOr.empty()) && mgefAnd && mgefOr) {
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
						if (currentform) {
							for (const auto& outfit : curobj->outfitItems) {
								if (outfit && outfit->formID == currentform->formID) {
									found = false;
									logger::debug(FMT_STRING("outfit exlucded {:08X} by form {:08X}"), curobj->formID, currentform->formID);
									break;
								}
							}
						}
					}
				}

				

				if (found) {
					patched.insert(curobj->formID);
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

								if (NamesPlugin(fileName)) {
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
									if (line.empty() || line[0] == skipChar) {
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
		return nullptr;
	}

	void patch(const OUTFIT::patch_instruction& line, RE::BGSOutfit* curobj)
	{
		

		if (IsSwitchOn(line.clear)) {
			try {
				curobj->outfitItems.clear();
				logger::debug(FMT_STRING("outfit formid: {:08X} cleared"), curobj->formID);
			} catch (const std::exception& e) {
			}
		}


		if (!line.objectsRemove.empty()) {
			for (const auto& objectToRemove : line.objectsRemove) {
				std::string        removeFormStr = objectToRemove;
				auto* resolvedRemoveForm = GetFormFromIdentifier(removeFormStr);
				auto* removeForm = resolvedRemoveForm;
				if (removeForm) {
					auto it = curobj->outfitItems.begin();
					while (it != curobj->outfitItems.end()) {
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
				if (currentform && currentform->formType != RE::FormType::Armor && currentform->formType != RE::FormType::LeveledItem) {
					logger::warn(FMT_STRING("outfit formid: {:08X} cannot add {:08X}: only armor and leveled items are allowed"), curobj->formID, currentform->formID);
					currentform = nullptr;
				}
				if (currentform) {
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
				if (currentform2 && currentform2->formType != RE::FormType::Armor && currentform2->formType != RE::FormType::LeveledItem) {
					logger::warn(FMT_STRING("outfit formid: {:08X} cannot replace with {:08X}: only armor and leveled items are allowed"), curobj->formID, currentform2->formID);
					currentform2 = nullptr;
				}
				if (currentform && currentform2) {
					for (auto& form : curobj->outfitItems) {
						if (form && form->formID == currentform->formID) {
							form = currentform2;
							logger::debug(FMT_STRING("outfit formid: {:08X} replaced {:08X} with {:08X} "), curobj->formID, currentform->formID, currentform2->formID);
						}
					}
				}
			}
		}







		


	}

}
