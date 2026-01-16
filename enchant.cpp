#include "enchant.h"
namespace ENCH
{

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		//// extract objects
		//std::regex objects_regex("filterByEnchs\\s*=([^:]+)", regex::icase);
		//std::smatch objects_match;
		//std::regex_search(line, objects_match, objects_regex);
		//std::vector<std::string> objects;
		//if (objects_match.empty() || objects_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string objects_str = objects_match[1];
		//	std::regex objects_list_regex("[a-zA-Z0-9_\\-. ]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
		//	std::sregex_iterator objects_end;
		//	while (objects_iterator != objects_end) {
		//		std::string tempVar = (*objects_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("filterByAlchs: {}"), tempVar);
		//		if (tempVar != "none") {
		//			objects.push_back(tempVar);
		//		}
		//		++objects_iterator;
		//	}
		//	l.objects = objects;
		//}

		extractData(line, "filterByEnchs\\s*=([^:]+)", l.objects);

		//// extract objectsExcluded
		//std::regex objectsExcluded_regex("filterByEnchsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch objectsExcluded_match;
		//std::regex_search(line, objectsExcluded_match, objectsExcluded_regex);
		//std::vector<std::string> objectsExcluded;
		//if (objectsExcluded_match.empty() || objectsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string objectsExcluded_str = objectsExcluded_match[1];
		//	std::regex objectsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
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

		extractData(line, "filterByEnchsExcluded\\s*=([^:]+)", l.objectExcluded);

		//// extract keywords
		//std::regex keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
		//std::smatch keywords_match;
		//std::regex_search(line, keywords_match, keywords_regex);
		//std::vector<std::string> keywords;
		//if (keywords_match.empty() || keywords_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string keywords_str = keywords_match[1];
		//	std::regex keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
		//	std::sregex_iterator keywords_end;
		//	while (keywords_iterator != keywords_end) {
		//		std::string keyword = (*keywords_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywords.push_back(keyword);
		//		}
		//		++keywords_iterator;
		//	}
		//	l.keywords = keywords;
		//}

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		//// extract keywords
		//std::regex keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsOr_match;
		//std::regex_search(line, keywordsOr_match, keywordsOr_regex);
		//std::vector<std::string> keywordsOr;
		//if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string keywordsOr_str = keywordsOr_match[1];
		//	std::regex keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
		//	std::sregex_iterator keywordsOr_end;
		//	while (keywordsOr_iterator != keywordsOr_end) {
		//		std::string keyword = (*keywordsOr_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsOr.push_back(keyword);
		//		}
		//		++keywordsOr_iterator;
		//	}
		//	l.keywordsOr = keywordsOr;
		//}

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		//// extract keywords
		//std::regex keywordsExcluded_regex("filterByKeywordsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsExcluded_match;
		//std::regex_search(line, keywordsExcluded_match, keywordsExcluded_regex);
		//std::vector<std::string> keywordsExcluded;
		//if (keywordsExcluded_match.empty() || keywordsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string keywordsExcluded_str = keywordsExcluded_match[1];
		//	std::regex keywordsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsExcluded_iterator(keywordsExcluded_str.begin(), keywordsExcluded_str.end(), keywordsExcluded_list_regex);
		//	std::sregex_iterator keywordsExcluded_end;
		//	while (keywordsExcluded_iterator != keywordsExcluded_end) {
		//		std::string keyword = (*keywordsExcluded_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsExcluded.push_back(keyword);
		//		}
		//		++keywordsExcluded_iterator;
		//	}
		//	l.keywordsExcluded = keywordsExcluded;
		//}

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		//// extract Mgefs
		//std::regex Mgefs_regex("filterByMgefs\\s*=([^:]+)", regex::icase);
		//std::smatch Mgefs_match;
		//std::regex_search(line, Mgefs_match, Mgefs_regex);
		//std::vector<std::string> Mgefs;
		//if (Mgefs_match.empty() || Mgefs_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string Mgefs_str = Mgefs_match[1];
		//	std::regex Mgefs_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator Mgefs_iterator(Mgefs_str.begin(), Mgefs_str.end(), Mgefs_list_regex);
		//	std::sregex_iterator Mgefs_end;
		//	while (Mgefs_iterator != Mgefs_end) {
		//		std::string keyword = (*Mgefs_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			Mgefs.push_back(keyword);
		//		}
		//		++Mgefs_iterator;
		//	}
		//	l.mgefs = Mgefs;
		//}

		extractData(line, "filterByMgefs\\s*=([^:]+)", l.mgefs);

		//// extract Mgefs
		//std::regex MgefsOr_regex("filterByMgefsOr\\s*=([^:]+)", regex::icase);
		//std::smatch MgefsOr_match;
		//std::regex_search(line, MgefsOr_match, MgefsOr_regex);
		//std::vector<std::string> MgefsOr;
		//if (MgefsOr_match.empty() || MgefsOr_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string MgefsOr_str = MgefsOr_match[1];
		//	std::regex MgefsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator MgefsOr_iterator(MgefsOr_str.begin(), MgefsOr_str.end(), MgefsOr_list_regex);
		//	std::sregex_iterator MgefsOr_end;
		//	while (MgefsOr_iterator != MgefsOr_end) {
		//		std::string keyword = (*MgefsOr_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			MgefsOr.push_back(keyword);
		//		}
		//		++MgefsOr_iterator;
		//	}
		//	l.mgefsOr = MgefsOr;
		//}

		extractData(line, "filterByMgefsOr\\s*=([^:]+)", l.mgefsOr);

		//// extract Mgefs
		//std::regex MgefsExcluded_regex("filterByMgefsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch MgefsExcluded_match;
		//std::regex_search(line, MgefsExcluded_match, MgefsExcluded_regex);
		//std::vector<std::string> MgefsExcluded;
		//if (MgefsExcluded_match.empty() || MgefsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string MgefsExcluded_str = MgefsExcluded_match[1];
		//	std::regex MgefsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator MgefsExcluded_iterator(MgefsExcluded_str.begin(), MgefsExcluded_str.end(), MgefsExcluded_list_regex);
		//	std::sregex_iterator MgefsExcluded_end;
		//	while (MgefsExcluded_iterator != MgefsExcluded_end) {
		//		std::string keyword = (*MgefsExcluded_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			MgefsExcluded.push_back(keyword);
		//		}
		//		++MgefsExcluded_iterator;
		//	}
		//	l.mgefsExcluded = MgefsExcluded;
		//}

		extractData(line, "filterByMgefsExcluded\\s*=([^:]+)", l.mgefsExcluded);

		//// extract keywordsToAdd
		//std::regex keywordsToAdd_regex("keywordsToAdd\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsToAdd_match;
		//std::regex_search(line, keywordsToAdd_match, keywordsToAdd_regex);
		//std::vector<std::string> keywordsToAdd;
		//if (keywordsToAdd_match.empty() || keywordsToAdd_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string keywordsToAdd_str = keywordsToAdd_match[1];
		//	std::regex keywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsToAdd_iterator(keywordsToAdd_str.begin(), keywordsToAdd_str.end(), keywordsToAdd_list_regex);
		//	std::sregex_iterator keywordsToAdd_end;
		//	while (keywordsToAdd_iterator != keywordsToAdd_end) {
		//		std::string keywordToAdd = (*keywordsToAdd_iterator)[0].str();
		//		keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
		//		keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
		//		if (keywordToAdd != "none") {
		//			//logger::info(FMT_STRING("keywordsToAdd: {}"), keywordToAdd);
		//			keywordsToAdd.push_back(keywordToAdd);
		//		}
		//		++keywordsToAdd_iterator;
		//	}
		//	l.keywordsToAdd = keywordsToAdd;
		//}

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		//// extract keywordsToRemove
		//std::regex keywordsToRemove_regex("keywordsToRemove\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsToRemove_match;
		//std::regex_search(line, keywordsToRemove_match, keywordsToRemove_regex);
		//std::vector<std::string> keywordsToRemove;
		//if (keywordsToRemove_match.empty() || keywordsToRemove_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string keywordsToRemove_str = keywordsToRemove_match[1];
		//	std::regex keywordsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsToRemove_iterator(keywordsToRemove_str.begin(), keywordsToRemove_str.end(), keywordsToRemove_list_regex);
		//	std::sregex_iterator keywordsToRemove_end;
		//	while (keywordsToRemove_iterator != keywordsToRemove_end) {
		//		std::string keywordToRemove = (*keywordsToRemove_iterator)[0].str();
		//		keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
		//		keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
		//		if (keywordToRemove != "none") {
		//			//logger::info(FMT_STRING("keywordsToRemove: {}"), keywordToRemove);
		//			keywordsToRemove.push_back(keywordToRemove);
		//		}
		//		++keywordsToRemove_iterator;
		//	}
		//	l.keywordsToRemove = keywordsToRemove;
		//}

		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		//std::regex add_regex("mgefsToAdd\\s*=([^:]+)", regex::icase);
		//std::smatch add_match;
		//std::regex_search(line, add_match, add_regex);
		//std::vector<std::string> add;
		//if (add_match.empty() || add_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string valueLine = add_match[1].str();
		//	std::vector<std::string> arr;

		//	// exclude the addToLL= part from the first string
		//	size_t startPos = valueLine.find("=") + 1;
		//	size_t pos = 0;
		//	std::string token;
		//	while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
		//		token = valueLine.substr(startPos, pos - startPos);
		//		token = trim(token);  // remove leading and trailing white spaces
		//		arr.push_back(token);
		//		startPos = pos + 1;
		//	}
		//	token = valueLine.substr(startPos);
		//	token = trim(token);  // remove leading and trailing white spaces
		//	arr.push_back(token);

		//	std::vector<std::vector<std::string>> arr2D(arr.size());

		//	for (int i = 0; i < arr.size(); i++) {
		//		std::vector<std::string> splitArr;
		//		size_t innerPos = 0;
		//		std::string innerToken;
		//		while ((innerPos = arr[i].find("~")) != std::string::npos) {
		//			innerToken = arr[i].substr(0, innerPos);
		//			innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//			splitArr.push_back(innerToken);
		//			arr[i].erase(0, innerPos + 1);
		//		}
		//		innerToken = arr[i];
		//		innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//		splitArr.push_back(innerToken);
		//		arr2D[i] = splitArr;
		//	}
		//	l.addedObjects = arr2D;
		//}

		extractToArr2D(line, "mgefsToAdd\\s*=([^:]+)", l.addedObjects);

		//std::regex change_regex("mgefsToChange\\s*=([^:]+)", regex::icase);
		//std::smatch change_match;
		//std::regex_search(line, change_match, change_regex);
		//std::vector<std::string> change;
		//if (change_match.empty() || change_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string valueLine = change_match[1].str();
		//	std::vector<std::string> arr;

		//	// exclude the changeToLL= part from the first string
		//	size_t startPos = valueLine.find("=") + 1;
		//	size_t pos = 0;
		//	std::string token;
		//	while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
		//		token = valueLine.substr(startPos, pos - startPos);
		//		token = trim(token);  // remove leading and trailing white spaces
		//		arr.push_back(token);
		//		startPos = pos + 1;
		//	}
		//	token = valueLine.substr(startPos);
		//	token = trim(token);  // remove leading and trailing white spaces
		//	arr.push_back(token);

		//	std::vector<std::vector<std::string>> arr2D(arr.size());

		//	for (int i = 0; i < arr.size(); i++) {
		//		std::vector<std::string> splitArr;
		//		size_t innerPos = 0;
		//		std::string innerToken;
		//		while ((innerPos = arr[i].find("~")) != std::string::npos) {
		//			innerToken = arr[i].substr(0, innerPos);
		//			innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//			splitArr.push_back(innerToken);
		//			arr[i].erase(0, innerPos + 1);
		//		}
		//		innerToken = arr[i];
		//		innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//		splitArr.push_back(innerToken);
		//		arr2D[i] = splitArr;
		//	}
		//	l.changedObjects = arr2D;
		//}

		extractToArr2D(line, "mgefsToChange\\s*=([^:]+)", l.changedObjects);
		extractToArr2D(line, "mgefsToChangeAdd\\s*=([^:]+)", l.changedAddObjects);
		extractToArr2D(line, "mgefsToChangeFirst\\s*=([^:]+)", l.changedFirstObjects);

		//	// extract removeMgefs
		//std::regex removeMgefs_regex("mgefsToRemove\\s*=([^:]+)", regex::icase);
		//std::smatch removeMgefs_match;
		//std::regex_search(line, removeMgefs_match, removeMgefs_regex);
		//std::vector<std::string> removeMgefs;
		//if (removeMgefs_match.empty() || removeMgefs_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string removeMgefs_str = removeMgefs_match[1];
		//	std::regex removeMgefs_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator removeMgefs_iterator(removeMgefs_str.begin(), removeMgefs_str.end(), removeMgefs_list_regex);
		//	std::sregex_iterator removeMgefs_end;
		//	while (removeMgefs_iterator != removeMgefs_end) {
		//		std::string keyword = (*removeMgefs_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			removeMgefs.push_back(keyword);
		//		}
		//		++removeMgefs_iterator;
		//	}
		//	l.removedObjects = removeMgefs;
		//}

		extractData(line, "mgefsToRemove\\s*=([^:]+)", l.removedObjects);

		//// extract type
		//std::regex type_regex("filterByType\\s*=([^:]+)", regex::icase);
		//std::smatch typematch;
		//std::regex_search(line, typematch, type_regex);
		//// extract the value after the equals sign
		//if (typematch.empty() || typematch[1].str().empty()) {
		//	l.filterType = "none";
		//} else {
		//	std::string value = typematch[1].str();
		//	value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		//	l.filterType = value;
		//}

		extractValueString(line, "filterByType\\s*=([^:]+)", l.filterType);


		extractValueString(line, "baseCost\\s*=([^:]+)", l.baseCost);
		extractValueString(line, "enchantmentAmount\\s*=([^:]+)", l.chargeOverride);
		extractValueString(line, "chargeTime\\s*=([^:]+)", l.chargeTime);
		extractValueString(line, "castType\\s*=([^:]+)", l.castType);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractValueString(line, "clear\\s*=([^:]+)", l.clear);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		extractDataStrings(line, "filterByMinimumSkillLevel\\s*=([^:]+)", l.filterByMinimumSkillLevel);
		extractData(line, "restrictToCastingType\\s*=([^:]+)", l.restrictToCastingType);
		extractData(line, "restrictToSchool\\s*=([^:]+)", l.restrictToSchool);
		extractData(line, "restrictToSpellType\\s*=([^:]+)", l.restrictToSpellType);
		
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		//RE::BSTArray<RE::AlchemyItem*> objectArray = dataHandler->GetFormArray<RE::AlchemyItem>();
		RE::BSTArray<RE::EnchantmentItem*> objectSpellArray = dataHandler->GetFormArray<RE::EnchantmentItem>();
		for (const auto& line : tokens) {
			
				if (!line.objects.empty()) {
					for (const auto& npcstring : line.objects) {
						RE::TESForm*       currentform = nullptr;
					RE::EnchantmentItem* curobj = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Enchantment) {
							//logger::debug("Form {:08X} ", currentform->formID);
							curobj = (RE::EnchantmentItem*)currentform;
							patch(line, curobj);
						}
					}
				}

				if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty()) {
					continue;
				}

			
			for (const auto& curobj : objectSpellArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;
				bool contains = false;
				bool containsOr = false;

				if ( !curobj || curobj->IsDeleted()) {
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

				if (!line.filterByEditorIdContains.empty()) {
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

				if (!line.filterByEditorIdContainsOr.empty()) {
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

				if ( (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.filterByMinimumSkillLevel.empty()) {
					for (const auto& skillLevel : line.filterByMinimumSkillLevel) {
						try {
							const int levelAsInt = std::stoi(skillLevel);

							if (!curobj->effects.empty() &&
								curobj->effects.front() &&
								curobj->effects.front()->baseEffect &&
								static_cast<int>(curobj->effects.front()->baseEffect->data.minimumSkill) == levelAsInt) {
								found = true;
								break;
							}

						} catch (...) {
							logger::warn("ERROR - scroll patcher - {:08X} - filterByMinimumSkillLevel", curobj->formID);
						}
					}
				}



				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm* currentform = nullptr;
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
						RE::TESForm* currentform = nullptr;
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

				if (!line.mgefs.empty()) {
					bool foundInList = false;
					for (const auto& mgefstring : line.mgefs) {
						foundInList = false;
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
								if (effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
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
				if (!line.mgefsOr.empty()) {
					for (const auto& mgefstring : line.mgefsOr) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;

							for (const auto& effect : curobj->effects) {
								if (effect->baseEffect->formID == keyword->formID) {
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

				if ((!line.mgefs.empty() || !line.mgefsOr.empty()) && mgefAnd && mgefOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by mgefs. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm* currentform = nullptr;
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

				if (!line.mgefsExcluded.empty() && found) {
					//logger::info("mgefs not empty");
					for (const auto& mgefstring : line.mgefsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
								if (effect->baseEffect->formID == keyword->formID) {
									found = false;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::EnchantmentItem* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Spell) {
							npc = (RE::EnchantmentItem*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::debug(FMT_STRING("enchantment {:08X} {} excluded."), curobj->formID, curobj->fullName);
								break;
							}
						}
					}
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

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					if (curobj && !curobj->effects.empty() && curobj->effects.front()->baseEffect) {
						const auto skill = curobj->effects.front()->baseEffect->data.associatedSkill;

						for (auto s : line.restrictToSchool) {
							s = toLowerCase(s);

							if (s == "destruction" && skill == RE::ActorValue::kDestruction) {
								foundSchool = true;
								break;
							}
							if (s == "restoration" && skill == RE::ActorValue::kRestoration) {
								foundSchool = true;
								break;
							}
							if (s == "alteration" && skill == RE::ActorValue::kAlteration) {
								foundSchool = true;
								break;
							}
							if (s == "conjuration" && skill == RE::ActorValue::kConjuration) {
								foundSchool = true;
								break;
							}
							if (s == "illusion" && skill == RE::ActorValue::kIllusion) {
								foundSchool = true;
								break;
							}
							if (s == "none" && skill == RE::ActorValue::kNone) {
								foundSchool = true;
								break;
							}
						}

						if (!foundSchool) {
							found = false;
						}
					}
				}

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					for (auto string : line.restrictToCastingType) {
						string = toLowerCase(string);
						if (string == "concentration" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
							foundCastingType = true;
						} else if (string == "constanteffect" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect) {
							foundCastingType = true;
						} else if (string == "fireandforget" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kFireAndForget) {
							foundCastingType = true;
						} else if (string == "scroll" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kScroll) {
							foundCastingType = true;
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToSpellType.empty()) {
					bool foundType = false;

					if (curobj) {
						const auto type = curobj->GetSpellType();

						for (auto t : line.restrictToSpellType) {
							t = toLowerCase(t);

							if (t == "spell" && type == RE::MagicSystem::SpellType::kSpell) {
								foundType = true;
								break;
							}
							if (t == "disease" && type == RE::MagicSystem::SpellType::kDisease) {
								foundType = true;
								break;
							}
							if (t == "power" && type == RE::MagicSystem::SpellType::kPower) {
								foundType = true;
								break;
							}
							if ((t == "lesserpower" || t == "lesser_power") &&
								type == RE::MagicSystem::SpellType::kLesserPower) {
								foundType = true;
								break;
							}
							if (t == "ability" && type == RE::MagicSystem::SpellType::kAbility) {
								foundType = true;
								break;
							}
							if (t == "poison" && type == RE::MagicSystem::SpellType::kPoison) {
								foundType = true;
								break;
							}
							if (t == "enchantment" && type == RE::MagicSystem::SpellType::kEnchantment) {
								foundType = true;
								break;
							}
							if ((t == "potion" || t == "alchemy") &&
								type == RE::MagicSystem::SpellType::kPotion) {
								foundType = true;
								break;
							}
							if ((t == "wordcraft" || t == "wortcraft") &&
								type == RE::MagicSystem::SpellType::kWortCraft) {
								foundType = true;
								break;
							}
							if (t == "ingredient" &&
								type == RE::MagicSystem::SpellType::kIngredient) {
								foundType = true;
								break;
							}
							if ((t == "leveledspell" || t == "leveled_spell") &&
								type == RE::MagicSystem::SpellType::kLeveledSpell) {
								foundType = true;
								break;
							}
							if (t == "addiction" &&
								type == RE::MagicSystem::SpellType::kAddiction) {
								foundType = true;
								break;
							}
							if ((t == "voicepower" || t == "voice_power" || t == "shout") &&
								type == RE::MagicSystem::SpellType::kVoicePower) {
								foundType = true;
								break;
							}
							if ((t == "staffenchantment" || t == "staff_enchantment" || t == "staff") &&
								type == RE::MagicSystem::SpellType::kStaffEnchantment) {
								foundType = true;
								break;
							}
							if (t == "scroll" &&
								type == RE::MagicSystem::SpellType::kScroll) {
								foundType = true;
								break;
							}
						}

						if (!foundType) {
							found = false;
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
		char skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						} else {
							std::string fileName = ent->d_name;
							size_t pos = fileName.find(extension);
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
								std::string line;
								std::ifstream infile;
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

	void patch(ENCH::line_content line, RE::EnchantmentItem* curobj) {
		

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("enchantment formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if ( !line.clear.empty() && line.clear != "none") {
			curobj->effects.clear();
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
		}

		if ( !line.removedObjects.empty()) {
			for (const auto& objectToRemove : line.removedObjects) {
				std::string        removeFormStr = objectToRemove;
				RE::EffectSetting* removeForm = (RE::EffectSetting*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->effects.begin();
					while (it != curobj->effects.end()) {
						if ((*it) && (*it)->baseEffect && (*it)->baseEffect->formID == removeForm->formID) {
							it = curobj->effects.erase(it);
							logger::debug(FMT_STRING("enchantment {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if ( !line.addedObjects.empty()) {
			for (const auto& objectToAdd : line.addedObjects) {
				std::string        addFormStr = objectToAdd[0];
				float              magnitude = std::stof(objectToAdd[1]);
				int32_t            duration = std::stoi(objectToAdd[2]);
				int32_t            area = std::stoi(objectToAdd[3]);
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
				if (addForm) {
					RE::Effect* tempEffectItem = new RE::Effect;
					tempEffectItem->baseEffect = addForm;
					tempEffectItem->effectItem.magnitude = magnitude;
					tempEffectItem->effectItem.duration = duration;
					tempEffectItem->effectItem.area = area;
					float durationForCalc = duration;
					if (durationForCalc == 0) {
						durationForCalc = 10;
					}
					tempEffectItem->cost = addForm->data.baseCost * pow(magnitude * durationForCalc*0.1, 1.1);
					
					curobj->effects.push_back(tempEffectItem);
					logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} magitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
					if (objectToAdd.size() == 5) {
						moveEffectToPosition0(curobj->effects, tempEffectItem);
						logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
					}
				}
			}
		}

		if ( !line.changedObjects.empty()) {
			for (const auto& objectToAdd : line.changedObjects) {
				std::string        addFormStr = objectToAdd[0];
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
				if (addForm) {  // Only proceed if addForm is not null
					float magnitude = 0.0f;
					if (objectToAdd[1] != "null") {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (objectToAdd[2] != "null") {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (objectToAdd[3] != "null") {
						area = std::stoi(objectToAdd[3]);
					}

					float multiplier = 0;
					if (objectToAdd[4] != "null") {
						multiplier = multiplier + std::stof(objectToAdd[4]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (objectToAdd[1] != "null") {
								effect->effectItem.magnitude = magnitude;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (objectToAdd[4] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
			}
		}

		if (!line.changedAddObjects.empty()) {
			for (const auto& objectToAdd : line.changedAddObjects) {
				std::string        addFormStr = objectToAdd[0];
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
				if (addForm) {  // Only proceed if addForm is not null
					float magnitude = 0.0f;
					if (objectToAdd[1] != "null") {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (objectToAdd[2] != "null") {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (objectToAdd[3] != "null") {
						area = std::stoi(objectToAdd[3]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (objectToAdd[1] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude + magnitude;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
			}
		}

		if (!line.changedFirstObjects.empty()) {
			for (const auto& objectToAdd : line.changedFirstObjects) {
				float magnitude = 0.0f;
				if (objectToAdd[0] != "null") {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (objectToAdd[1] != "null") {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (objectToAdd[2] != "null") {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (objectToAdd[3] != "null") {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				auto& effect = curobj->effects.front();
				if (effect && effect->baseEffect) {
					if (objectToAdd[0] != "null") {
						effect->effectItem.magnitude = magnitude;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
					if (objectToAdd[1] != "null") {
						effect->effectItem.duration = duration;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
					}
					if (objectToAdd[2] != "null") {
						effect->effectItem.area = area;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
					}
					if (objectToAdd[3] != "null") {
						effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
				}
			}
		}

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			try {
				curobj->data.costOverride = std::stoi(line.baseCost);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed baseCost to {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.chargeOverride.empty() && line.chargeOverride != "none") {
			try {
				curobj->data.chargeOverride = std::stoi(line.chargeOverride);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed enchantmentAmount to {}"), curobj->formID, curobj->fullName, curobj->data.chargeOverride);
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		//logger::debug("Spell Found True {}",line.chargeTime);
		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			curobj->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
		}

		if ( !line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				//logger::debug(FMT_STRING("spell formid: {:08X} {} flag {}"), curobj->formID, curobj->fullName, setFlag);
				if (toLowerCase(setFlag) == "costoverride") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "fooditem") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "extendduration") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled extendduration"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "costoverride") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fooditem") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "extendduration") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled extendduration"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

	}


}
