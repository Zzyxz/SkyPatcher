#ifndef PROJECTILE_H
#define PROJECTILE_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace PROJECTILE
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


		std::string speed;
		std::string range;
		std::string              rangeMult;
		std::string gravity;
		std::string              gravityMult;
		std::string sound;
		std::string type;
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::vector<std::string> modNames;
		std::vector<std::vector<std::string>> alternateTextures;
		std::vector<std::string>              alternateTexturesRemove;
		std::vector<std::string>              alternateTexturesFind;
		std::string                           alternateTexturesClear;
		std::string                           model;
		std::string                           explosion;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void  patch(PROJECTILE::patch_instruction line, RE::BGSProjectile* curobj);
}

#endif
