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
		std::string              damageToAdd;
		std::vector<std::string>              objectExcluded;
		std::string projectile;
		std::vector<std::string> formList;
		std::string fullName;
		std::vector<std::string> keywordsToAdd;
		std::string weightLessThan;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::vector<std::string> modNames;
		std::string              weight;
		std::string              weightMult;
		std::string              value;
		std::string              valueMult;
		std::vector<std::string>              hasPlugin;
		std::vector<std::string>              hasPluginOr;
		std::string                           restrictToBolts;
		std::string                           speed;
		std::string                           speedMult;
		std::string                           range;
		std::string                           rangeMult;
		std::string                           gravity;
		std::string                           gravityMult;
		std::vector<std::vector<std::string>> alternateTextures;
		std::vector<std::string>              alternateTexturesRemove;
		std::vector<std::string>              alternateTexturesFind;
		std::string                           alternateTexturesClear;
		std::string                           model;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;
		std::vector<std::string>              setFlags;
		std::vector<std::string>              removeFlags;
	};

	struct line_content create_patch_instruction_ammo(const std::string& line);
	void process_patch_instructions_ammo(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void* patch(AMMO::line_content line, RE::TESAmmo* curobj);
}

#endif
