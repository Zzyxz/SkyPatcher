#ifndef CELL_H
#define CELL_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace CELL
{

	struct patch_instruction
	{
		std::vector<std::string> skipLT;
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::string              acousticSpace;
		std::string              musicType;
		std::string              location;
		std::string              skyRegion;
		std::string              imageSpace;
		std::string              encounterZone;
		std::string              waterHeight;
		std::string              waterEnvMap;
		std::string              fullName;
		
		std::vector<std::string> filterByAcousticSpace;
		std::vector<std::string> filterByMusicType;
		std::vector<std::string> filterByLocation;
		std::vector<std::string> filterBySkyRegion;
		std::vector<std::string> filterByImageSpace;
		std::vector<std::string> setCellFlags;
		std::vector<std::string> removeCellFlags;
		std::vector<std::string> skipByModContains;
		std::string              waterType;
		std::string              fogNear;
		std::string              fogFar;
		std::string              fogPower;
		std::string              clipDist;
		std::string              ambientBlue;
		std::string              ambientGreen;
		std::string              ambientRed;
		std::string              directionalRed;
		std::string              directionalBlue;
		std::string              directionalGreen;
		std::string              fogColorNearRed;
		std::string              fogColorNearGreen;
		std::string              fogColorNearBlue;
		std::string directionalAmbientXMinRed;
		std::string directionalAmbientXMinGreen;
		std::string directionalAmbientXMinBlue;
		std::string directionalAmbientXMaxRed;
		std::string directionalAmbientXMaxGreen;
		std::string directionalAmbientXMaxBlue;

		std::string directionalAmbientYMinRed;
		std::string directionalAmbientYMinGreen;
		std::string directionalAmbientYMinBlue;
		std::string directionalAmbientYMaxRed;
		std::string directionalAmbientYMaxGreen;
		std::string directionalAmbientYMaxBlue;

		std::string directionalAmbientZMinRed;
		std::string directionalAmbientZMinGreen;
		std::string directionalAmbientZMinBlue;
		std::string directionalAmbientZMaxRed;
		std::string directionalAmbientZMaxGreen;
		std::string directionalAmbientZMaxBlue;

		std::string directionalAmbientSpecularRed;
		std::string directionalAmbientSpecularGreen;
		std::string directionalAmbientSpecularBlue;

		std::vector<std::string> setInheritanceFlags;
		std::vector<std::string> removeInheritanceFlags;

		std::string directionalXY;
		std::string directionalZ;


		
		std::string lightingTemplate;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
<<<<<<< HEAD
	void* patch(const CELL::patch_instruction& line, RE::TESObjectCELL* curobj);
=======
	void* patch(const CELL::patch_instruction& line, RE::TESObjectCELL* curobj);
>>>>>>> origin/new-main
}

#endif
