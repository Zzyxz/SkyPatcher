#ifndef SPELL_H
#define SPELL_H

using namespace std;

#	include "PCH.h"
#	include "dirent.h"
#	include "utility.h"
#	include <fstream>
#	include <iostream>
#	include <regex>
#	include <string>
#	include <vector>

namespace SPELL
{

	class LoadRaceDataHook : public RE::SpellItem
	{
	public:
		static void InstallHook();
		static bool Hook_InitItemImpl(RE::SpellItem* a_race, RE::TESFile* a_mod);

	private:
		static inline REL::Relocation<decltype(&Hook_InitItemImpl)> func;
	};
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> filterByCastingPerk;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> mgefs;
		std::vector<std::string> mgefsOr;
		std::vector<std::string> mgefsExcluded;
		std::vector<std::string> flagFilters;
		std::vector<std::string> flagFiltersOr;
		std::vector<std::string> flagFiltersExcluded;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> restrictToCastingType;
		
		std::string filterType;
		std::string capsvalue;
		std::string weight;
		std::string fullName;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::string> removedObjects;
		std::vector<std::vector<std::string>> changedObjects;
		std::vector<std::vector<std::string>> changedAddObjects;
		std::vector<std::vector<std::string>> changedFirstObjects;
		
		std::string                           baseCost;
		std::string                           halfCostPerk;
		std::string                           chargeTime;
		std::string                           castType;
		std::string                           equipSlot;
		std::string                           clear;

		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::vector<std::string> modNames;
		std::vector<std::string> filterByEditorIdContains;
		std::vector<std::string> filterByEditorIdContainsOr;
		std::vector<std::string> filterByEditorIdContainsExcluded;
		std::vector<std::string> restrictToSchool;
		
		
		
	};

	struct line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void               patch(SPELL::line_content line, RE::SpellItem* curobj);
}

#endif
