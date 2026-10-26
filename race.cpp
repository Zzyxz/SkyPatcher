#include "race.h"
#include "formfilter.h"
#include "parsetable.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_set>

namespace RACE
{

	std::list<patch_instruction> hooktokens;

	struct HookTokenSource
	{
		std::string plugin;
		std::string path;
	};
	std::vector<HookTokenSource> hookTokenSources;

	namespace
	{
		bool namesPlugin(const std::string& a_fileName)
		{
			const std::string lowerName = toLowerCase(a_fileName);
			return lowerName.find(".esp") != std::string::npos || lowerName.find(".esl") != std::string::npos ||
			       lowerName.find(".esm") != std::string::npos;
		}

		float randomInRange(float a_lo, float a_hi)
		{
			static thread_local std::mt19937 engine{ std::random_device{}() };
			constexpr double kLimit = 1000000000.0;
			const int lo = static_cast<int>(std::clamp(std::ceil(static_cast<double>(std::min(a_lo, a_hi))), -kLimit, kLimit));
			const int hi = static_cast<int>(std::clamp(std::floor(static_cast<double>(std::max(a_lo, a_hi))), -kLimit, kLimit));
			if (lo > hi) {
				return a_lo;
			}
			std::uniform_int_distribution<int> distribution(lo, hi);
			return static_cast<float>(distribution(engine));
		}
	}

	namespace
	{
#define RACEHOOK_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kRaceHookKeys[] = {
			{ "filterByRaces", sp::FieldKind::List, RACEHOOK_F(object) },
			{ "filterByRacesExcluded", sp::FieldKind::List, RACEHOOK_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, RACEHOOK_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, RACEHOOK_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, RACEHOOK_F(keywordsExcluded) },
			{ "attackDataToAdd", sp::FieldKind::AttackData3D, RACEHOOK_F(addedObjects) },
			{ "attackDataToChange", sp::FieldKind::AttackData3D, RACEHOOK_F(attackDataToChange) },
			{ "attackDataToRemove", sp::FieldKind::ListAssign, RACEHOOK_F(removeAttackData) },
			{ "filterByModNames", sp::FieldKind::ListAssign, RACEHOOK_F(modNames) },
		};

#undef RACEHOOK_F

		const sp::KeyTable& raceHookKeyTable()
		{
			static const sp::KeyTable table(kRaceHookKeys, std::size(kRaceHookKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instruction_hook(const std::string& line)
	{
		patch_instruction hookl;
		raceHookKeyTable().parse(line, &hookl);
		return hookl;
	}
	namespace
	{
#define RACE_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kRaceKeys[] = {
			{ "filterByRaces", sp::FieldKind::List, RACE_F(object) },
			{ "filterByRacesExcluded", sp::FieldKind::List, RACE_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, RACE_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, RACE_F(keywordsOr) },
			{ "filterByVoiceTypeOr", sp::FieldKind::List, RACE_F(filterByVoiceTypeOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, RACE_F(keywordsExcluded) },
			{ "changeStats", sp::FieldKind::MultiFloat, RACE_F(avifs), RACE_F(values1), RACE_F(values2) },
			{ "keywordsToAdd", sp::FieldKind::List, RACE_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, RACE_F(keywordsToRemove) },
			{ "filterByMaleModelContains", sp::FieldKind::ListAssign, RACE_F(filterByMaleModelContains) },
			{ "filterByFemaleModelContains", sp::FieldKind::ListAssign, RACE_F(filterByFemaleModelContains) },
			{ "keywordToAddIfKeywordsEIDDoesNotContain", sp::FieldKind::Arr2D, RACE_F(keywordsToAddIfKeywordsEIDDoNotContain) },
			{ "spellsToAdd", sp::FieldKind::List, RACE_F(spellsToAdd) },
			{ "spellsToRemove", sp::FieldKind::List, RACE_F(spellsToRemove) },
			{ "shoutsToAdd", sp::FieldKind::List, RACE_F(shoutsToAdd) },
			{ "shoutsToRemove", sp::FieldKind::List, RACE_F(shoutsToRemove) },
			{ "levSpellsToAdd", sp::FieldKind::List, RACE_F(levSpellsToAdd) },
			{ "levSpellsToRemove", sp::FieldKind::List, RACE_F(levSpellsToRemove) },
			{ "baseMass", sp::FieldKind::Value, RACE_F(baseMass) },
			{ "baseCarryweight", sp::FieldKind::Value, RACE_F(baseCarryweight) },
			{ "skin", sp::FieldKind::Value, RACE_F(skin) },
			{ "voiceTypeFemale", sp::FieldKind::Value, RACE_F(voiceTypeFemale) },
			{ "voiceTypeMale", sp::FieldKind::Value, RACE_F(voiceTypeMale) },
			{ "startingHealth", sp::FieldKind::Value, RACE_F(startingHealth) },
			{ "startingStamina", sp::FieldKind::Value, RACE_F(startingStamina) },
			{ "startingMagicka", sp::FieldKind::Value, RACE_F(startingMagicka) },
			{ "regenHealth", sp::FieldKind::Value, RACE_F(regenHealth) },
			{ "regenStamina", sp::FieldKind::Value, RACE_F(regenStamina) },
			{ "regenMagicka", sp::FieldKind::Value, RACE_F(regenMagicka) },
			{ "damageUnarmed", sp::FieldKind::Value, RACE_F(damageUnarmed) },
			{ "reachUnarmed", sp::FieldKind::Value, RACE_F(reachUnarmed) },
			{ "baseMassMult", sp::FieldKind::Value, RACE_F(baseMassMult) },
			{ "baseCarryweightMult", sp::FieldKind::Value, RACE_F(baseCarryweightMult) },
			{ "startingHealthMult", sp::FieldKind::Value, RACE_F(startingHealthMult) },
			{ "startingStaminaMult", sp::FieldKind::Value, RACE_F(startingStaminaMult) },
			{ "startingMagickaMult", sp::FieldKind::Value, RACE_F(startingMagickaMult) },
			{ "regenHealthMult", sp::FieldKind::Value, RACE_F(regenHealthMult) },
			{ "regenStaminaMult", sp::FieldKind::Value, RACE_F(regenStaminaMult) },
			{ "regenMagickaMult", sp::FieldKind::Value, RACE_F(regenMagickaMult) },
			{ "damageUnarmedMult", sp::FieldKind::Value, RACE_F(damageUnarmedMult) },
			{ "reachUnarmedMult", sp::FieldKind::Value, RACE_F(reachUnarmedMult) },
			{ "weightMale", sp::FieldKind::Value, RACE_F(weightMale) },
			{ "weightFemale", sp::FieldKind::Value, RACE_F(weightFemale) },
			{ "heightMale", sp::FieldKind::Value, RACE_F(heightMale) },
			{ "heightFemale", sp::FieldKind::Value, RACE_F(heightFemale) },
			{ "accelerationRate", sp::FieldKind::Value, RACE_F(rateAcceleration) },
			{ "decelerationRate", sp::FieldKind::Value, RACE_F(rateDeceleration) },
			{ "accelerationAngularRate", sp::FieldKind::Value, RACE_F(angularAccelerationRate) },
			{ "aimAngleTolerance", sp::FieldKind::Value, RACE_F(aimAngleTolerance) },
			{ "filterByModNames", sp::FieldKind::ListAssign, RACE_F(modNames) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, RACE_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, RACE_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, RACE_F(filterByEditorIdContainsExcluded) },
		};

#undef RACE_F

		const sp::KeyTable& raceKeyTable()
		{
			static const sp::KeyTable table(kRaceKeys, std::size(kRaceKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;
		raceKeyTable().parse(line, &l);

		logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		return l;
	}

	namespace
	{
		formfilter::MatchProgram compileRaceRule(const patch_instruction& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;

			if (!line.object.empty()) {
				const std::size_t first = p.preds.size();
				formfilter::addPred(p, Op::ObjectId, Mode::Or,
					formfilter::idOperands(line.object, RE::FormType::Race));
				formfilter::pushDisjunct(p, p.positive, first);
			}

			const bool playerGuard =
				line.filterByEditorIdContains.empty() != line.filterByEditorIdContainsOr.empty();
			std::size_t first = p.preds.size();
			if (!line.filterByEditorIdContains.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::And,
					formfilter::needleOperands(p, line.filterByEditorIdContains), playerGuard);
			}
			if (!line.filterByEditorIdContainsOr.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::Or,
					formfilter::needleOperands(p, line.filterByEditorIdContainsOr), playerGuard);
			}
			formfilter::pushDisjunct(p, p.positive, first);

			first = p.preds.size();
			if (!line.keywords.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::And,
					formfilter::ptrOperands(line.keywords, RE::FormType::Keyword));
			}
			if (!line.keywordsOr.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::Or,
					formfilter::ptrOperands(line.keywordsOr, RE::FormType::Keyword));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			formfilter::sortCheapestFirst(p.positive);

			p.catchAll = line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() &&
				line.filterByVoiceTypeOr.empty() && line.filterByMaleModelContains.empty() &&
				line.filterByFemaleModelContains.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty();

			return p;
		}

		struct RaceRuleExtras
		{
			std::vector<std::string> maleNeedles;
			std::vector<std::string> femaleNeedles;
			std::vector<RE::FormID>  voiceTypes;
			bool                     playerGuard = false;
		};

		RaceRuleExtras compileRaceExtras(const patch_instruction& line)
		{
			RaceRuleExtras x;
			x.maleNeedles.reserve(line.filterByMaleModelContains.size());
			for (const auto& s : line.filterByMaleModelContains) {
				x.maleNeedles.push_back(normalizePath(trim(toLowerCase(s))));
			}
			x.femaleNeedles.reserve(line.filterByFemaleModelContains.size());
			for (const auto& s : line.filterByFemaleModelContains) {
				x.femaleNeedles.push_back(normalizePath(trim(toLowerCase(s))));
			}
			x.voiceTypes = formfilter::resolveFormIDs(line.filterByVoiceTypeOr, RE::FormType::VoiceType);
			x.playerGuard =
				line.filterByEditorIdContains.empty() != line.filterByEditorIdContainsOr.empty();
			return x;
		}

		const std::string& loweredEditorId(const RE::TESForm* a_form, formfilter::MatchScratch& a_s)
		{
			if (!a_s.haveEditorId) {
				a_s.editorId = toLowerCase(clib_util::editorID::get_editorID(a_form));
				a_s.haveEditorId = true;
			}
			return a_s.editorId;
		}
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                 dataHandler = RE::TESDataHandler::GetSingleton();
		auto& RaceArray = dataHandler->GetFormArray<RE::TESRace>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESRace* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Race) {
						curobj = (RE::TESRace*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByVoiceTypeOr.empty() && line.filterByMaleModelContains.empty() && line.filterByFemaleModelContains.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			const formfilter::MatchProgram          program = compileRaceRule(line);
			const RaceRuleExtras                    extra = compileRaceExtras(line);
			const formfilter::ResolvedCommonFilters resolved =
				formfilter::resolve(view, RE::FormType::Race);

			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;

			std::string maleModel, femaleModel;

			for (const auto& curobj : RaceArray) {
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(program, curobj)) {
					continue;
				}

				target.form = curobj;
				target.kf = curobj;
				target.isPlayer = extra.playerGuard && curobj->formID == 0x000007;

				bool found = formfilter::runProgram(program, target, scratch);

				if (!found && !extra.maleNeedles.empty()) {
					maleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kMale].model.c_str())));
					for (const auto& searchModel : extra.maleNeedles) {
						if (maleModel.find(searchModel) != std::string::npos) {
							found = true;
							break;
						}
					}
				}

				if (!found && !extra.femaleNeedles.empty()) {
					femaleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kFemale].model.c_str())));
					for (const auto& searchModel : extra.femaleNeedles) {
						if (femaleModel.find(searchModel) != std::string::npos) {
							found = true;
							break;
						}
					}
				}

				if (!found && !extra.voiceTypes.empty()) {
					for (const RE::FormID id : extra.voiceTypes) {
						if (curobj->defaultVoiceTypes[RE::SEXES::kMale] && curobj->defaultVoiceTypes[RE::SEXES::kMale]->formID == id || curobj->defaultVoiceTypes[RE::SEXES::kFemale] && curobj->defaultVoiceTypes[RE::SEXES::kFemale]->formID == id) {
							found = true;
							break;
						}
					}
				}

				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(resolved, curobj, nullptr)) {
						found = false;
					}
				}



				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(resolved, curobj)) {
						found = false;
					}
				}

				if (!resolved.editorIdContainsExcluded.empty() && found) {
					const std::string& editorId = loweredEditorId(curobj, scratch);
					for (const auto& needle : resolved.editorIdContainsExcluded) {
						if (editorId.find(needle) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (found) {
					if (!patched.contains(curobj->formID)) {
						patch(line, curobj);
					}
				}
			}
		}
	}

	void hookpatch(const RACE::patch_instruction& line, RE::TESRace* curobj)
	{
		if (!curobj->attackDataMap) {
			logger::debug(FMT_STRING("race formid: {:08X} has no attack data, attack data rules skipped"), curobj->formID);
			return;
		}
		if (!line.addedObjects.empty()) {
			for (auto objectArr : line.addedObjects) {
				RE::NiPointer<RE::BGSAttackData> attackDataPtr{ ngcompat::CreateAttackData() };
				if (!attackDataPtr) {
					continue;
				}
				RE::BGSAttackData* newAttackData = attackDataPtr.get();
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
				for (auto object : objectArr) {
					if (object.empty() || (object.size() < 2 && toLowerCase(object[0]) != "resetflags" && toLowerCase(object[0]) != "removeflags")) {
						continue;
					}
					if (toLowerCase(object[0]) == "key") {
						newAttackData->event = object[1];
						logger::debug("added event: {}", object[1]);
					} else if (toLowerCase(object[0]) == "damagemult") {
						newAttackData->data.damageMult = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
					} else if (toLowerCase(object[0]) == "attackchance") {
						newAttackData->data.attackChance = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
					} else if (toLowerCase(object[0]) == "attackspell") {
						if (!IsNullValue(object[1])) {
							auto form = GetFormFromIdentifier(object[1]);
							if (form && (form->formType == RE::FormType::Spell || form->formType == RE::FormType::Scroll)) {
								newAttackData->data.attackSpell = static_cast<RE::SpellItem*>(form);
								logger::debug("race formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
							} else if (form) {
								logger::warn("race formid: {:08X} attackData - attackSpell {} is not a spell", curobj->formID, object[1]);
							}
						} else {
							newAttackData->data.attackSpell = nullptr;
							logger::debug("race formid: {:08X} attackData - attackspell nullptr", curobj->formID);
						}
					} else if (toLowerCase(object[0]) == "attackangle") {
						newAttackData->data.attackAngle = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
					} else if (toLowerCase(object[0]) == "strikeangle") {
						newAttackData->data.strikeAngle = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
					} else if (toLowerCase(object[0]) == "staggeroffset") {
						newAttackData->data.staggerOffset = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
					} else if (toLowerCase(object[0]) == "attacktype") {
						if (!IsNullValue(object[1])) {
							auto form = GetFormFromIdentifier(object[1]);
							if (form && form->formType == RE::FormType::Keyword) {
								newAttackData->data.attackType = static_cast<RE::BGSKeyword*>(form);
								logger::debug("race formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
							} else if (form) {
								logger::warn("race formid: {:08X} attackData - attackType {} is not a keyword", curobj->formID, object[1]);
							}
						} else {
							newAttackData->data.attackType = nullptr;
							logger::debug("race formid: {:08X} attackData - attacktype nullptr", curobj->formID);
						}
					} else if (toLowerCase(object[0]) == "knockdown") {
						newAttackData->data.knockDown = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
					} else if (toLowerCase(object[0]) == "recoverytime") {
						newAttackData->data.recoveryTime = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
					} else if (toLowerCase(object[0]) == "staminamult") {
						newAttackData->data.staminaMult = std::stof(object[1]);
						logger::debug("race formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
					} else if (toLowerCase(object[0]) == "resetflags") {
						newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
						logger::debug(FMT_STRING("race formid: {:08X} attackData - all flags reset"), curobj->formID);
					} else if (toLowerCase(object[0]) == "removeflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerRemoveFlag = toLowerCase(object[i]);
							if (lowerRemoveFlag == "ignoreweapon") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
							} else if (lowerRemoveFlag == "bashattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "powerattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "chargeattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "rotatingattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "continuousattack") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
							} else if (lowerRemoveFlag == "overridedata") {
								newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
							} else {
							}
						}

					} else if (toLowerCase(object[0]) == "setflags") {
						for (int i = 1; i < object.size(); ++i) {
							auto lowerSetFlag = toLowerCase(object[i]);
							if (lowerSetFlag == "ignoreweapon") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
							} else if (lowerSetFlag == "bashattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
							} else if (lowerSetFlag == "powerattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
							} else if (lowerSetFlag == "chargeattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
							} else if (lowerSetFlag == "rotatingattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
							} else if (lowerSetFlag == "continuousattack") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
							} else if (lowerSetFlag == "overridedata") {
								newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
								logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
							} else {
							}
						}
					} else {
						logger::debug("Unrecognized keyword: {}", object[0]);
					}
				}

				if (!ngcompat::AddOrReplaceAttackData(curobj->attackDataMap.get(), std::move(attackDataPtr))) {
					logger::warn("race {:08X}: an attack in attackDataToAdd has no key and was skipped", curobj->formID);
				}
			}
		}

		if (!line.removeAttackData.empty()) {
			for (auto attackData : line.removeAttackData) {
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					if (it->second && toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
						logger::debug("race formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					} else {
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
						if (object.size() < 2) {
							continue;
						}

						if (key) {
							if (toLowerCase(newAttackData->event.c_str()) != toLowerCase(object[1])) {
								break;
							} else {
								logger::debug("race formid: {:08X} attackData change - entry found {}", curobj->formID, newAttackData->event.c_str());
								key = false;
								continue;
							}
						}

						if (toLowerCase(object[0]) == "damagemult") {
							newAttackData->data.damageMult = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - damagemult {}", curobj->formID, newAttackData->data.damageMult);
						} else if (toLowerCase(object[0]) == "attackchance") {
							newAttackData->data.attackChance = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - attackchance {}", curobj->formID, newAttackData->data.attackChance);
						} else if (toLowerCase(object[0]) == "attackspell") {
							if (!IsNullValue(object[1])) {
								auto form = GetFormFromIdentifier(object[1]);
								if (form && (form->formType == RE::FormType::Spell || form->formType == RE::FormType::Scroll)) {
									newAttackData->data.attackSpell = static_cast<RE::SpellItem*>(form);
									logger::debug("race formid: {:08X} attackData changed - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
								} else if (form) {
									logger::warn("race formid: {:08X} attackData changed - attackSpell {} is not a spell", curobj->formID, object[1]);
								}
							} else {
								newAttackData->data.attackSpell = nullptr;
								logger::debug("race formid: {:08X} attackData changed - attackspell nullptr", curobj->formID);
							}
						} else if (toLowerCase(object[0]) == "attackangle") {
							newAttackData->data.attackAngle = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
						} else if (toLowerCase(object[0]) == "strikeangle") {
							newAttackData->data.strikeAngle = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
						} else if (toLowerCase(object[0]) == "staggeroffset") {
							newAttackData->data.staggerOffset = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
						} else if (toLowerCase(object[0]) == "attacktype") {
							if (!IsNullValue(object[1])) {
								auto form = GetFormFromIdentifier(object[1]);
								if (form && form->formType == RE::FormType::Keyword) {
									newAttackData->data.attackType = static_cast<RE::BGSKeyword*>(form);
									logger::debug("race formid: {:08X} attackData changed - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
								} else if (form) {
									logger::warn("race formid: {:08X} attackData changed - attackType {} is not a keyword", curobj->formID, object[1]);
								}
							} else {
								newAttackData->data.attackType = nullptr;
								logger::debug("race formid: {:08X} attackData changed - attacktype nullptr", curobj->formID);
							}
						} else if (toLowerCase(object[0]) == "knockdown") {
							newAttackData->data.knockDown = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - knockdown {}", curobj->formID, newAttackData->data.knockDown);
						} else if (toLowerCase(object[0]) == "recoverytime") {
							newAttackData->data.recoveryTime = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
						} else if (toLowerCase(object[0]) == "staminamult") {
							newAttackData->data.staminaMult = std::stof(object[1]);
							logger::debug("race formid: {:08X} attackData changed - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
						} else if (toLowerCase(object[0]) == "resetflags") {
							newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
							logger::debug(FMT_STRING("race formid: {:08X} attackData - all flags reset"), curobj->formID);
						} else if (toLowerCase(object[0]) == "removeflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerRemoveFlag = toLowerCase(object[i]);
								if (lowerRemoveFlag == "ignoreweapon") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled IgnoreWeapon"), curobj->formID);
								} else if (lowerRemoveFlag == "bashattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled BashAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "powerattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled PowerAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "chargeattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ChargeAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "rotatingattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled RotatingAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "continuousattack") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled ContinuousAttack"), curobj->formID);
								} else if (lowerRemoveFlag == "overridedata") {
									newAttackData->data.flags.reset(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag disabled OverrideData"), curobj->formID);
								} else {
								}
							}

						} else if (toLowerCase(object[0]) == "setflags") {
							for (int i = 1; i < object.size(); ++i) {
								auto lowerSetFlag = toLowerCase(object[i]);
								if (lowerSetFlag == "ignoreweapon") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kIgnoreWeapon);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled IgnoreWeapon"), curobj->formID);
								} else if (lowerSetFlag == "bashattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kBashAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled BashAttack"), curobj->formID);
								} else if (lowerSetFlag == "powerattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled PowerAttack"), curobj->formID);
								} else if (lowerSetFlag == "chargeattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kChargeAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ChargeAttack"), curobj->formID);
								} else if (lowerSetFlag == "rotatingattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kRotatingAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled RotatingAttack"), curobj->formID);
								} else if (lowerSetFlag == "continuousattack") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kContinuousAttack);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled ContinuousAttack"), curobj->formID);
								} else if (lowerSetFlag == "overridedata") {
									newAttackData->data.flags.set(RE::AttackData::AttackFlag::kOverrideData);
									logger::debug(FMT_STRING("race formid: {:08X} attackData - flag enabled OverrideData"), curobj->formID);
								} else {
								}
							}
						} else {
							logger::debug("Unrecognized keyword: {}", object[0]);
						}
					}
				}
			}
		}
	}

	void patch(const RACE::patch_instruction& line, RE::TESRace* curobj)
	{
		try {
		if (!line.avifs.empty()) {
			for (size_t i = 0; i < line.avifs.size(); i++) {
				std::string  string_form = line.avifs[i];

				float randomValue = 0;
				if (i < line.values1.size() && i < line.values2.size()) {
					const float lo = line.values1[i];
					const float hi = line.values2[i];
					randomValue = lo == hi ? lo : randomInRange(lo, hi);
				}

				const std::string lowerStat = toLowerCase(string_form);
				if (lowerStat == "health") {
					curobj->data.startingHealth = randomValue;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (lowerStat == "stamina") {
					curobj->data.startingStamina = randomValue;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (lowerStat == "magicka") {
					curobj->data.startingMagicka = randomValue;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else {
					logger::warn(FMT_STRING("race formid: {:08X} changeStats: unknown stat '{}' (health, stamina, magicka)"), curobj->formID, string_form);
				}
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToAddIfKeywordsEIDDoNotContain.empty()) {
			for (const auto& objectToAdd : line.keywordsToAddIfKeywordsEIDDoNotContain) {
				if (objectToAdd.size() < 2) {
					continue;
				}
				std::string        eidkeyword = (objectToAdd[0]);
				std::string  keywordString = (objectToAdd[1]);
				RE::TESForm* currentform = nullptr;
				bool               addKeyword = true;
				currentform = GetFormFromIdentifier(keywordString);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					for (std::uint32_t i = 0; i < curobj->numKeywords; ++i) {
						auto keyword = curobj->keywords[i];
						if (keyword && toLowerCase(keyword->formEditorID.c_str()).find(toLowerCase(eidkeyword)) != std::string::npos) {
							addKeyword = false;
						}

					}

					if (addKeyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} (keywordToAddIfKeywordsEIDDoNotContain)"), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.spellsToAdd.empty()) {
			for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddSpell((RE::SpellItem*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} added spell {:08X} "), curobj->formID, ((RE::SpellItem*)currentform)->formID);
				}
			}
		}

		if (!line.spellsToRemove.empty()) {
			for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Spell) {
					if (curobj->actorEffects) {
						curobj->actorEffects->RemoveSpell((RE::SpellItem*)currentform);
					}
					logger::debug(FMT_STRING("race formid: {:08X} removed spell {:08X} "), curobj->formID, ((RE::SpellItem*)currentform)->formID);
				}
			}
		}

		if (!line.levSpellsToAdd.empty()) {
			for (size_t i = 0; i < line.levSpellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.levSpellsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddLevSpell((RE::TESLevSpell*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} added levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
				}
			}
		}

		if (!line.levSpellsToRemove.empty()) {
			for (size_t i = 0; i < line.levSpellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.levSpellsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
					if (curobj->actorEffects) {
						curobj->actorEffects->RemoveLevSpell((RE::TESLevSpell*)currentform);
					}
					logger::debug(FMT_STRING("race formid: {:08X} removed levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
				}
			}
		}

		if (!line.shoutsToAdd.empty()) {
			for (size_t i = 0; i < line.shoutsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.shoutsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Shout) {
					if (!curobj->actorEffects) {
						curobj->actorEffects = new RE::TESSpellList::SpellData();
					}
					curobj->actorEffects->AddShout((RE::TESShout*)currentform);
					logger::debug(FMT_STRING("race formid: {:08X} added shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
				}
			}
		}

		if (!line.shoutsToRemove.empty()) {
			for (size_t i = 0; i < line.shoutsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.shoutsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Shout) {
					if (curobj->actorEffects) {
						curobj->actorEffects->RemoveShout((RE::TESShout*)currentform);
					}
					logger::debug(FMT_STRING("race formid: {:08X} removed shout {:08X} "), curobj->formID, ((RE::TESShout*)currentform)->formID);
				}
			}
		}

		if (!line.baseMass.empty() && line.baseMass != "none") {
			try {
				curobj->data.baseMass = std::stof(line.baseMass);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed baseMass {}"), curobj->formID, curobj->fullName, curobj->data.baseMass);
			} catch (const std::exception& e) {
			}
		}

		if (!line.baseCarryweight.empty() && line.baseCarryweight != "none") {
			try {
				curobj->data.baseCarryWeight = std::stof(line.baseCarryweight);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingHealth.empty() && line.startingHealth != "none") {
			try {
				curobj->data.startingHealth = std::stof(line.startingHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingStamina.empty() && line.startingStamina != "none") {
			try {
				curobj->data.startingStamina = std::stof(line.startingStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingMagicka.empty() && line.startingMagicka != "none") {
			try {
				curobj->data.startingMagicka = std::stof(line.startingMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenHealth.empty() && line.regenHealth != "none") {
			try {
				curobj->data.healthRegen = std::stof(line.regenHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenStamina.empty() && line.regenStamina != "none") {
			try {
				curobj->data.staminaRegen = std::stof(line.regenStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenMagicka.empty() && line.regenMagicka != "none") {
			try {
				curobj->data.magickaRegen = std::stof(line.regenMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.damageUnarmed.empty() && line.damageUnarmed != "none") {
			try {
				curobj->data.unarmedDamage = std::stof(line.damageUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::exception& e) {
			}
		}

		if (!line.reachUnarmed.empty() && line.reachUnarmed != "none") {
			try {
				curobj->data.unarmedReach = std::stof(line.reachUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::exception& e) {
			}
		}

		if (!line.baseMassMult.empty() && line.baseMassMult != "none") {
			try {
				curobj->data.baseMass = curobj->data.baseMass * std::stof(line.baseMassMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseMass {}"), curobj->formID, curobj->fullName, curobj->data.baseMass);
			} catch (const std::exception& e) {
			}
		}

		if (!line.baseCarryweightMult.empty() && line.baseCarryweightMult != "none") {
			try {
				curobj->data.baseCarryWeight = curobj->data.baseCarryWeight * std::stof(line.baseCarryweightMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingHealthMult.empty() && line.startingHealthMult != "none") {
			try {
				curobj->data.startingHealth = curobj->data.startingHealth * std::stof(line.startingHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingStaminaMult.empty() && line.startingStaminaMult != "none") {
			try {
				curobj->data.startingStamina = curobj->data.startingStamina * std::stof(line.startingStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::exception& e) {
			}
		}

		if (!line.startingMagickaMult.empty() && line.startingMagickaMult != "none") {
			try {
				curobj->data.startingMagicka = curobj->data.startingMagicka * std::stof(line.startingMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenHealthMult.empty() && line.regenHealthMult != "none") {
			try {
				curobj->data.healthRegen = curobj->data.healthRegen * std::stof(line.regenHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenStaminaMult.empty() && line.regenStaminaMult != "none") {
			try {
				curobj->data.staminaRegen = curobj->data.staminaRegen * std::stof(line.regenStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.regenMagickaMult.empty() && line.regenMagickaMult != "none") {
			try {
				curobj->data.magickaRegen = curobj->data.magickaRegen * std::stof(line.regenMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::exception& e) {
			}
		}

		if (!line.damageUnarmedMult.empty() && line.damageUnarmedMult != "none") {
			try {
				curobj->data.unarmedDamage = curobj->data.unarmedDamage * std::stof(line.damageUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::exception& e) {
			}
		}

		if (!line.reachUnarmedMult.empty() && line.reachUnarmedMult != "none") {
			try {
				curobj->data.unarmedReach = curobj->data.unarmedReach * std::stof(line.reachUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::exception& e) {
			}
		}

		if (!line.skin.empty() && line.skin != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.skin;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					curobj->skin = (RE::TESObjectARMO*)currentform;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed skin {:08X}"), curobj->formID, curobj->fullName, curobj->skin->formID);
				} else if (IsNullValue(line.skin)) {
					curobj->skin = nullptr;
					logger::debug(FMT_STRING("race formid: {:08X} {} changed skin to null (none)"), curobj->formID, curobj->fullName);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.voiceTypeMale.empty() && line.voiceTypeMale != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.voiceTypeMale;
				currentform = GetFormFromIdentifier(string_form);

				if (currentform && currentform->formType == RE::FormType::VoiceType) {
					curobj->defaultVoiceTypes[RE::SEXES::kMale] = static_cast<RE::BGSVoiceType*>(currentform);

					logger::debug(
						FMT_STRING("race formid: {:08X} {} changed voiceTypeMale {:08X}"),
						curobj->formID,
						curobj->fullName,
						curobj->defaultVoiceTypes[RE::SEXES::kMale]->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.voiceTypeFemale.empty() && line.voiceTypeFemale != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.voiceTypeFemale;
				currentform = GetFormFromIdentifier(string_form);

				if (currentform && currentform->formType == RE::FormType::VoiceType) {
					curobj->defaultVoiceTypes[RE::SEXES::kFemale] = static_cast<RE::BGSVoiceType*>(currentform);

					logger::debug(
						FMT_STRING("race formid: {:08X} {} changed voiceTypeFemale {:08X}"),
						curobj->formID,
						curobj->fullName,
						curobj->defaultVoiceTypes[RE::SEXES::kFemale]->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.heightMale.empty() && line.heightMale != "none") {
			try {
				curobj->data.height[RE::SEXES::kMale] = std::stof(line.heightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightMale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kMale]);
			} catch (const std::exception& e) {
			}
		}

		if (!line.heightFemale.empty() && line.heightFemale != "none") {
			try {
				curobj->data.height[RE::SEXES::kFemale] = std::stof(line.heightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightFemale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kFemale]);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMale.empty() && line.weightMale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kMale] = std::stof(line.weightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightMale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kMale]);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightFemale.empty() && line.weightFemale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kFemale] = std::stof(line.weightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightFemale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kFemale]);
			} catch (const std::exception& e) {
			}
		}

		if (!line.rateAcceleration.empty() && line.rateAcceleration != "none") {
			try {
				curobj->data.accelerate = std::stof(line.rateAcceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateAcceleration {}"), curobj->formID, curobj->fullName, curobj->data.accelerate);
			} catch (const std::exception& e) {
			}
		}

		if (!line.rateDeceleration.empty() && line.rateDeceleration != "none") {
			try {
				curobj->data.decelerate = std::stof(line.rateDeceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateDeceleration {}"), curobj->formID, curobj->fullName, curobj->data.decelerate);
			} catch (const std::exception& e) {
			}
		}

		if (!line.angularAccelerationRate.empty() && line.angularAccelerationRate != "none") {
			try {
				curobj->data.angleAccelerate = std::stof(line.angularAccelerationRate);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed angularAccelerationRate {}"), curobj->formID, curobj->fullName, curobj->data.angleAccelerate);
			} catch (const std::exception& e) {
			}
		}

		if (!line.aimAngleTolerance.empty() && line.aimAngleTolerance != "none") {
			try {
				curobj->data.aimAngleTolerance = std::stof(line.aimAngleTolerance);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed aimAngleTolerance {}"), curobj->formID, curobj->fullName, curobj->data.aimAngleTolerance);
			} catch (const std::exception& e) {
			}
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("race {:08X}: skipped remaining operations after invalid value: {}"), curobj ? curobj->formID : 0, e.what());
		}
	}

	void* readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR*                   dir;
		struct dirent*         ent;
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
						} else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();

								if (namesPlugin(fileName)) {
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
									line = StripLineComment(line);
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									tokens.push_back(create_patch_instruction(line));
								}
								infile.close();
								process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return nullptr;
	}
	void* readConfigHook(const std::string& folder)
	{
		hooktokens.clear();
		hookTokenSources.clear();
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR*                   dir;
		struct dirent*         ent;
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
						} else {
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
									line = StripLineComment(line);
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction_hook(line));
									hookTokenSources.push_back({ namesPlugin(fileName) ? fileName : std::string(), fullPath });
								}
								infile.close();
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return nullptr;
	}

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x13, &Hook_InitItemImpl);
	}

	void LoadRaceDataHook::Hook_InitItemImpl(RE::TESRace* curobj)
	{
		const bool wasInitialized = curobj->IsInitialized();
		func(curobj);
		if (!wasInitialized) {

			static std::vector<bool> tokenActive;
			static bool              tokenActiveReady = false;
			if (!tokenActiveReady) {
				std::vector<bool> active(hooktokens.size(), true);
				const auto* dataHandler = RE::TESDataHandler::GetSingleton();
				const bool  loadOrderKnown = dataHandler &&
					(dataHandler->GetLoadedModCount() > 0 || dataHandler->GetLoadedLightModCount() > 0);
				if (loadOrderKnown) {
					tokenActiveReady = true;
					std::unordered_set<std::string> loggedPaths;
					for (std::size_t i = 0; i < hookTokenSources.size() && i < active.size(); ++i) {
						const auto& source = hookTokenSources[i];
						if (source.plugin.empty() || IsPluginInstalled(source.plugin.c_str())) {
							continue;
						}
						active[i] = false;
						if (loggedPaths.insert(source.path).second) {
							logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), source.plugin, source.path);
						}
					}
				}
				tokenActive = std::move(active);
			}

			std::size_t tokenIndex = 0;
			for (const auto& line : hooktokens) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

				if (curobj->IsDeleted()) {
					return;
				}

				const bool tokenUsed = tokenIndex >= tokenActive.size() || tokenActive[tokenIndex];
				++tokenIndex;
				if (!tokenUsed) {
					continue;
				}

				const formfilter::CommonFilterView view{
					line.modNames, formfilter::noStrings(), formfilter::noStrings(),
					line.keywords, line.keywordsOr, line.keywordsExcluded,
					line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
					line.filterByEditorIdContainsExcluded, line.objectExcluded
				};

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!line.object.empty()) {
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Race) {
							if (curobj->formID == currentform->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!line.keywords.empty()) {
					keywordAnd = formfilter::matchesKeywordsAnd(view, curobj, nullptr);
				} else {
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					keywordOr = formfilter::matchesKeywordsOr(view, curobj, nullptr);
				} else {
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					found = true;
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Race)) {
						found = false;
					}
				}

				if (found) {
					try {
						hookpatch(line, curobj);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("race {:08X}: an attack data rule was skipped: {}"), curobj->formID, e.what());
					}
				}
			}

		}
	}
}
