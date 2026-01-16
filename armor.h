#ifndef ARMOR_H
#define ARMOR_H

using namespace std;

#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "PCH.h"
#include "utility.h"
#include "dirent.h"

namespace ARMOR {

struct patch_instruction_armor
{
	std::vector<std::string> object;
	std::vector<std::string> objectExcluded;
	std::vector<std::string> keywords;
	std::vector<std::string> keywordsOr;
	std::vector<std::string> keywordsExcluded;
	std::vector<std::string> damageTypes;
	std::vector<std::string> filterByArmorType;
	std::vector<std::string> filterByArmorTypeExcluded;
	std::vector<float> values1;
	std::vector<float> values2;
	std::string damageResist;
	std::string damageResistMult;
	std::string              damageResistMatch;
	
	std::string              altBlockMaterialType;
	std::string				 mirrorArmor;
	std::string              blockBashImpactDataSet;
	std::string              equipSlot;
	std::string              enchantAmount;
	std::vector<std::string> filterByEditorIdContains;
	std::vector<std::string> filterByEditorIdContainsOr;
	std::vector<std::string> filterByEditorIdContainsExcluded;
	std::vector<std::string> filterByNameContains;
	std::vector<std::string> filterByNameContainsOr;
	std::vector<std::string> filterByNameContainsExcluded;
	
	std::string              weight;
	std::string              weightMult;
	std::string              value;
	std::string              valueMult;
	std::string health;
	std::string              templateArmor;
	std::string              dwMatch;
	std::string              modelMatch;
	
	


	std::string objectEffect;
	std::vector<std::string> setBipedSlot;
	std::vector<std::string> removeBipedSlot;
	std::vector<std::string> bipedSlot;
	std::vector<std::string> bipedSlotOr;
	std::vector<std::string> bipedSlotExcluded;
	std::vector<std::string> keywordsToAdd;
	std::vector<std::string> keywordsToRemove;
	std::vector<std::string> armorAddonToAdd;
	std::vector<std::string> armorAddonToRemove;
	std::string              clearArmorAddons;
	std::vector<std::string> filterByArmorAddons;
	std::vector<std::string> filterByArmorAddonsOr;
	std::vector<std::string> filterByArmorAddonsExcluded;
	
	std::vector<std::vector<std::string>> reCalcArmorRating;
	std::vector<std::vector<std::string>> reCalcArmorRatingv2;
	
	std::vector<std::vector<std::string>> armaRaceToAdd;
	

	std::string fullName;
	std::string              armorType;
	std::string              modelFemale;
	std::string              modelMale;
	
	std::string              pickUpSound;
	std::string              putDownSound;
	std::vector<std::string> modNames;

	std::vector<std::string> hasPlugin;
	std::vector<std::string> hasPluginOr;

	std::vector<std::string> setFlags;
	std::vector<std::string> removeFlags;

	std::string minX;
	std::string minY;
	std::string minZ;
	std::string maxX;
	std::string maxY;
	std::string maxZ;

	std::vector<std::string> restrictKeywords;
	std::vector<std::string> restrictSlots;
};

struct patch_instruction_armor create_patch_instruction_armor(const std::string& line);
void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens);
void* readConfig(const std::string& folder);
void*                          patch(ARMOR::patch_instruction_armor line, RE::TESObjectARMO* curobj);

}

#endif  // PATCH_INSTRUCTION_ARMOR_H
