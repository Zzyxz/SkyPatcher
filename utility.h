#ifndef UTILITY
#define UTILITY

using namespace std;


#include <string>
#include "PCH.h"

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
bool IsPluginInstalled(const char* name);
RE::TESForm* GetFormFromIdentifier(const std::string& identifier);
std::string  toLowerCase(std::string pString);
std::string  trim(const std::string& str);
std::vector<std::string> splitRelationNumber(const std::string& input);
AttributePoints  calculateAttributePoints(int level, const StatWeights& weights, int points);
RE::BIPED_MODEL::BipedObjectSlot getBipedObjectSlot(int slot);
void  extractData(const std::string& line, const std::string& pattern, std::vector<std::string>& destination);
void  extractMultiDataInt(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag, std::vector<int>& pcMultFlag_min_values, std::vector<int>& pcMultFlag_max_values);
void extractMultiDataFloat(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag, std::vector<float>& pcMultFlag_min_values, std::vector<float>& pcMultFlag_max_values);
void                             extractMultiDataString(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
								std::vector<std::string>& pcMultFlag_min_values, std::vector<std::string>& pcMultFlag_max_values);

#endif
