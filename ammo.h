#ifndef AMMO_H
#define AMMO_H

using namespace std;

#	include "PCH.h"
#	include "dirent.h"
#	include "utility.h"
#	include <fstream>
#	include <iostream>
#	include <regex>
#	include <string>
#	include <vector>

namespace AMMO
{
	struct line_content
	{
		std::vector<std::string> ammo;
		std::string type;
		std::string damage;
		std::string damageMult;
		std::string projectile;
		std::vector<std::string> formList;
		std::string fullName;
		std::vector<std::string> keywordsToAdd;
		std::string weight;
		std::string weightLessThan;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
	};

	struct line_content create_patch_instruction_ammo(const std::string& line);
	void process_patch_instructions_ammo(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void* patch(AMMO::line_content line, RE::TESAmmo* curobj);
}

#endif
