#ifndef FACTION_H
#define FACTION_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace FACTION
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;

		std::vector<std::string> damageTypes;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::string              attackDamage;
		std::string              critDamage;
		std::string              critPercentMult;
		
		std::string              speed;
		std::string speedMult;
		
		std::string              actionpointcost;
		std::string              hittype;
		std::string              soundlevel;
		std::string              bashDamage;
		std::string              outOfRangeDamageMult;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> attachParentSlotKeywordsToAdd;
		std::vector<std::string> attachParentSlotKeywordsToRemove;
		std::vector<std::string> ammo;
		std::vector<std::string> ammoList;
		std::vector<std::string> aimModel;
		std::string              overrideProjectile;
		std::string              weaponList;
		std::string              fullName;
		std::string              springBackMult;
		std::string              accuracyMult;
		std::vector<std::string> damageTypesToRemove;
		std::string              recoilPerShotMin;
		std::string              recoilPerShotMax;
		std::vector<std::string> filterByFlagsExclude;

		std::string impactDataSet;
		std::string equipSound;
		std::string unequipSound;
		std::string reach;
		std::string rangeMin;
		std::string rangeMax;
		std::string stagger;
		std::string attackDamageMult;
		
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::vector<std::string> modNames;


	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(FACTION::patch_instruction line, RE::TESFaction* curobj);
}

#endif
