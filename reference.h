#ifndef REFERENCE_H
#define REFERENCE_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace REFERENCE
{

	struct REFRCommandData
	{
		bool disable = false;                       // disable the REFR on load
		RE::TESBoundObject* replaceWith = nullptr;


		REFRCommandData() = default;
	};

	
	struct RefrData
	{
		std::unordered_map<RE::FormID, REFRCommandData> refrDataMap;
	};


	struct patch_instruction
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::string baseobject;
		std::string disable;
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
		
		std::string weight;
		std::string weightMult;
		std::string value;
		std::string valueMult;
		std::vector<std::string>              modNames;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	RefrData readConfig(const std::string& folder);
	void* patch(REFERENCE::patch_instruction line, RE::FormID curobj);
}

#endif
