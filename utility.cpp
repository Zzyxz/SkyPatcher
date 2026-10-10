#include "utility.h"
#include "enginelists.h"
#include <mutex>
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace {
const std::regex& cached_regex(const std::string& pattern, std::regex::flag_type flags)
{
	static std::unordered_map<std::string, std::regex> cache;
	auto it = cache.find(pattern);
	if (it == cache.end()) {
		it = cache.emplace(pattern, std::regex(pattern, flags)).first;
	}
	return it->second;
}

struct ModLookupEntry
{
	const RE::TESFile* file = nullptr;
	RE::FormID         prefix = 0;
	bool               usable = false;
};

const ModLookupEntry& cached_mod_lookup(RE::TESDataHandler* dataHandler, std::string_view modName)
{
	static RE::TESDataHandler*                            owner = nullptr;
	static std::unordered_map<std::string, ModLookupEntry> cache;
	static std::string                                    key;

	if (owner != dataHandler) {
		cache.clear();
		owner = dataHandler;
	}

	key.assign(modName);
	for (auto& c : key) {
		if (c >= 'A' && c <= 'Z') {
			c = static_cast<char>(c - 'A' + 'a');
		}
	}

	auto it = cache.find(key);
	if (it == cache.end()) {
		ModLookupEntry entry;
		entry.file = dataHandler->LookupModByName(modName);
		entry.prefix = dataHandler->LookupFormID(0, modName);
		entry.usable = entry.file && entry.file->GetCompileIndex() != 0xFF;
		it = cache.emplace(key, entry).first;
	}
	return it->second;
}
}

RE::BIPED_MODEL::BipedObjectSlot getBipedObjectSlot(int slot)
{
	switch (slot) {
	case 0:
		return RE::BIPED_MODEL::BipedObjectSlot::kHead;
	case 1:
		return RE::BIPED_MODEL::BipedObjectSlot::kHair;
	case 2:
		return RE::BIPED_MODEL::BipedObjectSlot::kBody;
	case 3:
		return RE::BIPED_MODEL::BipedObjectSlot::kHands;
	case 4:
		return RE::BIPED_MODEL::BipedObjectSlot::kForearms;
	case 5:
		return RE::BIPED_MODEL::BipedObjectSlot::kAmulet;
	case 6:
		return RE::BIPED_MODEL::BipedObjectSlot::kRing;
	case 7:
		return RE::BIPED_MODEL::BipedObjectSlot::kFeet;
	case 8:
		return RE::BIPED_MODEL::BipedObjectSlot::kCalves;
	case 9:
		return RE::BIPED_MODEL::BipedObjectSlot::kShield;
	case 10:
		return RE::BIPED_MODEL::BipedObjectSlot::kTail;
	case 11:
		return RE::BIPED_MODEL::BipedObjectSlot::kLongHair;
	case 12:
		return RE::BIPED_MODEL::BipedObjectSlot::kCirclet;
	case 13:
		return RE::BIPED_MODEL::BipedObjectSlot::kEars;
	case 14:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMouth;
	case 15:
		return RE::BIPED_MODEL::BipedObjectSlot::kModNeck;
	case 16:
		return RE::BIPED_MODEL::BipedObjectSlot::kModChestPrimary;
	case 17:
		return RE::BIPED_MODEL::BipedObjectSlot::kModBack;
	case 18:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMisc1;
	case 19:
		return RE::BIPED_MODEL::BipedObjectSlot::kModPelvisPrimary;
	case 20:
		return RE::BIPED_MODEL::BipedObjectSlot::kDecapitateHead;
	case 21:
		return RE::BIPED_MODEL::BipedObjectSlot::kDecapitate;
	case 22:
		return RE::BIPED_MODEL::BipedObjectSlot::kModPelvisSecondary;
	case 23:
		return RE::BIPED_MODEL::BipedObjectSlot::kModLegRight;
	case 24:
		return RE::BIPED_MODEL::BipedObjectSlot::kModLegLeft;
	case 25:
		return RE::BIPED_MODEL::BipedObjectSlot::kModFaceJewelry;
	case 26:
		return RE::BIPED_MODEL::BipedObjectSlot::kModChestSecondary;
	case 27:
		return RE::BIPED_MODEL::BipedObjectSlot::kModShoulder;
	case 28:
		return RE::BIPED_MODEL::BipedObjectSlot::kModArmLeft;
	case 29:
		return RE::BIPED_MODEL::BipedObjectSlot::kModArmRight;
	case 30:
		return RE::BIPED_MODEL::BipedObjectSlot::kModMisc2;
	case 31:
		return RE::BIPED_MODEL::BipedObjectSlot::kFX01;
	default:
		return RE::BIPED_MODEL::BipedObjectSlot::kNone;
	}
}

AttributePoints calculateAttributePoints(int level, const StatWeights& weights, int points)
{
	AttributePoints result{};

	int totalPoints = points * (level - 1);

	float healthWeight = weights.healthWeight;
	float staminaWeight = weights.staminaWeight;
	float magickaWeight = weights.magickaWeight;

	float weightSum = healthWeight + staminaWeight + magickaWeight;
	if (weightSum == 0.0f) {
		healthWeight = 1.0f;
		staminaWeight = 1.0f;
		magickaWeight = 1.0f;
		weightSum = 3.0f;
	}

	result.health = static_cast<int>(totalPoints * healthWeight / weightSum);
	result.magicka = static_cast<int>(totalPoints * magickaWeight / weightSum);
	result.stamina = totalPoints - result.health - result.magicka;

	return result;
}

bool IsPluginInstalled(const char* name)
{
	return ngcompat::IsPluginLoaded(RE::TESDataHandler::GetSingleton(), name);
}

RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		const std::string_view modName{ identifier.data(), delimiter };
		const std::string      modForm = identifier.substr(delimiter + 1);

		uint32_t rawFormID = 0;
		try {
			rawFormID = static_cast<uint32_t>(std::stoul(modForm, nullptr, 16));
		} catch (const std::exception& e) {
			static std::mutex                      warnedLock;
			static std::unordered_set<std::string> warned;
			const std::scoped_lock                 lock(warnedLock);
			if (warned.insert(identifier).second) {
				logger::warn("Invalid form identifier '{}': {}", identifier, e.what());
			}
			return nullptr;
		}

		const ModLookupEntry& mod = cached_mod_lookup(dataHandler, modName);

		uint32_t formID = rawFormID & 0xFFFFFF;
		if (mod.file && mod.file->IsLight())
			formID = rawFormID & 0xFFF;

		if (REL::Module::IsVR()) {
			return ngcompat::LookupFormVR(dataHandler, formID, modName);
		}
		if (!mod.usable)
			return nullptr;
		const RE::FormID fullFormID = mod.prefix + formID;
		return fullFormID ? RE::TESForm::LookupByID(fullFormID) : nullptr;
	} else {
		return RE::TESForm::LookupByEditorID(identifier);
	}
	return nullptr;
}

RE::FormID BuildFormIDFromIdentifier(const std::string& identifier)
{
	auto* dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler)
		return 0x0;
	try {
	auto delimiter = identifier.find('|');
	if (delimiter == std::string::npos) {
		logger::warn("Invalid identifier format: {}", identifier);
		return 0x0;
	}
	std::string   modName = identifier.substr(0, delimiter);
	std::string   modForm = identifier.substr(delimiter + 1);
	std::uint32_t rawFormID = std::stoul(modForm, nullptr, 16);
	auto*         mod = dataHandler->LookupModByName(modName.c_str());
	if (!mod) {
		logger::warn("Mod not found: {}", modName);
		return 0x0;
	}
	std::uint32_t fullFormID = 0;
	if (mod->IsLight()) {
		fullFormID = 0xFE000000 | (static_cast<std::uint32_t>(mod->GetSmallFileCompileIndex()) << 12) | (rawFormID & 0xFFF);
	} else {
		fullFormID = (static_cast<std::uint32_t>(mod->GetCompileIndex()) << 24) | (rawFormID & 0xFFFFFF);
	}
	logger::debug("Built FormID {:08X} from {}", fullFormID, identifier);
	return fullFormID;
	} catch (const std::exception& e) {
		logger::warn("Invalid form identifier '{}': {}", identifier, e.what());
		return 0x0;
	}
}






std::string toLowerCase(std::string pString)
{
	std::transform(pString.begin(), pString.end(), pString.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	return pString;
}

std::string normalizePath(const std::string& path)
{
	std::string normalized = path;
	std::replace(normalized.begin(), normalized.end(), '\\', '/');
	return normalized;
}

std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> splitRelationNumber(const std::string& input)
{
	const std::regex&         re = cached_regex(R"(([<>]=?|[<>])\s*(\d+))", std::regex::ECMAScript);
	std::smatch               match;
	std::vector<std::string>  result;
	result.reserve(2);

	if (std::regex_search(input, match, re)) {
		result.emplace_back(match[2].first, match[2].second);
		result.emplace_back(match[1].first, match[1].second);
	} else {
		const std::regex& digitRe = cached_regex(R"(\d+)", std::regex::ECMAScript);
		if (std::regex_search(input, digitRe)) {
			result.push_back(input);
		} else {
			result.push_back("none");
		}
	}

	return result;
}

void extractValueString(const std::string& line, const std::string& regex_pattern_str, std::string& target_value)
{
	const std::regex& regex_pattern = cached_regex(regex_pattern_str, std::regex::ECMAScript);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);

	if (match.empty() || match[1].str().empty()) {
		target_value = "none";
	} else {
		std::string value = match[1].str();

		value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), ::isspace));

		value.erase(std::find_if_not(value.rbegin(), value.rend(), ::isspace).base(), value.end());
		target_value = value;
	}
}

void extractToArr2D(const std::string& line, const std::string& regex_pattern_str, std::vector<std::vector<std::string>>& target_value)
{
	const std::regex& regex_pattern = cached_regex(regex_pattern_str, std::regex::icase);
	std::smatch match;
	std::regex_search(line, match, regex_pattern);
	std::vector<std::string> arr;

	if (match.empty() || match[1].str().empty()) {
	} else {
		std::string valueLine = match[1].str();
		size_t      startPos = valueLine.find("=") + 1;
		size_t      pos = 0;
		std::string token;
		while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
			token = valueLine.substr(startPos, pos - startPos);
			token = trim(token);
			arr.push_back(token);
			startPos = pos + 1;
		}
		token = valueLine.substr(startPos);
		token = trim(token);
		arr.push_back(token);

		std::vector<std::vector<std::string>> arr2D(arr.size());

		for (size_t i = 0; i < arr.size(); i++) {
			std::vector<std::string> splitArr;
			size_t                   start = 0;
			while (start <= arr[i].size()) {
				const size_t innerPos = arr[i].find('~', start);
				const size_t end = (innerPos == std::string::npos) ? arr[i].size() : innerPos;
				splitArr.push_back(trim(arr[i].substr(start, end - start)));
				if (innerPos == std::string::npos) {
					break;
				}
				start = innerPos + 1;
			}
			arr2D[i] = std::move(splitArr);
		}
		target_value = std::move(arr2D);
	}
}

	void moveEffectToPosition0(RE::BSTArray<RE::Effect*>& effects, const RE::Effect* targetEffect)
	{
		if (!targetEffect || !targetEffect->baseEffect) {
			return;
		}
		auto it = std::find_if(effects.begin(), effects.end(), [targetEffect](const RE::Effect* x) {
			return x && x->baseEffect && x->baseEffect->formID == targetEffect->baseEffect->formID;
		});

	if (it != effects.end()) {
		std::rotate(effects.begin(), it, it + 1);
	}
}

void addAltTexture(RE::TESModelTextureSwap* curobj, RE::TESModelTextureSwap::AlternateTexture newTex)
{
	EngineLists::AddAlternateTexture(curobj, newTex.textureSet, newTex.name3D.c_str(), newTex.index3D);
}

void clearAltTextures(RE::TESModelTextureSwap* curobj)
{
	EngineLists::ClearAlternateTextures(curobj);
}

bool removeAltTextureByTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToRemove)
{
	return EngineLists::RemoveAlternateTextures(curobj, [textureSetToRemove](const RE::TESModelTextureSwap::AlternateTexture& altTex) {
		return altTex.textureSet == textureSetToRemove;
	}) > 0;
}

bool findTextureSet(RE::TESModelTextureSwap* curobj, RE::BGSTextureSet* textureSetToFind)
{
	if (!curobj->alternateTextures || curobj->numAlternateTextures == 0) {
		return false;
	}

	auto iter = std::find_if(curobj->alternateTextures,
		curobj->alternateTextures + curobj->numAlternateTextures,
		[textureSetToFind](const RE::TESModelTextureSwap::AlternateTexture& altTex) {
			return altTex.textureSet == textureSetToFind;
		});

	return iter != curobj->alternateTextures + curobj->numAlternateTextures;
}

RE::TESClass* copyClass(RE::TESClass* aobj)
{
	auto       form = aobj;  
	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESClass>();
	auto       newClass = factory->Create();
	RE::FormID newForm = newClass->GetFormID();
	memcpy(newClass, form, sizeof(*form));
	newClass->formID = newForm;
	return newClass;
}

RE::BGSKeyword* createKeyword()
{

	auto       factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
	auto       newKeyword = factory->Create();
	newKeyword->formEditorID = "SkyPatcher_RandomVisualStyleAssigned";
	return newKeyword;
}

 RE::ActorValue StringToActorValue(const std::string& valueString) {
	static const std::unordered_map<std::string, RE::ActorValue> actorValueMap = {
		{"null", RE::ActorValue::kNone},
		{"aggression", RE::ActorValue::kAggression},
		{"confidence", RE::ActorValue::kConfidence},
		{"energy", RE::ActorValue::kEnergy},
		{"morality", RE::ActorValue::kMorality},
		{"mood", RE::ActorValue::kMood},
		{"assistance", RE::ActorValue::kAssistance},
		{"onehanded", RE::ActorValue::kOneHanded},
		{"twohanded", RE::ActorValue::kTwoHanded},
		{"archery", RE::ActorValue::kArchery},
		{"block", RE::ActorValue::kBlock},
		{"smithing", RE::ActorValue::kSmithing},
		{"heavyarmor", RE::ActorValue::kHeavyArmor},
		{"lightarmor", RE::ActorValue::kLightArmor},
		{"pickpocket", RE::ActorValue::kPickpocket},
		{"lockpicking", RE::ActorValue::kLockpicking},
		{"sneak", RE::ActorValue::kSneak},
		{"alchemy", RE::ActorValue::kAlchemy},
		{"speech", RE::ActorValue::kSpeech},
		{"alteration", RE::ActorValue::kAlteration},
		{"conjuration", RE::ActorValue::kConjuration},
		{"destruction", RE::ActorValue::kDestruction},
		{"illusion", RE::ActorValue::kIllusion},
		{"restoration", RE::ActorValue::kRestoration},
		{"enchanting", RE::ActorValue::kEnchanting},
		{"health", RE::ActorValue::kHealth},
		{"magicka", RE::ActorValue::kMagicka},
		{"stamina", RE::ActorValue::kStamina},
		{"healrate", RE::ActorValue::kHealRate},
		{"magickarate", RE::ActorValue::kMagickaRate},
		{"staminarate", RE::ActorValue::kStaminaRate},
		{"speedmult", RE::ActorValue::kSpeedMult},
		{"inventoryweight", RE::ActorValue::kInventoryWeight},
		{"carryweight", RE::ActorValue::kCarryWeight},
		{"criticalchance", RE::ActorValue::kCriticalChance},
		{"meleedamage", RE::ActorValue::kMeleeDamage},
		{"unarmeddamage", RE::ActorValue::kUnarmedDamage},
		{"mass", RE::ActorValue::kMass},
		{"voicepoints", RE::ActorValue::kVoicePoints},
		{"voicerate", RE::ActorValue::kVoiceRate},
		{"damageresist", RE::ActorValue::kDamageResist},
		{"poisonresist", RE::ActorValue::kPoisonResist},
		{"resistfire", RE::ActorValue::kResistFire},
		{"resistshock", RE::ActorValue::kResistShock},
		{"resistfrost", RE::ActorValue::kResistFrost},
		{"resistmagic", RE::ActorValue::kResistMagic},
		{"resistdisease", RE::ActorValue::kResistDisease},
		{"perceptioncondition", RE::ActorValue::kPerceptionCondition},
		{"endurancecondition", RE::ActorValue::kEnduranceCondition},
		{"leftattackcondition", RE::ActorValue::kLeftAttackCondition},
		{"rightattackcondition", RE::ActorValue::kRightAttackCondition},
		{"leftmobilitycondition", RE::ActorValue::kLeftMobilityCondition},
		{"rightmobilitycondition", RE::ActorValue::kRightMobilityCondition},
		{"braincondition", RE::ActorValue::kBrainCondition},
		{"paralysis", RE::ActorValue::kParalysis},
		{"invisibility", RE::ActorValue::kInvisibility},
		{"nightvision", RE::ActorValue::kNightEye},
		{"wardpower", RE::ActorValue::kWardPower},
		{"rightitemcharge", RE::ActorValue::kRightItemCharge},
		{"armorperks", RE::ActorValue::kArmorPerks},
		{"shieldperks", RE::ActorValue::kShieldPerks},
		{"warddeflection", RE::ActorValue::kWardDeflection},
		{"variable01", RE::ActorValue::kVariable01},
		{"variable02", RE::ActorValue::kVariable02},
		{"variable03", RE::ActorValue::kVariable03},
		{"variable04", RE::ActorValue::kVariable04},
		{"variable05", RE::ActorValue::kVariable05},
		{"variable06", RE::ActorValue::kVariable06},
		{"variable07", RE::ActorValue::kVariable07},
		{"variable08", RE::ActorValue::kVariable08},
		{"variable09", RE::ActorValue::kVariable09},
		{"variable10", RE::ActorValue::kVariable10},
		{"bowstaggerbonus", RE::ActorValue::kBowStaggerBonus},
		{"telekinesis", RE::ActorValue::kTelekinesis},
		{"favoractive", RE::ActorValue::kFavorActive},
		{"favorsperday", RE::ActorValue::kFavorsPerDay},
		{"favorsperdaytimer", RE::ActorValue::kFavorsPerDayTimer},
		{"leftitemcharge", RE::ActorValue::kLeftItemCharge},
		{"absorbchance", RE::ActorValue::kAbsorbChance},
		{"blindness", RE::ActorValue::kBlindness},
		{"fame", RE::ActorValue::kFame},
		{"infamy", RE::ActorValue::kInfamy},
		{"jumpingbonus", RE::ActorValue::kJumpingBonus},
		{"onehandedmodifier", RE::ActorValue::kOneHandedModifier},
		{"twohandedmodifier", RE::ActorValue::kTwoHandedModifier},
		{"marksmanmodifier", RE::ActorValue::kMarksmanModifier},
		{"blockmodifier", RE::ActorValue::kBlockModifier},
		{"smithingmodifier", RE::ActorValue::kSmithingModifier},
		{"heavyarmormodifier", RE::ActorValue::kHeavyArmorModifier},
		{"lightarmormodifier", RE::ActorValue::kLightArmorModifier},
		{"pickpocketmodifier", RE::ActorValue::kPickpocketModifier},
		{"lockpickingmodifier", RE::ActorValue::kLockpickingModifier},
		{"sneakingmodifier", RE::ActorValue::kSneakingModifier},
		{"alchemymodifier", RE::ActorValue::kAlchemyModifier},
		{"speechcraftmodifier", RE::ActorValue::kSpeechcraftModifier},
		{"alterationmodifier", RE::ActorValue::kAlterationModifier},
		{"conjurationmodifier", RE::ActorValue::kConjurationModifier},
		{"destructionmodifier", RE::ActorValue::kDestructionModifier},
		{"illusionmodifier", RE::ActorValue::kIllusionModifier},
		{"restorationmodifier", RE::ActorValue::kRestorationModifier},
		{"enchantingmodifier", RE::ActorValue::kEnchantingModifier},
		{"dragonrend", RE::ActorValue::kDragonRend},
		{"attackdamagemult", RE::ActorValue::kAttackDamageMult},
		{"healratemult", RE::ActorValue::kHealRateMult},
		{"magickaratemult", RE::ActorValue::kMagickaRateMult},
		{"staminaratemult", RE::ActorValue::kStaminaRateMult},
		{"werewolfperks", RE::ActorValue::kWerewolfPerks},
		{"vampireperks", RE::ActorValue::kVampirePerks},
		{"grabactoroffset", RE::ActorValue::kGrabActorOffset},
		{"grabbed", RE::ActorValue::kGrabbed},
		{"deprecated05", RE::ActorValue::kDEPRECATED05},
		{"reflectdamage", RE::ActorValue::kReflectDamage}
	};

	auto it = actorValueMap.find(valueString);
	if (it != actorValueMap.end()) {
		return it->second;
	}

	return RE::ActorValue::kNone;
}


bool replaceObjectInContainer(RE::TESContainer& container, RE::TESBoundObject* oldObject, RE::TESBoundObject* newObject)
{
	bool changed = false;
	container.ForEachContainerObject([&](RE::ContainerObject& entry) {
		if (entry.obj == oldObject) {
			entry.obj = newObject;
			changed = true;
		}
		return RE::BSContainer::ForEachResult::kContinue;
	});
	return changed;
}

bool changeContainerObjectCount(RE::TESContainer& container, RE::TESBoundObject* object, std::int32_t count)
{
	bool changed = false;
	container.ForEachContainerObject([&](RE::ContainerObject& entry) {
		if (!object || entry.obj == object) {
			entry.count = count;
			changed = true;
		}
		return RE::BSContainer::ForEachResult::kContinue;
	});
	return changed;
}

bool multiplyContainerObjectCount(RE::TESContainer& container, RE::TESBoundObject* object, float multiplier)
{
	bool changed = false;
	container.ForEachContainerObject([&](RE::ContainerObject& entry) {
		if (!object || entry.obj == object) {
			entry.count = static_cast<std::int32_t>(std::round(static_cast<float>(entry.count) * multiplier));
			changed = true;
		}
		return RE::BSContainer::ForEachResult::kContinue;
	});
	return changed;
}

void setMoralityLevel(RE::TESNPC* npc, RE::ACTOR_MORALITY level)
{
	if (!npc) return;
	const auto value = static_cast<std::uint8_t>(level);
	npc->aiData.morality1 = (value & 1) != 0;
	npc->aiData.morality2 = (value & 2) != 0;
}

void setMoodLevel(RE::TESNPC* npc, RE::ACTOR_MOOD level)
{
	if (!npc) return;
	const auto value = static_cast<std::uint8_t>(level);
	npc->aiData.mood1 = (value & 1) != 0;
	npc->aiData.mood2 = (value & 2) != 0;
	npc->aiData.mood3 = (value & 4) != 0;
}

RE::FIGHT_REACTION GetFightReaction(const std::string& value)
{
	std::string v = value;
	std::transform(v.begin(), v.end(), v.begin(), ::tolower);

	if (v == "neutral" || v == "0") {
		return RE::FIGHT_REACTION::kNeutral;
	}
	if (v == "enemy" || v == "1") {
		return RE::FIGHT_REACTION::kEnemy;
	}
	if (v == "ally" || v == "2") {
		return RE::FIGHT_REACTION::kAlly;
	}
	if (v == "friend" || v == "3") {
		return RE::FIGHT_REACTION::kFriend;
	}

	return RE::FIGHT_REACTION::kNeutral;
}



bool createsFaceNPCLoop(const RE::TESNPC* target, const RE::TESNPC* source)
{
	int depth = 0;
	for (const auto* npc = source; npc; npc = npc->faceNPC) {
		if (npc == target || ++depth > 64) {
			return true;
		}
	}
	return false;
}

std::optional<float> tryParseFloat(const std::string& text)
{
	try {
		return std::stof(text);
	} catch (const std::exception&) {
		return std::nullopt;
	}
}

void copyKeywordArray(RE::BGSKeywordForm* target, const RE::BGSKeywordForm* source)
{
	if (!target || !source) {
		return;
	}
	RE::BGSKeyword** copy = nullptr;
	if (source->keywords && source->numKeywords > 0) {
		copy = RE::calloc<RE::BGSKeyword*>(source->numKeywords);
		if (!copy) {
			return;
		}
		std::copy_n(source->keywords, source->numKeywords, copy);
	}
	target->keywords = copy;
	target->numKeywords = copy ? source->numKeywords : 0;
}
