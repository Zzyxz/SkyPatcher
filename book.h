#ifndef BOOK_H
#define BOOK_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace BOOK
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> filterBySpellKeywordsOr;
		std::vector<std::string> filterByCastingPerk;
		std::vector<std::string> damageTypes;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::string              attackDamage;
		std::string              critDamage;
		std::string              critPercentMult;

		std::string speed;
		std::string speedMult;

		std::string              actionpointcost;
		std::string              hittype;
		std::string              soundlevel;
		std::string              bashDamage;
		std::string              outOfRangeDamageMult;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> filterByFlags;
		std::vector<std::string> attachParentSlotKeywordsToRemove;
		std::vector<std::string> ammo;
		std::vector<std::string> ammoList;
		std::vector<std::string> aimModel;
		std::string              fullName;

		std::string              teachSpell;
		std::string              teachSkill;
		std::string              description;
		std::string              accuracyMult;
		std::vector<std::string> damageTypesToRemove;
		std::string              recoilPerShotMin;
		std::string              recoilPerShotMax;
		std::vector<std::string> filterByFlagsExclude;
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::string              impactDataSet;
		std::string              pickUpSound;
		std::string              putDownSound;
		std::string              type;
		std::string              rangeMin;
		std::string              rangeMax;
		std::string              stagger;
		std::string              inventoryArt;

		std::string                           weight;
		std::string                           weightMult;
		std::string                           value;
		std::string                           valueMult;
		std::vector<std::vector<std::string>> alternateTextures;
		std::vector<std::string>              alternateTexturesRemove;
		std::vector<std::string>              alternateTexturesFind;
		std::string                           alternateTexturesClear;
		std::string                           model;

		std::vector<std::string> modNames;

		std::string minX;
		std::string minY;
		std::string minZ;
		std::string maxX;
		std::string maxY;
		std::string maxZ;

		std::vector<std::string> filterByEditorIdContains;
		std::vector<std::string> filterByEditorIdContainsOr;
		std::vector<std::string> filterByEditorIdContainsExcluded;
		
	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(BOOK::patch_instruction line, RE::TESObjectBOOK* curobj);
}

#endif
