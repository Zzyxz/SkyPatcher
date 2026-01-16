#ifndef OUTFIT_H
#define OUTFIT_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace OUTFIT
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> addedObjects;
		std::vector<std::string> objectsRemove;
		std::vector<std::string> formsToReplace;
		std::vector<std::string> formsToReplaceWith;
		std::string              clear;
		std::vector<std::string> formAnd;
		std::vector<std::string> formOr;
		std::vector<std::string> formExcluded;
		std::vector<std::string> modNames;
	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void patch(OUTFIT::patch_instruction line, RE::BGSOutfit* curobj);
}

#endif
