#ifndef NPC_H
#define NPC_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>


namespace NPC
{

	class LoadRaceDataHook : public RE::TESNPC
	{
	public:
		static void InstallHook();
		static bool Hook_Load(RE::TESNPC* a_race, RE::TESFile* a_mod);

	private:
		static inline REL::Relocation<decltype(&Hook_Load)> func;
	};

	struct TintLayerOverlay {
		uint16_t tintIndex = 0;
		RE::Color color{};  // Defaults to (0,0,0,0)
		uint16_t interpolationValue = 100;  // 1.0f * 100
		uint16_t preset = 0;

		bool operator==(const TintLayerOverlay& other) const {
			return tintIndex == other.tintIndex && preset == other.preset;
		}
	};



	struct NPCData
	{
		RE::TESNPC*                 baseObject = nullptr;
		RE::TESNPC*                 npcVisual = nullptr;
		RE::BGSColorForm*			haircolor = nullptr;
		RE::BGSVoiceType*           voiceType = nullptr;
		RE::TESRace*                npcRace = nullptr;
		RE::TESObjectARMO*          skin = nullptr;
		bool                        skinWasSet = false;
		bool                        restrictToTraits = false;
		RE::TESClass*               npcClass = nullptr;
		int							styleChance = 100;
		int							minLevel = -99;
		bool						minLevelWasSet = false;
		int							maxLevel = -99;
		bool						maxLevelWasSet = false;
		bool						changeVoice = false;
		bool						updateFlags = false;
		bool						updateTemplateFlags = false;
		RE::TESCombatStyle*         npcCombatStyle = nullptr;
		RE::BGSListForm*			npcCombatStyleList = nullptr;
		int                         npcCombatStyleChance = 100;
		RE::BGSListForm*			npcVisualStyleList = nullptr;
		int                         npcVisualStyleChance = 100;
		std::vector<RE::SpellItem*> spellItems = {};
		std::vector<RE::BGSPerk*>   perks = {};
		int16_t                     health = -1;
		int16_t                     stamina = -1;
		int16_t                     magicka = -1;
		std::vector<TintLayerOverlay> overlaysToAdd = {};
		NPCData() = default;
		
	};

	struct ActorData
	{
		std::unordered_map<RE::FormID, NPCData> npcData;
		std::unordered_map<RE::FormID, std::string> ignoreList;
	};

	struct patch_instruction
	{
		std::vector<std::vector<std::vector<std::string>>> addedAttackData;
		std::vector<std::vector<std::vector<std::string>>> attackDataToChange;
		
		
		std::vector<std::string> removeAttackData;
		std::vector<std::string> restrictToFlags;
		std::vector<std::string> restrictToTemplateFlags;
		
		std::vector<std::string> object;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> keywords;
		std::vector<std::string> filterCombatStyle;

		std::vector<std::string>              keywordsOr;
		std::vector<std::string>              keywordsExcluded;
		std::vector<std::string>              races;
		std::vector<std::string>              restrictToRaces;
		
		std::vector<std::string>              filterFactions;
		std::vector<std::string>              filterFactionsOr;
		std::vector<std::string>              filterFactionsExcluded;
		std::vector<std::string>              filterByEditorIdContains;
		std::vector<std::string>              filterByEditorIdContainsOr;
		std::vector<std::string>              filterByEditorIdContainsExcluded;

		
		std::vector<std::string>              factionsToRemove;
		std::vector<std::string>              factionsToAdd;
		std::vector<float>                    factionsToAddRank1;
		std::vector<float>                    factionsToAddRank2;
		std::vector<std::vector<std::string>> addedContainerObjectsOnce;
		std::vector<std::vector<std::string>> aggressionRadiusRanges;
		std::vector<std::vector<std::string>> overlayToAdd;
		
		std::vector<std::vector<std::string>> removeContainerObjectsByCount;
		std::vector<std::string>              removeContainerObjectsByKeyword;
		std::string                           calcStats;
		std::string                           weight;
		std::string                           height;
		std::string                           kPCLevelMult;
		std::string                           kEssential;
		std::string                           kProtected;
		std::string                           rvsRestrictToTraits;
		std::string                           haircolor;
		
		std::vector<std::vector<std::string>> rcslist;
		std::string                           copyVisualStyle;
		std::vector<std::vector<std::string>> setRandomVisualStyle;
		std::vector<std::vector<std::string>> addRandomVisualStyle;

		std::vector<std::vector<std::string>> skillRestrict;
		std::vector<std::string>              restrictToMaleModelContains;

		std::string              isFemale;
		std::string              restrictGender;
		std::string              clearClassFromAttributes;
		std::vector<std::string> restrictToCombatStyle;
		std::vector<std::string> restrictToKeyword;
		
		std::string              filterPCMult;
		std::string              filterAutoCalc;
		std::string              filterEssential;
		std::string              filterProtected;

		std::string level_min;
		std::string level_max;

		std::vector<std::string> avifs;
		std::vector<float>       values1;
		std::vector<float>       values2;
		std::vector<std::string> skills;
		std::vector<int>         skillvalues1;
		std::vector<int>         skillvalues2;
		std::vector<std::string> pcMultFlag;
		std::vector<int>         PCvalues1;
		std::vector<int>         PCvalues2;

		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::vector<std::string> perksToAdd;
		std::vector<std::string> spellsToAdd;
		std::vector<std::string> spellsToRemove;

		std::string              outfitDefault;
		std::string              outfitSleep;
		std::string              deathItem;
		std::string              skin;
		std::string              race;
		std::string              fullName;
		std::string              shortName;
		std::string              attackRace;
		std::string              healthBonus;
		std::string              magickaBonus;
		std::string              staminaBonus;
		std::vector<std::string> filterClass;
		std::vector<std::string> filterByOutfit;

		std::vector<std::string> filterClassExclude;

		std::string Class;
		std::string voiceType;
		std::string xpValueOffset;
		std::string level;
		std::string calcLevelMin;
		std::string calcLevelMax;
		std::string speedMultiplier;
		std::string clearInventory;

		std::vector<std::string> setTemplateFlags;
		std::string setAggression;
		std::string setConfidence;
		std::string setAssistance;
		std::string setMorality;
		std::string setMood;
		std::string aggressionRadiusBehavior;
		
		std::vector<std::string> removeTemplateFlags;
		std::vector<std::string> setFlags;
		std::vector<std::string> removeFlags;
		std::vector<std::string> restrictToVoiceType;
		

		std::vector<std::string> levSpellsToAdd;
		std::vector<std::string> levSpellsToRemove;
		std::vector<std::string> shoutsToAdd;
		std::vector<std::string> shoutsToRemove;

		std::vector<std::string> objectsToAdd;
		std::vector<std::vector<std::string>> objectsToReplace;
		std::vector<std::string> objectsToAddValue;
		std::vector<std::string> objectsToRemove;
		std::vector<std::string> objectsToRemoveNew;
		std::vector<std::string> modNames;
		std::string              baseObject;
	};

	struct patch_instruction create_patch_instruction(const std::string& line);
	//void                     process_patch_instructions(const patch_instruction& line);
	void process_patch_instructions(const std::list<patch_instruction>& tokens);

	ActorData readConfig(const std::string& folder);
	void*     patch(NPC::patch_instruction line, RE::TESNPC* curobj);
	void*     patchActor(NPC::patch_instruction line, RE::Character* curActObj);
	void      setbEnableSetLevelDirectlyByPCMult();
	bool      findObject(patch_instruction line, RE::TESNPC* curobj);
	void*     readConfigHook(const std::string& folder);
}

#endif
