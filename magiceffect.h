#ifndef MGEF_H
#define MGEF_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace MGEF
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> effectExcluded;
		std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> restrictToDeliveryType;
		std::vector<std::string> restrictToActorValue;
		std::vector<std::string> restrictToMagicSkill;
		
		
		std::vector<std::string> restrictToCastingType;
		std::vector<std::string> filterArchetype;
		std::vector<std::vector<std::string>> soundsToChange;
		std::vector<std::string> filterBySounds;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::string              magicItemDescription;
		std::vector<std::string> filterByEditorIdContains;
		std::vector<std::string> filterByEditorIdContainsOr;
		std::vector<std::string> filterByEditorIdContainsExcluded;
		std::string filterByDamageHealthMgefs;
		std::vector<std::string> restrictToResistType;
		std::string				restrictToArea;
		std::string		 restrictToDetrimental;
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
		std::string sound;

		std::string enchantArt;
		std::string enchantShader;
		
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::vector<std::string> modNames;
		std::vector<std::string> filterByModNamesLastOverriddenExcluded;
		std::vector<std::string> hitShaderExcluded;
		
		std::vector<std::string> ModNamesExcluded;
		std::string              baseCost;
		std::string              minimumSkillLevel;
		std::string              spellmakingCastingTime;
		std::string              spellmakingArea;
		std::string              projectile;
		std::string              castType;
		std::string              impactData;
		std::string              castingArt;
		std::string              skillUsageMult;
		std::string              perk;
		std::string              enchantAmount;
		std::string				 hitShader;
		std::string              removeEdgeGlow;
		
		
		//std::string              sounds;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(MGEF::patch_instruction line, RE::EffectSetting* curobj);
}

#endif
