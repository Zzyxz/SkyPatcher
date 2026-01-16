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

	float healthWeight = weights.healthWeight;
	float staminaWeight = weights.staminaWeight;
	float magickaWeight = weights.magickaWeight;

	// Check if the sum of weights is zero
	float weightSum = healthWeight + staminaWeight + magickaWeight;
	if (weightSum == 0) {
		// Set default weights if the sum of weights is zero
		healthWeight = 1.0f;
		staminaWeight = 1.0f;
		magickaWeight = 1.0f;
		weightSum = 3.0f;  // Update the sum of weights
	}

	// Check if healthWeight is zero
	if (healthWeight == 0) {
		result.health = 0;
	} else {
		// Calculate Health Points
		result.health = totalPoints * healthWeight / weightSum;
	}

	// Check if magickaWeight is zero
	if (magickaWeight == 0) {
		result.magicka = 0;
	} else {
		// Calculate Magicka Points
		int magickaPoints = totalPoints * magickaWeight / weightSum;
		result.magicka = magickaPoints;
	}

	// Check if staminaWeight is zero
	if (staminaWeight == 0) {
		result.stamina = 0;
	} else {
		// Calculate Stamina Points
		int staminaPoints = (totalPoints - result.magicka) * staminaWeight / weightSum;
		result.stamina = staminaPoints;
	}

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

//RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
//{
//	auto dataHandler = RE::TESDataHandler::GetSingleton();
//	auto delimiter = identifier.find('|');
//	if (delimiter != std::string::npos) {
//		std::string modName = identifier.substr(0, delimiter);
//		std::string modForm = identifier.substr(delimiter + 1);
//		uint32_t    formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
//		auto*       mod = dataHandler->LookupModByName(modName.c_str());
//		if (mod && mod->IsLight())
//			formID = std::stoul(modForm, nullptr, 16) & 0xFFF;
//		return dataHandler->LookupForm(formID, modName.c_str());
//	}
//	return nullptr;
//}

RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		// Handle the case when '|' is present in the identifier
		std::string modName = identifier.substr(0, delimiter);
		std::string modForm = identifier.substr(delimiter + 1);
		uint32_t    formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
		auto*       mod = dataHandler->LookupModByName(modName.c_str());
		if (mod && mod->IsLight())
			formID = std::stoul(modForm, nullptr, 16) & 0xFFF;
		return dataHandler->LookupForm(formID, modName.c_str());
	} else {
		return RE::TESForm::LookupByEditorID(identifier);
	}
	return nullptr;
}

RE::TESForm* GetRefrFormFromIdentifier(const std::string& identifier)
{
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler)
		return nullptr;

	auto delimiter = identifier.find('|');
	if (delimiter == std::string::npos) {
		logger::warn("Invalid identifier format: {}", identifier);
		return nullptr;
	}

	std::string modName = identifier.substr(0, delimiter);
	std::string modForm = identifier.substr(delimiter + 1);
	std::uint32_t rawFormID = std::stoul(modForm, nullptr, 16);

	auto* mod = dataHandler->LookupModByName(modName.c_str());
	if (!mod) {
		logger::warn("Mod not found: {}", modName);
		return nullptr;
	}

	std::uint32_t fullFormID = 0;

	if (mod->IsLight()) {
		fullFormID = 0xFE000000 | (mod->GetCompileIndex() << 12) | (rawFormID & 0xFFF);
	}
	else {
		fullFormID = (mod->GetCompileIndex() << 24) | (rawFormID & 0xFFFFFF);
	}

	logger::debug("Looking up REFR FormID {:08X} from {}", fullFormID, modName);

	auto* form = RE::TESForm::LookupByID(fullFormID);
	if (!form) {
		logger::warn("No form found for FormID: {:08X}", fullFormID);
	}
	return form;
}

RE::FormID BuildFormIDFromIdentifier(const std::string& identifier)
{
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler)
		return 0x0;

	auto delimiter = identifier.find('|');
	if (delimiter == std::string::npos) {
		logger::warn("Invalid identifier format: {}", identifier);
		return 0x0;
	}

	std::string modName = identifier.substr(0, delimiter);
	std::string modForm = identifier.substr(delimiter + 1);
	std::uint32_t rawFormID = std::stoul(modForm, nullptr, 16);

	auto* mod = dataHandler->LookupModByName(modName.c_str());
	if (!mod) {
		logger::warn("Mod not found: {}", modName);
		return 0x0;
	}

	std::uint32_t fullFormID = 0;

	if (mod->IsLight()) {
		fullFormID = 0xFE000000 | (mod->GetCompileIndex() << 12) | (rawFormID & 0xFFF);
	}
	else {
		fullFormID = (mod->GetCompileIndex() << 24) | (rawFormID & 0xFFFFFF);
	}

	//logger::debug("Built FormID {:08X} from {}", fullFormID, identifier);
	return fullFormID;
}






std::string toLowerCase(std::string pString)
{
	std::string lowercasepString = pString;
	std::transform(lowercasepString.begin(), lowercasepString.end(), lowercasepString.begin(), [](unsigned char c) { return std::tolower(c); });

	return lowercasepString;
}

// Function to convert string to hexadecimal representation
std::string toHex(const std::string& str)
{
	std::stringstream hexStream;
	hexStream << std::hex << std::setfill('0');
	for (const auto& ch : str) {
		hexStream << std::setw(2) << static_cast<int>(static_cast<unsigned char>(ch));
	}
	return hexStream.str();
}

// Function to replace backslashes with forward slashes
std::string normalizePath(const std::string& path)
{
	std::string normalized = path;
	std::replace(normalized.begin(), normalized.end(), '\\', '/');
	return normalized;
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

	// Find all matches in the input line
	std::regex_search(line, match, regex_pattern);

	if (!match.empty() && !match[1].str().empty()) {
		std::string data_str = match[1];

		// Check if data_str contains "|"
		std::regex           list_regex("[^,]+", std::regex::icase);
		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
		std::sregex_iterator end;

		while (iterator != end) {
			std::string tempVar = (*iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());

			if (tempVar != "none") {
				destination.push_back(tempVar);
			}
			++iterator;
		}
	}
}

void extractDataStrings(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
{
	std::regex  regex_pattern(pattern, std::regex::icase);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);

	std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
	if (!match.empty() && !match[1].str().empty()) {
		std::string          data_str = match[1];
		std::regex           list_regex("[^,]+", std::regex::icase);
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

void extractValueString(const std::string& line, const std::string& regex_pattern_str, std::string& target_value)
{
	std::regex  regex_pattern(regex_pattern_str);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);

	if (match.empty() || match[1].str().empty()) {
		target_value = "none";
	} else {
		std::string value = match[1].str();

		// Remove leading whitespaces
		value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), ::isspace));

		// Remove trailing whitespaces
		value.erase(std::find_if_not(value.rbegin(), value.rend(), ::isspace).base(), value.end());
		target_value = value;
	}
}

void extractToArr2D(const std::string& line, const std::string& regex_pattern_str, std::vector<std::vector<std::string>>& target_value)
{
	std::regex  regex_pattern(regex_pattern_str, std::regex::icase);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);
	std::vector<std::string> arr;

	if (match.empty() || match[1].str().empty()) {
		// Empty
	} else {
		std::string valueLine = match[1].str();
		//logger::debug("valueLine {}", valueLine);
		// Exclude the part after "=" from the first string
		size_t      startPos = valueLine.find("=") + 1;
		size_t      pos = 0;
		std::string token;
		while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
			token = valueLine.substr(startPos, pos - startPos);
			token = trim(token);
			arr.push_back(token);
			startPos = pos + 1;
		}
		token = valueLine.substr(startPos);
		token = trim(token);
		arr.push_back(token);

		std::vector<std::vector<std::string>> arr2D(arr.size());

		for (int i = 0; i < arr.size(); i++) {
			std::vector<std::string> splitArr;
			size_t                   innerPos = 0;
			std::string              innerToken;
			while ((innerPos = arr[i].find("~")) != std::string::npos) {
				innerToken = arr[i].substr(0, innerPos);
				innerToken = trim(innerToken);
				splitArr.push_back(innerToken);
				arr[i].erase(0, innerPos + 1);
			}
			innerToken = arr[i];
			innerToken = trim(innerToken);
			splitArr.push_back(innerToken);
			arr2D[i] = splitArr;
		}
		target_value = arr2D;
	}
}

void moveEffectToPosition0(RE::BSTArray<RE::Effect*>& effects, const RE::Effect* targetEffect)
{
	auto it = std::find_if(effects.begin(), effects.end(), [targetEffect](const RE::Effect* x) {
		return x->baseEffect->formID == targetEffect->baseEffect->formID;
	});

	if (it != effects.end()) {
		std::rotate(effects.begin(), it, it + 1);
	}
}

void addAltTexture(RE::TESModelTextureSwap* curobj, RE::TESModelTextureSwap::AlternateTexture newTex)
{
	// Calculate the new size
	const auto newSize = curobj->numAlternateTextures + 1;

		//	const auto newData = calloc<ContainerObject*>(newSize);
	// Allocate memory for the new array
	RE::MemoryManager mm = *RE::MemoryManager::GetSingleton();
	auto              newTextures = RE::calloc<RE::TESModelTextureSwap::AlternateTexture>(newSize);
			//auto newTextures = calloc<RE::TESModelTextureSwap::AlternateTexture*>(newSize, sizeof(RE::TESModelTextureSwap::AlternateTexture));

	if (!newTextures) {
		// Handle allocation failure, e.g., by throwing an exception or returning an error code
		return;
	}

// Copy existing elements
	if (curobj->alternateTextures && curobj->numAlternateTextures > 0) {
		std::copy(curobj->alternateTextures, curobj->alternateTextures + curobj->numAlternateTextures, newTextures);
		RE::free(curobj->alternateTextures);
	}


	// Add the new element
	//newTextures[curobj->numAlternateTextures] = newTex;
	newTextures[newSize - 1] = newTex;

	// Sort the array based on index3D
	std::sort(newTextures, newTextures + newSize, [](const RE::TESModelTextureSwap::AlternateTexture& a, const RE::TESModelTextureSwap::AlternateTexture& b) {
		return a.index3D < b.index3D;
	});

	// Update the object with the new array and size
	curobj->alternateTextures = newTextures;
	curobj->numAlternateTextures = newSize;
}

void clearAltTextures(RE::TESModelTextureSwap* curobj)
{
	if (curobj->numAlternateTextures > 0) {
		// Free the memory allocated for the alternateTextures array
		//RE::free(curobj->alternateTextures);

		// Reset the pointers and size
		curobj->alternateTextures = nullptr;
		curobj->numAlternateTextures = 0;
	}
}

bool removeAltTextureByTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToRemove)
{
	if (!curobj->alternateTextures || curobj->numAlternateTextures == 0) {
		// No textures to remove
		return false;
	}

	// Find the iterator pointing to the element to remove
	auto iterToRemove = std::remove_if(curobj->alternateTextures,
		curobj->alternateTextures + curobj->numAlternateTextures,
		[textureSetToRemove](const RE::TESModelTextureSwap::AlternateTexture& altTex) {
			return altTex.textureSet == textureSetToRemove;
		});

	// Check if the element was found
	if (iterToRemove != curobj->alternateTextures + curobj->numAlternateTextures) {
		// Element found, update the size
		curobj->numAlternateTextures = static_cast<std::size_t>(std::distance(curobj->alternateTextures, iterToRemove));

		if (curobj->numAlternateTextures > 0) {
			// Allocate memory for the new array
			auto newTextures = reinterpret_cast<RE::TESModelTextureSwap::AlternateTexture*>(calloc(curobj->numAlternateTextures, sizeof(RE::TESModelTextureSwap::AlternateTexture)));

			if (!newTextures) {
				// Handle allocation failure, e.g., by throwing an exception or returning an error code
				return false;
			}

			// Copy existing elements excluding the removed one
			std::copy(curobj->alternateTextures, iterToRemove, newTextures);
			std::copy(iterToRemove + 1, curobj->alternateTextures + curobj->numAlternateTextures, newTextures);

			// Free the old array
			RE::free(curobj->alternateTextures);

			// Update the object with the new array
			curobj->alternateTextures = newTextures;
			return true;
		} else {
			// No remaining textures, free the old array
			std::free(curobj->alternateTextures);
			curobj->alternateTextures = nullptr;
			return true;
		}
	}
	return false;
}

bool findTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToFind)
{
	if (!curobj->alternateTextures || curobj->numAlternateTextures == 0) {
		// No textures to find
		return false;
	}

	// Use std::find_if to search for the texture set
	auto iter = std::find_if(curobj->alternateTextures,
		curobj->alternateTextures + curobj->numAlternateTextures,
		[textureSetToFind](const RE::TESModelTextureSwap::AlternateTexture& altTex) {
			return altTex.textureSet == textureSetToFind;
		});

	// Return true if the texture set was found, false otherwise
	return iter != curobj->alternateTextures + curobj->numAlternateTextures;
}

RE::TESClass* copyClass(RE::TESClass* aobj)
{
	auto       form = aobj;  
	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESClass>();
	auto       newClass = factory->Create();
	RE::FormID newForm = newClass->GetFormID();
	memcpy(newClass, form, sizeof(*form));
	newClass->formID = newForm;
	//logger::debug("newClass {:08X}",newClass->formID);
	return newClass;
}

RE::BGSKeyword* createKeyword()
{

	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
	auto       newKeyword = factory->Create();
	//RE::FormID newForm = newKeyword->GetFormID();
	//newKeyword->formID = newForm;
	newKeyword->formEditorID = "SkyPatcher_RandomVisualStyleAssigned";
	//logger::debug("newClass {:08X}",newClass->formID);
	return newKeyword;
}

RE::BGSListForm* createListForm()
{

	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>();
	auto       newKeyword = factory->Create();
	RE::FormID newForm = newKeyword->GetFormID();
	newKeyword->formID = newForm;
	//newKeyword->formEditorID = "SkyPatcher_RandomVisualStyleAssigned";
	//logger::debug("newClass {:08X}",newClass->formID);
	return newKeyword;
}

RE::TESObjectREFR* createREFR()
{
	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESObjectREFR>();
	auto       newRefr = factory->Create();
	RE::FormID newForm = newRefr->GetFormID();
	newRefr->formID = newForm;
	return newRefr;
}

 RE::ActorValue StringToActorValue(const std::string& valueString) {
	static const std::unordered_map<std::string, RE::ActorValue> actorValueMap = {
		{"null", RE::ActorValue::kNone},
		{"aggression", RE::ActorValue::kAggression},
		{"confidence", RE::ActorValue::kConfidence},
		{"energy", RE::ActorValue::kEnergy},
		{"morality", RE::ActorValue::kMorality},
		{"mood", RE::ActorValue::kMood},
		{"assistance", RE::ActorValue::kAssistance},
		{"onehanded", RE::ActorValue::kOneHanded},
		{"twohanded", RE::ActorValue::kTwoHanded},
		{"archery", RE::ActorValue::kArchery},
		{"block", RE::ActorValue::kBlock},
		{"smithing", RE::ActorValue::kSmithing},
		{"heavyarmor", RE::ActorValue::kHeavyArmor},
		{"lightarmor", RE::ActorValue::kLightArmor},
		{"pickpocket", RE::ActorValue::kPickpocket},
		{"lockpicking", RE::ActorValue::kLockpicking},
		{"sneak", RE::ActorValue::kSneak},
		{"alchemy", RE::ActorValue::kAlchemy},
		{"speech", RE::ActorValue::kSpeech},
		{"alteration", RE::ActorValue::kAlteration},
		{"conjuration", RE::ActorValue::kConjuration},
		{"destruction", RE::ActorValue::kDestruction},
		{"illusion", RE::ActorValue::kIllusion},
		{"restoration", RE::ActorValue::kRestoration},
		{"enchanting", RE::ActorValue::kEnchanting},
		{"health", RE::ActorValue::kHealth},
		{"magicka", RE::ActorValue::kMagicka},
		{"stamina", RE::ActorValue::kStamina},
		{"healrate", RE::ActorValue::kHealRate},
		{"magickarate", RE::ActorValue::kMagickaRate},
		{"staminarate", RE::ActorValue::kStaminaRate},
		{"speedmult", RE::ActorValue::kSpeedMult},
		{"inventoryweight", RE::ActorValue::kInventoryWeight},
		{"carryweight", RE::ActorValue::kCarryWeight},
		{"criticalchance", RE::ActorValue::kCriticalChance},
		{"meleedamage", RE::ActorValue::kMeleeDamage},
		{"unarmeddamage", RE::ActorValue::kUnarmedDamage},
		{"mass", RE::ActorValue::kMass},
		{"voicepoints", RE::ActorValue::kVoicePoints},
		{"voicerate", RE::ActorValue::kVoiceRate},
		{"damageresist", RE::ActorValue::kDamageResist},
		{"poisonresist", RE::ActorValue::kPoisonResist},
		{"resistfire", RE::ActorValue::kResistFire},
		{"resistshock", RE::ActorValue::kResistShock},
		{"resistfrost", RE::ActorValue::kResistFrost},
		{"resistmagic", RE::ActorValue::kResistMagic},
		{"resistdisease", RE::ActorValue::kResistDisease},
		{"perceptioncondition", RE::ActorValue::kPerceptionCondition},
		{"endurancecondition", RE::ActorValue::kEnduranceCondition},
		{"leftattackcondition", RE::ActorValue::kLeftAttackCondition},
		{"rightattackcondition", RE::ActorValue::kRightAttackCondition},
		{"leftmobilitycondition", RE::ActorValue::kLeftMobilityCondition},
		{"rightmobilitycondition", RE::ActorValue::kRightMobilityCondition},
		{"braincondition", RE::ActorValue::kBrainCondition},
		{"paralysis", RE::ActorValue::kParalysis},
		{"invisibility", RE::ActorValue::kInvisibility},
		{"nightvision", RE::ActorValue::kNightEye},
		{"wardpower", RE::ActorValue::kWardPower},
		{"rightitemcharge", RE::ActorValue::kRightItemCharge},
		{"armorperks", RE::ActorValue::kArmorPerks},
		{"shieldperks", RE::ActorValue::kShieldPerks},
		{"warddeflection", RE::ActorValue::kWardDeflection},
		{"variable01", RE::ActorValue::kVariable01},
		{"variable02", RE::ActorValue::kVariable02},
		{"variable03", RE::ActorValue::kVariable03},
		{"variable04", RE::ActorValue::kVariable04},
		{"variable05", RE::ActorValue::kVariable05},
		{"variable06", RE::ActorValue::kVariable06},
		{"variable07", RE::ActorValue::kVariable07},
		{"variable08", RE::ActorValue::kVariable08},
		{"variable09", RE::ActorValue::kVariable09},
		{"variable10", RE::ActorValue::kVariable10},
		{"bowstaggerbonus", RE::ActorValue::kBowStaggerBonus},
		{"telekinesis", RE::ActorValue::kTelekinesis},
		{"favoractive", RE::ActorValue::kFavorActive},
		{"favorsperday", RE::ActorValue::kFavorsPerDay},
		{"favorsperdaytimer", RE::ActorValue::kFavorsPerDayTimer},
		{"leftitemcharge", RE::ActorValue::kLeftItemCharge},
		{"absorbchance", RE::ActorValue::kAbsorbChance},
		{"blindness", RE::ActorValue::kBlindness},
		{"fame", RE::ActorValue::kFame},
		{"infamy", RE::ActorValue::kInfamy},
		{"jumpingbonus", RE::ActorValue::kJumpingBonus},
		{"onehandedmodifier", RE::ActorValue::kOneHandedModifier},
		{"twohandedmodifier", RE::ActorValue::kTwoHandedModifier},
		{"marksmanmodifier", RE::ActorValue::kMarksmanModifier},
		{"blockmodifier", RE::ActorValue::kBlockModifier},
		{"smithingmodifier", RE::ActorValue::kSmithingModifier},
		{"heavyarmormodifier", RE::ActorValue::kHeavyArmorModifier},
		{"lightarmormodifier", RE::ActorValue::kLightArmorModifier},
		{"pickpocketmodifier", RE::ActorValue::kPickpocketModifier},
		{"lockpickingmodifier", RE::ActorValue::kLockpickingModifier},
		{"sneakingmodifier", RE::ActorValue::kSneakingModifier},
		{"alchemymodifier", RE::ActorValue::kAlchemyModifier},
		{"speechcraftmodifier", RE::ActorValue::kSpeechcraftModifier},
		{"alterationmodifier", RE::ActorValue::kAlterationModifier},
		{"conjurationmodifier", RE::ActorValue::kConjurationModifier},
		{"destructionmodifier", RE::ActorValue::kDestructionModifier},
		{"illusionmodifier", RE::ActorValue::kIllusionModifier},
		{"restorationmodifier", RE::ActorValue::kRestorationModifier},
		{"enchantingmodifier", RE::ActorValue::kEnchantingModifier},
		{"dragonrend", RE::ActorValue::kDragonRend},
		{"attackdamagemult", RE::ActorValue::kAttackDamageMult},
		{"healratemult", RE::ActorValue::kHealRateMult},
		{"magickaratemult", RE::ActorValue::kMagickaRateMult},
		{"staminaratemult", RE::ActorValue::kStaminaRateMult},
		{"werewolfperks", RE::ActorValue::kWerewolfPerks},
		{"vampireperks", RE::ActorValue::kVampirePerks},
		{"grabactoroffset", RE::ActorValue::kGrabActorOffset},
		{"grabbed", RE::ActorValue::kGrabbed},
		{"deprecated05", RE::ActorValue::kDEPRECATED05},
		{"reflectdamage", RE::ActorValue::kReflectDamage}
	};

	auto it = actorValueMap.find(valueString);
	if (it != actorValueMap.end()) {
		return it->second;
	}

	// Return a default value or handle the error as necessary
	return RE::ActorValue::kNone; // Example default value
}




//void TESContainer::CopyObjectList(const std::vector<ContainerObject*>& a_copiedData)
//{
//	const auto oldData = containerObjects;
//
//	const auto newSize = a_copiedData.size();
//	const auto newData = calloc<ContainerObject*>(newSize);
//	std::ranges::copy(a_copiedData, newData);
//
//	numContainerObjects = static_cast<std::uint32_t>(newSize);
//	containerObjects = newData;
//
//	free(oldData);
//}
//
//bool AddTextures(RE::TESModelTextureSwap::AlternateTexture* a_object)
//{
//	bool added = false;
//	for (std::uint32_t i = 0; i < numContainerObjects; ++i) {
//		if (const auto entry = containerObjects[i]; entry && entry->obj == a_object) {
//			entry->count += a_count;
//			added = true;
//			break;
//		}
//	}
//	if (!added) {
//		std::vector<ContainerObject*> copiedData{ containerObjects, containerObjects + numContainerObjects };
//		const auto                    newObj = new ContainerObject(a_object, a_count, a_owner);
//		copiedData.push_back(newObj);
//		CopyObjectList(copiedData);
//		return true;
//	}
//	return added;
//}

