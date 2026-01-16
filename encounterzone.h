#ifndef ENCOUNTERZONE_H
#define ENCOUNTERZONE_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace ENCOUNTERZONE
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;

		std::string minLevel;
		std::string minLevelAdd;
		std::string minLevelMult;
		std::string maxLevel;
		std::string maxLevelAdd;
		std::string maxLevelMult;
		std::string location;
		std::string              faction;
		std::string              rank;
		std::vector<std::string> modNames;

		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(ENCOUNTERZONE::patch_instruction line, RE::BGSEncounterZone* curobj);
}

#endif
