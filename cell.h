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
		std::string              skyRegion;
		std::string              imageSpace;
		std::string              encounterZone;
		std::string              waterHeight;
		std::string              waterEnvMap;
		std::string              fullName;
		
		std::vector<std::string> filterByAcousticSpace;
		std::vector<std::string> filterByMusicType;
		std::vector<std::string> filterBySkyRegion;
		std::vector<std::string> filterByImageSpace;
		std::vector<std::string> setCellFlags;
		std::vector<std::string> removeCellFlags;
		std::vector<std::string> skipByModContains;
		;
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
		// X
		std::string directionalAmbientXMinRed;
		std::string directionalAmbientXMinGreen;
		std::string directionalAmbientXMinBlue;
		std::string directionalAmbientXMaxRed;
		std::string directionalAmbientXMaxGreen;
		std::string directionalAmbientXMaxBlue;

		// Y
		std::string directionalAmbientYMinRed;
		std::string directionalAmbientYMinGreen;
		std::string directionalAmbientYMinBlue;
		std::string directionalAmbientYMaxRed;
		std::string directionalAmbientYMaxGreen;
		std::string directionalAmbientYMaxBlue;

		// Z
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


		
		/*std::vector<std::string> filterAmmos;
		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;

		std::vector<std::string> damageTypes;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::string              attackDamage;
		std::string              critDamage;
		std::string              critPercentMult;
		std::string              weight;
		std::string              speed;

		std::string              capsvalue;
		std::string              actionpointcost;
		std::string              hittype;
		std::string              soundlevel;
		std::string              bashDamage;
		std::string              outOfRangeDamageMult;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> attachParentSlotKeywordsToAdd;
		std::vector<std::string> attachParentSlotKeywordsToRemove;
		std::vector<std::string> ammo;
		std::vector<std::string> ammoList;
		std::vector<std::string> aimModel;
		std::string              overrideProjectile;
		std::string              weaponList;
		std::string              fullName;
		std::string              springBackMult;
		std::string              accuracyMult;
		std::vector<std::string> damageTypesToRemove;
		std::string              recoilPerShotMin;
		std::string              recoilPerShotMax;
		std::vector<std::string> filterByFlagsExclude;

		std::string impactDataSet;
		std::string equipSound;
		std::string unequipSound;
		std::string reach;
		std::string rangeMin;
		std::string rangeMax;*/

		std::string lightingTemplate;

	};

	struct patch_instruction create_patch_instructions(const std::string& line);

	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	void* readConfig(const std::string& folder);
	void* patch(CELL::patch_instruction line, RE::TESObjectCELL* curobj);
}

#endif
