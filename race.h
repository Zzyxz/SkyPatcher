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
	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;

		std::vector<std::string> avifs;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> spellsToAdd;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::vector<std::string>> modfiedObjects;
		//attackDataToAdd
		//attackDataToModify

	};

	struct patch_instruction create_patch_instruction(const std::string& line);
	void                     process_patch_instructions(const std::list<patch_instruction>& tokens);
	void*                    readConfig(const std::string& folder);
}

#endif
