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
	std::vector<float> values1;
	std::vector<float> values2;
	std::string damageResist;
	std::string damageResistMult;
	
	std::string weight;
	std::string health;
	std::string objectEffect;
	std::vector<std::string> setBipedSlot;
	std::vector<std::string> removeBipedSlot;
	std::vector<std::string> bipedSlot;
	std::vector<std::string> bipedSlotOr;
	std::vector<std::string> bipedSlotExcluded;
	std::vector<std::string> keywordsToAdd;
	std::vector<std::string> keywordsToRemove;
	std::vector<std::string> attachParentSlotKeywordsToAdd;
	std::vector<std::string> attachParentSlotKeywordsToRemove;
	std::string fullName;
};

struct patch_instruction_armor create_patch_instruction_armor(const std::string& line);
void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens);
void* readConfig(const std::string& folder);

}

#endif  // PATCH_INSTRUCTION_ARMOR_H
