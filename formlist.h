#ifndef FORMLIST_H
#define FORMLIST_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace FORMLIST
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectsAdd;
		std::vector<std::string> objectsRemove;
		std::vector<std::string> formsToReplace;
		std::vector<std::string> formsToReplaceWith;
		std::vector<std::string> modNames;
		std::string clear;
	};

	struct line_content create_patch_instruction(const std::string& line);
	void                process_patch_instructions(const std::list<line_content>& tokens);
	void*               readConfig(const std::string& folder);
}

#endif
