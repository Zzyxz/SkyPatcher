#ifndef NPC_H
#define NPC_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace NPC
{

	struct ActorData
	{
		std::unordered_map<RE::FormID, int16_t> healthMap;
		std::unordered_map<RE::FormID, int16_t> staminaMap;
		std::unordered_map<RE::FormID, int16_t> magickaMap;
		int16_t                                 playerHealth = 0;
		bool                                    bPlayerHealth = false;
		int16_t                                 playerStamina = 0;
		bool                                    bPlayerStamina = false;
		int16_t                                 playerMagicka = 0;
		bool                                    bPlayerMagicka = false;
		std::unordered_map<RE::FormID, bool>	autoHealthMap;
		std::unordered_map<RE::FormID, bool>    autoStaminaMap;
		std::unordered_map<RE::FormID, bool>    autoMagickaMap;								
	};

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> races;
		std::vector<std::string> filterFactions;
		std::vector<std::string> filterFactionsOr;
		std::vector<std::string> filterFactionsExcluded;
		std::vector<std::string> factionsToRemove;
		std::vector<std::string> factionsToAdd;
		std::vector<float>       factionsToAddRank1;
		std::vector<float>       factionsToAddRank2;

		std::string calcStats;
		std::string kPCLevelMult;
		std::string kEssential;
		std::string kProtected;
		std::string isFemale;

		std::string level_min;
		std::string level_max;

		std::vector<std::string> avifs;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::vector<std::string> pcMultFlag;
		std::vector<int>       PCvalues1;
		std::vector<int>       PCvalues2;
		
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> perksToAdd;
		std::vector<std::string> spellsToAdd;

		std::string              outfitDefault;
		std::string              outfitSleep;
		std::string              deathItem;
		std::string              skin;
		std::string              race;
		std::string              staminaBonus;
		std::vector<std::string> filterClass;
		std::vector<std::string> filterClassExclude;
		
		std::string              Class;
		std::string              xpValueOffset;
		std::string              level;
		std::string              calcLevelMin;
		std::string              calcLevelMax;


		std::vector<std::string> objectsToAdd;
		std::vector<std::string> objectsToAddValue;
		std::vector<std::string> objectsToRemove;
	};



	struct patch_instruction create_patch_instruction(const std::string& line);
	//void                     process_patch_instructions(const patch_instruction& line);
	void                     process_patch_instructions(const std::list<patch_instruction>& tokens);
	
	ActorData readConfig(const std::string& folder);
	void* patch(NPC::patch_instruction line, RE::TESNPC* curobj);

}

#endif
