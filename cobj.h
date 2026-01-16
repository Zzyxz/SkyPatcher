#ifndef COBJ_H
#define COBJ_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace COBJ
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> filterByWorkBenchKeyword;
		
		std::vector<std::string> objectExcluded;
		std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;

		std::vector<std::string> keywordsNew;
		std::vector<std::string> keywordsNewOr;
		std::vector<std::string> keywordsNewExcluded;

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
		std::vector<std::string> restrictToKeywords;
		
		std::vector<std::string>              filterWorkbenchKeywords;
		std::vector<std::string>              filterWorkbenchKeywordsExcluded;
		std::vector<std::string>              filterByCreatedObjects;
		std::vector<std::string>			  filterByRecipeIngredients;
		std::vector<std::string>              filterByRecipeIngredientsOr;
		std::vector<std::string>              filterByRecipeIngredientsExcluded;
		std::vector<std::string>              removeInstancesContainerObjects;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;
		
		std::string clearIngredients;
		std::string                           workbenchKeyword;
		std::string                           createdObject;
		std::string countMult;
		std::string                           count;

		std::vector<std::vector<std::string>> changeCobjsCountByMult;
		std::vector<std::vector<std::string>> changeCobjsCount;
		std::vector<std::string>              modNames;
		std::vector<std::vector<std::string>> addedContainerObjects;
		std::vector<std::vector<std::string>> addedContainerObjectsAdjusted;
		std::vector<std::vector<std::string>> removeContainerObjects;
		std::vector<std::vector<std::string>> replaceObjectInContainer;
		
	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(COBJ::patch_instruction line, RE::BGSConstructibleObject* curobj);
}

#endif
