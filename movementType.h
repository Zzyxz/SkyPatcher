#ifndef MOVETYPE_H
#define MOVETYPE_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace MOVETYPE
{

	struct patch_instruction
	{
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;


		std::string              walkLeft;
		std::string              runLeft;
		std::string              walkRight;
		std::string              runRight;
		std::string              walkForward;
		std::string              runForward;
		std::string              walkBack;
		std::string              runBack;
		std::string              rotateInPlaceWalk;
		std::string              rotateInPlaceRun;
		std::string              rotateWhileMovingRun;
		std::vector<std::string>              modNames;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(MOVETYPE::patch_instruction line, RE::BGSMovementType* curobj);
}

#endif
