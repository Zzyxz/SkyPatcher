#ifndef WEAPON_H
#define WEAPON_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace WEAPON
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> filterByFirstPersonModelOr;
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::string              dwMatch;
		std::vector<std::string> damageTypes;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::string              attackDamage;
		std::string              critDamage;
		std::string              critPercentMult;
		std::string				 mirrorWeapon;
		std::string				 critDamageToAdd;
		std::string				 critDamageMult;
		std::string              speed;
		std::string				 speedMult;
		std::string              attackDamageToAdd;
		std::string              templateWeapon;
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
		std::vector<std::string> filterByWFlags;
		std::string              critDamageSetToBase;
		
		std::string              altBlockMaterialType;
		std::string blockBashImpactDataSet;
		std::string              equipSlot;

		
		std::string impactDataSet;
		std::string equipSound;
		std::string unequipSound;
		std::string failSound;
		std::string reach;
		std::string rangeMin;
		std::string rangeMax;
		std::string stagger;
		std::string attackDamageMult;
		std::vector<std::string> embeddedAVs;
		
		std::string weight;
		std::string weightMult;
		std::string value;
		std::string valueMult;
		std::string objectEffect;
		std::string model;
		std::string modelFirstPerson;
		std::string enchantAmount;
		std::string attackDamageMatch;
		std::vector<std::string> filterAnimationTypeOr;
		
		std::vector<std::string> setAnimationType;
		std::vector<std::string> setSkillType;
		std::vector<std::string> filterByEditorIdContains;
		std::vector<std::string> filterByEditorIdContainsOr;
		std::vector<std::string> filterByEditorIdContainsExcluded;
		std::vector<std::string> hasPlugin;
		std::vector<std::string> hasPluginOr;
		std::string minX;
		std::string minY;
		std::string minZ;
		std::string maxX;
		std::string maxY;
		std::string maxZ;
		std::string acswMatch;
		std::vector<std::string> restrictToSkill;
		std::vector<std::string> modNames;
	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(WEAPON::patch_instruction line, RE::TESObjectWEAP* curobj);
}

#endif
