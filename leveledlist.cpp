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

		using func_t = decltype(&FreeLeveledList);
		const REL::Relocation<func_t> func{ REL::ID(296092) };
		return func(a_lobj, arg2);
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
		for (uint32_t ii = 0; ii < size; ii++)
			newLL->ll[ii] = a_llad[ii];

		levl->entries.resize(sizeof(RE::LEVELED_OBJECT) * newLL->count);
		
		for (int i = 0; i < newLL->count; i++) {
			levl->entries[i] = newLL->ll[i];  
		}

		//= nLL->ll;
		levl->numEntries = static_cast<int8_t>(size);
	}

	void ClearLL(RE::TESLeveledList* levl)
	{
		if (!levl)
			return;

		if (!levl->entries.empty()) {
			memset(levl->entries.data(), 0, levl->numEntries * sizeof(RE::LEVELED_OBJECT));
			FreeLeveledList(levl->entries.data());

			levl->entries.clear();
			levl->numEntries = 0;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		// Regular expression pattern to extract level, count and chance
		// extract addToLL
		std::regex  addToLL_regex("addToLL\\s*=([^:]+)", regex::icase);
		std::smatch addToLL_match;
		std::regex_search(line, addToLL_match, addToLL_regex);
		std::vector<std::string> addToLL;
		if (addToLL_match.empty() || addToLL_match[1].str().empty()) {
			//empty
		} else {
			std::string              valueLine = addToLL_match[1].str();
			std::vector<std::string> arr;
			// exclude the addToLL= part from the first string
			size_t      startPos = valueLine.find("=") + 1;
			size_t      pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				arr.push_back(token);
				startPos = pos + 1;
			}
			arr.push_back(valueLine.substr(startPos));

			for (std::string s : arr) {
				std::cout << "String: " << s << std::endl;
				std::vector<std::string> splitArr;
				size_t                   innerPos = 0;
				std::string              innerToken;
				while ((innerPos = s.find("~")) != std::string::npos) {
					innerToken = s.substr(0, innerPos);
					splitArr.push_back(innerToken);
					s.erase(0, innerPos + 1);
				}
				splitArr.push_back(s);

				l.object = splitArr[0];
				l.level = splitArr[1];
				l.reference = splitArr[2];
				l.count = splitArr[3];
				l.chance = splitArr[4];
			}
		}

		//// extract objects
		//std::regex  objects_regex("filterByLLs\\s*=([^:]+)", regex::icase);
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

		extractData(line, "filterByLLs\\s*=([^:]+)", l.objects);

		// extract noFilterLL
		std::regex  noFilterLL_regex("noFilterLL\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLmatch;
		std::regex_search(line, noFilterLLmatch, noFilterLL_regex);
		// extract the value after the equals sign
		if (noFilterLLmatch.empty() || noFilterLLmatch[1].str().empty()) {
			l.noFilterLL = "none";
		} else {
			std::string noFilterLLvalue = noFilterLLmatch[1].str();
			noFilterLLvalue.erase(noFilterLLvalue.begin(), std::find_if_not(noFilterLLvalue.begin(), noFilterLLvalue.end(), ::isspace));
			noFilterLLvalue.erase(std::find_if_not(noFilterLLvalue.rbegin(), noFilterLLvalue.rend(), ::isspace).base(), noFilterLLvalue.end());
			l.noFilterLL = noFilterLLvalue;
		}

		//// extract objectsContainer
		//std::regex  objectsContainer_regex("filterByContainers\\s*=([^:]+)", regex::icase);
		//std::smatch objectsContainer_match;
		//std::regex_search(line, objectsContainer_match, objectsContainer_regex);
		//std::vector<std::string> objectsContainer;
		//if (objectsContainer_match.empty() || objectsContainer_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objectsContainer_str = objectsContainer_match[1];
		//	std::regex           objectsContainer_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objectsContainer_iterator(objectsContainer_str.begin(), objectsContainer_str.end(), objectsContainer_list_regex);
		//	std::sregex_iterator objectsContainer_end;
		//	while (objectsContainer_iterator != objectsContainer_end) {
		//		std::string tempVar = (*objectsContainer_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			objectsContainer.push_back(tempVar);
		//		}
		//		++objectsContainer_iterator;
		//	}
		//	l.containerObjects = objectsContainer;
		//}

		extractData(line, "filterByContainers\\s*=([^:]+)", l.containerObjects);

		std::regex  add_regex("addToLLs\\s*=([^:]+)", regex::icase);
		std::smatch add_match;
		std::regex_search(line, add_match, add_regex);
		std::vector<std::string> add;
		if (add_match.empty() || add_match[1].str().empty()) {
			//empty
		} else {
			std::string              valueLine = add_match[1].str();
			std::vector<std::string> arr;

			// exclude the addToLL= part from the first string
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
			l.addedObjects = arr2D;
		}

		std::regex  addContainer_regex("addToContainers\\s*=([^:]+)", regex::icase);
		std::smatch addContainer_match;
		std::regex_search(line, addContainer_match, addContainer_regex);
		std::vector<std::string> addContainer;
		if (addContainer_match.empty() || addContainer_match[1].str().empty()) {
			//empty
		} else {
			std::string              valueLine = addContainer_match[1].str();
			std::vector<std::string> arr;

			// exclude the addToLL= part from the first string
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
			l.addedContainerObjects = arr2D;
		}

		//// extract objectsToRemove
		//std::regex  objectsToRemove_regex("removeFromContainers\\s*=([^:]+)", regex::icase);
		//std::smatch objectsToRemove_match;
		//std::regex_search(line, objectsToRemove_match, objectsToRemove_regex);
		//std::vector<std::string> objectsToRemove;
		//if (objectsToRemove_match.empty() || objectsToRemove_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objectsToRemove_str = objectsToRemove_match[1];
		//	std::regex           objectsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objectsToRemove_iterator(objectsToRemove_str.begin(), objectsToRemove_str.end(), objectsToRemove_list_regex);
		//	std::sregex_iterator objectsToRemove_end;
		//	while (objectsToRemove_iterator != objectsToRemove_end) {
		//		std::string spellToAdd = (*objectsToRemove_iterator)[0].str();
		//		spellToAdd.erase(spellToAdd.begin(), std::find_if_not(spellToAdd.begin(), spellToAdd.end(), ::isspace));
		//		spellToAdd.erase(std::find_if_not(spellToAdd.rbegin(), spellToAdd.rend(), ::isspace).base(), spellToAdd.end());
		//		if ((*objectsToRemove_iterator)[0].str() != "none") {
		//			//logger::info(FMT_STRING("objectsToRemove: {}"), spellToAdd);
		//			objectsToRemove.push_back(spellToAdd);
		//		}
		//		++objectsToRemove_iterator;
		//	}
		//	l.removedContainerObjects = objectsToRemove;
		//}

		extractData(line, "removeFromContainers\\s*=([^:]+)", l.removedContainerObjects);

		std::regex  remove_regex("removeFromLLs\\s*=([^:]+)", regex::icase);
		std::smatch remove_match;
		std::regex_search(line, remove_match, remove_regex);
		std::vector<std::string> remove;
		if (remove_match.empty() || remove_match[1].str().empty()) {
			//empty
		} else {
			std::string              valueLine = remove_match[1].str();
			std::vector<std::string> arr;

			// exclude the removeToLL= part from the first string
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
			l.removedObjects = arr2D;
		}

		// extract clear
		std::regex  clearlist_regex("clear\\s*=([^:]+)", regex::icase);
		std::smatch clearlistmatch;
		std::regex_search(line, clearlistmatch, clearlist_regex);
		// extract the value after the equals sign
		if (clearlistmatch.empty() || clearlistmatch[1].str().empty()) {
			l.clear = "none";
		} else {
			std::string clearlistvalue = clearlistmatch[1].str();
			clearlistvalue.erase(std::remove_if(clearlistvalue.begin(), clearlistvalue.end(), ::isspace), clearlistvalue.end());
			l.clear = clearlistvalue;
		}

		// extract calcforLevel
		std::regex  calcforLevellist_regex("calcForLevel\\s*=([^:]+)", regex::icase);
		std::smatch calcforLevellistmatch;
		std::regex_search(line, calcforLevellistmatch, calcforLevellist_regex);
		// extract the value after the equals sign
		if (calcforLevellistmatch.empty() || calcforLevellistmatch[1].str().empty()) {
			l.calcForLevel = "none";
		} else {
			std::string calcforLevellistvalue = calcforLevellistmatch[1].str();
			calcforLevellistvalue.erase(std::remove_if(calcforLevellistvalue.begin(), calcforLevellistvalue.end(), ::isspace), calcforLevellistvalue.end());
			l.calcForLevel = calcforLevellistvalue;
		}

		// extract calcEachItem
		std::regex  calcEachItemlist_regex("calcEachItem\\s*=([^:]+)", regex::icase);
		std::smatch calcEachItemlistmatch;
		std::regex_search(line, calcEachItemlistmatch, calcEachItemlist_regex);
		// extract the value after the equals sign
		if (calcEachItemlistmatch.empty() || calcEachItemlistmatch[1].str().empty()) {
			l.calcEachItem = "none";
		} else {
			std::string calcEachItemlistvalue = calcEachItemlistmatch[1].str();
			calcEachItemlistvalue.erase(std::remove_if(calcEachItemlistvalue.begin(), calcEachItemlistvalue.end(), ::isspace), calcEachItemlistvalue.end());
			l.calcEachItem = calcEachItemlistvalue;
		}

		// extract calcLevelAndEachItem
		std::regex  calcLevelAndEachItemlist_regex("calcLevelAndEachItem\\s*=([^:]+)", regex::icase);
		std::smatch calcLevelAndEachItemlistmatch;
		std::regex_search(line, calcLevelAndEachItemlistmatch, calcLevelAndEachItemlist_regex);
		// extract the value after the equals sign
		if (calcLevelAndEachItemlistmatch.empty() || calcLevelAndEachItemlistmatch[1].str().empty()) {
			l.calcForLevelAndEachItem = "none";
		} else {
			std::string calcLevelAndEachItemlistvalue = calcLevelAndEachItemlistmatch[1].str();
			calcLevelAndEachItemlistvalue.erase(std::remove_if(calcLevelAndEachItemlistvalue.begin(), calcLevelAndEachItemlistvalue.end(), ::isspace), calcLevelAndEachItemlistvalue.end());
			l.calcForLevelAndEachItem = calcLevelAndEachItemlistvalue;
		}

		// extract calcUseAll
		std::regex  calcUseAlllist_regex("calcUseAll\\s*=([^:]+)", regex::icase);
		std::smatch calcUseAlllistmatch;
		std::regex_search(line, calcUseAlllistmatch, calcUseAlllist_regex);
		// extract the value after the equals sign
		if (calcUseAlllistmatch.empty() || calcUseAlllistmatch[1].str().empty()) {
			l.calcUseAll = "none";
		} else {
			std::string calcUseAlllistvalue = calcUseAlllistmatch[1].str();
			calcUseAlllistvalue.erase(std::remove_if(calcUseAlllistvalue.begin(), calcUseAlllistvalue.end(), ::isspace), calcUseAlllistvalue.end());
			l.calcUseAll = calcUseAlllistvalue;
		}

		std::regex  templateKeyword_regex("addTemplateKeywords\\s*=([^:]+)", regex::icase);
		std::smatch templateKeyword_match;
		std::regex_search(line, templateKeyword_match, templateKeyword_regex);
		std::vector<std::string> templateKeyword;
		if (templateKeyword_match.empty() || templateKeyword_match[1].str().empty()) {
			//empty
		} else {
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

		//// extract removeWeaponsByKeyword
		//std::regex  removeWeaponsByKeyword_regex("removeObjectsByKeyword\\s*=([^:]+)", regex::icase);
		//std::smatch removeWeaponsByKeyword_match;
		//std::regex_search(line, removeWeaponsByKeyword_match, removeWeaponsByKeyword_regex);
		//std::vector<std::string> removeWeaponsByKeyword;
		//if (removeWeaponsByKeyword_match.empty() || removeWeaponsByKeyword_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          removeWeaponsByKeyword_str = removeWeaponsByKeyword_match[1];
		//	std::regex           removeWeaponsByKeyword_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator removeWeaponsByKeyword_iterator(removeWeaponsByKeyword_str.begin(), removeWeaponsByKeyword_str.end(), removeWeaponsByKeyword_list_regex);
		//	std::sregex_iterator removeWeaponsByKeyword_end;
		//	while (removeWeaponsByKeyword_iterator != removeWeaponsByKeyword_end) {
		//		std::string keywordToRemove = (*removeWeaponsByKeyword_iterator)[0].str();
		//		keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
		//		keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
		//		if (keywordToRemove != "none") {
		//			//logger::info(FMT_STRING("removeWeaponsByKeyword: {}"), keywordToRemove);
		//			removeWeaponsByKeyword.push_back(keywordToRemove);
		//		}
		//		++removeWeaponsByKeyword_iterator;
		//	}
		//	l.removeItemsByKeyword = removeWeaponsByKeyword;
		//}

		extractData(line, "removeObjectsByKeyword\\s*=([^:]+)", l.removeItemsByKeyword);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                    dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESLevItem*> objectArray = dataHandler->GetFormArray<RE::TESLevItem>();

		const auto                       containerHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectCONT*> containerArray = containerHandler->GetFormArray<RE::TESObjectCONT>();

		for (const auto& line : tokens) {
			//New
			if (!line.objects.empty()) {
				for (const auto& objectstring : line.objects) {
					RE::TESForm*    currentform = nullptr;
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
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectCONT* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Container) {
						object = (RE::TESObjectCONT*)currentform;
						patchContainer(line, object);
					}
				}
			}

			if (!line.noFilterLL.empty() && toLowerCase(line.noFilterLL) == "true") {
				for (const auto& curobj : objectArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}
					patch(line, curobj);
				}
			}

			//for (const auto& curobj : objectArray) {
			//	bool found = false;

			//	//Legacy
			//	if (!line.object.empty()) {
			//		RE::TESForm* currentform = nullptr;
			//		RE::TESLevItem* npc = nullptr;
			//		std::string string_form = line.object;
			//		currentform = GetFormFromIdentifier(string_form);
			//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
			//			npc = (RE::TESLevItem*)currentform;
			//			if (curobj->formID == npc->formID) {
			//				found = true;
			//				//logger::debug(FMT_STRING("LVLI Found {:08X}"), npc->formID);
			//			}
			//		}
			//	}

			//	//New
			//	if (!line.objects.empty()) {
			//		//logger::info("npc not empty");
			//		for (const auto& objectstring : line.objects) {
			//			RE::TESForm* currentform = nullptr;
			//			RE::TESLevItem* object = nullptr;

			//			std::string string_form = objectstring;
			//			currentform = GetFormFromIdentifier(string_form);
			//			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
			//				object = (RE::TESLevItem*)currentform;

			//				if (curobj->formID == object->formID) {
			//					found = true;
			//					//logger::debug("LL found: {:08X} flags {}", object->formID, object->llFlags);
			//					break;
			//				}
			//			}
			//		}
			//	}

			//	if (!line.object.empty() && !line.objects.empty()) {
			//		found = true;
			//	}

			//	if (found && !line.removedObjects.empty()) {
			//		std::vector<RE::LEVELED_OBJECT> llVec = GetLL(curobj);

			//		for (const auto& objectsToRemove : line.removedObjects) {
			//			RE::TESForm* delForm = GetFormFromIdentifier(objectsToRemove[0]);
			//			if (!delForm) {
			//				logger::debug(FMT_STRING("Form not found {}, skipping."), objectsToRemove[0]);
			//				continue;
			//			}

			//			std::string relationSign = "";
			//			uint16_t number;
			//			bool hasNumberLvl = false;
			//			bool hasNumberCount = false;

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
			//		SetLL(curobj, llVec);
			//	}

			//	if (found && !line.templateKeyword.empty()) {

			//		for (const auto& object : line.templateKeyword) {
			//
			//			RE::BGSKeyword* addForm = (RE::BGSKeyword*)GetFormFromIdentifier(object[0]);
			//			changeKeyword_TESLevItem(curobj, addForm, std::stoi(object[1]));
			//			logger::debug(FMT_STRING("leveled Lists formid: {:08X} added/changed {:08X} {} {}"), curobj->formID, addForm->formID, addForm->formEditorID, object[1]);
			//
			//		}

			//		//curobj->keywordChances

			//	}

			//	if (found && !line.clear.empty() && line.clear != "none") {
			//		if (line.clear == "yes" || line.clear == "true") {
			//			logger::debug(FMT_STRING("leveled lists {:08X} cleared"), curobj->formID);
			//			ClearLL(curobj);

			//		}
			//	}
			//	if (found && !line.calcForLevel.empty() && line.calcForLevel != "none") {
			//		if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
			//			curobj->llFlags = 1;
			//			logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
			//		}
			//	}

			//	if (found && !line.calcEachItem.empty() && line.calcEachItem != "none") {
			//		if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
			//			curobj->llFlags = 2;
			//			logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
			//		}
			//	}

			//	if (found && !line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
			//		if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
			//			curobj->llFlags = 3;
			//			logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
			//		}
			//	}

			//	if (found && !line.calcUseAll.empty() && line.calcUseAll != "none") {
			//			curobj->llFlags = 4;
			//			curobj->maxUseAllCount = std::stoi(line.calcUseAll);
			//			logger::debug(FMT_STRING("leveled lists {:08X} set to Use All with amount {}"), curobj->formID, line.calcUseAll);
			//	}

			//	if (found && !line.addedObjects.empty()) {
			//		std::vector<RE::LEVELED_OBJECT> llVec = GetLL(curobj);
			//		for (const auto& objectToAdd : line.addedObjects) {
			//			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
			//			std::string addFormStr = objectToAdd[0];
			//			uint16_t level = std::stoi(objectToAdd[1]);
			//			uint16_t count = std::stoi(objectToAdd[2]);
			//			uint8_t chanceNone = std::stoi(objectToAdd[3]);
			//			RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

			//			if (addForm) {
			//				llVec.push_back({ addForm, nullptr, count, level, static_cast<int8_t>(chanceNone) });
			//				logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
			//			} else {
			//				logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
			//			}
			//		}
			//		std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
			//			return a.level < b.level;
			//		});
			//		SetLL(curobj, llVec);
			//	}

			//	////////////////////////////////////////////
			//	//Legacy
			//	////////////////////////////////////////////
			//	if (found && !line.object.empty()) {
			//		std::vector<RE::LEVELED_OBJECT> llVec = GetLL(curobj);
			//		std::string addFormStr = line.reference;
			//		uint16_t level = std::stoi(line.level);
			//		uint16_t count = std::stoi(line.count);
			//		uint8_t chanceNone = std::stoi(line.chance);
			//		RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

			//		llVec.push_back({ addForm, nullptr, count, level, static_cast<int8_t>(chanceNone) });

			//		std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
			//			return a.level < b.level;
			//		});

			//		logger::debug(FMT_STRING("LVLI {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
			//		SetLL(curobj, llVec);
			//	}
			//}//after LLs

			//for (const auto& curobj : containerArray) {
			//	bool found = false;

			//	if (!line.containerObjects.empty()) {
			//		//logger::info("npc not empty");
			//		for (const auto& objectstring : line.containerObjects) {
			//			RE::TESForm* currentform = nullptr;
			//			RE::TESObjectCONT* object = nullptr;

			//			std::string string_form = objectstring;
			//			currentform = GetFormFromIdentifier(string_form);
			//			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kCONT) {
			//				object = (RE::TESObjectCONT*)currentform;

			//				if (curobj->formID == object->formID) {
			//					found = true;
			//					//logger::debug("LL found: {:08X} flags {}", object->formID, object->llFlags);
			//					break;
			//				}
			//			}
			//		}
			//	}

			//	if (found && !line.addedContainerObjects.empty()) {
			//		for (const auto& objectToAdd : line.addedContainerObjects) {
			//			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
			//			std::string addFormStr = objectToAdd[0];
			//			uint32_t count = std::stoi(objectToAdd[1]);
			//			RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

			//			if (addForm) {
			//				curobj->AddObject(addForm, count, nullptr);
			//				logger::debug(FMT_STRING("leveled Lists - container {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
			//			} else {
			//				logger::critical(FMT_STRING("leveled Lists - container {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
			//			}
			//		}
			//	}

			//	if (found && !line.removedContainerObjects.empty()) {
			//		for (const auto& objectToAdd : line.removedContainerObjects) {
			//			//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
			//			std::string addFormStr = objectToAdd;
			//			RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

			//			if (addForm) {
			//				curobj->RemoveObject(addForm);
			//				logger::debug(FMT_STRING("leveled Lists - container {:08X} removed Form {:08X}"), curobj->formID, addForm->formID);
			//			} else {
			//				logger::critical(FMT_STRING("leveled Lists - container {:08X} Form not found: {}"), curobj->formID, objectToAdd);
			//			}
			//		}
			//	}

			//}
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

	void patch(LEVELEDLIST::line_content line, RE::TESLevItem* curobj)
	{
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
					} else if (templvl.size() == 1 && templvl[0] != "none") {
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
						} else if (relationSign == ">") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level > number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == ">=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level >= number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == "<=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.level <= number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} level <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == "") {
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
					} else if (tempcount.size() == 1 && tempcount[0] != "none") {
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
						} else if (relationSign == ">") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count > number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count > {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == ">=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count >= number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count >= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == "<=") {
							llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
								return x.count <= number && x.form == delForm;
							}),
								llVec.end());
							logger::debug(FMT_STRING("leveled Lists {:08X} count <= {} removed Form {:08X}"), curobj->formID, number, delForm->formID);
						} else if (relationSign == "") {
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
			createLL(curobj, llVec);
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
			createLL(curobj, llVec);
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
		//if (!line.calcForLevel.empty() && line.calcForLevel != "none") {
		//	if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
		//		curobj->llFlags = 1;
		//		logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
		//	}
		//}

		//if (!line.calcEachItem.empty() && line.calcEachItem != "none") {
		//	if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
		//		curobj->llFlags = 2;
		//		logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
		//	}
		//}

		//if (!line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
		//	if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
		//		curobj->llFlags = 3;
		//		logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
		//	}
		//}

		if (!line.calcUseAll.empty() && line.calcUseAll != "none") {
			curobj->llFlags = RE::TESLeveledList::kUseAll;
			logger::debug(FMT_STRING("leveled lists {:08X} set to Use All with amount {}"), curobj->formID, line.calcUseAll);
		}

		if (!line.addedObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);

			for (const auto& objectToAdd : line.addedObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string  addFormStr = objectToAdd[0];
				uint16_t     level = std::stoi(objectToAdd[1]);
				uint16_t     count = std::stoi(objectToAdd[2]);
				//uint8_t      chanceNone = std::stoi(objectToAdd[3]);
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

				if (addForm) {
					llVec.push_back({ addForm, count, level });
					logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
			std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
				return a.level < b.level;
			});
			createLL(curobj, llVec);
		}
	}

	void patchContainer(LEVELEDLIST::line_content line, RE::TESObjectCONT* curobj)
	{
		if (!line.addedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd[0];
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					curobj->AddObjectToContainer(addForm, count, nullptr);
					logger::debug(FMT_STRING("leveled Lists - container {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("leveled Lists - container {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.removedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd;
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					curobj->RemoveObjectFromContainer(addForm, 1);
					logger::debug(FMT_STRING("leveled Lists - container {:08X} removed Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("leveled Lists - container {:08X} Form not found: {}"), curobj->formID, objectToAdd);
				}
			}
		}
	}

}
