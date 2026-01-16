#ifndef RACE_H
#define RACE_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace RACE
{

		class LoadRaceDataHook : public RE::TESRace
	{
	public:
		static void InstallHook();
		static bool Hook_InitItemImpl(RE::TESRace* a_race);

	private:
		static inline REL::Relocation<decltype(&Hook_InitItemImpl)> func;
	};

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> filterByVoiceTypeOr;

		std::vector<std::string> avifs;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> spellsToAdd;
		std::vector<std::string> spellsToRemove;
		std::vector<std::string> levSpellsToAdd;
		std::vector<std::string> levSpellsToRemove;
		std::vector<std::string> shoutsToAdd;
		std::vector<std::string> shoutsToRemove;
		std::vector<std::string>              filterByMaleModelContains;
		std::vector<std::string>              filterByFemaleModelContains;
		std::vector<std::vector<std::string>> keywordsToAddIfKeywordsEIDDoNotContain;
		
		std::vector<std::vector<std::vector<std::string>>> addedObjects;
		std::vector<std::vector<std::vector<std::string>>> attackDataToChange;
		
		std::vector<std::vector<std::string>> modfiedObjects;
		std::string                           skin;
		std::string                           baseMass;
		std::string                           baseMassMult;
		std::string                           baseCarryweight;
		std::string                           baseCarryweightMult;
		std::string                           startingHealth;
		std::string                           startingStamina;
		std::string                           startingMagicka;
		std::string                           startingHealthMult;
		std::string                           startingStaminaMult;
		std::string                           startingMagickaMult;
		std::string                           regenHealth;
		std::string                           regenStamina;
		std::string                           regenMagicka;
		std::string                           regenHealthMult;
		std::string                           regenStaminaMult;
		std::string                           regenMagickaMult;
		std::string                           damageUnarmed;
		std::string                           damageUnarmedMult;
		std::string                           reachUnarmed;
		std::string                           reachUnarmedMult;
		std::string                           attackRace;
		std::string                           rateAcceleration;
		std::string                           rateDeceleration;
		std::string                           angularAccelerationRate;
		std::string                           aimAngleTolerance;
		
		std::vector<std::string> filterByEditorIdContains;
		std::vector<std::string> filterByEditorIdContainsOr;
		std::vector<std::string> filterByEditorIdContainsExcluded;
		
		
		std::string heightMale;
		std::string heightFemale;
		std::string weightMale;
		std::string weightFemale;
		std::vector<std::string> modNames;
		std::vector<std::string> removeAttackData;
		//attackDataToAdd
		//attackDataToModify

	};

	struct patch_instruction create_patch_instruction(const std::string& line);
	void                     process_patch_instructions(const std::list<patch_instruction>& tokens);
	void*                    readConfig(const std::string& folder);
	void*                    readConfigHook(const std::string& folder);
	void					 patch(RACE::patch_instruction line, RE::TESRace* curobj);
	void                     hookpatch(RACE::patch_instruction line, RE::TESRace* curobj);
}

#endif
