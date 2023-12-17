#ifndef ALCH_H
#define ALCH_H

using namespace std;

#	include "PCH.h"
#	include "dirent.h"
#	include "utility.h"
#	include <fstream>
#	include <iostream>
#	include <regex>
#	include <string>
#	include <vector>

namespace ALCH
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> mgefs;
		std::vector<std::string> mgefsOr;
		std::vector<std::string> mgefsExcluded;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::string filterType;
		std::string capsvalue;
		std::string weight;
		std::string fullName;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::string> removedObjects;
		std::vector<std::vector<std::string>> changedObjects;
	};

	struct line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
}

#endif
