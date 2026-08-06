#include "leveledlist.h"
#include "formfilter.h"
#include "parsetable.h"
namespace LEVELEDLIST
{
	namespace
	{
#define LL_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))
		constexpr sp::KeySpec kLeveledListKeys[] = {
			{ "filterByLLs", sp::FieldKind::List, LL_F(objects) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContainsExcluded) },
			{ "filterByLLNPCs", sp::FieldKind::List, LL_F(containerObjects) },
			{ "addToLLs", sp::FieldKind::Arr2D, LL_F(addedObjects) },
			{ "addOnceToLLs", sp::FieldKind::Arr2D, LL_F(addedObjectsOnce) },
			{ "removeFromLLs", sp::FieldKind::Arr2D, LL_F(removedObjects) },
			{ "clear", sp::FieldKind::Value, LL_F(clear) },
			{ "calcForLevel", sp::FieldKind::Value, LL_F(calcForLevel) },
			{ "calcEachItem", sp::FieldKind::Value, LL_F(calcEachItem) },
			{ "calcLevelAndEachItem", sp::FieldKind::Value, LL_F(calcForLevelAndEachItem) },
			{ "calcUseAll", sp::FieldKind::Value, LL_F(calcUseAll) },
			{ "clearFlags", sp::FieldKind::Value, LL_F(clearFlags) },
			{ "addTemplateKeywords", sp::FieldKind::Arr2D, LL_F(templateKeyword) },
			{ "removeObjectsByKeyword", sp::FieldKind::List, LL_F(removeItemsByKeyword) },
			{ "chanceGlobal", sp::FieldKind::Value, LL_F(llglobal) },
			{ "chanceNone", sp::FieldKind::Value, LL_F(chanceNone) },
			{ "formsToReplace", sp::FieldKind::Arr2D, LL_F(replaceObjects) },
			{ "objectMultCount", sp::FieldKind::Arr2D, LL_F(objectMultCount) },
		};
#undef LL_F

		const sp::KeyTable& leveledListKeyTable()
		{
			static const sp::KeyTable table(kLeveledListKeys, std::size(kLeveledListKeys));
			return table;
		}
	}

	void FreeLeveledList(RE::LEVELED_OBJECT* a_lobj, uint32_t arg2 = 0x3)
	{
		if (!a_lobj)
			return;

		RE::free(a_lobj);
	}

	struct LL_ALLOC
	{
		uint32_t           count;
		RE::LEVELED_OBJECT ll[];
	};

	LL_ALLOC* reserveNewLevL(uint32_t entryCnt)
	{
		RE::MemoryManager mm = *RE::MemoryManager::GetSingleton();
		return (LL_ALLOC*)mm.Allocate(sizeof(RE::LEVELED_OBJECT) * entryCnt + sizeof(size_t), 0, 0);
	}

	std::vector<RE::LEVELED_OBJECT> convertLLtoVec(RE::TESLeveledList* levl)
	{
		std::vector<RE::LEVELED_OBJECT> loVec;

		if (!levl || levl->entries.empty() || levl->numEntries == 0)
			return loVec;

		for (uint32_t i = 0; i < static_cast<uint32_t>(levl->numEntries); i++)
			loVec.push_back(levl->entries[i]);

		return loVec;
	}

	void createLL(RE::TESLeveledList* levl, const std::vector<RE::LEVELED_OBJECT>& a_llad)
	{
		if (!levl)
			return;

		if (levl->entries.empty()) {
			memset(levl->entries.data(), 0, levl->numEntries * sizeof(RE::LEVELED_OBJECT));
			FreeLeveledList(levl->entries.data());

			levl->entries.clear();
			levl->numEntries = 0;
		}

		if (a_llad.empty())
			return;

		uint32_t  size = static_cast<uint32_t>(a_llad.size());
		LL_ALLOC* newLL = reserveNewLevL(size);
		if (!newLL)
			return;

		newLL->count = size;
		for (uint32_t ii = 0; ii < size; ii++) {
			newLL->ll[ii] = a_llad[ii];
		}

		levl->entries.resize(newLL->count);

		for (int i = 0; i < newLL->count; i++) {
			levl->entries[i] = newLL->ll[i];
		}

		levl->numEntries = static_cast<int8_t>(size);
	}

	void ClearLL(RE::TESLeveledList* levl)
	{
		if (!levl)
			return;

		if (!levl->entries.empty()) {
			memset(levl->entries.data(), 0, levl->numEntries * sizeof(RE::LEVELED_OBJECT));

			levl->entries.clear();
			levl->numEntries = 0;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		leveledListKeyTable().parse(line, &l);

		std::regex  noFilterLL_regex("noFilterLL\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLmatch;
		std::regex_search(line, noFilterLLmatch, noFilterLL_regex);
		if (noFilterLLmatch.empty() || noFilterLLmatch[1].str().empty()) {
			l.noFilterLL = "none";
		}
		else {
			std::string noFilterLLvalue = noFilterLLmatch[1].str();
			noFilterLLvalue.erase(noFilterLLvalue.begin(), std::find_if_not(noFilterLLvalue.begin(), noFilterLLvalue.end(), ::isspace));
			noFilterLLvalue.erase(std::find_if_not(noFilterLLvalue.rbegin(), noFilterLLvalue.rend(), ::isspace).base(), noFilterLLvalue.end());
			l.noFilterLL = noFilterLLvalue;
		}

		std::regex  noFilterLLNPC_regex("noFilterLLNPC\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLNPCmatch;
		std::regex_search(line, noFilterLLNPCmatch, noFilterLLNPC_regex);
		if (noFilterLLNPCmatch.empty() || noFilterLLNPCmatch[1].str().empty()) {
			l.noFilterLLNPC = "none";
		}
		else {
			std::string noFilterLLNPCvalue = noFilterLLNPCmatch[1].str();
			noFilterLLNPCvalue.erase(noFilterLLNPCvalue.begin(), std::find_if_not(noFilterLLNPCvalue.begin(), noFilterLLNPCvalue.end(), ::isspace));
			noFilterLLNPCvalue.erase(std::find_if_not(noFilterLLNPCvalue.rbegin(), noFilterLLNPCvalue.rend(), ::isspace).base(), noFilterLLNPCvalue.end());
			l.noFilterLLNPC = noFilterLLNPCvalue;
		}

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                    dataHandler = RE::TESDataHandler::GetSingleton();




		for (const auto& line : tokens) {
			if (!line.objects.empty()) {
				for (const auto& objectstring : line.objects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevItem* object = nullptr;

					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::LeveledItem) {
						object = (RE::TESLevItem*)currentform;
						if (static_cast<uint32_t>(object->numEntries) > 120) {
							logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), object->formID);
							continue;
						}
						patch(line, object);
					}
				}
			}


			if (!line.containerObjects.empty()) {
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevCharacter* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::LeveledNPC) {
						object = (RE::TESLevCharacter*)currentform;
						patch(line, object);
					}
				}
			}

			if (!line.noFilterLL.empty() && toLowerCase(line.noFilterLL) == "true") {
<<<<<<< HEAD
				auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
=======
				const auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
>>>>>>> origin/new-main
				for (const auto& curobj : objectArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					patch(line, curobj);
				}
			}

			if (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty() || !line.filterByEditorIdContainsExcluded.empty()) {
<<<<<<< HEAD
				const formfilter::CommonFilterView view{
					formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
					formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
					line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
					line.filterByEditorIdContainsExcluded, formfilter::noStrings()
				};

				auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
=======
				const auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
>>>>>>> origin/new-main
				for (const auto& curobj : objectArray) {

					bool contains = false;
					bool containsOr = false;
					bool found = false;

					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					if (!line.filterByEditorIdContains.empty()) {
						contains = formfilter::matchesEditorIdAnd(view, curobj);
					}
					else {
						contains = true;
					}

					if (!line.filterByEditorIdContainsOr.empty()) {
						containsOr = formfilter::matchesEditorIdOr(view, curobj);
					}
					else {
						containsOr = true;
					}

					if ((!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
						found = true;
					}

					if (!line.filterByEditorIdContainsExcluded.empty() && found) {
						if (formfilter::hasExcludedEditorId(view, curobj)) {
							found = false;
						}
					}

					if (found) {
						patch(line, curobj);
					}
				}
			}


			if (!line.noFilterLLNPC.empty() && toLowerCase(line.noFilterLLNPC) == "true") {
<<<<<<< HEAD
				auto& tesLevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();
=======
				const auto& tesLevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();
>>>>>>> origin/new-main
				for (const auto& curobj : tesLevCharArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					patch(line, curobj);
				}
			}



		}
	}

	void* readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
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
						}
						else {
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
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}
	template <typename T>
<<<<<<< HEAD
	void patch(const LEVELEDLIST::line_content& line, T* curobj)
=======
	void patch(const LEVELEDLIST::line_content& line, T* curobj)
>>>>>>> origin/new-main
	{


		if (!line.replaceObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);

			for (const auto& objectToreplace : line.replaceObjects) {
				std::string formToReplaceString = objectToreplace[0];
				std::string formToReplaceWithString = objectToreplace[1];
				int          count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				RE::TESForm* formToReplaceWith = GetFormFromIdentifier(formToReplaceWithString);
				for (auto& obj : llVec) {
					if (obj.form->formID == formToReplace->formID) {
						obj.form = formToReplaceWith;
						count++;
					}
				}
				std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
					return a.level < b.level;
					});
				if (llVec.size() >= 1) {
					createLL(curobj, llVec);
					logger::debug(FMT_STRING("leveled Lists {:08X} all forms of {:08X} have been replaced by {:08X}. Total replaced: {}"), curobj->formID, formToReplace->formID, formToReplaceWith->formID, count);
				}
				else {
					ClearLL(curobj);
					logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
				}
			}
		}


		if (!line.removedObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);

			for (const auto& objectsToRemove : line.removedObjects) {
				RE::TESForm* delForm = GetFormFromIdentifier(objectsToRemove[0]);
				if (!delForm) {
					logger::debug(FMT_STRING("Form not found {}, skipping."), objectsToRemove[0]);
					continue;
				}

				std::string relationSign = "";
				uint16_t    number;
				bool        hasNumberLvl = false;
				bool        hasNumberCount = false;

				if (objectsToRemove.size() >= 2) {
					std::vector<std::string> templvl = splitRelationNumber(objectsToRemove[1]);
					if (templvl.size() == 2 && templvl[0] != "none") {
						number = std::stoi(templvl[0]);
						relationSign = templvl[1];
						hasNumberLvl = true;
					}
					else if (templvl.size() == 1 && templvl[0] != "none") {
						number = std::stoi(templvl[0]);
						hasNumberLvl = true;
					}

					if (hasNumberLvl) {
						if (relationSign == "<") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level < number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level < {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == ">") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level > number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == ">=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level >= number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == "<=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level <= number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == "") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level == number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level == {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
					}
				}

				if (objectsToRemove.size() >= 3) {
					std::vector<std::string> tempcount = splitRelationNumber(objectsToRemove[2]);

					relationSign = "";

					if (tempcount.size() == 2 && tempcount[0] != "none") {
						number = std::stoi(tempcount[0]);
						relationSign = tempcount[1];
						hasNumberCount = true;
					}
					else if (tempcount.size() == 1 && tempcount[0] != "none") {
						number = std::stoi(tempcount[0]);
						hasNumberCount = true;
					}

					if (hasNumberCount) {
						if (relationSign == "<") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count < number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count < {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == ">") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count > number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == ">=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count >= number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == "<=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count <= number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
						else if (relationSign == "") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count == number && x.form == delForm;
								}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count == {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						}
					}
				}

				if (!hasNumberLvl && !hasNumberCount) {
					llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
						return x.form == delForm;
						}),
						llVec.end());
					logger::debug(FMT_STRING("leveled Lists {:08X} removed all Forms {:08X}"), curobj->formID, delForm->formID);
				}
			}

			std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
				return a.level < b.level;
				});
			if (llVec.size() >= 1) {
				createLL(curobj, llVec);
			}
			else {
				ClearLL(curobj);
				logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
			}
		}

		if (!line.removeItemsByKeyword.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
			for (size_t i = 0; i < line.removeItemsByKeyword.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.removeItemsByKeyword[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
						RE::BGSKeywordForm* keyForm = x.form->As<RE::BGSKeywordForm>();
						if (keyForm && keyForm->HasKeyword((RE::BGSKeyword*)currentform)) {
							logger::debug(FMT_STRING("leveled Lists {:08X} removed item {:08X} with keyword {:08X} {} "), curobj->formID, x.form->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
							return true;
						}

						return false;
						}),
						llVec.end());
				}
			}
			if (llVec.size() >= 1) {
				createLL(curobj, llVec);
			}
			else {
				ClearLL(curobj);
				logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
			}
		}

		if (!line.clear.empty() && line.clear != "none") {
			if (line.clear == "yes" || line.clear == "true") {
				logger::debug(FMT_STRING("leveled lists {:08X} cleared"), curobj->formID);
				ClearLL(curobj);
			}
		}

		if (!line.calcForLevel.empty() && line.calcForLevel != "none") {
			if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
				curobj->llFlags = RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
			}
		}

		if (!line.calcEachItem.empty() && line.calcEachItem != "none") {
			if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
				curobj->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
			}
		}

		if (!line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
			if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
				curobj->llFlags = static_cast<RE::TESLeveledList::Flag>(
					RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel |
					RE::TESLeveledList::Flag::kCalculateForEachItemInCount);
				logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
			}
		}

		if (!line.calcUseAll.empty() && line.calcUseAll != "none") {
			curobj->llFlags = RE::TESLeveledList::kUseAll;
			logger::debug(FMT_STRING("leveled lists {:08X} set to Use All"), curobj->formID);
		}

		if (!line.clearFlags.empty() && line.clearFlags != "none") {
			curobj->llFlags = resetAllFlags(curobj->llFlags);
			logger::debug(FMT_STRING("leveled lists {:08X} cleared all flags"), curobj->formID);
		}

		if (!line.addedObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
			for (const auto& objectToAdd : line.addedObjects) {
				if (objectToAdd.empty()) { logger::warn("leveled list {:08X}: empty addedObjects entry", curobj->formID); continue; }
				uint16_t level = 1;
				uint16_t count = 1;

				std::string  addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					level = std::stoi(objectToAdd[1]);
				}
				if (objectToAdd.size() > 2) {
					count = std::stoi(objectToAdd[2]);
				}
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);
				if (addForm) {
					llVec.push_back({ addForm, count, level });
					logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
			std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
				return a.level < b.level;
				});
			createLL(curobj, llVec);
		}

		if (!line.addedObjectsOnce.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
			for (const auto& objectToAdd : line.addedObjectsOnce) {
				if (objectToAdd.empty()) { logger::warn("leveled list {:08X}: empty addedObjectsOnce entry", curobj->formID); continue; }
				uint16_t level = 1;
				uint16_t count = 1;

				std::string  addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					level = std::stoi(objectToAdd[1]);
				}
				if (objectToAdd.size() > 2) {
					count = std::stoi(objectToAdd[2]);
				}
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

				if (addForm) {
					auto it = std::find_if(llVec.begin(), llVec.end(), [addForm](const RE::LEVELED_OBJECT& obj) {
						return obj.form->formID == addForm->formID;
						});

					if (it == llVec.end()) {
						llVec.push_back({ addForm, count, level });
						logger::debug(FMT_STRING("leveled Lists {:08X} added once Form {:08X}"), curobj->formID, addForm->formID);
					}
					else {
						logger::debug(FMT_STRING("leveled Lists {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
					}
				}
				else {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}

			std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
				return a.level < b.level;
				});

			createLL(curobj, llVec);
		}


		if (!line.llglobal.empty() && line.llglobal != "none") {
			if (toLowerCase(line.llglobal) != "null") {
				RE::TESForm* addForm = GetFormFromIdentifier(line.llglobal);
				if (addForm && addForm->As<RE::TESGlobal>()) {
					curobj->chanceGlobal = addForm->As<RE::TESGlobal>();
					logger::debug(FMT_STRING("leveled lists {:08X} added global variable chance {:08X}"), curobj->formID, addForm->formID);
				}
			}
			else {
				curobj->chanceGlobal = nullptr;
				logger::debug(FMT_STRING("leveled lists {:08X} removed global variable chance"), curobj->formID);
			}
		}

		if (!line.chanceNone.empty() && line.chanceNone != "none") {
			curobj->chanceNone = static_cast<int8_t>(std::stoi(line.chanceNone));
			logger::debug(FMT_STRING("leveled Lists {:08X} set chance of none to {}"), curobj->formID, curobj->chanceNone);
		}

		if (!line.objectMultCount.empty() && curobj->numEntries > 0) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
			for (const auto& objectToreplace : line.objectMultCount) {
				std::string formToReplaceString = objectToreplace[0];
				float multiplier = std::stof(objectToreplace[1]);
				int          count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				if (formToReplace  ) {
					for (auto& obj : llVec) {
						if (obj.form->formID == formToReplace->formID) {
							obj.count = static_cast<uint16_t>(std::ceil(obj.count * multiplier));
							count++;
						}
					}
					std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
						return a.level < b.level;
						});
					if (llVec.size() >= 1 && count > 0) {
						createLL(curobj, llVec);
						logger::debug(FMT_STRING("leveled Lists {:08X} all matching forms of {:08X} have their count multiplied by {}. Total changes: {}"), curobj->formID, formToReplace->formID, multiplier, count);
					}
				}

			}
		}


	}

}
