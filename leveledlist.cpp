#include "leveledlist.h"
namespace LEVELEDLIST
{

	//void __fastcall TESLeveledList::FreeLeveledList(TESLeveledList* this)
	//{
	//	int* v2;             // rsi
	//	int v3;              // ebx
	//	LEVELED_OBJECT* v4;  // rcx
	//	void* v5;            // rbp
	//	v2 = (int*)(*((_QWORD*)NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)tls_index) + 2496i64);
	//	v3 = *v2;
	//	*v2 = 98;
	//	v4 = (LEVELED_OBJECT*)*((_QWORD*)this + 3);
	//	if (v4)
	//		LEVELED_OBJECT::`vector deleting destructor'(v4, 3); v5 = (void*)*((_QWORD*)this + 4);
	//	*((_QWORD*)this + 3) = 0i64;
	//	*((_BYTE*)this + 41) = 0;
	//	if ( `MemoryManager::Instance '::`2' ::uiinitFence != 2)
	//		MemoryManager::UpdateInitState(
	//     `MemoryManager::Instance '::`2' ::cbuffer,
	//			(unsigned int*)&`MemoryManager::Instance '::`2' ::uiinitFence);
	//	MemoryManager::Deallocate((MemoryManager*)`MemoryManager::Instance '::`2' ::cbuffer, v5, 0);
	//	*((_QWORD*)this + 4) = 0i64;
	//	*((_BYTE*)this + 40) = 0;
	//	*v2 = v3;
	//}

	//void FreeLeveledList(RE::TESLeveledList* ll)
	//{
	//	using func_t = decltype(&FreeLeveledList);
	//	const REL::Relocation<func_t> func{ REL::ID(645214) };  //14015BA50
	//	return func(ll);
	//}

	void FreeLeveledList(RE::LEVELED_OBJECT* a_lobj, uint32_t arg2 = 0x3)
	{
		if (!a_lobj)
			return;

		RE::free(a_lobj);

		//using func_t = decltype(&FreeLeveledList);
		//const REL::Relocation<func_t> func{ REL::ID(296092) };
		//return func(a_lobj, arg2);
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
			//logger::debug("ceateLL size after clear entries numEntries {}", levl->entries.size(), levl->numEntries);
		}

		if (a_llad.empty())
			return;

		uint32_t  size = static_cast<uint32_t>(a_llad.size());
		//logger::debug("ceateLL size {}", size);
		LL_ALLOC* newLL = reserveNewLevL(size);
		if (!newLL)
			return;

		newLL->count = size;
		for (uint32_t ii = 0; ii < size; ii++) {
			newLL->ll[ii] = a_llad[ii];
			//logger::debug("ceateLL size for {}", ii);
		}

		//logger::debug("ceateLL size before resize entries {} numEntries {}", levl->entries.size(), levl->numEntries);

		levl->entries.resize(newLL->count);

		//logger::debug("ceateLL size newLL->count {}", newLL->count);

		//logger::debug("ceateLL size before set entries {} numEntries {}", levl->entries.size(), levl->numEntries);

		for (int i = 0; i < newLL->count; i++) {
			levl->entries[i] = newLL->ll[i];
			//logger::debug("ceateLL size for {}", i);
		}

		//= nLL->ll;
		levl->numEntries = static_cast<int8_t>(size);
		//logger::debug("ceateLL size static_cast<int8_t>(size) {}", static_cast<int8_t>(size));
		//logger::debug("ceateLL size entries {} numEntries {}", levl->entries.size(), levl->numEntries);
	}

	void ClearLL(RE::TESLeveledList* levl)
	{
		if (!levl)
			return;

		if (!levl->entries.empty()) {
			memset(levl->entries.data(), 0, levl->numEntries * sizeof(RE::LEVELED_OBJECT));
			//FreeLeveledList(levl->entries.data());

			levl->entries.clear();
			levl->numEntries = 0;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;




		extractData(line, "filterByLLs\\s*=([^:]+)", l.objects);

		// extract noFilterLL
		std::regex  noFilterLL_regex("noFilterLL\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLmatch;
		std::regex_search(line, noFilterLLmatch, noFilterLL_regex);
		// extract the value after the equals sign
		if (noFilterLLmatch.empty() || noFilterLLmatch[1].str().empty()) {
			l.noFilterLL = "none";
		}
		else {
			std::string noFilterLLvalue = noFilterLLmatch[1].str();
			noFilterLLvalue.erase(noFilterLLvalue.begin(), std::find_if_not(noFilterLLvalue.begin(), noFilterLLvalue.end(), ::isspace));
			noFilterLLvalue.erase(std::find_if_not(noFilterLLvalue.rbegin(), noFilterLLvalue.rend(), ::isspace).base(), noFilterLLvalue.end());
			l.noFilterLL = noFilterLLvalue;
		}

		// extract noFilterLLNPC
		std::regex  noFilterLLNPC_regex("noFilterLLNPC\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLNPCmatch;
		std::regex_search(line, noFilterLLNPCmatch, noFilterLLNPC_regex);
		// extract the value after the equals sign
		if (noFilterLLNPCmatch.empty() || noFilterLLNPCmatch[1].str().empty()) {
			l.noFilterLLNPC = "none";
		}
		else {
			std::string noFilterLLNPCvalue = noFilterLLNPCmatch[1].str();
			noFilterLLNPCvalue.erase(noFilterLLNPCvalue.begin(), std::find_if_not(noFilterLLNPCvalue.begin(), noFilterLLNPCvalue.end(), ::isspace));
			noFilterLLNPCvalue.erase(std::find_if_not(noFilterLLNPCvalue.rbegin(), noFilterLLNPCvalue.rend(), ::isspace).base(), noFilterLLNPCvalue.end());
			l.noFilterLLNPC = noFilterLLNPCvalue;
		}

		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);


		extractData(line, "filterByLLNPCs\\s*=([^:]+)", l.containerObjects);

		extractToArr2D(line, "addToLLs\\s*=([^:]+)", l.addedObjects);
		extractToArr2D(line, "addOnceToLLs\\s*=([^:]+)", l.addedObjectsOnce);

		extractToArr2D(line, "removeFromLLs\\s*=([^:]+)", l.removedObjects);

		extractValueString(line, "clear\\s*=([^:]+)", l.clear);

		extractValueString(line, "calcForLevel\\s*=([^:]+)", l.calcForLevel);

		extractValueString(line, "calcEachItem\\s*=([^:]+)", l.calcEachItem);

		extractValueString(line, "calcLevelAndEachItem\\s*=([^:]+)", l.calcForLevelAndEachItem);

		extractValueString(line, "calcUseAll\\s*=([^:]+)", l.calcUseAll);

		extractValueString(line, "clearFlags\\s*=([^:]+)", l.clearFlags);


		std::regex  templateKeyword_regex("addTemplateKeywords\\s*=([^:]+)", regex::icase);
		std::smatch templateKeyword_match;
		std::regex_search(line, templateKeyword_match, templateKeyword_regex);
		std::vector<std::string> templateKeyword;
		if (templateKeyword_match.empty() || templateKeyword_match[1].str().empty()) {
			//empty
		}
		else {
			std::string              valueLine = templateKeyword_match[1].str();
			std::vector<std::string> arr;

			// exclude the templateKeywordToLL= part from the first string
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
			l.templateKeyword = arr2D;
		}

		extractData(line, "removeObjectsByKeyword\\s*=([^:]+)", l.removeItemsByKeyword);
		extractValueString(line, "chanceGlobal\\s*=([^:]+)", l.llglobal);
		extractValueString(line, "chanceNone\\s*=([^:]+)", l.chanceNone);
		extractToArr2D(line, "formsToReplace\\s*=([^:]+)", l.replaceObjects);
		extractToArr2D(line, "objectMultCount\\s*=([^:]+)", l.objectMultCount);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                    dataHandler = RE::TESDataHandler::GetSingleton();




		for (const auto& line : tokens) {
			//New
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
						//logger::debug(FMT_STRING("leveled lists {:08X} call patch."), object->formID);
						patch(line, object);
					}
				}
			}


			if (!line.containerObjects.empty()) {
				//logger::info("npc not empty");
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevCharacter* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::LeveledNPC) {
						object = (RE::TESLevCharacter*)currentform;
						//logger::debug(FMT_STRING("leveled list char"));
						patch(line, object);
					}
				}
			}

			if (!line.noFilterLL.empty() && toLowerCase(line.noFilterLL) == "true") {
				RE::BSTArray<RE::TESLevItem*> objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
				for (const auto& curobj : objectArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					patch(line, curobj);
				}
			}

			if (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty() || !line.filterByEditorIdContainsExcluded.empty()) {
				RE::BSTArray<RE::TESLevItem*> objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
				for (const auto& curobj : objectArray) {

					bool contains = false;
					bool containsOr = false;
					bool found = false;

					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					if (!line.filterByEditorIdContains.empty()) {
						for (const auto& editorString : line.filterByEditorIdContains) {
							if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
								contains = true;
							}
							else {
								contains = false;
								break;
							}
						}
					}
					else {
						contains = true;
					}

					if (!line.filterByEditorIdContainsOr.empty()) {
						//logger::info("keywords not empty");
						for (const auto& editorString : line.filterByEditorIdContainsOr) {
							if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
								containsOr = true;
								break;
							}
						}
					}
					else {
						containsOr = true;
					}

					if ((!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
						//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
						found = true;
					}

					if (!line.filterByEditorIdContainsExcluded.empty() && found) {
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


			if (!line.noFilterLLNPC.empty() && toLowerCase(line.noFilterLLNPC) == "true") {
				RE::BSTArray<RE::TESLevCharacter*> tesLevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();
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
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}
	template <typename T>
	void patch(LEVELEDLIST::line_content line, T* curobj)
	{


		if (!line.replaceObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);

			for (const auto& objectToreplace : line.replaceObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string formToReplaceString = objectToreplace[0];
				std::string formToReplaceWithString = objectToreplace[1];
				int          count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				RE::TESForm* formToReplaceWith = GetFormFromIdentifier(formToReplaceWithString);
				// Replace logic for the entire vector
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
						//logger::debug(FMT_STRING("leveled Lists {:08X} cannot removed item {:08X} not a keyword {:08X} {} "), curobj->formID, x.form->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);

						return false;
						}),
						llVec.end());
				}
			}
			//if ()
			if (llVec.size() >= 1) {
				createLL(curobj, llVec);
			}
			else {
				ClearLL(curobj);
				logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
			}
		}

		//if (!line.templateKeyword.empty()) {
		//	for (const auto& object : line.templateKeyword) {
		//		RE::BGSKeyword* addForm = (RE::BGSKeyword*)GetFormFromIdentifier(object[0]);
		//		changeKeyword_TESLevItem(curobj, addForm, std::stoi(object[1]));
		//		logger::debug(FMT_STRING("leveled Lists formid: {:08X} added/changed {:08X} {} {}"), curobj->formID, addForm->formID, addForm->formEditorID, object[1]);
		//	}

		//	//curobj->keywordChances
		//}

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
			//logger::debug(FMT_STRING("leveled Lists {:08X} size before adding {}"), curobj->formID, line.addedObjects.size());
			for (const auto& objectToAdd : line.addedObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				uint16_t level = 1;
				uint16_t count = 1;

				std::string  addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					level = std::stoi(objectToAdd[1]);
				}
				if (objectToAdd.size() > 2) {
					count = std::stoi(objectToAdd[2]);
				}
				//uint8_t      chanceNone = std::stoi(objectToAdd[3]);
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
			//logger::debug(FMT_STRING("leveled Lists {:08X} size before adding {}"), curobj->formID, line.addedObjectsOnce.size());
			for (const auto& objectToAdd : line.addedObjectsOnce) {
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
					// Check if addForm already exists in llVec
					auto it = std::find_if(llVec.begin(), llVec.end(), [addForm](const RE::LEVELED_OBJECT& obj) {
						return obj.form->formID == addForm->formID;
						});

					if (it == llVec.end()) {
						// addForm not found, add it to llVec
						llVec.push_back({ addForm, count, level });
						logger::debug(FMT_STRING("leveled Lists {:08X} added once Form {:08X}"), curobj->formID, addForm->formID);
					}
					else {
						// addForm already exists, log a message or handle it accordingly
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
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string formToReplaceString = objectToreplace[0];
				float multiplier = std::stof(objectToreplace[1]);
				int          count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				// Replace logic for the entire vector
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



	//void patchLevChar(LEVELEDLIST::line_content line, RE::TESLevCharacter* curobj)
	//{
	//	

	//	if (!line.removedObjects.empty()) {
	//		std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
	//		//logger::debug(FMT_STRING("leveled Lists {:08X} size of line {} size of entries {}"), curobj->formID, line.removedObjects.size(), llVec.size());
	//		for (const auto& objectsToRemove : line.removedObjects) {
	//			RE::TESForm* delForm = GetFormFromIdentifier(objectsToRemove[0]);
	//			if (!delForm) {
	//				logger::debug(FMT_STRING("Form not found {}, skipping."), objectsToRemove[0]);
	//				continue;
	//			}

	//			std::string relationSign = "";
	//			uint16_t    number;
	//			bool        hasNumberLvl = false;
	//			bool        hasNumberCount = false;

	//			if (objectsToRemove.size() >= 2) {
	//				std::vector<std::string> templvl = splitRelationNumber(objectsToRemove[1]);
	//				if (templvl.size() == 2 && templvl[0] != "none") {
	//					number = std::stoi(templvl[0]);
	//					relationSign = templvl[1];
	//					hasNumberLvl = true;
	//				} else if (templvl.size() == 1 && templvl[0] != "none") {
	//					number = std::stoi(templvl[0]);
	//					hasNumberLvl = true;
	//				}

	//				if (hasNumberLvl) {
	//					if (relationSign == "<") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.level < number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} level < {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == ">") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.level > number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} level > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == ">=") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.level >= number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} level >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == "<=") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.level <= number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} level <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == "") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.level == number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} level == {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					}
	//				}
	//			}

	//			if (objectsToRemove.size() >= 3) {
	//				std::vector<std::string> tempcount = splitRelationNumber(objectsToRemove[2]);

	//				relationSign = "";

	//				if (tempcount.size() == 2 && tempcount[0] != "none") {
	//					number = std::stoi(tempcount[0]);
	//					relationSign = tempcount[1];
	//					hasNumberCount = true;
	//				} else if (tempcount.size() == 1 && tempcount[0] != "none") {
	//					number = std::stoi(tempcount[0]);
	//					hasNumberCount = true;
	//				}

	//				if (hasNumberCount) {
	//					if (relationSign == "<") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.count < number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} count < {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == ">") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.count > number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} count > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == ">=") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.count >= number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} count >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == "<=") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.count <= number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} count <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					} else if (relationSign == "") {
	//						llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//							return x.count == number && x.form == delForm;
	//						}),
	//							llVec.end());
	//						logger::debug(FMT_STRING("leveled Lists {:08X} count == {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
	//					}
	//				}
	//			}

	//			if (!hasNumberLvl && !hasNumberCount) {
	//				llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//					return x.form == delForm;
	//				}),
	//					llVec.end());
	//				logger::debug(FMT_STRING("leveled Lists {:08X} removed all Forms {:08X}"), curobj->formID, delForm->formID);
	//			}

	//		}

	//		std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
	//			return a.level < b.level;
	//		});
	//		
	//		if (llVec.size() >= 1) {
	//			createLL(curobj, llVec);
	//		} else {
	//			ClearLL(curobj);
	//			logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
	//		}

	//	}

	//	if (!line.removeItemsByKeyword.empty()) {
	//		std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
	//		for (size_t i = 0; i < line.removeItemsByKeyword.size(); i++) {
	//			RE::TESForm* currentform = nullptr;
	//			std::string  string_form = line.removeItemsByKeyword[i];
	//			currentform = GetFormFromIdentifier(string_form);
	//			if (currentform && currentform->formType == RE::FormType::Keyword) {
	//				llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
	//					RE::BGSKeywordForm* keyForm = x.form->As<RE::BGSKeywordForm>();
	//					if (keyForm && keyForm->HasKeyword((RE::BGSKeyword*)currentform)) {
	//						logger::debug(FMT_STRING("leveled Lists {:08X} removed item {:08X} with keyword {:08X} {} "), curobj->formID, x.form->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
	//						return true;
	//					}
	//					return false;
	//				}),
	//					llVec.end());
	//			}
	//		}
	//		if (llVec.size() >= 1) {
	//			createLL(curobj, llVec);
	//		} else {
	//			ClearLL(curobj);
	//			logger::debug(FMT_STRING("leveled Lists {:08X} is now empty"), curobj->formID);
	//		}
	//	}

	//	//if (!line.formsToReplace.empty()) {
	//	//	std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
	//	//	if (!llVec.empty()) {
	//	//		for (uint32_t i = 0; i < line.formsToReplace.size(); i++) {
	//	//			RE::TESForm* currentform = nullptr;
	//	//			RE::TESForm* currentform2 = nullptr;
	//	//			std::string  string_form = line.formsToReplace[i];
	//	//			std::string  string_form2 = line.formsToReplaceWith[i];
	//	//			currentform = GetFormFromIdentifier(string_form);
	//	//			currentform2 = GetFormFromIdentifier(string_form2);
	//	//			if (currentform && currentform2 ) {
	//	//				// Loop through all elements in arrayOfForms and replace matches
	//	//				for (auto form : llVec) {
	//	//					if (form.form && form.form->formID == currentform->formID) {
	//	//						form.form = currentform2;  // Replace the matching form
	//	//						logger::debug(FMT_STRING("leveled Lists: {:08X} replaced {:08X} with {:08X} "), curobj->formID, currentform->formID, currentform2->formID);
	//	//					}
	//	//				}
	//	//			}
	//	//		}
	//	//	}
	//	//}

	//	
	//	if (!line.clear.empty() && line.clear != "none") {
	//		if (line.clear == "yes" || line.clear == "true") {
	//			logger::debug(FMT_STRING("leveled lists {:08X} cleared"), curobj->formID);
	//			ClearLL(curobj);
	//		}
	//	}
	//	
	//	if (!line.calcForLevel.empty() && line.calcForLevel != "none") {
	//		if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
	//			curobj->llFlags = RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel;
	//			logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
	//		}
	//	}

	//	if (!line.calcEachItem.empty() && line.calcEachItem != "none") {
	//		if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
	//			curobj->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
	//			logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
	//		}
	//	}

	//	if (!line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
	//		if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
	//			curobj->llFlags = static_cast<RE::TESLeveledList::Flag>(
	//				RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel |
	//				RE::TESLeveledList::Flag::kCalculateForEachItemInCount);
	//			logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
	//		}
	//	}

	//	if (!line.calcUseAll.empty() && line.calcUseAll != "none") {
	//		curobj->llFlags = RE::TESLeveledList::kUseAll;
	//		logger::debug(FMT_STRING("leveled lists {:08X} set to Use All"), curobj->formID);
	//	}

	//	if (!line.clearFlags.empty() && line.clearFlags != "none") {
	//		curobj->llFlags = resetAllFlags(curobj->llFlags);
	//		logger::debug(FMT_STRING("leveled lists {:08X} cleared all flags"), curobj->formID);
	//	}

	//	if (!line.addedObjects.empty()) {
	//		std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
	//		//logger::debug(FMT_STRING("leveled Lists {:08X} size before adding {}"), curobj->formID, line.addedObjects.size());
	//		for (const auto& objectToAdd : line.addedObjects) {
	//			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
	//			std::string addFormStr = objectToAdd[0];
	//			uint16_t    level = std::stoi(objectToAdd[1]);
	//			uint16_t    count = std::stoi(objectToAdd[2]);
	//			//uint8_t      chanceNone = std::stoi(objectToAdd[3]);
	//			RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

	//			if (addForm) {
	//				llVec.push_back({ addForm, count, level });
	//				logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
	//			} else {
	//				logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
	//			}
	//		}
	//		std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
	//			return a.level < b.level;
	//		});
	//		createLL(curobj, llVec);
	//		//logger::debug(FMT_STRING("leveled Lists {:08X} after adding {}"), curobj->formID, curobj->entries.size());
	//	}

	//	if (!line.addedObjectsOnce.empty()) {
	//		std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
	//		//logger::debug(FMT_STRING("leveled Lists {:08X} size before adding {}"), curobj->formID, line.addedObjectsOnce.size());
	//		for (const auto& objectToAdd : line.addedObjectsOnce) {
	//			std::string  addFormStr = objectToAdd[0];
	//			uint16_t     level = std::stoi(objectToAdd[1]);
	//			uint16_t     count = std::stoi(objectToAdd[2]);
	//			RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

	//			if (addForm) {
	//				// Check if addForm already exists in llVec
	//				auto it = std::find_if(llVec.begin(), llVec.end(), [addForm](const RE::LEVELED_OBJECT& obj) {
	//					return obj.form->formID == addForm->formID;
	//				});

	//				if (it == llVec.end()) {
	//					// addForm not found, add it to llVec
	//					llVec.push_back({ addForm, count, level });
	//					logger::debug(FMT_STRING("leveled Lists {:08X} added once Form {:08X}"), curobj->formID, addForm->formID);
	//				} else {
	//					// addForm already exists, log a message or handle it accordingly
	//					logger::debug(FMT_STRING("leveled Lists {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
	//				}
	//			} else {
	//				logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
	//			}
	//		}
	//	}


	//}

}
