#ifndef UTILITY
#define UTILITY

using namespace std;

#include "PCH.h"
#include <string>

// Function to reset all flags
template <typename Enum>
Enum& resetAllFlags(Enum& flags)
{
	flags = static_cast<Enum>(0);
	return flags;
}

struct StatWeights
{
	int healthWeight;
	int staminaWeight;
	int magickaWeight;
};

struct AttributePoints
{
	int health;
	int stamina;
	int magicka;
};
bool                             IsPluginInstalled(const char* name);
RE::ActorValue StringToActorValue(const std::string& valueString);
RE::TESForm*                     GetFormFromIdentifier(const std::string& identifier);
RE::FormID BuildFormIDFromIdentifier(const std::string& identifier);

std::string                      toLowerCase(std::string pString);
std::string                      trim(const std::string& str);
std::vector<std::string>         splitRelationNumber(const std::string& input);
AttributePoints                  calculateAttributePoints(int level, const StatWeights& weights, int points);
RE::BIPED_MODEL::BipedObjectSlot getBipedObjectSlot(int slot);
void                             extractValueString(const std::string& line, const std::string& regex_pattern_str, std::string& target_value);
void                             moveEffectToPosition0(RE::BSTArray<RE::Effect*>& effects, const RE::Effect* targetEffect);
void                             extractToArr2D(const std::string& line, const std::string& regex_pattern_str, std::vector<std::vector<std::string>>& target_value);
void                             addAltTexture(RE::TESModelTextureSwap* curobj, RE::TESModelTextureSwap::AlternateTexture newTex);
void                             clearAltTextures(RE::TESModelTextureSwap* curobj);
bool                             removeAltTextureByTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToRemove);
bool                             findTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToFind);
RE::TESClass*                    copyClass(RE::TESClass* aobj);
RE::BGSKeyword*					 createKeyword();
std::string                      normalizePath(const std::string& path);
<<<<<<< HEAD

=======
std::string                      toHex(const std::string& str);
RE::TESObjectREFR*               createREFR();
RE::FIGHT_REACTION               GetFightReaction(const std::string& value);
>>>>>>> origin/new-main

#endif
