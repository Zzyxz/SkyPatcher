#ifndef CONTAINER_H
#define CONTAINER_H

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

namespace CONTAINER
{

	extern int iAllowLeveledListsAddedToContainers; 

	class LoadRaceDataHook : public RE::TESObjectCONT
	{
	public:
		static void InstallHook();
		static bool Hook_InitItemImpl(RE::TESObjectCONT* a_race);

	private:
		static inline REL::Relocation<decltype(&Hook_InitItemImpl)> func;
	};

	struct line_content
	{
		std::string object;
		std::string level;
		std::string reference;
		std::string count;
		std::string chance;
		std::string clear;
		std::string calcForLevel;
		std::string calcEachItem;
		std::string calcForLevelAndEachItem;
		std::string calcUseAll;

		std::vector<std::string>              objects;
		std::vector<std::string>              containerObjects;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::vector<std::string>> addedContainerObjectsOnce;
		std::vector<std::vector<std::string>> addedContainerObjects;
		std::vector<std::string>              removedContainerObjects;
		std::vector<std::vector<std::string>> removeContainerObjectsByCount;
		std::vector<std::string>              removeContainerObjectsByKeyword;
		
		std::vector<std::vector<std::string>> objectMultCount;
		std::vector<std::vector<std::string>> removedObjects;
		std::vector<std::vector<std::string>> templateKeyword;
		std::vector<std::vector<std::string>> objectsToReplace;
		
		std::vector<std::string>              removeItemsByKeyword;
		std::string                           noFilterLL;
		std::vector<std::string>              modNames;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;
	};

	struct line_content create_patch_instruction(const std::string& line);
	void                process_patch_instructions(const std::list<line_content>& tokens);
	void*               readConfig(const std::string& folder);
	void                process_patch_instructions(const std::list<line_content>& tokens);
	//void                patch(CONTAINER::line_content line, RE::TESLevItem* curobj);
	void                patchContainer(CONTAINER::line_content line, RE::TESObjectCONT* curobj);
	bool findObject(line_content line, RE::TESObjectCONT* curobj);
	void*               readConfigHook(const std::string& folder);
}

#endif
