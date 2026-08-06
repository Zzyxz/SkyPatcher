#include "npc.h"
#include "formfilter.h"
#include "parsetable.h"
#include <algorithm>
#include <unordered_map>

namespace NPC
{

	std::list<patch_instruction> hooktokens;
	ActorData aData;
	bool      bEnableSetLevelDirectlyByPCMult = false;

	namespace
	{
#define NPC_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kNpcKeys[] = {
			{ "filterByNpcs", sp::FieldKind::List, NPC_F(object) },
			{ "filterByNpcsExcluded", sp::FieldKind::List, NPC_F(objectExcluded) },
			{ "filterByDefaultOutfits", sp::FieldKind::List, NPC_F(filterByOutfit) },
			{ "filterByKeywords", sp::FieldKind::List, NPC_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, NPC_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, NPC_F(keywordsExcluded) },
			{ "filterByRaces", sp::FieldKind::List, NPC_F(races) },
			{ "restrictToRaces", sp::FieldKind::List, NPC_F(restrictToRaces) },
			{ "filterByClass", sp::FieldKind::List, NPC_F(filterClass) },
			{ "filterByCombatStyle", sp::FieldKind::List, NPC_F(filterCombatStyle) },
			{ "filterByClassExclude", sp::FieldKind::List, NPC_F(filterClassExclude) },
			{ "filterByFactions", sp::FieldKind::List, NPC_F(filterFactions) },
			{ "filterByFactionsOr", sp::FieldKind::List, NPC_F(filterFactionsOr) },
			{ "filterByFactionsExcluded", sp::FieldKind::List, NPC_F(filterFactionsExcluded) },
			{ "restrictToMaleModelContains", sp::FieldKind::ListAssign, NPC_F(restrictToMaleModelContains) },
			{ "setAutoCalcStats", sp::FieldKind::Value, NPC_F(calcStats) },
			{ "setPcLevelMult", sp::FieldKind::MultiInt, NPC_F(pcMultFlag), NPC_F(PCvalues1), NPC_F(PCvalues2) },
			{ "setEssential", sp::FieldKind::Value, NPC_F(kEssential) },
			{ "setProtected", sp::FieldKind::Value, NPC_F(kProtected) },
			{ "levelRange", sp::FieldKind::LevelRange, NPC_F(level_min), NPC_F(level_max) },
			{ "changeStats", sp::FieldKind::MultiFloat, NPC_F(avifs), NPC_F(values1), NPC_F(values2) },
			{ "changeSkills", sp::FieldKind::MultiInt, NPC_F(skills), NPC_F(skillvalues1), NPC_F(skillvalues2) },
			{ "restrictToSkill", sp::FieldKind::Arr2D, NPC_F(skillRestrict) },
			{ "factionsToAdd", sp::FieldKind::FactionRanks, NPC_F(factionsToAdd), NPC_F(factionsToAddRank1), NPC_F(factionsToAddRank2) },
			{ "keywordsToAdd", sp::FieldKind::List, NPC_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, NPC_F(keywordsToRemove) },
			{ "factionsToRemove", sp::FieldKind::List, NPC_F(factionsToRemove) },
			{ "perksToAdd", sp::FieldKind::List, NPC_F(perksToAdd) },
			{ "objectsToAdd", sp::FieldKind::ObjectsToAdd, NPC_F(objectsToAdd), NPC_F(objectsToAddValue) },
			{ "ObjectsToRemove", sp::FieldKind::List, NPC_F(objectsToRemove) },
			{ "objectsToRemove", sp::FieldKind::List, NPC_F(objectsToRemoveNew) },
			{ "outfitDefault", sp::FieldKind::Value, NPC_F(outfitDefault) },
			{ "outfitSleep", sp::FieldKind::Value, NPC_F(outfitSleep) },
			{ "deathItem", sp::FieldKind::Value, NPC_F(deathItem) },
			{ "skin", sp::FieldKind::Value, NPC_F(skin) },
			{ "race", sp::FieldKind::Value, NPC_F(race) },
			{ "class", sp::FieldKind::Value, NPC_F(Class) },
			{ "level", sp::FieldKind::Value, NPC_F(level) },
			{ "calcLevelMin", sp::FieldKind::Value, NPC_F(calcLevelMin) },
			{ "calcLevelMax", sp::FieldKind::Value, NPC_F(calcLevelMax) },
			{ "filterByGender", sp::FieldKind::Value, NPC_F(isFemale) },
			{ "restrictToGender", sp::FieldKind::Value, NPC_F(restrictGender) },
			{ "restrictToCombatStyle", sp::FieldKind::ListAssign, NPC_F(restrictToCombatStyle) },
			{ "restrictToKeywords", sp::FieldKind::ListAssign, NPC_F(restrictToKeyword) },
			{ "healthBonus", sp::FieldKind::Value, NPC_F(healthBonus) },
			{ "staminaBonus", sp::FieldKind::Value, NPC_F(staminaBonus) },
			{ "magickaBonus", sp::FieldKind::Value, NPC_F(magickaBonus) },
			{ "filterByModNames", sp::FieldKind::ListAssign, NPC_F(modNames) },
			{ "shoutsToAdd", sp::FieldKind::List, NPC_F(shoutsToAdd) },
			{ "shoutsToRemove", sp::FieldKind::List, NPC_F(shoutsToRemove) },
			{ "levSpellsToAdd", sp::FieldKind::List, NPC_F(levSpellsToAdd) },
			{ "levSpellsToRemove", sp::FieldKind::List, NPC_F(levSpellsToRemove) },
			{ "spellsToAdd", sp::FieldKind::List, NPC_F(spellsToAdd) },
			{ "spellsToRemove", sp::FieldKind::List, NPC_F(spellsToRemove) },
			{ "fullName", sp::FieldKind::ValueTilde, NPC_F(fullName) },
			{ "speedMultiplier", sp::FieldKind::Value, NPC_F(speedMultiplier) },
			{ "raceAttack", sp::FieldKind::Value, NPC_F(attackRace) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, NPC_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, NPC_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, NPC_F(filterByEditorIdContainsExcluded) },
			{ "clearClassFromAttributes", sp::FieldKind::Value, NPC_F(clearClassFromAttributes) },
			{ "filterByPCLevelMult", sp::FieldKind::Value, NPC_F(filterPCMult) },
			{ "filterByAutoCalc", sp::FieldKind::Value, NPC_F(filterAutoCalc) },
			{ "filterByEssential", sp::FieldKind::Value, NPC_F(filterEssential) },
			{ "filterByProtected", sp::FieldKind::Value, NPC_F(filterProtected) },
			{ "voiceType", sp::FieldKind::Value, NPC_F(voiceType) },
			{ "restrictToVoiceType", sp::FieldKind::List, NPC_F(restrictToVoiceType) },
			{ "setTemplateFlags", sp::FieldKind::ListAssign, NPC_F(setTemplateFlags) },
			{ "removeTemplateFlags", sp::FieldKind::ListAssign, NPC_F(removeTemplateFlags) },
			{ "setFlags", sp::FieldKind::ListAssign, NPC_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, NPC_F(removeFlags) },
			{ "clearInventory", sp::FieldKind::Value, NPC_F(clearInventory) },
			{ "removeInventoryObjectsByCount", sp::FieldKind::Arr2D, NPC_F(removeContainerObjectsByCount) },
			{ "removeInventoryObjectsByKeywords", sp::FieldKind::List, NPC_F(removeContainerObjectsByKeyword) },
			{ "addOnceToInventory", sp::FieldKind::Arr2D, NPC_F(addedContainerObjectsOnce) },
			{ "setRandomCombatStyle", sp::FieldKind::Arr2D, NPC_F(rcslist) },
			{ "copyVisualStyle", sp::FieldKind::Value, NPC_F(copyVisualStyle) },
			{ "rvsRestrictToTraits", sp::FieldKind::Value, NPC_F(rvsRestrictToTraits) },
			{ "setRandomVisualStyle", sp::FieldKind::Arr2D, NPC_F(setRandomVisualStyle) },
			{ "aggressionRadiusRanges", sp::FieldKind::Arr2D, NPC_F(aggressionRadiusRanges) },
			{ "addRandomVisualStyle", sp::FieldKind::Arr2D, NPC_F(addRandomVisualStyle) },
			{ "weight", sp::FieldKind::Value, NPC_F(weight) },
			{ "height", sp::FieldKind::Value, NPC_F(height) },
			{ "setAggression", sp::FieldKind::Value, NPC_F(setAggression) },
			{ "setAssistance", sp::FieldKind::Value, NPC_F(setAssistance) },
			{ "setConfidence", sp::FieldKind::Value, NPC_F(setConfidence) },
			{ "setMood", sp::FieldKind::Value, NPC_F(setMood) },
			{ "setMorality", sp::FieldKind::Value, NPC_F(setMorality) },
			{ "aggressionRadiusBehavior", sp::FieldKind::Value, NPC_F(aggressionRadiusBehavior) },
			{ "attackDataToRemove", sp::FieldKind::ListAssign, NPC_F(removeAttackData) },
			{ "restrictToFlags", sp::FieldKind::ListAssign, NPC_F(restrictToFlags) },
			{ "restrictToTemplateFlags", sp::FieldKind::ListAssign, NPC_F(restrictToTemplateFlags) },
			{ "haircolor", sp::FieldKind::Value, NPC_F(haircolor) },
			{ "objectsToReplace", sp::FieldKind::Arr2D, NPC_F(objectsToReplace) },
			{ "attackDataToAdd", sp::FieldKind::AttackData3D, NPC_F(addedAttackData) },
			{ "attackDataToChange", sp::FieldKind::AttackData3D, NPC_F(attackDataToChange) },
			{ "changeBaseObject", sp::FieldKind::Value, NPC_F(baseObject) },
		};

#undef NPC_F

		const sp::KeyTable& npcKeyTable()
		{
			static const sp::KeyTable table(kNpcKeys, std::size(kNpcKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;
		npcKeyTable().parse(line, &l);
		return l;
	}


	namespace
	{
		bool liveScalarFilter(const std::string& a_value)
		{
			return !a_value.empty() && a_value != "none";
		}

		bool npcRuleInvalidatesIndex(const patch_instruction& line)
		{
			return !line.keywordsToAdd.empty() || !line.keywordsToRemove.empty() ||
				!line.factionsToAdd.empty() || !line.factionsToRemove.empty() ||
				liveScalarFilter(line.outfitDefault) || liveScalarFilter(line.race) ||
				liveScalarFilter(line.Class) || liveScalarFilter(line.clearClassFromAttributes);
		}

		void buildNpcIndex(const RE::BSTArray<RE::TESNPC*>& a_array, formfilter::FormSignalIndex& a_index)
		{
			const std::uint32_t n = static_cast<std::uint32_t>(a_array.size());
			for (std::uint32_t i = 0; i < n; ++i) {
				RE::TESNPC* npc = a_array[i];
				if (!npc) {
					continue;
				}
				formfilter::addKeywordSignals(a_index, static_cast<const RE::BGSKeywordForm*>(npc), i);
				if (npc->race) {
					a_index.add(formfilter::sigForm(npc->race->formID), i);
					formfilter::addKeywordSignals(a_index, static_cast<const RE::BGSKeywordForm*>(npc->race), i);
				}
				if (npc->npcClass) {
					a_index.add(formfilter::sigForm(npc->npcClass->formID), i);
				}
				if (npc->combatStyle) {
					a_index.add(formfilter::sigForm(npc->combatStyle->formID), i);
				}
				if (npc->defaultOutfit) {
					a_index.add(formfilter::sigForm(npc->defaultOutfit->formID), i);
				}
				for (const auto& rank : npc->factions) {
					if (rank.faction) {
						a_index.add(formfilter::sigForm(rank.faction->formID), i);
					}
				}
			}
		}
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		const auto start_time{ std::chrono::system_clock::now() };
		logger::debug("processing patch instructions");
		const auto                dataHandler = RE::TESDataHandler::GetSingleton();
		auto& NPCArray = dataHandler->GetFormArray<RE::TESNPC>();

		logger::debug("NPC count: {}", NPCArray.size());

		formfilter::FormSignalIndex index;
		bool                        indexBuilt = false;
		bool                        indexStale = false;
		std::vector<std::uint64_t>  keys;
		std::vector<std::uint32_t>  candidates;

		for (const auto& line : tokens) {
			indexStale = indexStale || npcRuleInvalidatesIndex(line);

			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					if (auto* currentform = GetFormFromIdentifier(npcstring); currentform && currentform->formType == RE::FormType::NPC) {
						patch(line, currentform->As<RE::TESNPC>());
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterClass.empty() && line.filterCombatStyle.empty() && line.filterFactions.empty() && line.filterFactionsOr.empty() && line.filterByOutfit.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::MatchProgram program = makeFilterView(line);
			const bool keyed = !indexStale && formfilter::programSignals(program, keys);
			if (keyed) {
				if (!indexBuilt) {
					buildNpcIndex(NPCArray, index);
					indexBuilt = true;
					++formfilter::stats().indexBuilds;
				}
				index.gather(keys, candidates);
			}
			const std::uint32_t scanCount = static_cast<std::uint32_t>(keyed ? candidates.size() : NPCArray.size());
			if (keyed) {
				++formfilter::stats().rulesKeyed;
				formfilter::stats().candidateVisits += scanCount;
			} else {
				if (indexStale) {
					++formfilter::stats().rulesStale;
				}
				++formfilter::stats().rulesScanned;
				formfilter::stats().scanVisits += scanCount;
			}

			for (std::uint32_t scanIdx = 0; scanIdx < scanCount; ++scanIdx) {
				const auto& curobj = NPCArray[keyed ? candidates[scanIdx] : scanIdx];
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}
				if (findObject(line, program, curobj)) {
					patch(line, curobj);
				}
			}
		}

		const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
		double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
		logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> NPC Patching (segment) finished in {} seconds", seconds);
	}


	namespace
	{
		using formfilter::ActionRow;

		constexpr ActionRow kAggressionRows[] = {
			{ "calmed", static_cast<std::uint32_t>(RE::ACTOR_AGGRESSION::kCalmed) },
			{ "unaggressive", static_cast<std::uint32_t>(RE::ACTOR_AGGRESSION::kUnaggressive) },
			{ "aggressive", static_cast<std::uint32_t>(RE::ACTOR_AGGRESSION::kAggressive) },
			{ "veryaggressive", static_cast<std::uint32_t>(RE::ACTOR_AGGRESSION::kVeryAggressive) },
			{ "frenzied", static_cast<std::uint32_t>(RE::ACTOR_AGGRESSION::kFrenzied) },
		};

		constexpr ActionRow kConfidenceRows[] = {
			{ "cowardly", static_cast<std::uint32_t>(RE::ACTOR_CONFIDENCE::kCowardly) },
			{ "cautious", static_cast<std::uint32_t>(RE::ACTOR_CONFIDENCE::kCautious) },
			{ "average", static_cast<std::uint32_t>(RE::ACTOR_CONFIDENCE::kAverage) },
			{ "brave", static_cast<std::uint32_t>(RE::ACTOR_CONFIDENCE::kBrave) },
			{ "foolhardy", static_cast<std::uint32_t>(RE::ACTOR_CONFIDENCE::kFoolhardy) },
		};

		constexpr ActionRow kAssistanceRows[] = {
			{ "helpsnobody", static_cast<std::uint32_t>(RE::ACTOR_ASSISTANCE::kHelpsNobody) },
			{ "helpsallies", static_cast<std::uint32_t>(RE::ACTOR_ASSISTANCE::kHelpsAllies) },
			{ "helpsfriends", static_cast<std::uint32_t>(RE::ACTOR_ASSISTANCE::kHelpsFriends) },
		};

		constexpr ActionRow kMoralityRows[] = {
			{ "anycrime", static_cast<std::uint32_t>(RE::ACTOR_MORALITY::kAnyCrime) },
			{ "violenceagainstenemy", static_cast<std::uint32_t>(RE::ACTOR_MORALITY::kViolenceAgainstEnemy) },
			{ "propertycrimeonly", static_cast<std::uint32_t>(RE::ACTOR_MORALITY::kPropertyCrimeOnly) },
			{ "nocrime", static_cast<std::uint32_t>(RE::ACTOR_MORALITY::kNoCrime) },
		};

		constexpr ActionRow kMoodRows[] = {
			{ "neutral", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kNeutral) },
			{ "angry", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kAngry) },
			{ "fear", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kFear) },
			{ "happy", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kHappy) },
			{ "sad", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kSad) },
			{ "surprised", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kSurprised) },
			{ "puzzled", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kPuzzled) },
			{ "disgusted", static_cast<std::uint32_t>(RE::ACTOR_MOOD::kDisgusted) },
		};

		constexpr ActionRow kAggroRadiusRows[] = {
			{ "attack", static_cast<std::uint32_t>(RE::ACTOR_AGGRO_RADIUS::kAttack), 0, "attack" },
			{ "warn", static_cast<std::uint32_t>(RE::ACTOR_AGGRO_RADIUS::kWarn), 0, "warn" },
			{ "attackandwarn", static_cast<std::uint32_t>(RE::ACTOR_AGGRO_RADIUS::kWarnAndAttack), 0, "attack and warn" },
		};

		constexpr ActionRow kSkillRows[] = {
			{ "onehanded", RE::TESNPC::Skills::kOneHanded },
			{ "twohanded", RE::TESNPC::Skills::kTwoHanded },
			{ "marksman", RE::TESNPC::Skills::kMarksman },
			{ "block", RE::TESNPC::Skills::kBlock },
			{ "smithing", RE::TESNPC::Skills::kSmithing },
			{ "heavyarmor", RE::TESNPC::Skills::kHeavyArmor },
			{ "lightarmor", RE::TESNPC::Skills::kLightArmor },
			{ "pickpocket", RE::TESNPC::Skills::kPickpocket },
			{ "lockpicking", RE::TESNPC::Skills::kLockpicking },
			{ "sneak", RE::TESNPC::Skills::kSneak },
			{ "alchemy", RE::TESNPC::Skills::kAlchemy },
			{ "speechcraft", RE::TESNPC::Skills::kSpeechcraft },
			{ "alteration", RE::TESNPC::Skills::kAlteration },
			{ "conjuration", RE::TESNPC::Skills::kConjuration },
			{ "destruction", RE::TESNPC::Skills::kDestruction },
			{ "illusion", RE::TESNPC::Skills::kIllusion },
			{ "restoration", RE::TESNPC::Skills::kRestoration },
			{ "enchanting", RE::TESNPC::Skills::kEnchanting },
		};

		constexpr ActionRow kTemplateFlagRows[] = {
			{ "traits", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits) },
			{ "stats", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats) },
			{ "factions", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions) },
			{ "spells", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells) },
			{ "aidata", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData) },
			{ "aipackages", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages) },
			{ "unused", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused) },
			{ "basedata", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData) },
			{ "inventory", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory) },
			{ "script", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript) },
			{ "aidefpacklist", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList) },
			{ "attackdata", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData) },
			{ "keywords", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords) },
			{ "copiedtemplate", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate) },
		};

		constexpr ActionRow kBaseFlagRows[] = {
			{ "female", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kFemale) },
			{ "essential", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kEssential) },
			{ "ischargenfacepreset", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset) },
			{ "respawn", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kRespawn) },
			{ "autocalcstats", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats) },
			{ "unique", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kUnique) },
			{ "doesntaffectstealthmeter", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter) },
			{ "pclevelmult", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult) },
			{ "usestemplate", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kUsesTemplate) },
			{ "calcfalltemplates", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates) },
			{ "protected", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kProtected) },
			{ "norumors", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kNoRumors) },
			{ "summonable", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kSummonable) },
			{ "doesntbleed", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kDoesntBleed) },
			{ "bleedoutoverride", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride) },
			{ "oppositegenderanims", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims) },
			{ "simpleactor", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kSimpleActor) },
			{ "loopedscript", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kLoopedScript) },
			{ "noactivation", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kNoActivation) },
			{ "loopedaudio", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kLoopedAudio) },
			{ "isghost", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kIsGhost) },
			{ "invulnerable", static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kInvulnerable) },
		};
		enum class StatSlot : std::uint32_t
		{
			Health,
			Stamina,
			Magicka
		};
		enum class StatMode : std::uint8_t
		{
			Set,
			Mult,
			Calc,
			ByClass
		};
		constexpr ActionRow kStatRows[] = {
			{ "health", static_cast<std::uint32_t>(StatSlot::Health), static_cast<std::uint8_t>(StatMode::Set) },
			{ "healthmult", static_cast<std::uint32_t>(StatSlot::Health), static_cast<std::uint8_t>(StatMode::Mult) },
			{ "stamina", static_cast<std::uint32_t>(StatSlot::Stamina), static_cast<std::uint8_t>(StatMode::Set) },
			{ "staminamult", static_cast<std::uint32_t>(StatSlot::Stamina), static_cast<std::uint8_t>(StatMode::Mult) },
			{ "magicka", static_cast<std::uint32_t>(StatSlot::Magicka), static_cast<std::uint8_t>(StatMode::Set) },
			{ "magickamult", static_cast<std::uint32_t>(StatSlot::Magicka), static_cast<std::uint8_t>(StatMode::Mult) },
			{ "calchealth", static_cast<std::uint32_t>(StatSlot::Health), static_cast<std::uint8_t>(StatMode::Calc) },
			{ "calcstamina", static_cast<std::uint32_t>(StatSlot::Stamina), static_cast<std::uint8_t>(StatMode::Calc) },
			{ "calcmagicka", static_cast<std::uint32_t>(StatSlot::Magicka), static_cast<std::uint8_t>(StatMode::Calc) },
			{ "byclassstamina", static_cast<std::uint32_t>(StatSlot::Stamina), static_cast<std::uint8_t>(StatMode::ByClass) },
			{ "byclassmagicka", static_cast<std::uint32_t>(StatSlot::Magicka), static_cast<std::uint8_t>(StatMode::ByClass) },
		};
		constexpr ActionRow kAttackFlagRows[] = {
			{ "ignoreweapon", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kIgnoreWeapon), 0, "IgnoreWeapon" },
			{ "bashattack", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kBashAttack), 0, "BashAttack" },
			{ "powerattack", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kPowerAttack), 0, "PowerAttack" },
			{ "chargeattack", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kChargeAttack), 0, "ChargeAttack" },
			{ "rotatingattack", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kRotatingAttack), 0, "RotatingAttack" },
			{ "continuousattack", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kContinuousAttack), 0, "ContinuousAttack" },
			{ "overridedata", static_cast<std::uint32_t>(RE::AttackData::AttackFlag::kOverrideData), 0, "OverrideData" },
		};


	}

	void* patch(const NPC::patch_instruction& line, RE::TESNPC* curobj)
	{




		if (!line.calcStats.empty() && line.calcStats != "none" && curobj->formID != 0x000007) {
			if (line.calcStats == "yes" || line.calcStats == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("Enabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (curobj->IsAutoCalc() && (line.calcStats == "no" || line.calcStats == "false")) {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
				logger::debug(FMT_STRING("Disabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.pcMultFlag.empty() && curobj->formID != 0x000007) {
			if (toLowerCase(line.pcMultFlag[0]) == "yes" || toLowerCase(line.pcMultFlag[0]) == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				logger::debug(FMT_STRING("Enabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (curobj->HasPCLevelMult() && (toLowerCase(line.pcMultFlag[0]) == "no" || toLowerCase(line.pcMultFlag[0]) == "false")) {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
				if (!line.PCvalues1.empty()) {
					if (!bEnableSetLevelDirectlyByPCMult) {
						float tempLevel = static_cast<float>(curobj->actorData.calcLevelMin + curobj->actorData.calcLevelMax);
						if (tempLevel >= 1) {
							tempLevel = (std::ceil(tempLevel / 2));

							if (tempLevel >= line.PCvalues1[0]) {
								logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {} , set to Fallback {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, tempLevel, line.PCvalues1[0]);
								curobj->actorData.level = line.PCvalues1[0];

							}
							else {
								curobj->actorData.level = static_cast<uint16_t>(tempLevel);
								logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} calculated level from min {} and max {} / 2 = {}, Fallback is {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin, curobj->actorData.calcLevelMax, curobj->actorData.level, line.PCvalues1[0]);
							}

						}
						else {
							curobj->actorData.level = line.PCvalues1[0];
							logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {} (Fallback)"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
						}
					}
					else {
						curobj->actorData.level = line.PCvalues1[0];
						logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {}"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
					}

				}
				else {
					logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
				}
			}
		}

		if (!line.kEssential.empty() && line.kEssential != "none" && curobj->formID != 0x000007) {
			if (line.kEssential == "yes" || line.kEssential == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("Enabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (line.kEssential == "no" || line.kEssential == "false") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kEssential);
				logger::debug(FMT_STRING("Disabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.kProtected.empty() && line.kProtected != "none" && curobj->formID != 0x000007) {
			if (line.kProtected == "yes" || line.kProtected == "true") {
				curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("Enabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
			else if (line.kProtected == "no" || line.kProtected == "false") {
				curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kProtected);
				logger::debug(FMT_STRING("Disabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.rcslist.empty()) {
			for (const auto& tempCs : line.rcslist) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(tempCs[0]);
				if (currentform && currentform->formType == RE::FormType::FormList) {
					aData.npcData[curobj->formID].npcCombatStyleList = ((RE::BGSListForm*)currentform);

					if (tempCs.size() > 1) {
						aData.npcData[curobj->formID].npcCombatStyleChance = std::stoi(tempCs[1]);
						logger::debug(FMT_STRING("NPC {:08X} {} will get random combat style from {:08X} with a chance of {}%"), curobj->formID, curobj->fullName, currentform->formID, tempCs[1]);
					}
					else {
						logger::debug(FMT_STRING("NPC {:08X} {} will get random combat style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				}
			}
		}

		if (!line.setAggression.empty() && line.setAggression != "none") {
			std::string lowerFlag = toLowerCase(line.setAggression);
			if (const auto* row = formfilter::findActionLowered(kAggressionRows, lowerFlag)) {
				curobj->SetAggressionLevel(static_cast<RE::ACTOR_AGGRESSION>(row->value));
				logger::debug(FMT_STRING("npc formid: {:08X} {} aggression level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown aggression level '{}' provided in restrictToTemplateFlags"), lowerFlag);
			}
		}

		if (!line.setConfidence.empty() && line.setConfidence != "none") {
			std::string lowerFlag = toLowerCase(line.setConfidence);
			if (const auto* row = formfilter::findActionLowered(kConfidenceRows, lowerFlag)) {
				curobj->SetConfidenceLevel(static_cast<RE::ACTOR_CONFIDENCE>(row->value));
				logger::debug(FMT_STRING("npc formid: {:08X} {} confidence level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown confidence level '{}' provided in setConfidence"), lowerFlag);
			}
		}

		if (!line.setAssistance.empty() && line.setAssistance != "none") {
			std::string lowerFlag = toLowerCase(line.setAssistance);
			if (const auto* row = formfilter::findActionLowered(kAssistanceRows, lowerFlag)) {
				curobj->SetAssistanceLevel(static_cast<RE::ACTOR_ASSISTANCE>(row->value));
				logger::debug(FMT_STRING("npc formid: {:08X} {} assistance level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown assistance level '{}' provided in setAssistance"), lowerFlag);
			}
		}

		if (!line.setMorality.empty() && line.setMorality != "none") {
			std::string lowerFlag = toLowerCase(line.setMorality);
			if (const auto* row = formfilter::findActionLowered(kMoralityRows, lowerFlag)) {
			setMoralityLevel(curobj, static_cast<RE::ACTOR_MORALITY>(row->value));
				logger::debug(FMT_STRING("npc formid: {:08X} {} morality level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown morality level '{}' provided in setMorality"), lowerFlag);
			}
		}

		if (!line.setMood.empty() && line.setMood != "none") {
			std::string lowerFlag = toLowerCase(line.setMood);
			if (const auto* row = formfilter::findActionLowered(kMoodRows, lowerFlag)) {
			setMoodLevel(curobj, static_cast<RE::ACTOR_MOOD>(row->value));
				logger::debug(FMT_STRING("npc formid: {:08X} {} mood level set to: {}"),
					curobj->formID, curobj->fullName, lowerFlag);
			}
			else {
				logger::warn(FMT_STRING("Unknown mood level '{}' provided in setMood"), lowerFlag);
			}
		}

		if (!line.aggressionRadiusBehavior.empty() && line.aggressionRadiusBehavior != "none") {
			std::string lowerString = toLowerCase(line.aggressionRadiusBehavior);
			if (lowerString == "true") {
				curobj->aiData.aggroRadiusBehaviour = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} enabled aggression radius"), curobj->formID, curobj->fullName);
			}
			else if (lowerString == "false") {
				curobj->aiData.aggroRadiusBehaviour = false;
				logger::debug(FMT_STRING("npc formid: {:08X} {} disabled aggression radius"), curobj->formID, curobj->fullName);
			}
			else {
				logger::debug(FMT_STRING("npc formid: {:08X} {} wrong parameter for aggressionRadiusBehavior allowed are true / false"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.aggressionRadiusRanges.empty()) {
			for (const auto& aRanges : line.aggressionRadiusRanges) {
				if (aRanges.size() < 2) {
					logger::warn("Skipping invalid aggression radius entry, expected at least 2 elements");
					continue;
				}

				int radius = 0;

				try {
					radius = std::stoi(aRanges[1]);
				}
				catch (const std::exception& e) {
					logger::error("Invalid aggression radius value '{}': {}", aRanges[1], e.what());
					continue;
				}

				if (const auto* row = formfilter::findAction(kAggroRadiusRows, aRanges[0])) {
					curobj->aiData.aggroRadius[row->value] = radius;
					logger::debug(FMT_STRING("npc formid: {:08X} {} set {} radius to {}"), curobj->formID, curobj->fullName, row->label, radius);
				}
			}
		}



		if (!line.setRandomVisualStyle.empty()) {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.setRandomVisualStyle[0][0]);
			if (currentform && currentform->formType == RE::FormType::FormList) {
				aData.npcData[curobj->formID].npcVisualStyleList = ((RE::BGSListForm*)currentform);

				if (line.setRandomVisualStyle[0].size() > 1) {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X} with a chance of {}%"), curobj->formID, curobj->fullName, currentform->formID, line.setRandomVisualStyle[0][1]);
					aData.npcData[curobj->formID].styleChance = std::stoi(line.setRandomVisualStyle[0][1]);
				}
				else {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
				}
				if (line.setRandomVisualStyle[0].size() > 2) {
					aData.npcData[curobj->formID].changeVoice = true;
				}
				if (!line.rvsRestrictToTraits.empty() && line.rvsRestrictToTraits != "none") {
					aData.npcData[curobj->formID].restrictToTraits = true;
				}

			}
		}

		if (!line.addRandomVisualStyle.empty()) {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.addRandomVisualStyle[0][0]);
			int added = 0;
			if (currentform && currentform->formType == RE::FormType::FormList) {

				if (aData.npcData[curobj->formID].npcVisualStyleList == nullptr)
				{
					aData.npcData[curobj->formID].npcVisualStyleList = ((RE::BGSListForm*)currentform);
					added = ((RE::BGSListForm*)currentform)->forms.size();
				}
				else {
					for (auto tForm : ((RE::BGSListForm*)currentform)->forms) {
						if (tForm) {
							auto& targetForms = aData.npcData[curobj->formID].npcVisualStyleList->forms;
							if (std::find(targetForms.begin(), targetForms.end(), tForm) == targetForms.end()) {
								added++;
								targetForms.push_back(tForm);
							}
						}
					}
				}

				if (line.addRandomVisualStyle[0].size() > 1) {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X} with a chance of {}%  (added random visual styles {})"), curobj->formID, curobj->fullName, currentform->formID, line.addRandomVisualStyle[0][1], added);
					aData.npcData[curobj->formID].styleChance = std::stoi(line.addRandomVisualStyle[0][1]);
				}
				else {
					logger::debug(FMT_STRING("NPC {:08X} {} will get random visual style from {:08X}   (added random visual styles {})"), curobj->formID, curobj->fullName, currentform->formID, added);
				}
				if (line.addRandomVisualStyle[0].size() > 2) {
					aData.npcData[curobj->formID].changeVoice = true;
				}
				if (!line.rvsRestrictToTraits.empty() && line.rvsRestrictToTraits != "none") {
					aData.npcData[curobj->formID].restrictToTraits = true;
				}

			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.keywordsToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("npc formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.keywordsToRemove[i]);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("npc formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.factionsToRemove.empty()) {
			for (size_t i = 0; i < line.factionsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.factionsToRemove[i]);

				if (currentform && currentform->formType == RE::FormType::Faction) {
					for (auto it = curobj->factions.begin(); it != curobj->factions.end(); ++it) {
						if (it->faction && it->faction->formID == currentform->formID) {
							curobj->factions.erase(it);
							logger::debug(
								FMT_STRING("npc formid: {:08X} removed faction {:08X} {} "),
								curobj->formID,
								static_cast<RE::TESFaction*>(currentform)->formID,
								static_cast<RE::TESFaction*>(currentform)->fullName);
							break;
						}
					}
				}
			}
		}

		if (!line.factionsToAdd.empty()) {
			for (size_t i = 0; i < line.factionsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.factionsToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::Faction) {
					float finalValue = 0;
					bool factionFound = false;
					for (auto& faction : curobj->factions) {
						if (faction.faction && faction.faction->formID && faction.faction->formID == currentform->formID) {
							factionFound = true;
							break;
						}
					}
					if (!factionFound) {
						RE::FACTION_RANK newFactionRank;
						newFactionRank.faction = (RE::TESFaction*)currentform;
						newFactionRank.rank = finalValue;
						curobj->factions.push_back(newFactionRank);
						logger::debug(FMT_STRING("npc formid: {:08X} {} added faction {:08X} with rank {}"), curobj->formID, curobj->fullName, ((RE::TESFaction*)currentform)->formID, finalValue);
					}
				}
			}
		}

		if (!line.avifs.empty()) {
			for (size_t i = 0; i < line.avifs.size(); i++) {
				std::string  string_form = line.avifs[i];
				const std::string lowerAvif = toLowerCase(string_form);

				float finalValue = 0;

				if (line.level_min == "none") {
					if (!line.values1.empty() && !line.values2.empty()) {
						unsigned seed = static_cast<unsigned>(std::time(nullptr)) + static_cast<unsigned>(std::clock());
						std::srand(seed);
						finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
					}

				}
				else if (line.level_min != "none" && line.level_max != "none" && line.level_min != line.level_max && line.values1[i] != line.values2[i]) {
					int level_min = std::stoi(line.level_min);
					int level_max = std::stoi(line.level_max);
					if (level_min < level_max) {
						int avif_min = line.values1[i];
						int avif_max = line.values2[i];
						float health_increase = avif_max - avif_min;
						float level_increase = level_max - level_min;
						float proportional_increase = health_increase / level_increase;

						int target_level = curobj->actorData.level;
						finalValue = avif_min;

						if (target_level >= level_min && target_level <= level_max) {
							float level_difference = target_level - level_min;
							finalValue = avif_min + (level_difference * proportional_increase);
						}
						else if (target_level < level_min) {
							finalValue = avif_min;
						}
						else {
							finalValue = avif_max;
						}
					}
					else {
						finalValue = line.values1[i];
						logger::info(FMT_STRING("Error: Level Min > Level Max  using default value for av {}"), finalValue);
					}

				}
				else {
					finalValue = line.values1[i];
				}
				StatWeights sw;
				if (curobj->npcClass) {
					sw.healthWeight = curobj->npcClass->data.attributeWeights.health;
					sw.staminaWeight = curobj->npcClass->data.attributeWeights.stamina;
					sw.magickaWeight = curobj->npcClass->data.attributeWeights.magicka;
				}
				else {
					sw.healthWeight = 1;
					sw.staminaWeight = 1;
					sw.magickaWeight = 1;
				}

				auto it2 = aData.npcData.find(curobj->formID);

				if (const auto* statRow = formfilter::findActionLowered(kStatRows, lowerAvif)) {
					const auto slot = static_cast<StatSlot>(statRow->value);
					const auto mode = static_cast<StatMode>(statRow->mode);

					const std::int16_t oldOffset =
						slot == StatSlot::Health ? curobj->actorData.healthOffset :
						slot == StatSlot::Stamina ? curobj->actorData.staminaOffset :
													curobj->actorData.magickaOffset;
					const int weight =
						slot == StatSlot::Health ? sw.healthWeight :
						slot == StatSlot::Stamina ? sw.staminaWeight :
													sw.magickaWeight;
					const std::string& bonusText =
						slot == StatSlot::Health ? line.healthBonus :
						slot == StatSlot::Stamina ? line.staminaBonus :
													line.magickaBonus;
					const char* const slotName =
						slot == StatSlot::Health ? "health" :
						slot == StatSlot::Stamina ? "stamina" :
													"magicka";

					const auto assignOffset = [&](auto a_value) {
						switch (slot) {
						case StatSlot::Health:
							curobj->actorData.healthOffset = a_value;
							break;
						case StatSlot::Stamina:
							curobj->actorData.staminaOffset = a_value;
							break;
						default:
							curobj->actorData.magickaOffset = a_value;
							break;
						}
					};
					const auto readOffset = [&]() -> std::int16_t {
						switch (slot) {
						case StatSlot::Health:
							return curobj->actorData.healthOffset;
						case StatSlot::Stamina:
							return curobj->actorData.staminaOffset;
						default:
							return curobj->actorData.magickaOffset;
						}
					};
					const auto commit = [&]() {
						switch (slot) {
						case StatSlot::Health:
							curobj->SetBaseActorValue(RE::ActorValue::kHealth, curobj->actorData.healthOffset);
							curobj->playerSkills.health = curobj->actorData.healthOffset;
							if (it2 == aData.npcData.end()) {
								aData.npcData[curobj->formID].health = curobj->actorData.healthOffset;
							}
							else {
								it2->second.health = curobj->actorData.healthOffset;
							}
							break;
						case StatSlot::Stamina:
							curobj->SetBaseActorValue(RE::ActorValue::kStamina, curobj->actorData.staminaOffset);
							curobj->playerSkills.stamina = curobj->actorData.staminaOffset;
							if (it2 == aData.npcData.end()) {
								aData.npcData[curobj->formID].stamina = curobj->actorData.staminaOffset;
							}
							else {
								it2->second.stamina = curobj->actorData.staminaOffset;
							}
							break;
						default:
							curobj->SetBaseActorValue(RE::ActorValue::kMagicka, curobj->actorData.magickaOffset);
							curobj->playerSkills.magicka = curobj->actorData.magickaOffset;
							if (it2 == aData.npcData.end()) {
								aData.npcData[curobj->formID].magicka = curobj->actorData.magickaOffset;
							}
							else {
								it2->second.magicka = curobj->actorData.magickaOffset;
							}
							break;
						}
					};

					switch (mode) {
					case StatMode::Set:
						assignOffset(finalValue);
						logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), readOffset());
						commit();
						break;

					case StatMode::Mult:
						logger::debug(FMT_STRING("npc formid: {:08X} {} multiplied {} to {} * {} = {}"), curobj->formID, curobj->fullName, slotName, oldOffset, line.values1[i], oldOffset * line.values1[i]);
						assignOffset(oldOffset * line.values1[i]);
						commit();
						break;

					case StatMode::Calc: {
						auto tempResult = calculateAttributePoints(curobj->actorData.level, sw, line.values1[i]);
						int  points =
							slot == StatSlot::Health ? tempResult.health :
							slot == StatSlot::Stamina ? tempResult.stamina :
														tempResult.magicka;

						if (!bonusText.empty() && bonusText != "none") {
							points = points + std::stoi(bonusText);
							logger::debug(FMT_STRING("npc formid: {:08X} {} added {} bonus of {}"), curobj->formID, curobj->fullName, slotName, bonusText);
						}

						assignOffset(points);
						logger::debug(FMT_STRING("npc formid: {:08X} {} calculated {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), readOffset());
						commit();
						break;
					}

					default: {
						int points = 0;

						if (weight > 0) {
							points = static_cast<int>(finalValue);
						}
						else {
							points = 0;
						}

						if (!bonusText.empty() && bonusText != "none") {
							points = points + std::stoi(bonusText);
							logger::debug(FMT_STRING("npc formid: {:08X} {} added {} bonus of {}"), curobj->formID, curobj->fullName, slotName, bonusText);
						}

						assignOffset(points);
						logger::debug(FMT_STRING("npc formid: {:08X} {} setByClass {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), readOffset());
						commit();
						break;
					}
					}
				}




			}
		}

		if (!line.perksToAdd.empty()) {
			for (size_t i = 0; i < line.perksToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.perksToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::Perk) {
					curobj->AddPerk((RE::BGSPerk*)currentform, 1);
					logger::debug(FMT_STRING("npc formid: {:08X} added perk {:08X} {} "), curobj->formID, ((RE::BGSPerk*)currentform)->formID, ((RE::BGSPerk*)currentform)->fullName);
					auto it2 = aData.npcData.find(curobj->formID);
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].perks.push_back(((RE::BGSPerk*)currentform));
					}
					else {
						it2->second.perks.push_back(((RE::BGSPerk*)currentform));
					}

				}
			}
		}

		if (!line.outfitDefault.empty() && line.outfitDefault != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.outfitDefault);
			if (currentform && currentform->formType == RE::FormType::Outfit) {
				curobj->defaultOutfit = (RE::BGSOutfit*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed outfit default to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
			}
		}

		if (!line.outfitSleep.empty() && line.outfitSleep != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.outfitSleep);
			if (currentform && currentform->formType == RE::FormType::Outfit) {
				curobj->sleepOutfit = (RE::BGSOutfit*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed outfit sleep to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
			}
		}

		if (!line.deathItem.empty() && line.deathItem != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.deathItem);
			if (currentform && currentform->formType == RE::FormType::LeveledItem) {
				curobj->deathItem = (RE::TESLevItem*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to {:08X} "), curobj->formID, ((RE::TESLevItem*)currentform)->formID);
			}
			else if (line.deathItem == "null") {
				curobj->deathItem = nullptr;
				logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to null (none) "), curobj->formID);
			}
		}

		if (!line.skin.empty() && line.skin != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.skin);
			if (currentform && currentform->formType == RE::FormType::Armor) {
				curobj->skin = (RE::TESObjectARMO*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed skin to {:08X} "), curobj->formID, ((RE::TESObjectARMO*)currentform)->formID);
				aData.npcData[curobj->formID].skin = (RE::TESObjectARMO*)currentform;
				aData.npcData[curobj->formID].skinWasSet = true;
			}
			else if (line.skin == "null") {
				curobj->skin = nullptr;
				logger::debug(FMT_STRING("npc formid: {:08X} changed skin to null (none) "), curobj->formID);
				aData.npcData[curobj->formID].skin = nullptr;
				aData.npcData[curobj->formID].skinWasSet = true;
			}
		}

		if (!line.race.empty() && line.race != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.race);
			if (currentform && currentform->formType == RE::FormType::Race) {
				curobj->race = (RE::TESRace*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed race to {:08X} "), curobj->formID, ((RE::TESRace*)currentform)->formID);
			}
		}

		if (!line.Class.empty() && line.Class != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.Class);
			if (currentform && currentform->formType == RE::FormType::Class) {
				curobj->npcClass = (RE::TESClass*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed class to {:08X} "), curobj->formID, ((RE::TESClass*)currentform)->formID);
			}
		}

		if (!line.haircolor.empty() && line.haircolor != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.haircolor);
			if (currentform && currentform->formType == RE::FormType::ColorForm) {
				curobj->SetHairColor((RE::BGSColorForm*)currentform);
				logger::debug(FMT_STRING("npc formid: {:08X} changed hair color to {:08X} "), curobj->formID, currentform->formID);
				aData.npcData[curobj->formID].haircolor = (RE::BGSColorForm*)currentform;
			}
		}

		if (!line.voiceType.empty() && line.voiceType != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.voiceType);
			if (currentform && currentform->formType == RE::FormType::VoiceType) {
				curobj->voiceType = ((RE::BGSVoiceType*)currentform);
				aData.npcData[curobj->formID].voiceType = (RE::BGSVoiceType*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed voiceType to {:08X} "), curobj->formID, (currentform)->formID);
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed weight to {}"), curobj->formID, curobj->fullName, curobj->weight);
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.height.empty() && line.height != "none") {
			try {
				curobj->height = std::stof(line.height);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed height to {}"), curobj->formID, curobj->fullName, curobj->height);
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.level.empty() && line.level != "none") {
			try {
				curobj->actorData.level = std::stof(line.level);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed level to {}"), curobj->formID, curobj->fullName, curobj->actorData.level);
			}
			catch (const std::exception& e) {
			}
		}
		if (!line.calcLevelMin.empty() && line.calcLevelMin != "none") {
			try {
				curobj->actorData.calcLevelMin = std::stoi(line.calcLevelMin);
				aData.npcData[curobj->formID].minLevel = std::stoi(line.calcLevelMin);
				aData.npcData[curobj->formID].minLevelWasSet = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMin to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin);
			}
			catch (const std::exception& e) {
			}
		}
		if (!line.calcLevelMax.empty() && line.calcLevelMax != "none") {
			try {
				curobj->actorData.calcLevelMax = std::stoi(line.calcLevelMax);
				aData.npcData[curobj->formID].maxLevel = std::stoi(line.calcLevelMax);
				aData.npcData[curobj->formID].maxLevelWasSet = true;
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMax to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMax);
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.spellsToAdd.empty()) {
			for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.spellsToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					curobj->GetSpellList()->AddSpell((RE::SpellItem*)currentform);
					logger::debug(FMT_STRING("npc formid: {:08X} {} added spell {:08X} {} "), curobj->formID, curobj->fullName, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
					auto it2 = aData.npcData.find(curobj->formID);
					if (it2 == aData.npcData.end()) {
						aData.npcData[curobj->formID].spellItems.push_back(((RE::SpellItem*)currentform));
					}
					else {
						it2->second.spellItems.push_back(((RE::SpellItem*)currentform));
					}
				}
			}
		}

		if (!line.spellsToRemove.empty()) {
			for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.spellsToRemove[i]);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					if (curobj->actorEffects) {
						curobj->GetSpellList()->RemoveSpell((RE::SpellItem*)currentform);
					}
					logger::debug(FMT_STRING("npc formid: {:08X} {} remove spell {:08X} {} "), curobj->formID, curobj->fullName, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
				}
			}
		}

		if (!line.levSpellsToAdd.empty()) {
			for (size_t i = 0; i < line.levSpellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.levSpellsToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddLevSpell((RE::TESLevSpell*)currentform);
					logger::debug(FMT_STRING("npc formid: {:08X} added levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
				}
			}
		}

		if (!line.levSpellsToRemove.empty()) {
			for (size_t i = 0; i < line.levSpellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.levSpellsToRemove[i]);
				if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
					if (curobj->actorEffects) {
						curobj->actorEffects->RemoveLevSpell((RE::TESLevSpell*)currentform);
					}
					logger::debug(FMT_STRING("npc formid: {:08X} removed levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
				}
			}
		}

		if (!line.shoutsToAdd.empty()) {
			for (size_t i = 0; i < line.shoutsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.shoutsToAdd[i]);
				if (currentform && currentform->formType == RE::FormType::Shout) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddShout((RE::TESShout*)currentform);
					logger::debug(FMT_STRING("npc formid: {:08X} added shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
				}
			}
		}

		if (!line.shoutsToRemove.empty()) {
			for (size_t i = 0; i < line.shoutsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.shoutsToRemove[i]);
				if (currentform && currentform->formType == RE::FormType::Shout) {
					if (curobj->actorEffects) {
						curobj->actorEffects->RemoveShout((RE::TESShout*)currentform);
					}
					logger::debug(FMT_STRING("npc formid: {:08X} removed shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
				}
			}
		}

		if (!line.clearInventory.empty() && line.clearInventory != "none") {
			curobj->RemoveAllObjectFromContainer();
			logger::debug(FMT_STRING("npc formid: {:08X} removed all objects "), curobj->formID);
		}

		if (!line.objectsToRemove.empty()) {
			for (size_t i = 0; i < line.objectsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.objectsToRemove[i]);
				if (currentform) {
					RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
					if (bo) {
						curobj->RemoveObjectInstancesFromContainer(bo);
						logger::debug(FMT_STRING("npc formid: {:08X} {} removed all object instances {:08X} "), curobj->formID, curobj->fullName, bo->formID);
					}
				}
			}
		}

		if (!line.objectsToRemoveNew.empty()) {
			for (size_t i = 0; i < line.objectsToRemoveNew.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.objectsToRemoveNew[i]);
				if (currentform) {
					RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
					if (bo) {
						curobj->RemoveObjectInstancesFromContainer(bo);
						logger::debug(FMT_STRING("npc formid: {:08X} {} removed all object instances {:08X} "), curobj->formID, curobj->fullName, bo->formID);
					}
				}
			}
		}



		if (!line.removeContainerObjectsByCount.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByCount) {
				if (objectToAdd.size() < 2) { logger::warn("npc {:08X}: invalid removeContainerObjectsByCount entry, expected 2 values", curobj->formID); continue; }
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(objectToAdd[0]);

				if (addForm) {
					curobj->RemoveObjectFromContainer(addForm, count);
					logger::debug(FMT_STRING("npc formid: {:08X} removed by count Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("npc formid: {:08X} Form not found: {} (removeContainerObjectsByCount)"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.removeContainerObjectsByKeyword.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByKeyword) {
				RE::BGSKeyword* addForm = (RE::BGSKeyword*)GetFormFromIdentifier(objectToAdd);
				if (addForm) {
					curobj->RemoveObjectFromContainerByKeyword(addForm);
					logger::debug(FMT_STRING("npc formid: {:08X} removed by keyword Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("npc formid: {:08X} Form not found: {} (remove by keyword)"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.objectsToAdd.empty()) {
			for (size_t i = 0; i < line.objectsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				currentform = GetFormFromIdentifier(line.objectsToAdd[i]);
				if (currentform) {
					RE::TESBoundObject* bo = (RE::TESBoundObject*)currentform;
					if (bo) {
						curobj->AddObjectToContainer(bo, std::stoi(line.objectsToAddValue[i]), nullptr);
						logger::debug(FMT_STRING("npc formid: {:08X} added object {:08X} {} "), curobj->formID, ((RE::TESBoundObject*)currentform)->formID, line.objectsToAddValue[i]);
					}
				}
			}
		}

		if (!line.addedContainerObjectsOnce.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjectsOnce) {
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(objectToAdd[0]);
				if (addForm) {
					bool formExists = curobj->CountObjectsInContainer(addForm) != 0;
					if (!formExists) {
						curobj->AddObjectToContainer(addForm, count, nullptr);
						logger::debug(FMT_STRING("npc {:08X} added once Form  {:08X}"), curobj->formID, addForm->formID);
					}
					else {
						logger::debug(FMT_STRING("npc {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
					}
				}
				else {
					logger::critical(FMT_STRING("npc {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.objectsToReplace.empty()) {
			for (const auto& objectToAdd : line.objectsToReplace) {
				if (objectToAdd.size() < 2) { logger::warn("npc {:08X}: invalid objectsToReplace entry, expected 2 values", curobj->formID); continue; }
				std::string         object1 = objectToAdd[0];
				std::string         object2 = objectToAdd[1];
				RE::TESBoundObject* oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(object1);
				RE::TESBoundObject* newObject = (RE::TESBoundObject*)GetFormFromIdentifier(object2);

				if (oldObject && newObject) {
					replaceObjectInContainer(*curobj, oldObject, newObject);
					logger::debug(FMT_STRING("npc formid: {:08X} replaced Form {:08X} with Form {:08X} "), curobj->formID, oldObject->formID, newObject->formID);
				}
			}
		}


		if (!line.speedMultiplier.empty() && line.speedMultiplier != "none") {
			try {
				curobj->actorData.speedMult = std::stof(line.speedMultiplier);
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed speedMultiplier to {}"), curobj->formID, curobj->fullName, curobj->actorData.speedMult);
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.shortName.empty() && line.shortName != "none") {
			try {
				logger::debug(FMT_STRING("npc formid: {:08X} {} changed shortname to {}"), curobj->formID, curobj->shortName, line.shortName);
				curobj->shortName = line.shortName;
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.skills.empty()) {
			for (size_t i = 0; i < line.skills.size(); i++) {
				std::string skill = line.skills[i];
				int         value = line.skillvalues1[i];

				if (const auto* row = formfilter::findAction(kSkillRows, skill)) {
					curobj->playerSkills.values[row->value] = value;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed skill {} to {}"), curobj->formID, curobj->fullName, skill, value);
				}
				else {
				}
			}

		}



		if (!line.clearClassFromAttributes.empty() && line.clearClassFromAttributes != "none") {
			if (curobj->npcClass) {
				auto newClass = copyClass(curobj->npcClass)->As<RE::TESClass>();
				newClass->data.attributeWeights.health = 0;
				newClass->data.attributeWeights.stamina = 0;
				newClass->data.attributeWeights.magicka = 0;
				curobj->npcClass = newClass;
				logger::debug(FMT_STRING("npc formid: {:08X} {} copied class and cleared attributes"), curobj->formID, curobj->fullName);
				auto it2 = aData.npcData.find(curobj->formID);
				if (it2 == aData.npcData.end()) {
					aData.npcData[curobj->formID].npcClass = newClass;
				}
				else {
					it2->second.npcClass = newClass;
				}
			}
		}

		if (!line.setTemplateFlags.empty()) {
			for (const auto& setFlag : line.setTemplateFlags) {
				if (const auto* row = formfilter::findAction(kTemplateFlagRows, setFlag)) {
					curobj->actorData.templateUseFlags.set(static_cast<RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG>(row->value));
					logger::debug(FMT_STRING("npc formid: {:08X} {} template flag enabled {}"), curobj->formID, curobj->fullName, setFlag);
					aData.npcData[curobj->formID].updateTemplateFlags = true;
				}
			}
		}


		if (!line.removeTemplateFlags.empty()) {
			for (const auto& removeFlag : line.removeTemplateFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);

				if (const auto* row = formfilter::findActionLowered(kTemplateFlagRows, lowerFlag)) {
					curobj->actorData.templateUseFlags.reset(static_cast<RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG>(row->value));
					logger::debug(FMT_STRING("npc formid: {:08X} {} template flag disabled {}"), curobj->formID, curobj->fullName, lowerFlag);
					aData.npcData[curobj->formID].updateTemplateFlags = true;
				}
				else {
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);
				if (const auto* row = formfilter::findActionLowered(kBaseFlagRows, lowerFlag)) {
					curobj->actorData.actorBaseFlags.reset(static_cast<RE::ACTOR_BASE_DATA::Flag>(row->value));
					logger::debug(FMT_STRING("npc formid: {:08X} {} flag disabled {}"), curobj->formID, curobj->fullName, lowerFlag);
					aData.npcData[curobj->formID].updateFlags = true;
				}
				else {
					logger::warn(FMT_STRING("npc formid: {:08X} {} invalid flag: {}"), curobj->formID, curobj->fullName, lowerFlag);
				}
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				std::string flag = toLowerCase(setFlag);
				if (const auto* row = formfilter::findActionLowered(kBaseFlagRows, flag)) {
					curobj->actorData.actorBaseFlags.set(static_cast<RE::ACTOR_BASE_DATA::Flag>(row->value));
					logger::debug(FMT_STRING("npc formid: {:08X} {} flag enabled {}"), curobj->formID, curobj->fullName, flag);
					aData.npcData[curobj->formID].updateFlags = true;
				}
				else {
					logger::warn(FMT_STRING("npc formid: {:08X} {} unrecognized flag: {}"), curobj->formID, curobj->fullName, flag);
				}
			}
		}

		if (!line.copyVisualStyle.empty() && line.copyVisualStyle != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.copyVisualStyle);
			if (currentform) {
				RE::TESNPC* bo = (RE::TESNPC*)currentform;
				if (bo) {
					aData.npcData[curobj->formID].npcVisual = bo;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed visual to {:08X} Start"), curobj->formID, curobj->fullName, bo->formID);
					curobj->height = bo->height;
					curobj->weight = bo->weight;
					curobj->faceNPC = bo;
					curobj->tintLayers = bo->tintLayers;
					curobj->bodyTintColor = bo->bodyTintColor;
					curobj->headRelatedData = bo->headRelatedData;
					for (int i = 0; i < bo->numHeadParts; i++) {
						if (bo->headParts && bo->headParts[i]) {
							logger::debug("-> npc formid: {:08X} {} Change Head Part {:08X} {}", curobj->formID, curobj->fullName, bo->headParts[i]->formID, bo->headParts[i]->fullName);
							curobj->ChangeHeadPart(bo->headParts[i]);
						}
					}
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed visual to {:08X} Completed"), curobj->formID, curobj->fullName, bo->formID);
				}
			}
		}

		if (!line.baseObject.empty() && line.baseObject != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.baseObject);
			if (currentform && currentform->formType == RE::FormType::NPC) {
				aData.npcData[curobj->formID].baseObject = (RE::TESNPC*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} Replace with {:08X}"), curobj->formID, currentform->formID);
			}
		}

		if (!line.addedAttackData.empty()) {
			RE::BGSAttackData* newAttackData = RE::BGSAttackData::Create();
			newAttackData->IncRefCount();
			newAttackData->data.damageMult = 1;
			newAttackData->data.attackChance = 1;
			newAttackData->data.attackSpell = nullptr;
			newAttackData->data.attackAngle = 0;
			newAttackData->data.strikeAngle = 50;
			newAttackData->data.staggerOffset = 0;
			newAttackData->data.attackType = nullptr;
			newAttackData->data.knockDown = 0;
			newAttackData->data.recoveryTime = 0;
			newAttackData->data.staminaMult = 1;
			newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
			curobj->attackDataMap->attackDataMap.insert({ newAttackData->event, RE::NiPointer(newAttackData) });

			for (const auto& objectArr : line.addedAttackData) {
				for (const auto& object : objectArr) {
					const std::string lowerKey = toLowerCase(object[0]);
					if (lowerKey == "key") {
						newAttackData->event = object[1];
						logger::debug("added event: {}", object[1]);
					}
					else if (lowerKey == "damagemult") {
						newAttackData->data.damageMult = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
					}
					else if (lowerKey == "attackchance") {
						newAttackData->data.attackChance = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
					}
					else if (lowerKey == "attackspell") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackSpell = (RE::SpellItem*)form;
								logger::debug("npc formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
							}
						}
						else {
							newAttackData->data.attackSpell = nullptr;
							logger::debug("npc formid: {:08X} attackData - attackspell nullptr", curobj->formID);
						}
					}
					else if (lowerKey == "attackangle") {
						newAttackData->data.attackAngle = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
					}
					else if (lowerKey == "strikeangle") {
						newAttackData->data.strikeAngle = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
					}
					else if (lowerKey == "staggeroffset") {
						newAttackData->data.staggerOffset = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
					}
					else if (lowerKey == "attacktype") {
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackType = (RE::BGSKeyword*)form;
								logger::debug("npc formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
							}
						}
						else {
							newAttackData->data.attackType = nullptr;
							logger::debug("npc formid: {:08X} attackData - attacktype nullptr", curobj->formID);
						}
					}
					else if (lowerKey == "knockdown") {
						newAttackData->data.knockDown = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
					}
					else if (lowerKey == "recoverytime") {
						newAttackData->data.recoveryTime = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
					}
					else if (lowerKey == "staminamult") {
						newAttackData->data.staminaMult = std::stof(object[1]);
						logger::debug("npc formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
					}
					else if (lowerKey == "resetflags") {
						newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
						logger::debug(FMT_STRING("npc formid: {:08X} attackData - all flags reset"), curobj->formID);
					}
					else if (lowerKey == "removeflags") {
						for (int i = 1; i < object.size(); ++i) {
							if (const auto* row = formfilter::findAction(kAttackFlagRows, object[i])) {
								newAttackData->data.flags.reset(static_cast<RE::AttackData::AttackFlag>(row->value));
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled {}"), curobj->formID, row->label);
							}
							else {
							}
						}

					}
					else if (lowerKey == "setflags") {
						for (int i = 1; i < object.size(); ++i) {
							if (const auto* row = formfilter::findAction(kAttackFlagRows, object[i])) {
								newAttackData->data.flags.set(static_cast<RE::AttackData::AttackFlag>(row->value));
								logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled {}"), curobj->formID, row->label);
							}
							else {
							}
						}
					}
					else {
						logger::debug("Unrecognized keyword: {}", object[0]);
					}
				}
			}
		}

		if (!line.removeAttackData.empty()) {
			for (const auto& attackData : line.removeAttackData) {
				const std::string lowerAttackData = toLowerCase(attackData);
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					if (toLowerCase(it->second.get()->event.c_str()) == lowerAttackData) {
						logger::debug("npc formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					}
					else {
						++it;
					}
				}
			}
		}

		if (!line.attackDataToChange.empty()) {
			for (auto& attackData : curobj->attackDataMap->attackDataMap) {

				if (!attackData.second)
					continue;

				auto newAttackData = attackData.second.get();
				if (!newAttackData)
					continue;
				for (auto& objectArr : line.attackDataToChange) {
					bool key = true;
					for (auto& object : objectArr) {
						if (object[1].empty()) {
							continue;
						}

						if (key) {
							if (!(newAttackData->event.c_str() == object[1])) {
								break;
							}
							else {
								logger::debug("npc formid: {:08X} attackData change - entry found {}", curobj->formID, newAttackData->event.c_str());
								key = false;
								continue;
							}
						}

						const std::string lowerKey = toLowerCase(object[0]);
						if (lowerKey == "damagemult") {
							newAttackData->data.damageMult = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - damagemult {}", curobj->formID, newAttackData->data.damageMult);
						}
						else if (lowerKey == "attackchance") {
							newAttackData->data.attackChance = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - attackchance {}", curobj->formID, newAttackData->data.attackChance);
						}
						else if (lowerKey == "attackspell") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackSpell = (RE::SpellItem*)form;
									logger::debug("npc formid: {:08X} attackData changed - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
								}
							}
							else {
								newAttackData->data.attackSpell = nullptr;
								logger::debug("npc formid: {:08X} attackData changed - attackspell nullptr", curobj->formID);
							}
						}
						else if (lowerKey == "attackangle") {
							newAttackData->data.attackAngle = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
						}
						else if (lowerKey == "strikeangle") {
							newAttackData->data.strikeAngle = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
						}
						else if (lowerKey == "staggeroffset") {
							newAttackData->data.staggerOffset = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
						}
						else if (lowerKey == "attacktype") {
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackType = (RE::BGSKeyword*)form;
									logger::debug("npc formid: {:08X} attackData changed - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
								}
							}
							else {
								newAttackData->data.attackType = nullptr;
								logger::debug("npc formid: {:08X} attackData changed - attacktype nullptr", curobj->formID);
							}
						}
						else if (lowerKey == "knockdown") {
							newAttackData->data.knockDown = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - knockdown {}", curobj->formID, newAttackData->data.knockDown);
						}
						else if (lowerKey == "recoverytime") {
							newAttackData->data.recoveryTime = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
						}
						else if (lowerKey == "staminamult") {
							newAttackData->data.staminaMult = std::stof(object[1]);
							logger::debug("npc formid: {:08X} attackData changed - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
						}
						else if (lowerKey == "resetflags") {
							newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
							logger::debug(FMT_STRING("npc formid: {:08X} attackData - all flags reset"), curobj->formID);
						}
						else if (lowerKey == "removeflags") {
							for (int i = 1; i < object.size(); ++i) {
								if (const auto* row = formfilter::findAction(kAttackFlagRows, object[i])) {
									newAttackData->data.flags.reset(static_cast<RE::AttackData::AttackFlag>(row->value));
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag disabled {}"), curobj->formID, row->label);
								}
								else {
								}
							}

						}
						else if (lowerKey == "setflags") {
							for (int i = 1; i < object.size(); ++i) {
								if (const auto* row = formfilter::findAction(kAttackFlagRows, object[i])) {
									newAttackData->data.flags.set(static_cast<RE::AttackData::AttackFlag>(row->value));
									logger::debug(FMT_STRING("npc formid: {:08X} attackData - flag enabled {}"), curobj->formID, row->label);
								}
								else {
								}
							}
						}
						else {
							logger::debug("Unrecognized keyword: {}", object[0]);
						}
					}
				}


				for (const auto& attackData : line.removeAttackData) {
					const std::string lowerAttackData = toLowerCase(attackData);
					for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
						if (toLowerCase(it->second.get()->event.c_str()) == lowerAttackData) {
							logger::debug("npc formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
							it = curobj->attackDataMap->attackDataMap.erase(it);
						}
						else {
							++it;
						}
					}
				}






			}
		}
	}

	ActorData readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string            currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string  fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						}
						else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();

								if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr)) {
									if (!IsPluginInstalled(modname)) {
										logger::info("************************************************************");
										logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), modname, fullPath);
										logger::info("************************************************************");
										continue;
									}
								}
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string                  line;
								std::ifstream                infile;
								std::list<patch_instruction> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									tokens.push_back(create_patch_instruction(line));
								}
								infile.close();

								std::vector<patch_instruction> tokensVector(tokens.begin(), tokens.end());
								process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return aData;
	}

	void* readConfigHook(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string            currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string  fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						}
						else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string   line;
								std::ifstream infile;

								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction(line));
								}
								infile.close();
							}
						}
					}
				}
				closedir(dir);
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

	void setbEnableSetLevelDirectlyByPCMult()
	{
		bEnableSetLevelDirectlyByPCMult = true;
	}

	namespace
	{
		bool flagPolarity(const std::string& a_lower, const char* a_set, const char* a_clear, bool& a_expectSet)
		{
			if (a_lower == a_set) {
				a_expectSet = true;
				return true;
			}
			if (a_lower == a_clear) {
				a_expectSet = false;
				return true;
			}
			return false;
		}

		void addFlagPredicate(formfilter::MatchProgram& a_p, std::vector<formfilter::Predicate>& a_stage,
			RE::ACTOR_BASE_DATA::Flag a_flag, bool a_expectSet)
		{
			const std::vector<std::uint64_t> vals{ static_cast<std::uint64_t>(a_flag) };
			a_stage.push_back(formfilter::makePredicate(a_p, formfilter::Op::NpcFlag,
				a_expectSet ? formfilter::Mode::And : formfilter::Mode::Exclude, vals, false));
		}

		const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG>& templateFlagMap()
		{
			static const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG> m = {
				{ "traits", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits },
				{ "stats", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats },
				{ "factions", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kFactions },
				{ "spells", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kSpells },
				{ "aidata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIData },
				{ "aipackages", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIPackages },
				{ "unused", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kUnused },
				{ "basedata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kBaseData },
				{ "inventory", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kInventory },
				{ "script", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kScript },
				{ "aidefpacklist", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAIDefPackList },
				{ "attackdata", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData },
				{ "keywords", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kKeywords },
				{ "copiedtemplate", RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kCopiedTemplate }
			};
			return m;
		}

		const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::Flag>& baseFlagMap()
		{
			static const std::unordered_map<std::string, RE::ACTOR_BASE_DATA::Flag> m = {
				{ "female", RE::ACTOR_BASE_DATA::Flag::kFemale },
				{ "essential", RE::ACTOR_BASE_DATA::Flag::kEssential },
				{ "ischargenfacepreset", RE::ACTOR_BASE_DATA::Flag::kIsChargenFacePreset },
				{ "respawn", RE::ACTOR_BASE_DATA::Flag::kRespawn },
				{ "autocalcstats", RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats },
				{ "unique", RE::ACTOR_BASE_DATA::Flag::kUnique },
				{ "doesntaffectstealthmeter", RE::ACTOR_BASE_DATA::Flag::kDoesntAffectStealthMeter },
				{ "pclevelmult", RE::ACTOR_BASE_DATA::Flag::kPCLevelMult },
				{ "usestemplate", RE::ACTOR_BASE_DATA::Flag::kUsesTemplate },
				{ "calcfalltemplates", RE::ACTOR_BASE_DATA::Flag::kCalcForAllTemplates },
				{ "protected", RE::ACTOR_BASE_DATA::Flag::kProtected },
				{ "norumors", RE::ACTOR_BASE_DATA::Flag::kNoRumors },
				{ "summonable", RE::ACTOR_BASE_DATA::Flag::kSummonable },
				{ "doesntbleed", RE::ACTOR_BASE_DATA::Flag::kDoesntBleed },
				{ "bleedoutoverride", RE::ACTOR_BASE_DATA::Flag::kBleedoutOverride },
				{ "oppositegenderanims", RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims },
				{ "simpleactor", RE::ACTOR_BASE_DATA::Flag::kSimpleActor },
				{ "loopedscript", RE::ACTOR_BASE_DATA::Flag::kLoopedScript },
				{ "noactivation", RE::ACTOR_BASE_DATA::Flag::kNoActivation },
				{ "loopedaudio", RE::ACTOR_BASE_DATA::Flag::kLoopedAudio },
				{ "isghost", RE::ACTOR_BASE_DATA::Flag::kIsGhost },
				{ "invulnerable", RE::ACTOR_BASE_DATA::Flag::kInvulnerable }
			};
			return m;
		}

		bool lookupSkill(const std::string& a_lower, std::uint64_t& a_out)
		{
			static const std::unordered_map<std::string, std::uint64_t> m = {
				{ "onehanded", RE::TESNPC::Skills::kOneHanded },
				{ "twohanded", RE::TESNPC::Skills::kTwoHanded },
				{ "marksman", RE::TESNPC::Skills::kMarksman },
				{ "block", RE::TESNPC::Skills::kBlock },
				{ "smithing", RE::TESNPC::Skills::kSmithing },
				{ "heavyarmor", RE::TESNPC::Skills::kHeavyArmor },
				{ "lightarmor", RE::TESNPC::Skills::kLightArmor },
				{ "pickpocket", RE::TESNPC::Skills::kPickpocket },
				{ "lockpicking", RE::TESNPC::Skills::kLockpicking },
				{ "sneak", RE::TESNPC::Skills::kSneak },
				{ "alchemy", RE::TESNPC::Skills::kAlchemy },
				{ "speechcraft", RE::TESNPC::Skills::kSpeechcraft },
				{ "alteration", RE::TESNPC::Skills::kAlteration },
				{ "conjuration", RE::TESNPC::Skills::kConjuration },
				{ "destruction", RE::TESNPC::Skills::kDestruction },
				{ "illusion", RE::TESNPC::Skills::kIllusion },
				{ "restoration", RE::TESNPC::Skills::kRestoration },
				{ "enchanting", RE::TESNPC::Skills::kEnchanting }
			};
			const auto it = m.find(a_lower);
			if (it == m.end()) {
				return false;
			}
			a_out = it->second;
			return true;
		}
	}

	formfilter::MatchProgram makeFilterView(const patch_instruction& line)
	{
		using formfilter::Mode;
		using formfilter::Op;

		formfilter::MatchProgram p;
		p.modNames = &line.modNames;

		std::size_t first = p.preds.size();
		if (!line.keywords.empty()) {
			formfilter::addPred(p, Op::Keyword, Mode::And, formfilter::ptrOperands(line.keywords, RE::FormType::Keyword), true);
		}
		if (!line.keywordsOr.empty()) {
			formfilter::addPred(p, Op::Keyword, Mode::Or, formfilter::ptrOperands(line.keywordsOr, RE::FormType::Keyword), true);
		}
		formfilter::pushDisjunct(p, p.positive, first);

		if (!line.races.empty()) {
			first = p.preds.size();
			formfilter::addPred(p, Op::Race, Mode::Or, formfilter::idOperands(line.races, RE::FormType::Race), true);
			formfilter::pushDisjunct(p, p.positive, first);
		}
		if (!line.filterByOutfit.empty()) {
			first = p.preds.size();
			formfilter::addPred(p, Op::Outfit, Mode::Or, formfilter::idOperands(line.filterByOutfit, RE::FormType::Outfit), false);
			formfilter::pushDisjunct(p, p.positive, first);
		}
		if (!line.filterClass.empty()) {
			first = p.preds.size();
			formfilter::addPred(p, Op::NpcClass, Mode::Or, formfilter::idOperands(line.filterClass, RE::FormType::Class), false);
			formfilter::pushDisjunct(p, p.positive, first);
		}
		if (!line.filterCombatStyle.empty()) {
			first = p.preds.size();
			formfilter::addPred(p, Op::CombatStyle, Mode::Or, formfilter::idOperands(line.filterCombatStyle, RE::FormType::CombatStyle), false);
			formfilter::pushDisjunct(p, p.positive, first);
		}
		formfilter::sortCheapestFirst(p.positive);

		struct
		{
			const std::string&        raw;
			RE::ACTOR_BASE_DATA::Flag flag;
		} const assigns[] = {
			{ line.filterPCMult, RE::ACTOR_BASE_DATA::Flag::kPCLevelMult },
			{ line.filterAutoCalc, RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats },
			{ line.filterEssential, RE::ACTOR_BASE_DATA::Flag::kEssential },
			{ line.filterProtected, RE::ACTOR_BASE_DATA::Flag::kProtected },
		};
		for (const auto& a : assigns) {
			if (!liveScalarFilter(a.raw)) {
				continue;
			}
			p.unkeyable = true;
			bool expectSet = false;
			if (flagPolarity(toLowerCase(a.raw), "true", "false", expectSet)) {
				addFlagPredicate(p, p.assigns, a.flag, expectSet);
			}
		}

		first = p.preds.size();
		if (!line.filterByEditorIdContains.empty()) {
			formfilter::addPred(p, Op::EditorId, Mode::And, formfilter::needleOperands(p, line.filterByEditorIdContains), true);
		}
		if (!line.filterByEditorIdContainsOr.empty()) {
			formfilter::addPred(p, Op::EditorId, Mode::Or, formfilter::needleOperands(p, line.filterByEditorIdContainsOr), true);
		}
		formfilter::pushDisjunct(p, p.positive2, first);

		first = p.preds.size();
		if (!line.filterFactions.empty()) {
			formfilter::addPred(p, Op::Faction, Mode::And, formfilter::ptrOperands(line.filterFactions, RE::FormType::Faction), true);
		}
		if (!line.filterFactionsOr.empty()) {
			formfilter::addPred(p, Op::Faction, Mode::Or, formfilter::ptrOperands(line.filterFactionsOr, RE::FormType::Faction), true);
		}
		formfilter::pushDisjunct(p, p.positive2, first);
		formfilter::sortCheapestFirst(p.positive2);

		p.catchAll = line.object.empty() && line.races.empty() && line.keywords.empty() &&
			line.keywordsOr.empty() && line.filterClass.empty() && line.filterCombatStyle.empty() &&
			line.filterFactions.empty() && line.filterFactionsOr.empty() && line.filterByOutfit.empty() &&
			line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty();

		bool expectSet = false;
		if (liveScalarFilter(line.isFemale) &&
			flagPolarity(toLowerCase(line.isFemale), "female", "male", expectSet)) {
			addFlagPredicate(p, p.negative, RE::ACTOR_BASE_DATA::Flag::kFemale, expectSet);
		}
		if (!line.keywordsExcluded.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::Keyword, Mode::Exclude,
				formfilter::ptrOperands(line.keywordsExcluded, RE::FormType::Keyword), true));
		}
		if (!line.filterFactionsExcluded.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::Faction, Mode::Exclude,
				formfilter::ptrOperands(line.filterFactionsExcluded, RE::FormType::Faction), true));
		}
		if (!line.filterByEditorIdContainsExcluded.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::EditorId, Mode::Exclude,
				formfilter::needleOperands(p, line.filterByEditorIdContainsExcluded), true));
		}
		if (!line.filterClassExclude.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::NpcClass, Mode::Exclude,
				formfilter::idOperands(line.filterClassExclude, RE::FormType::Class), false));
		}
		if (!line.objectExcluded.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::ObjectId, Mode::Exclude,
				formfilter::idOperands(line.objectExcluded, RE::FormType::NPC), false));
		}
		if (!line.skillRestrict.empty()) {
			std::vector<std::uint64_t> ranges;
			for (const auto& skillArr : line.skillRestrict) {
				const int lo = std::stoi(skillArr[1]);
				const int hi = std::stoi(skillArr[2]);
				std::uint64_t skill = 0;
				if (!lookupSkill(toLowerCase(skillArr[0]), skill) || lo < 0 || hi < 0) {
					continue;
				}
				ranges.push_back(skill);
				ranges.push_back(static_cast<std::uint64_t>(lo));
				ranges.push_back(static_cast<std::uint64_t>(hi));
			}
			if (!ranges.empty()) {
				p.negative.push_back(formfilter::makePredicate(p, Op::NpcSkillRange, Mode::And, ranges, false));
			}
		}
		if (!line.restrictToMaleModelContains.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::ModelContains, Mode::And,
				formfilter::needleOperands(p, line.restrictToMaleModelContains, true), false));
		}
		if (liveScalarFilter(line.restrictGender) &&
			flagPolarity(toLowerCase(line.restrictGender), "female", "male", expectSet)) {
			addFlagPredicate(p, p.negative, RE::ACTOR_BASE_DATA::Flag::kFemale, expectSet);
		}
		if (!line.restrictToTemplateFlags.empty()) {
			std::vector<std::uint64_t> bits;
			for (const auto& s : line.restrictToTemplateFlags) {
				const auto it = templateFlagMap().find(toLowerCase(s));
				if (it != templateFlagMap().end()) {
					bits.push_back(static_cast<std::uint64_t>(it->second));
				}
			}
			if (!bits.empty()) {
				p.negative.push_back(formfilter::makePredicate(p, Op::NpcTemplateFlag, Mode::And, bits, false));
			}
		}
		if (!line.restrictToFlags.empty()) {
			std::vector<std::uint64_t> bits;
			for (const auto& s : line.restrictToFlags) {
				const auto it = baseFlagMap().find(toLowerCase(s));
				if (it != baseFlagMap().end()) {
					bits.push_back(static_cast<std::uint64_t>(it->second));
				}
			}
			if (!bits.empty()) {
				p.negative.push_back(formfilter::makePredicate(p, Op::NpcFlag, Mode::And, bits, false));
			}
		}
		if (!line.restrictToCombatStyle.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::CombatStyle, Mode::Or,
				formfilter::idOperands(line.restrictToCombatStyle, RE::FormType::CombatStyle), false));
		}
		if (!line.restrictToRaces.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::Race, Mode::Or,
				formfilter::idOperands(line.restrictToRaces, RE::FormType::Race), false));
		}
		if (!line.restrictToVoiceType.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::VoiceType, Mode::Or,
				formfilter::idOperands(line.restrictToVoiceType, RE::FormType::VoiceType), false));
		}
		if (!line.restrictToKeyword.empty()) {
			p.negative.push_back(formfilter::makePredicate(p, Op::Keyword, Mode::Or,
				formfilter::ptrOperands(line.restrictToKeyword, RE::FormType::Keyword), false));
		}

		return p;
	}

	bool findObject(const patch_instruction& line, RE::TESNPC* curobj)
	{
		return findObject(line, makeFilterView(line), curobj);
	}

	bool findObject(const patch_instruction&, const formfilter::MatchProgram& program, RE::TESNPC* curobj)
	{
		static formfilter::MatchScratch scratch;

		formfilter::MatchTarget t;
		t.form = curobj;
		t.kf = static_cast<const RE::BGSKeywordForm*>(curobj);
		t.alt = static_cast<const RE::BGSKeywordForm*>(curobj->race);
		t.npc = curobj;
		t.isPlayer = curobj->formID == 0x000007;
		return formfilter::runProgram(program, t, scratch);
	}


	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x6, &Hook_Load);
		logger::debug("{} Done!", __FUNCTION__);
	}

	bool LoadRaceDataHook::Hook_Load(RE::TESNPC* curobj, RE::TESFile* a_mod)
	{
		auto result = func(curobj, a_mod);
		if (result) {

			for (const auto& line : hooktokens) {
				bool found = false;
				if (curobj->IsDeleted()) {
					return result;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const& modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				if (!line.object.empty()) {
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							if (curobj->formID == currentform->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (found || findObject(line, curobj))
					patch(line, curobj);
			}

		}
		return result;
	}

}
