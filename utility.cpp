#include "utility.h"
#include <cmath>
#include <iostream>
#include <regex>
#include <unordered_map>



RE::BIPED_MODEL::BipedObjectSlot getBipedObjectSlot(int slot)
{
	switch (slot) {
	case 0:
		return RE::BIPED_MODEL::BipedObjectSlot::kHead;
	case 1:
		return RE::BIPED_MODEL::BipedObjectSlot::kHair;
	case 2:
		return RE::BIPED_MODEL::BipedObjectSlot::kBody;
	case 3:
		return RE::BIPED_MODEL::BipedObjectSlot::kHands;
	case 4:
		return RE::BIPED_MODEL::BipedObjectSlot::kForearms;
	case 5:
		return RE::BIPED_MODEL::BipedObjectSlot::kAmulet;
	case 6:
		return RE::BIPED_MODEL::BipedObjectSlot::kRing;
	case 7:
		return RE::BIPED_MODEL::BipedObjectSlot::kFeet;
	case 8:
		return RE::BIPED_MODEL::BipedObjectSlot::kCalves;
	case 9:
		return RE::BIPED_MODEL::BipedObjectSlot::kShield;
	case 10:
		return RE::BIPED_MODEL::BipedObjectSlot::kTail;
	case 11:
		return RE::BIPED_MODEL::BipedObjectSlot::kLongHair;
	case 12:
		return RE::BIPED_MODEL::BipedObjectSlot::kCirclet;
	case 13:
		return RE::BIPED_MODEL::BipedObjectSlot::kEars;
	case 14:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMouth;
	case 15:
		return RE::BIPED_MODEL::BipedObjectSlot::kModNeck;
	case 16:
		return RE::BIPED_MODEL::BipedObjectSlot::kModChestPrimary;
	case 17:
		return RE::BIPED_MODEL::BipedObjectSlot::kModBack;
	case 18:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMisc1;
	case 19:
		return RE::BIPED_MODEL::BipedObjectSlot::kModPelvisPrimary;
	case 20:
		return RE::BIPED_MODEL::BipedObjectSlot::kDecapitateHead;
	case 21:
		return RE::BIPED_MODEL::BipedObjectSlot::kDecapitate;
	case 22:
		return RE::BIPED_MODEL::BipedObjectSlot::kModPelvisSecondary;
	case 23:
		return RE::BIPED_MODEL::BipedObjectSlot::kModLegRight;
	case 24:
		return RE::BIPED_MODEL::BipedObjectSlot::kModLegLeft;
	case 25:
		return RE::BIPED_MODEL::BipedObjectSlot::kModFaceJewelry;
	case 26:
		return RE::BIPED_MODEL::BipedObjectSlot::kModChestSecondary;
	case 27:
		return RE::BIPED_MODEL::BipedObjectSlot::kModShoulder;
	case 28:
		return RE::BIPED_MODEL::BipedObjectSlot::kModArmLeft;
	case 29:
		return RE::BIPED_MODEL::BipedObjectSlot::kModArmRight;
	case 30:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMisc2;
	case 31:
		return RE::BIPED_MODEL::BipedObjectSlot::kFX01;
	default:
		return RE::BIPED_MODEL::BipedObjectSlot::kNone;
	}
}

AttributePoints calculateAttributePoints(int level, const StatWeights& weights, int points)
{
	AttributePoints result;

	// Calculate the total number of points available
	int totalPoints = points * (level - 1);

	// Check and replace zero weights with 1
	float healthWeight = (weights.healthWeight != 0) ? weights.healthWeight : 1;
	float staminaWeight = (weights.staminaWeight != 0) ? weights.staminaWeight : 1;
	float magickaWeight = (weights.magickaWeight != 0) ? weights.magickaWeight : 1;

	// Calculate Magicka Points
	int magickaPoints = totalPoints * magickaWeight / (healthWeight + staminaWeight + magickaWeight);
	result.magicka = magickaPoints;

	// Calculate Stamina Points
	int staminaPoints = (totalPoints - magickaPoints) * staminaWeight / (healthWeight + staminaWeight);
	result.stamina = staminaPoints;

	// Calculate Health Points
	result.health = totalPoints - magickaPoints - staminaPoints;

	return result;
}


bool IsPluginInstalled(const char* name)
{
	auto  dataHandler = RE::TESDataHandler::GetSingleton();
	auto* modInfo = dataHandler->LookupLoadedModByName(name);
	if (modInfo)
		return true;

	modInfo = dataHandler->LookupLoadedLightModByName(name);
	if (modInfo)
		return true;

	return false;
}

template <typename T>
T GetOffset(const void* baseObject, int offset)
{
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = identifier.substr(0, delimiter);
		std::string modForm = identifier.substr(delimiter + 1);
		uint32_t    formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
		auto*       mod = dataHandler->LookupModByName(modName.c_str());
		if (mod->IsLight())
			formID = std::stoul(modForm, nullptr, 16) & 0xFFF;
		return dataHandler->LookupForm(formID, modName.c_str());
	}
	return nullptr;
}

std::string toLowerCase(std::string pString)
{
	std::string lowercasepString = pString;
	std::transform(lowercasepString.begin(), lowercasepString.end(), lowercasepString.begin(), [](unsigned char c) { return std::tolower(c); });

	return lowercasepString;
}

std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> splitRelationNumber(const std::string& input)
{
	std::regex               re(R"(([<>]=?|[<>])\s*(\d+))");
	std::smatch              match;
	std::vector<std::string> result;

	if (std::regex_search(input, match, re)) {
		std::string relation_sign = match[1];
		std::string number_str = match[2];
		result.push_back(number_str);
		result.push_back(relation_sign);
	} else if (std::regex_search(input, std::regex(R"(\d+)"))) {
		result.push_back(input);
	} else {
		result.push_back("none");
	}

	return result;
}


//extract functions
// Function to extract data from a line using a regex pattern
void extractData(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
{
	std::regex  regex_pattern(pattern, std::regex::icase);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);

	std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
	if (!match.empty() && !match[1].str().empty()) {
		std::string          data_str = match[1];
		std::regex           list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", std::regex::icase);
		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
		std::sregex_iterator end;

		while (iterator != end) {
			std::string tempVar = (*iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());

			if (tempVar != "none") {
				(*data).push_back(tempVar);
			}
			++iterator;
		}
	}

	destination = (*data);
}

void extractMultiDataString(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<std::string>& pcMultFlag_min_values, std::vector<std::string>& pcMultFlag_max_values)
{
	std::regex  pcMultFlag_regex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	std::regex_search(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
		std::regex  pcMultFlag_list_regex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back((*pcMultFlag_iterator)[2]);
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? (*pcMultFlag_iterator)[3] : (*pcMultFlag_iterator)[2]);

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiDataInt(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<int>& pcMultFlag_min_values, std::vector<int>& pcMultFlag_max_values)
{
	std::regex  pcMultFlag_regex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	std::regex_search(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
		std::regex  pcMultFlag_list_regex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stoi((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stoi((*pcMultFlag_iterator)[3]) : std::stoi((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiDataFloat(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<float>& pcMultFlag_min_values, std::vector<float>& pcMultFlag_max_values)
{
	std::regex  pcMultFlag_regex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	std::regex_search(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
		std::regex  pcMultFlag_list_regex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stof((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stof((*pcMultFlag_iterator)[3]) : std::stof((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}
