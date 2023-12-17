#include "armor.h"
#include "utility.h"

namespace ARMOR
{

struct patch_instruction_armor create_patch_instruction_armor(const std::string& line)
{
	patch_instruction_armor l;

	// extract objects
	std::regex objects_regex("filterByArmors\\s*=([^:]+)", regex::icase);
	std::smatch objects_match;
	std::regex_search(line, objects_match, objects_regex);
	std::vector<std::string> objects;
	if (objects_match.empty() || objects_match[1].str().empty()) {
		//empty
	} else {
		std::string objects_str = objects_match[1];
		std::regex objects_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
		std::sregex_iterator objects_end;
		while (objects_iterator != objects_end) {
			std::string tempVar = (*objects_iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
			//logger::info(FMT_STRING("Race: {}"), race);
			if (tempVar != "none") {
				objects.push_back(tempVar);
			}
			++objects_iterator;
		}
		l.object = objects;
	}

	// extract objectsExcluded
	std::regex objectsExcluded_regex("filterByArmorsExcluded\\s*=([^:]+)", regex::icase);
	std::smatch objectsExcluded_match;
	std::regex_search(line, objectsExcluded_match, objectsExcluded_regex);
	std::vector<std::string> objectsExcluded;
	if (objectsExcluded_match.empty() || objectsExcluded_match[1].str().empty()) {
		//empty
	} else {
		std::string objectsExcluded_str = objectsExcluded_match[1];
		std::regex objectsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator objectsExcluded_iterator(objectsExcluded_str.begin(), objectsExcluded_str.end(), objectsExcluded_list_regex);
		std::sregex_iterator objectsExcluded_end;
		while (objectsExcluded_iterator != objectsExcluded_end) {
			std::string tempVar = (*objectsExcluded_iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
			if (tempVar != "none") {
				objectsExcluded.push_back(tempVar);
			}
			++objectsExcluded_iterator;
		}
		l.objectExcluded = objectsExcluded;
	}

	// extract keywords
	std::regex keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
	std::smatch keywords_match;
	std::regex_search(line, keywords_match, keywords_regex);
	std::vector<std::string> keywords;
	if (keywords_match.empty() || keywords_match[1].str().empty()) {
		//empty
	} else {
		std::string keywords_str = keywords_match[1];
		std::regex keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
		std::sregex_iterator keywords_end;
		while (keywords_iterator != keywords_end) {
			std::string keyword = (*keywords_iterator)[0].str();
			keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
			keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
			if (keyword != "none") {
				keywords.push_back(keyword);
			}
			++keywords_iterator;
		}
		l.keywords = keywords;
	}

	// extract keywords
	std::regex keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
	std::smatch keywordsOr_match;
	std::regex_search(line, keywordsOr_match, keywordsOr_regex);
	std::vector<std::string> keywordsOr;
	if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
		//empty
	} else {
		std::string keywordsOr_str = keywordsOr_match[1];
		std::regex keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
		std::sregex_iterator keywordsOr_end;
		while (keywordsOr_iterator != keywordsOr_end) {
			std::string keyword = (*keywordsOr_iterator)[0].str();
			keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
			keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
			if (keyword != "none") {
				keywordsOr.push_back(keyword);
			}
			++keywordsOr_iterator;
		}
		l.keywordsOr = keywordsOr;
	}

	// extract keywords
	std::regex keywordsExcluded_regex("filterByKeywordsExcluded\\s*=([^:]+)", regex::icase);
	std::smatch keywordsExcluded_match;
	std::regex_search(line, keywordsExcluded_match, keywordsExcluded_regex);
	std::vector<std::string> keywordsExcluded;
	if (keywordsExcluded_match.empty() || keywordsExcluded_match[1].str().empty()) {
		//empty
	} else {
		std::string keywordsExcluded_str = keywordsExcluded_match[1];
		std::regex keywordsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator keywordsExcluded_iterator(keywordsExcluded_str.begin(), keywordsExcluded_str.end(), keywordsExcluded_list_regex);
		std::sregex_iterator keywordsExcluded_end;
		while (keywordsExcluded_iterator != keywordsExcluded_end) {
			std::string keyword = (*keywordsExcluded_iterator)[0].str();
			keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
			keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
			if (keyword != "none") {
				keywordsExcluded.push_back(keyword);
			}
			++keywordsExcluded_iterator;
		}
		l.keywordsExcluded = keywordsExcluded;
	}

	// damageResist
	std::regex damageResist_regex("damageResist\\s*=([^:]+)", regex::icase);
	std::smatch damageResistmatch;
	std::regex_search(line, damageResistmatch, damageResist_regex);
	// extract the value after the equals sign
	if (damageResistmatch.empty() || damageResistmatch[1].str().empty()) {
		l.damageResist = "none";
	} else {
		std::string damageResistvalue = damageResistmatch[1].str();
		damageResistvalue.erase(std::remove_if(damageResistvalue.begin(), damageResistvalue.end(), ::isspace), damageResistvalue.end());
		l.damageResist = damageResistvalue;
	}

		// multiplyDamageResist
	std::regex  multiplyDamageResist_regex("damageResistMultiply\\s*=([^:]+)", regex::icase);
	std::smatch multiplyDamageResistmatch;
	std::regex_search(line, multiplyDamageResistmatch, multiplyDamageResist_regex);
	// extract the value after the equals sign
	if (multiplyDamageResistmatch.empty() || multiplyDamageResistmatch[1].str().empty()) {
		l.damageResistMult = "none";
	} else {
		std::string multiplyDamageResistvalue = multiplyDamageResistmatch[1].str();
		multiplyDamageResistvalue.erase(std::remove_if(multiplyDamageResistvalue.begin(), multiplyDamageResistvalue.end(), ::isspace), multiplyDamageResistvalue.end());
		l.damageResistMult = multiplyDamageResistvalue;
	}

	// health
	std::regex health_regex("health\\s*=([^:]+)", regex::icase);
	std::smatch healthmatch;
	std::regex_search(line, healthmatch, health_regex);
	// extract the value after the equals sign
	if (healthmatch.empty() || healthmatch[1].str().empty()) {
		l.health = "none";
	} else {
		std::string healthvalue = healthmatch[1].str();
		healthvalue.erase(std::remove_if(healthvalue.begin(), healthvalue.end(), ::isspace), healthvalue.end());
		l.health = healthvalue;
	}

	// weight
	std::regex weight_regex("weight\\s*=([^:]+)", regex::icase);
	std::smatch weightmatch;
	std::regex_search(line, weightmatch, weight_regex);
	// extract the value after the equals sign
	if (weightmatch.empty() || weightmatch[1].str().empty()) {
		l.weight = "none";
	} else {
		std::string weightvalue = weightmatch[1].str();
		weightvalue.erase(std::remove_if(weightvalue.begin(), weightvalue.end(), ::isspace), weightvalue.end());
		l.weight = weightvalue;
	}

			// extract objectEffect
	std::regex objectEffect_regex("objectEffect\\s*=([^:]+)", regex::icase);
	std::smatch objectEffectmatch;
	std::regex_search(line, objectEffectmatch, objectEffect_regex);
	// extract the value after the equals sign
	if (objectEffectmatch.empty() || objectEffectmatch[1].str().empty()) {
		l.objectEffect = "none";
	} else {
		std::string keyword = objectEffectmatch[1].str();
		keyword.erase(std::remove_if(keyword.begin(), keyword.end(), ::isspace), keyword.end());
		l.objectEffect = keyword;
	}

	// extract keywordsToAdd
	std::regex keywordsToAdd_regex("keywordsToAdd\\s*=([^:]+)", regex::icase);
	std::smatch keywordsToAdd_match;
	std::regex_search(line, keywordsToAdd_match, keywordsToAdd_regex);
	std::vector<std::string> keywordsToAdd;
	if (keywordsToAdd_match.empty() || keywordsToAdd_match[1].str().empty()) {
		//empty
	} else {
		std::string keywordsToAdd_str = keywordsToAdd_match[1];
		std::regex keywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator keywordsToAdd_iterator(keywordsToAdd_str.begin(), keywordsToAdd_str.end(), keywordsToAdd_list_regex);
		std::sregex_iterator keywordsToAdd_end;
		while (keywordsToAdd_iterator != keywordsToAdd_end) {
			std::string keywordToAdd = (*keywordsToAdd_iterator)[0].str();
			keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
			keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
			if (keywordToAdd != "none") {
				//logger::info(FMT_STRING("keywordsToAdd: {}"), keywordToAdd);
				keywordsToAdd.push_back(keywordToAdd);
			}
			++keywordsToAdd_iterator;
		}
		l.keywordsToAdd = keywordsToAdd;
	}

	// extract keywordsToRemove
	std::regex keywordsToRemove_regex("keywordsToRemove\\s*=([^:]+)", regex::icase);
	std::smatch keywordsToRemove_match;
	std::regex_search(line, keywordsToRemove_match, keywordsToRemove_regex);
	std::vector<std::string> keywordsToRemove;
	if (keywordsToRemove_match.empty() || keywordsToRemove_match[1].str().empty()) {
		//empty
	} else {
		std::string keywordsToRemove_str = keywordsToRemove_match[1];
		std::regex keywordsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator keywordsToRemove_iterator(keywordsToRemove_str.begin(), keywordsToRemove_str.end(), keywordsToRemove_list_regex);
		std::sregex_iterator keywordsToRemove_end;
		while (keywordsToRemove_iterator != keywordsToRemove_end) {
			std::string keywordToRemove = (*keywordsToRemove_iterator)[0].str();
			keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
			keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
			if (keywordToRemove != "none") {
				//logger::info(FMT_STRING("keywordsToRemove: {}"), keywordToRemove);
				keywordsToRemove.push_back(keywordToRemove);
			}
			++keywordsToRemove_iterator;
		}
		l.keywordsToRemove = keywordsToRemove;
	}

	// extract attachParentSlotKeywordsToAdd
	std::regex attachParentSlotKeywordsToAdd_regex("attachParentSlotKeywordsToAdd\\s*=([^:]+)", regex::icase);
	std::smatch attachParentSlotKeywordsToAdd_match;
	std::regex_search(line, attachParentSlotKeywordsToAdd_match, attachParentSlotKeywordsToAdd_regex);
	std::vector<std::string> attachParentSlotKeywordsToAdd;
	if (attachParentSlotKeywordsToAdd_match.empty() || attachParentSlotKeywordsToAdd_match[1].str().empty()) {
		// ammos_match[1] is null
	} else {
		std::string attachParentSlotKeywordsToAdd_str = attachParentSlotKeywordsToAdd_match[1];
		std::regex attachParentSlotKeywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator attachParentSlotKeywordsToAdd_iterator(attachParentSlotKeywordsToAdd_str.begin(), attachParentSlotKeywordsToAdd_str.end(), attachParentSlotKeywordsToAdd_list_regex);
		std::sregex_iterator attachParentSlotKeywordsToAdd_end;
		while (attachParentSlotKeywordsToAdd_iterator != attachParentSlotKeywordsToAdd_end) {
			std::string keywordToAdd = (*attachParentSlotKeywordsToAdd_iterator)[0].str();
			keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
			keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
			if (keywordToAdd != "none") {
				//logger::info(FMT_STRING("attachParentSlotKeywordsToAdd: {}"), keywordToAdd);
				attachParentSlotKeywordsToAdd.push_back(keywordToAdd);
			}
			++attachParentSlotKeywordsToAdd_iterator;
		}
		l.attachParentSlotKeywordsToAdd = attachParentSlotKeywordsToAdd;
	}

		// extract attachParentSlotKeywordsToRemove
	std::regex attachParentSlotKeywordsToRemove_regex("attachParentSlotKeywordsToRemove\\s*=([^:]+)", regex::icase);
	std::smatch attachParentSlotKeywordsToRemove_match;
	std::regex_search(line, attachParentSlotKeywordsToRemove_match, attachParentSlotKeywordsToRemove_regex);
	std::vector<std::string> attachParentSlotKeywordsToRemove;
	if (attachParentSlotKeywordsToRemove_match.empty() || attachParentSlotKeywordsToRemove_match[1].str().empty()) {
		// ammos_match[1] is null
	} else {
		std::string attachParentSlotKeywordsToRemove_str = attachParentSlotKeywordsToRemove_match[1];
		std::regex attachParentSlotKeywordsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		std::sregex_iterator attachParentSlotKeywordsToRemove_iterator(attachParentSlotKeywordsToRemove_str.begin(), attachParentSlotKeywordsToRemove_str.end(), attachParentSlotKeywordsToRemove_list_regex);
		std::sregex_iterator attachParentSlotKeywordsToRemove_end;
		while (attachParentSlotKeywordsToRemove_iterator != attachParentSlotKeywordsToRemove_end) {
			std::string keywordToRemove = (*attachParentSlotKeywordsToRemove_iterator)[0].str();
			keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
			keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
			if (keywordToRemove != "none") {
				//logger::info(FMT_STRING("attachParentSlotKeywordsToRemove: {}"), keywordToRemove);
				attachParentSlotKeywordsToRemove.push_back(keywordToRemove);
			}
			++attachParentSlotKeywordsToRemove_iterator;
		}
		l.attachParentSlotKeywordsToRemove = attachParentSlotKeywordsToRemove;
	}

	// extract filterBipedSlot
	std::regex filterBipedSlot_regex("filterByBipedSlots\\s*=([^:]+)", regex::icase);
	std::smatch filterBipedSlot_match;
	std::regex_search(line, filterBipedSlot_match, filterBipedSlot_regex);
	// extract the value after the equals sign
	if (filterBipedSlot_match.empty() || filterBipedSlot_match[1].str().empty()) {
		//empty
	} else {
		std::string value = filterBipedSlot_match[1].str();
		value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		std::stringstream ss(value);
		std::string item;
		while (std::getline(ss, item, ',')) {
			l.bipedSlot.push_back(item);
		}
	}

	// extract filterBipedSlotOr
	std::regex filterBipedSlotOr_regex("filterByBipedSlotsOr\\s*=([^:]+)", regex::icase);
	std::smatch filterBipedSlotOr_match;
	std::regex_search(line, filterBipedSlotOr_match, filterBipedSlotOr_regex);
	// extract the value after the equals sign
	if (filterBipedSlotOr_match.empty() || filterBipedSlotOr_match[1].str().empty()) {
		//empty
	} else {
		std::string value = filterBipedSlotOr_match[1].str();
		value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		std::stringstream ss(value);
		std::string item;
		while (std::getline(ss, item, ',')) {
			l.bipedSlotOr.push_back(item);
		}
	}

	// extract filterBipedSlotExcluded
	std::regex filterBipedSlotExcluded_regex("filterByBipedSlotsExcluded\\s*=([^:]+)", regex::icase);
	std::smatch filterBipedSlotExcluded_match;
	std::regex_search(line, filterBipedSlotExcluded_match, filterBipedSlotExcluded_regex);
	// extract the value after the equals sign
	if (filterBipedSlotExcluded_match.empty() || filterBipedSlotExcluded_match[1].str().empty()) {
		//empty
	} else {
		std::string value = filterBipedSlotExcluded_match[1].str();
		value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		std::stringstream ss(value);
		std::string item;
		while (std::getline(ss, item, ',')) {
			l.bipedSlot.push_back(item);
		}
	}

	// extract setBipedSlot
	std::regex setBipedSlot_regex("bipedSlotsToAdd\\s*=([^:]+)", regex::icase);
	std::smatch setBipedSlot_match;
	std::regex_search(line, setBipedSlot_match, setBipedSlot_regex);
	// extract the value after the equals sign
	if (setBipedSlot_match.empty() || setBipedSlot_match[1].str().empty()) {
	} else {
		std::string value = setBipedSlot_match[1].str();
		value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		std::stringstream ss(value);
		std::string item;
		while (std::getline(ss, item, ',')) {
			l.setBipedSlot.push_back(item);
		}
	}

	// extract removeBipedSlot
	std::regex removeBipedSlot_regex("bipedSlotsToRemove\\s*=([^:]+)", regex::icase);
	std::smatch removeBipedSlot_match;
	std::regex_search(line, removeBipedSlot_match, removeBipedSlot_regex);
	// extract the value after the equals sign
	if (removeBipedSlot_match.empty() || removeBipedSlot_match[1].str().empty()) {
		//empty
	} else {
		std::string value = removeBipedSlot_match[1].str();
		value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		std::stringstream ss(value);
		std::string item;
		while (std::getline(ss, item, ',')) {
			l.removeBipedSlot.push_back(item);
		}
	}

	// extract DamageTypes
	std::regex DamageTypes_regex("changeDamageTypes\\s*=([^:]+)", regex::icase);
	std::smatch DamageTypes_match;
	std::regex_search(line, DamageTypes_match, DamageTypes_regex);
	std::vector<std::string> DamageTypes_before_eq;
	std::vector<float> DamageTypes_min_values;
	std::vector<float> DamageTypes_max_values;
	if (DamageTypes_match.empty() || DamageTypes_match[1].str().empty()) {
		//empty
	} else {
		std::string DamageTypes_str = DamageTypes_match[1];
		std::regex DamageTypes_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
		std::sregex_iterator DamageTypes_iterator(DamageTypes_str.begin(), DamageTypes_str.end(), DamageTypes_list_regex);
		std::sregex_iterator DamageTypes_end;
		while (DamageTypes_iterator != DamageTypes_end) {
			std::string avif = (*DamageTypes_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			DamageTypes_before_eq.push_back(avif);
			DamageTypes_min_values.push_back(std::stof((*DamageTypes_iterator)[2]));
			if ((*DamageTypes_iterator)[3] != "") {
				DamageTypes_max_values.push_back(std::stof((*DamageTypes_iterator)[3]));
			} else {
				DamageTypes_max_values.push_back(std::stof((*DamageTypes_iterator)[2]));
			}
			std::string val1 = ((*DamageTypes_iterator)[2]);
			std::string val2 = ((*DamageTypes_iterator)[3] != "") ? ((*DamageTypes_iterator)[3]) : ((*DamageTypes_iterator)[2]);
			//logger::info(FMT_STRING("avif: {}"), avif);
			//logger::info(FMT_STRING("value1: {}"), val1);
			//logger::info(FMT_STRING("value2: {}"), val2);
			++DamageTypes_iterator;
		}
		l.damageTypes = DamageTypes_before_eq;
		l.values1 = DamageTypes_min_values;
		l.values2 = DamageTypes_max_values;
	}

	// extract fullName
	std::regex fullName_regex("fullName\\s*=\\s*~([^~]+?)\\s*~");
	std::smatch namematch;
	std::regex_search(line, namematch, fullName_regex);
	// extract the value after the equals sign
	if (namematch.empty() || namematch[1].str().empty()) {
		l.fullName = "none";
	} else {
		std::string namevalue = namematch[1].str();
		namevalue.erase(namevalue.begin(), std::find_if_not(namevalue.begin(), namevalue.end(), ::isspace));
		namevalue.erase(std::find_if_not(namevalue.rbegin(), namevalue.rend(), ::isspace).base(), namevalue.end());
		l.fullName = namevalue;
	}

	return l;
}



void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens)
{
	logger::debug("processing patch instructions");
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::TESObjectARMO*> ArmorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
	for (const auto& line : tokens) {
		for (const auto& curobj : ArmorArray) {
			bool found = false;
			bool keywordAnd = false;
			bool keywordOr = false;
			bool bipedSlotAnd = false;
			bool bipedSlotOr = false;
			if (!line.object.empty()) {
				//logger::info("npc not empty");
				for (const auto& ammostring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectARMO* ammo = nullptr;

					std::string string_form = ammostring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Armor) {
						ammo = (RE::TESObjectARMO*)currentform;

						if (curobj->formID == ammo->formID) {
							found = true;
							//logger::info("NPC found.");
							break;
						}
					}
				}
			}

			if (!line.bipedSlot.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlot) {
					int iSlot = std::stoi(bipedSlot);
					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & slot) {
						bipedSlotAnd = true;
					} else {
						bipedSlotAnd = false;
						break;
					}
				}

			} else {
				bipedSlotAnd = true;
			}

			if (!line.bipedSlotOr.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlotOr) {
					int iSlot = std::stoi(bipedSlot);

					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & slot) {
						bipedSlotOr = true;
						break;
					}
				}
				if (found) {
					logger::debug(FMT_STRING("Armor has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
				}
			} else {
				bipedSlotOr = true;
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
							//logger::debug(FMT_STRING("KeywordAnd armor does not have all keywords"));
							break;
						}
						//logger::debug(FMT_STRING("KeywordAnd armor true"));
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
							//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X}"), curobj->formID);
							//logger::info("Keyword found.");
							break;
						}
					}
				}
			} else {
				//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
				keywordOr = true;
			}

			if ((!line.bipedSlot.empty() || !line.bipedSlotOr.empty()) && bipedSlotAnd && bipedSlotOr) {
				//logger::debug(FMT_STRING("Found a matching armor by bipedSlots. {:08X} {}"), curobj->formID, curobj->fullName);
				found = true;
			}

			if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
				//logger::debug(FMT_STRING("Found a matching armor by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
				found = true;
			}

			if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.bipedSlot.empty()) {
				found = true;
			}

			if (!line.keywordsExcluded.empty()) {
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
							//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
							//logger::info("Keyword found.");
							break;
						}
					}
				}
			}

			if (!line.bipedSlotExcluded.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlot) {
					int iSlot = std::stoi(bipedSlot);
					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & slot) {
						found = false;
						logger::debug(FMT_STRING("Armor Excluded has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
						break;
					}
					//logger::debug(FMT_STRING("KeywordAnd armor true"));
				}
			}

			if (!line.objectExcluded.empty()) {
				//logger::info("npc not empty");
				for (const auto& npcstring : line.objectExcluded) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectARMO* npc = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Armor) {
						npc = (RE::TESObjectARMO*)currentform;

						if (curobj->formID == npc->formID) {
							found = false;
							//logger::info("NPC found.");
							break;
						}
					}
				}
			}

			if (found && !line.damageResist.empty() && line.damageResist != "none") {
				try {
					curobj->armorRating = stof(line.damageResist);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating);
				} catch (const std::invalid_argument& e) {
				}
			}

			if (found && !line.damageResistMult.empty() && line.damageResistMult != "none") {
				try {
					curobj->armorRating = (uint32_t)(curobj->armorRating * stof(line.damageResistMult));
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed(multiplied) armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating);
				} catch (const std::invalid_argument& e) {
				}
			}

			if (found && !line.objectEffect.empty() && line.objectEffect != "none") {
				RE::TESForm* currentform = nullptr;
				RE::EnchantmentItem* obj = nullptr;
				std::string string_form = line.objectEffect;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Enchantment) {
					obj = (RE::EnchantmentItem*)currentform;
					curobj->formEnchanting = obj;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed object effect to {:08X}"), curobj->formID, curobj->fullName, obj->formID);
				} else if (toLowerCase(line.objectEffect) == "null") {
					curobj->formEnchanting = nullptr;
					logger::debug(FMT_STRING("armor formid: {:08X} {} removed object effect"), curobj->formID, curobj->fullName);
				}
			}

			//if (found && !line.health.empty() && line.health != "none") {
			//	try {
			//		curobj->armorData.health = stof(line.health);
			//		logger::debug(FMT_STRING("armor formid: {:08X} {} changed health {}"), curobj->formID, curobj->fullName, curobj->armorData.health);
			//	} catch (const std::invalid_argument& e) {
			//	}
			//}

			if (found && !line.weight.empty() && line.weight != "none") {
				try {
					curobj->weight = stof(line.weight);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
				} catch (const std::invalid_argument& e) {
				}
			}

			if (found && !line.keywordsToAdd.empty()) {
				for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.keywordsToAdd[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Keyword) {
						curobj->AddKeyword((RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("armor formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}

			if (found && !line.keywordsToRemove.empty()) {
				for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.keywordsToRemove[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Keyword) {
						curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("armor formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}

			//if (found && !line.attachParentSlotKeywordsToAdd.empty()) {
			//	for (size_t i = 0; i < line.attachParentSlotKeywordsToAdd.size(); i++) {
			//		RE::TESForm* currentform = nullptr;
			//		std::string string_form = line.attachParentSlotKeywordsToAdd[i];
			//		currentform = GetFormFromIdentifier(string_form);
			//		if (currentform && currentform->formType == RE::FormType::Keyword) {
			//			if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
			//				curobj->attachParents.AddKeyword((RE::BGSKeyword*)currentform);
			//				logger::debug(FMT_STRING("armor formid: {:08X} added attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			//			}
			//		}
			//	}
			//}

			//if (found && !line.attachParentSlotKeywordsToRemove.empty()) {
			//	for (size_t i = 0; i < line.attachParentSlotKeywordsToRemove.size(); i++) {
			//		RE::TESForm* currentform = nullptr;
			//		std::string string_form = line.attachParentSlotKeywordsToRemove[i];
			//		currentform = GetFormFromIdentifier(string_form);
			//		if (currentform && currentform->formType == RE::FormType::Keyword) {
			//			if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
			//				curobj->attachParents.RemoveKeyword((RE::BGSKeyword*)currentform);

			//				logger::debug(FMT_STRING("armor formid: {:08X} removed attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			//			}
			//		}
			//	}
			//}

			//if (found && !line.setBipedSlot.empty()) {
			//	for (const auto& slotString : line.setBipedSlot) {
			//		int slot = std::stoi(slotString);
			//		curobj->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
			//		logger::debug(FMT_STRING("armor added bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
			//		for (const auto& arma : curobj->modelArray) {
			//			arma.armorAddon->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
			//			//logger::debug(FMT_STRING("Added bipedSlot to {:08X} Slot {}"), arma.armorAddon->formID, slotString);
			//		}
			//	}
			//}

			//if (found && !line.removeBipedSlot.empty()) {
			//	for (const auto& slotString : line.removeBipedSlot) {
			//		int slot = std::stoi(slotString);
			//		curobj->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
			//		logger::debug(FMT_STRING("armor removed bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
			//		for (const auto& arma : curobj->modelArray) {
			//			arma.armorAddon->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
			//			//logger::debug(FMT_STRING("Removed bipedSlot to {:08X} Slot {}"), arma.armorAddon->formID, slotString);
			//		}
			//	}
			//}

			//if (found && !line.damageTypes.empty()) {
			//	//logger::info("found! patching values");
			//	//for (const auto& avifstring : line.avifs)
			//	for (size_t i = 0; i < line.damageTypes.size(); i++) {
			//		RE::TESForm* currentform = nullptr;
			//		std::string string_form = line.damageTypes[i];
			//		currentform = GetFormFromIdentifier(string_form);
			//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
			//			//logger::info("avif valid!");
			//			float finalValue = 0;

			//			if (!line.values1.empty() && !line.values2.empty()) {
			//				finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
			//			}

			//			changeDamageType_Armor(curobj, (RE::BGSDamageType*)currentform, finalValue);
			//			logger::debug(FMT_STRING("armor formid: {:08X} {} changed damage type {:08X} to {}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID, finalValue);
			//		}
			//	}
			//}

			if (found && !line.fullName.empty() && line.fullName != "none") {
				try {
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
					curobj->fullName = line.fullName;
				} catch (const std::invalid_argument& e) {
				}
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
							std::list<patch_instruction_armor> tokens;
							infile.open(fullPath);
							while (std::getline(infile, line)) {
								if (line[0] == skipChar) {
									continue;
								}

								if (line.empty()) {
									continue;
								}

								tokens.push_back(create_patch_instruction_armor(line));
							}
							infile.close();
							process_patch_instructions_armor(tokens);
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
