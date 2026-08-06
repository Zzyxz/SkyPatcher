#ifndef LEVELED_LIST_H
#define LEVELED_LIST_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

namespace LEVELEDLIST
{
	struct line_content
	{
		std::string object;
		std::string level;
		std::string reference;
		std::string count;
		std::string chance;
		std::string clear;
		std::string clearFlags;
		std::string calcForLevel;
		std::string calcEachItem;
		std::string calcForLevelAndEachItem;
		std::string calcUseAll;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;
		std::vector<std::string>              objects;
		std::vector<std::string>              containerObjects;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::vector<std::string>> objectMultCount;
		
		std::vector<std::vector<std::string>> replaceObjects;
		std::vector<std::vector<std::string>> addedObjectsOnce;
		std::vector<std::vector<std::string>> addedContainerObjects;
		std::vector<std::string>              removedContainerObjects;
		std::vector<std::vector<std::string>> removedObjects;
		std::vector<std::vector<std::string>> templateKeyword;
		std::vector<std::string>              removeItemsByKeyword;
		std::string                           noFilterLL;
		std::string                           noFilterLLNPC;
		std::string                           llglobal;
		std::string                           chanceNone;

		
	};

	struct line_content create_patch_instruction(const std::string& line);
	void                process_patch_instructions(const std::list<line_content>& tokens);
	void*               readConfig(const std::string& folder);
	void                process_patch_instructions(const std::list<line_content>& tokens);
	template <typename T>
	void patch(const LEVELEDLIST::line_content& line, T* curobj);
	void                patchLevChar(LEVELEDLIST::line_content line, RE::TESLevCharacter* curobj);
}

#endif
