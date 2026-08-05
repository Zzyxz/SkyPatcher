#include "race.h"
#include "formfilter.h"
#include "parsetable.h"

namespace RACE
{

	std::list<patch_instruction> hooktokens;

	// ======================================================================
	// PHASE 2 -- RACE's two key tables (create_patch_instruction_hook and
	// create_patch_instruction both build the same `patch_instruction`, but
	// with different call lists, so each gets its own table). One row per
	// upstream (active) extract*() call / bespoke block, in the same order.
	// See parsetable.h.
	//
	// "attackDataToAdd" / "attackDataToChange" in the hook table: the
	// upstream inline block (comma x '~' x '=' grid, trimmed with
	// utility::trim -- SPACES only, matching parsetable.h's trimSpaces) is
	// byte-for-byte parsetable.h's detail::applyAttackData3D, so
	// FieldKind::AttackData3D is exact, not an approximation.
	//
	// "changeStats" in the main table: the upstream inline "avifs" block
	// uses the pattern
	//   ([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\s*=\s*([\d.]+)(?:\s*~\s*([\d.]+))?
	// which is character-for-character detail::factionRankRegex() (the same
	// regex NPC's factionsToAdd uses), applied to the same
	// (names, lo, hi) triple shape, including the "none" first-name break.
	// FieldKind::FactionRanks is therefore exact here too.
	//
	// NOT converted: "startingHealthMult" / "startingStaminaMult" /
	// "startingMagickaMult". Upstream's patterns are
	// "KEY\s*= ([^:]+)" -- note the LITERAL SPACE between '=' and the
	// capture group, which every other Value key omits. That requires a
	// literal space immediately after '=' in the config line itself (e.g.
	// "startingHealthMult= 5" matches but "startingHealthMult=5" does not,
	// leaving the field "none"), which is not the "KEY\s*=([^:]+)" shape
	// parsetable.h's FieldKind::Value assumes (see parsetable.h's exactness
	// argument, property 1). Left on the original extractValueString calls
	// below, appended after the table parse.
	// ======================================================================
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
		//logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		//logger::info("returning patch instructions");
		return hookl;
	}
	//
	//
	//
	//
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
			{ "changeStats", sp::FieldKind::FactionRanks, RACE_F(avifs), RACE_F(values1), RACE_F(values2) },
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
			// GAP: startingHealthMult / startingStaminaMult / startingMagickaMult
			// are NOT here -- see the comment above kRaceHookKeys. Applied via
			// the original extractValueString calls after the table parse.
			{ "regenHealthMult", sp::FieldKind::Value, RACE_F(regenHealthMult) },
			{ "regenStaminaMult", sp::FieldKind::Value, RACE_F(regenStaminaMult) },
			{ "regenMagickaMult", sp::FieldKind::Value, RACE_F(regenMagickaMult) },
			{ "damageUnarmedMult", sp::FieldKind::Value, RACE_F(damageUnarmedMult) },
			{ "reachUnarmedMult", sp::FieldKind::Value, RACE_F(reachUnarmedMult) },
			{ "weightMale", sp::FieldKind::Value, RACE_F(weightMale) },
			{ "weightFemale", sp::FieldKind::Value, RACE_F(weightFemale) },
			{ "heightMale", sp::FieldKind::Value, RACE_F(heightMale) },
			{ "heightFemale", sp::FieldKind::Value, RACE_F(heightFemale) },
			// upstream extracts "heightFemale" twice into the same member;
			// kept as two rows for a literal transliteration (harmless, like
			// WEAPON's duplicate "soundLevel" rows).
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

		// GAP: not on the table -- see the comment above kRaceHookKeys.
		extractValueString(line, "startingHealthMult\\s*= ([^:]+)", l.startingHealthMult);
		extractValueString(line, "startingStaminaMult\\s*= ([^:]+)", l.startingStaminaMult);
		extractValueString(line, "startingMagickaMult\\s*= ([^:]+)", l.startingMagickaMult);

		logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		//logger::info("returning patch instructions");
		return l;
	}

	// ==================================================================
	// PHASE 2 -- RACE's rule compiler, for the MAIN (scan) path only.
	//
	// PARTIAL CONVERSION, deliberately. Compiled into a
	// formfilter::MatchProgram: filterByRaces (an own-formID compare, i.e.
	// Op::ObjectId), the EditorID pair, the keyword pair and the catch-all.
	//
	// LEFT INLINE, because the opcode table has no equivalent:
	//   * filterByMaleModelContains / filterByFemaleModelContains --
	//     Op::ModelContains exists but reads `npc->race->skeletonModels[kMale]`
	//     and passes VACUOUSLY for a raceless NPC. Here the subject IS the
	//     race, both sexes are filtered, and there is no vacuous case. Using
	//     it would mean widening an opcode three verified types share.
	//   * filterByVoiceTypeOr -- Op::VoiceType reads `npc->voiceType`; a race
	//     has TWO (defaultVoiceTypes[kMale] and [kFemale]) and either may
	//     match.
	// Both are still hoisted: the needles are normalised once per rule
	// (upstream re-ran normalizePath(trim(toLowerCase(needle))) on every
	// (pair, needle)), the race's own model path is computed once per pair
	// (upstream re-ran it on every needle), and the voice types are resolved
	// once per rule.
	//
	// The HOOK path (LoadRaceDataHook::Hook_InitItemImpl, below) is NOT
	// converted -- see the note there.
	// ==================================================================
	namespace
	{
		formfilter::MatchProgram compileRaceRule(const patch_instruction& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;

			// filterByRaces, re-tested inside the scan: `curobj->formID ==
			// namedRace->formID` for any identifier that resolves to a Race.
			if (!line.object.empty()) {
				const std::size_t first = p.preds.size();
				formfilter::addPred(p, Op::ObjectId, Mode::Or,
					formfilter::idOperands(line.object, RE::FormType::Race));
				formfilter::pushDisjunct(p, p.positive, first);
			}

			// DRIFT, PRESERVED: the inert `formID != 0x000007` player guard.
			// It is only reached when the corresponding list is EMPTY, so it
			// can only matter when exactly one of the two is present -- which
			// is exactly Predicate::skipIfPlayer. A TESRace can never carry
			// the player NPC's formID.
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
			std::vector<std::string> maleNeedles;    // normalizePath(trim(toLowerCase(...)))
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
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESRace* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Race) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESRace*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByVoiceTypeOr.empty() && line.filterByMaleModelContains.empty() && line.filterByFemaleModelContains.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			//logger::info("processing config line");
			// PROTOTYPE: one view per rule, not per (rule, race) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			// PHASE 1 + 2: the rule is resolved and compiled ONCE.
			const formfilter::MatchProgram          program = compileRaceRule(line);
			const RaceRuleExtras                    extra = compileRaceExtras(line);
			const formfilter::ResolvedCommonFilters resolved =
				formfilter::resolve(view, RE::FormType::Race);

			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;

			// The two skeleton paths are memoised per PAIR: upstream rebuilt
			// normalizePath(trim(toLowerCase(...))) inside the needle loop.
			std::string maleModel, femaleModel;

			for (const auto& curobj : RaceArray) {
				//logger::info("processing npc");
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(program, curobj)) {
					continue;
				}

				target.form = curobj;
				target.kf = curobj;
				target.isPlayer = extra.playerGuard && curobj->formID == 0x000007;

				// filterByRaces + EditorID pair + keyword pair + catch-all.
				bool found = formfilter::runProgram(program, target, scratch);

				// --- filters the opcode table does not cover ----------------
				// All three only ever write `found = true`, so evaluating them
				// after the program computes the same OR upstream did.
				if (!found && !extra.maleNeedles.empty()) {
					maleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kMale].model.c_str())));
					for (const auto& searchModel : extra.maleNeedles) {
						if (maleModel.find(searchModel) != std::string::npos) {
							found = true;
							//logger::info("found model");
							break;
						}
					}
				}

				if (!found && !extra.femaleNeedles.empty()) {
					femaleModel = normalizePath(trim(toLowerCase(curobj->skeletonModels[RE::SEXES::kFemale].model.c_str())));
					for (const auto& searchModel : extra.femaleNeedles) {
						if (femaleModel.find(searchModel) != std::string::npos) {
							found = true;
							//logger::info("found model");
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

				// --- negative stage, in upstream source order ---------------
				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(resolved, curobj, nullptr)) {
						found = false;
					}
				}



				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty")
					// Migration fix: was previously RE::FormType::Keyword (copy-paste
					// bug), which made this exclusion never match any race. The hook
					// path's copy of this same filter (below) always used the correct
					// RE::FormType::Race.
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
					patch(line, curobj);
				}

				//
			}
		}
	}

	//
	void hookpatch(const RACE::patch_instruction& line, RE::TESRace* curobj)
	{
		//logger::debug("start Patch race {:08X}", curobj->formID);
		if (!line.addedObjects.empty()) {
			//bool               key = true;
			RE::BGSAttackData* newAttackData = RE::BGSAttackData::Create();
			newAttackData->IncRefCount();
			//newAttackData->event = "attackStart";
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
			//logger::debug("attackData size before {}", curobj->attackDataMap.get()->attackDataMap.size());
			curobj->attackDataMap->attackDataMap.insert({ newAttackData->event, RE::NiPointer(newAttackData) });
			//logger::debug("attackData size after {}", curobj->attackDataMap.get()->attackDataMap.size());

			for (auto objectArr : line.addedObjects) {
				for (auto object : objectArr) {
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
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackSpell = (RE::SpellItem*)form;
								logger::debug("race formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
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
						if (object[1] != "null") {
							auto form = GetFormFromIdentifier(object[1]);
							if (form) {
								newAttackData->data.attackType = (RE::BGSKeyword*)form;
								logger::debug("race formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
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
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
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
								// Handle the case where an invalid flag is provided
								// You might want to add error handling or logging here
							}
						}
					} else {
						// Handle the case where the keyword is not recognized
						// You may want to log an error or take appropriate action
						logger::debug("Unrecognized keyword: {}", object[0]);
					}
				}
			}
		}

		if (!line.removeAttackData.empty()) {
			for (auto attackData : line.removeAttackData) {
				//logger::debug("string {}", attackData);
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					//logger::debug("for {}", it->second.get()->event.c_str());
					if (toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
						logger::debug("race formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					} else {
						// Move to the next iterator
						++it;
					}
				}
			}
		}

		if (!line.attackDataToChange.empty()) {
			//logger::debug("attackDataMap size {}", curobj->attackDataMap->attackDataMap.size());
			for (auto& attackData : curobj->attackDataMap->attackDataMap) {
				
				if (!attackData.second)
					continue;

				auto newAttackData = attackData.second.get();
				if (!newAttackData)
					continue;
				//logger::debug("processing: {}", newAttackData->event.c_str());
				//logger::debug("attackDataToChange size {}", line.attackDataToChange.size());
				for (auto& objectArr : line.attackDataToChange) {
					//attackData loop from race


					bool key = true;
					//line from ini
					//logger::debug("processing attackMap {} objectArr {} ", newAttackData->event.c_str(), objectArr.size());
					for (auto& object : objectArr) {
						//logger::debug("object-> 0 {} 1 {}", object[0], object[1]);
						if (object.size() < 2) {
							continue;
						}

						if (key) {
							if (!(newAttackData->event.c_str() == object[1])) {
								//logger::debug("race formid: {:08X} attackData change - entry not found ", curobj->formID);
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
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackSpell = (RE::SpellItem*)form;
									logger::debug("race formid: {:08X} attackData changed - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
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
							if (object[1] != "null") {
								auto form = GetFormFromIdentifier(object[1]);
								if (form) {
									newAttackData->data.attackType = (RE::BGSKeyword*)form;
									logger::debug("race formid: {:08X} attackData changed - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
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
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
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
									// Handle the case where an invalid flag is provided
									// You might want to add error handling or logging here
								}
							}
						} else {
							// Handle the case where the keyword is not recognized
							// You may want to log an error or take appropriate action
							logger::debug("Unrecognized keyword: {}", object[0]);
						}
					}
				}
			}

			//

			for (auto attackData : line.removeAttackData) {
				//logger::debug("string {}", attackData);
				for (auto it = curobj->attackDataMap->attackDataMap.begin(); it != curobj->attackDataMap->attackDataMap.end();) {
					//logger::debug("for {}", it->second.get()->event.c_str());
					if (toLowerCase(it->second.get()->event.c_str()) == toLowerCase(attackData)) {
						logger::debug("race formid: {:08X} removing attackData with event {} ", curobj->formID, it->second.get()->event.c_str());
						it = curobj->attackDataMap->attackDataMap.erase(it);
					} else {
						// Move to the next iterator
						++it;
					}
				}
			}
		}
	}
	//

	void patch(const RACE::patch_instruction& line, RE::TESRace* curobj)
	{
		if (!line.avifs.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.avifs.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.avifs[i];

				//logger::info("avif valid!");
				float randomValue = 0;
				if (!line.values1.empty() && !line.values2.empty()) {
					//std::srand(std::time(nullptr));
					randomValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
				}

				if (toLowerCase(string_form) == "health") {
					curobj->data.startingHealth = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (toLowerCase(string_form) == "stamina") {
					curobj->data.startingStamina = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				} else if (toLowerCase(string_form) == "magicka") {
					curobj->data.startingMagicka = randomValue;
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed {} to {}"), curobj->formID, curobj->fullName, string_form.c_str(), randomValue);
				}
				logger::debug(FMT_STRING("race formid: {:08X} {} changed {:08X} {} {}"), curobj->formID, curobj->formEditorID, ((RE::ActorValueInfo*)curobj)->formID, ((RE::ActorValueInfo*)curobj)->fullName, randomValue);
			}
		}

		if (!line.keywordsToAdd.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
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
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (const auto& objectToAdd : line.keywordsToAddIfKeywordsEIDDoNotContain) {
				if (objectToAdd.size() < 2) {
					continue;
				}
				std::string        eidkeyword = (objectToAdd[0]);
				std::string  keywordString = (objectToAdd[1]);
				RE::TESForm* currentform = nullptr;
				bool               addKeyword = true;
				currentform = GetFormFromIdentifier(keywordString);
				//logger::info("keywordstring {}", keywordString);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					//logger::info("keyword found ");
					for (std::uint32_t i = 0; i < curobj->numKeywords; ++i) {
						auto keyword = curobj->keywords[i];
						//logger::info("loop keywords {} keyword {} search for {} ", i, keyword->formEditorID, eidkeyword);
						if (keyword && toLowerCase(keyword->formEditorID.c_str()).find(toLowerCase(eidkeyword)) != std::string::npos) {
							addKeyword = false;
							//logger::info("keywords has keyword with editor ID ");
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
			for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::LeveledSpell) {
					if (!curobj->actorEffects) {
						curobj->actorEffects->RemoveLevSpell((RE::TESLevSpell*)currentform);
					}
					logger::debug(FMT_STRING("race formid: {:08X} removed levSpell {:08X} "), curobj->formID, ((RE::TESLevSpell*)currentform)->formID);
				}
			}
		}

		if (!line.shoutsToAdd.empty()) {
			for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToAdd[i];
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
			for (size_t i = 0; i < line.spellsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.spellsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Shout) {
					if (!curobj->actorEffects) {
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
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCarryweight.empty() && line.baseCarryweight != "none") {
			try {
				curobj->data.baseCarryWeight = std::stof(line.baseCarryweight);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingHealth.empty() && line.startingHealth != "none") {
			try {
				curobj->data.startingHealth = std::stof(line.startingHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingStamina.empty() && line.startingStamina != "none") {
			try {
				curobj->data.startingStamina = std::stof(line.startingStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingMagicka.empty() && line.startingMagicka != "none") {
			try {
				curobj->data.startingMagicka = std::stof(line.startingMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenHealth.empty() && line.regenHealth != "none") {
			try {
				curobj->data.healthRegen = std::stof(line.regenHealth);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenStamina.empty() && line.regenStamina != "none") {
			try {
				curobj->data.staminaRegen = std::stof(line.regenStamina);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenMagicka.empty() && line.regenMagicka != "none") {
			try {
				curobj->data.magickaRegen = std::stof(line.regenMagicka);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageUnarmed.empty() && line.damageUnarmed != "none") {
			try {
				curobj->data.unarmedDamage = std::stof(line.damageUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reachUnarmed.empty() && line.reachUnarmed != "none") {
			try {
				curobj->data.unarmedReach = std::stof(line.reachUnarmed);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseMassMult.empty() && line.baseMassMult != "none") {
			try {
				curobj->data.baseMass = curobj->data.baseMass * std::stof(line.baseMassMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseMass {}"), curobj->formID, curobj->fullName, curobj->data.baseMass);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCarryweightMult.empty() && line.baseCarryweightMult != "none") {
			try {
				curobj->data.baseCarryWeight = curobj->data.baseCarryWeight * std::stof(line.baseCarryweightMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied baseCarryweight {}"), curobj->formID, curobj->fullName, curobj->data.baseCarryWeight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingHealthMult.empty() && line.startingHealthMult != "none") {
			try {
				curobj->data.startingHealth = curobj->data.startingHealth * std::stof(line.startingHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingHealth {}"), curobj->formID, curobj->fullName, curobj->data.startingHealth);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingStaminaMult.empty() && line.startingStaminaMult != "none") {
			try {
				curobj->data.startingStamina = curobj->data.startingStamina * std::stof(line.startingStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingStamina {}"), curobj->formID, curobj->fullName, curobj->data.startingStamina);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.startingMagickaMult.empty() && line.startingMagickaMult != "none") {
			try {
				curobj->data.startingMagicka = curobj->data.startingMagicka * std::stof(line.startingMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied startingMagicka {}"), curobj->formID, curobj->fullName, curobj->data.startingMagicka);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenHealthMult.empty() && line.regenHealthMult != "none") {
			try {
				curobj->data.healthRegen = curobj->data.healthRegen * std::stof(line.regenHealthMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenHealth {}"), curobj->formID, curobj->fullName, curobj->data.healthRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenStaminaMult.empty() && line.regenStaminaMult != "none") {
			try {
				curobj->data.staminaRegen = curobj->data.staminaRegen * std::stof(line.regenStaminaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenStamina {}"), curobj->formID, curobj->fullName, curobj->data.staminaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.regenMagickaMult.empty() && line.regenMagickaMult != "none") {
			try {
				curobj->data.magickaRegen = curobj->data.magickaRegen * std::stof(line.regenMagickaMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied regenMagicka {}"), curobj->formID, curobj->fullName, curobj->data.magickaRegen);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageUnarmedMult.empty() && line.damageUnarmedMult != "none") {
			try {
				curobj->data.unarmedDamage = curobj->data.unarmedDamage * std::stof(line.damageUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied damageUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reachUnarmedMult.empty() && line.reachUnarmedMult != "none") {
			try {
				curobj->data.unarmedReach = curobj->data.unarmedReach * std::stof(line.reachUnarmedMult);
				logger::debug(FMT_STRING("race formid: {:08X} {} multiplied reachUnarmed {}"), curobj->formID, curobj->fullName, curobj->data.unarmedReach);
			} catch (const std::invalid_argument& e) {
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
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.heightMale.empty() && line.heightMale != "none") {
			try {
				curobj->data.height[RE::SEXES::kMale] = std::stof(line.heightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightMale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kMale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.heightFemale.empty() && line.heightFemale != "none") {
			try {
				curobj->data.height[RE::SEXES::kFemale] = std::stof(line.heightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed heightFemale {}"), curobj->formID, curobj->fullName, curobj->data.height[RE::SEXES::kFemale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightMale.empty() && line.weightMale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kMale] = std::stof(line.weightMale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightMale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kMale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightFemale.empty() && line.weightFemale != "none") {
			try {
				curobj->data.weight[RE::SEXES::kFemale] = std::stof(line.weightFemale);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed weightFemale {}"), curobj->formID, curobj->fullName, curobj->data.weight[RE::SEXES::kFemale]);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rateAcceleration.empty() && line.rateAcceleration != "none") {
			try {
				curobj->data.accelerate = std::stof(line.rateAcceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateAcceleration {}"), curobj->formID, curobj->fullName, curobj->data.accelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rateDeceleration.empty() && line.rateDeceleration != "none") {
			try {
				curobj->data.decelerate = std::stof(line.rateDeceleration);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed rateDeceleration {}"), curobj->formID, curobj->fullName, curobj->data.decelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.angularAccelerationRate.empty() && line.angularAccelerationRate != "none") {
			try {
				curobj->data.angleAccelerate = std::stof(line.angularAccelerationRate);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed angularAccelerationRate {}"), curobj->formID, curobj->fullName, curobj->data.angleAccelerate);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.aimAngleTolerance.empty() && line.aimAngleTolerance != "none") {
			try {
				curobj->data.aimAngleTolerance = std::stof(line.aimAngleTolerance);
				logger::debug(FMT_STRING("race formid: {:08X} {} changed aimAngleTolerance {}"), curobj->formID, curobj->fullName, curobj->data.aimAngleTolerance);
			} catch (const std::invalid_argument& e) {
			}
		}

		//if (!line.addedObjects.empty()) {
		//	bool               key = true;
		//	RE::BGSAttackData* newAttackData = RE::BGSAttackData::Create();
		//	newAttackData->IncRefCount();
		//	//newAttackData->event = "attackStart";
		//	newAttackData->data.damageMult = 1;
		//	newAttackData->data.attackChance = 1;
		//	newAttackData->data.attackSpell = nullptr;
		//	newAttackData->data.attackAngle = 0;
		//	newAttackData->data.strikeAngle = 50;
		//	newAttackData->data.staggerOffset = 0;
		//	newAttackData->data.attackType = nullptr;
		//	newAttackData->data.knockDown = 0;
		//	newAttackData->data.recoveryTime = 0;
		//	newAttackData->data.staminaMult = 1;
		//	newAttackData->data.flags = RE::AttackData::AttackFlag::kNone;
		//	//logger::debug("attackData size before {}", curobj->attackDataMap.get()->attackDataMap.size());
		//	curobj->attackDataMap->attackDataMap.insert({ newAttackData->event, RE::NiPointer(newAttackData) });
		//	//logger::debug("attackData size after {}", curobj->attackDataMap.get()->attackDataMap.size());

		//	for (auto object : line.addedObjects) {
		//		if (key) {
		//			newAttackData->event = object[0];
		//			logger::debug("added event: {}", object[0]);
		//			key = false;
		//			continue;
		//		}

		//		if (toLowerCase(object[0]) == "damagemult") {
		//			newAttackData->data.damageMult = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - damagemult {}", curobj->formID, newAttackData->data.damageMult);
		//		} else if (toLowerCase(object[0]) == "attackchance") {
		//			newAttackData->data.attackChance = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - attackchance {}", curobj->formID, newAttackData->data.attackChance);
		//		} else if (toLowerCase(object[0]) == "attackspell") {
		//			if (object[1] != "null") {
		//				auto form = GetFormFromIdentifier(object[1]);
		//				if (form) {
		//					newAttackData->data.attackSpell = (RE::SpellItem*)form;
		//					logger::debug("race formid: {:08X} attackData - attackspell {:08X}", curobj->formID, newAttackData->data.attackSpell->formID);
		//				}
		//			} else {
		//				newAttackData->data.attackSpell = nullptr;
		//				logger::debug("race formid: {:08X} attackData - attackspell nullptr", curobj->formID);
		//			}
		//		} else if (toLowerCase(object[0]) == "attackangle") {
		//			newAttackData->data.attackAngle = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - attackangle {}", curobj->formID, newAttackData->data.attackAngle);
		//		} else if (toLowerCase(object[0]) == "strikeangle") {
		//			newAttackData->data.strikeAngle = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - strikeangle {}", curobj->formID, newAttackData->data.strikeAngle);
		//		} else if (toLowerCase(object[0]) == "staggeroffset") {
		//			newAttackData->data.staggerOffset = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - staggeroffset {}", curobj->formID, newAttackData->data.staggerOffset);
		//		} else if (toLowerCase(object[0]) == "attacktype") {
		//			if (object[1] != "null") {
		//				auto form = GetFormFromIdentifier(object[1]);
		//				if (form) {
		//					newAttackData->data.attackType = (RE::BGSKeyword*)form;
		//					logger::debug("race formid: {:08X} attackData - attacktype {:08X}", curobj->formID, newAttackData->data.attackType->formID);
		//				}
		//			} else {
		//				newAttackData->data.attackType = nullptr;
		//				logger::debug("race formid: {:08X} attackData - attacktype nullptr", curobj->formID);
		//			}
		//		} else if (toLowerCase(object[0]) == "knockdown") {
		//			newAttackData->data.knockDown = std::stoi(object[1]);
		//			logger::debug("race formid: {:08X} attackData - knockdown {}", curobj->formID, newAttackData->data.knockDown);
		//		} else if (toLowerCase(object[0]) == "recoverytime") {
		//			newAttackData->data.recoveryTime = std::stoi(object[1]);
		//			logger::debug("race formid: {:08X} attackData - recoverytime {}", curobj->formID, newAttackData->data.recoveryTime);
		//		} else if (toLowerCase(object[0]) == "staminamult") {
		//			newAttackData->data.staminaMult = std::stof(object[1]);
		//			logger::debug("race formid: {:08X} attackData - staminamult {}", curobj->formID, newAttackData->data.staminaMult);
		//		} else if (toLowerCase(object[0]) == "flags") {
		//			newAttackData->data.flags = static_cast<RE::AttackData::AttackFlag>(std::stoi(object[1]));
		//			logger::debug("race formid: {:08X} attackData - flags ", curobj->formID);
		//		} else {
		//			// Handle the case where the keyword is not recognized
		//			// You may want to log an error or take appropriate action
		//			logger::debug("Unrecognized keyword: {}", toLowerCase(object[0]));
		//		}
		//	}
		//}
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
									if (line[0] == skipChar) {
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
	}

	void* readConfigHook(const std::string& folder)
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
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string   line;
								std::ifstream infile;

								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction_hook(line));
								}
								infile.close();
								//process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

	REL::Relocation<decltype(&LoadRaceDataHook::Hook_InitItemImpl)> func;

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x13, &Hook_InitItemImpl);
		logger::debug("{} Done!", __FUNCTION__);
	}

	// ==================================================================
	// PHASE 2 -- the HOOK path is DELIBERATELY NOT CONVERTED.
	//
	// Every filter it carries (filterByRaces as a bare formID compare, the
	// keyword pair, the catch-all, filterByKeywordsExcluded and
	// filterByRacesExcluded) IS covered by the opcode table, so unlike the
	// scan path above this one could be converted in full. It is not, for two
	// reasons:
	//
	//  1. A MatchProgram amortises rule compilation over a FORM LOOP. There is
	//     no form loop here -- Hook_InitItemImpl is called once per TESRace
	//     and matches that ONE race against every hooktoken. Compiling per
	//     invocation would add an allocation per (race, rule) and buy nothing.
	//
	//  2. Compiling once and caching would be a real behaviour change.
	//     GetFormFromIdentifier runs here DURING form loading, and upstream
	//     re-resolves on every invocation; a cache would freeze the answers
	//     the first hooked race saw. Whether a later-loading form resolves is
	//     exactly what this path can observe.
	//
	// It is also the one filter path in this file that `match_rate` does NOT
	// cover: the apply harness drives process_patch_instructions, not the
	// vtable hook. An unmeasured rewrite of it is what this project avoids.
	// ==================================================================
	bool LoadRaceDataHook::Hook_InitItemImpl(RE::TESRace* curobj)
	{

		auto result = func(curobj);
		if (result) {

			//logger::info("processing npc {:08X} {}", curobj->formID, clib_util::editorID::get_editorID(curobj));
			for (const auto& line : hooktokens) {
				//logger::info("processing config line");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

				if (curobj->IsDeleted()) {
					return result;
				}

				// PROTOTYPE: one view per rule (curobj is already fixed for this
				// hook invocation, so there is no separate per-form loop to hoist
				// the view out of).
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
					//logger::debug("objects size {}", line.object.size());
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;

						std::string string_form = npcstring;
						//logger::info("looking up npc {:08X} {} {}", curobj->formID, clib_util::editorID::get_editorID(curobj), string_form);
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							if (curobj->formID == currentform->formID) {
								//logger::debug("found objects curobj {:08X} currentform {:08X} string {}", curobj->formID, currentform->formID, string_form);
								found = true;
								break;
							}
								
								//hookpatch(line, curobj);
							//continue;
						}
					}
				}

				//if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
				//	//logger::info("continue");
				//	continue;
				//}

				//if (!line.object.empty()) {
				//	//logger::info("npc not empty");
				//	for (const auto& npcstring : line.object) {
				//		RE::TESForm* currentform = nullptr;
				//		RE::TESRace* npc = nullptr;

				//		std::string string_form = npcstring;
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::FormType::Race) {
				//			npc = (RE::TESRace*)currentform;

				//			if (curobj->formID == npc->formID) {
				//				found = true;
				//				//logger::info("NPC found.");
				//				break;
				//			}
				//		}
				//	}
				//}
				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					keywordAnd = formfilter::matchesKeywordsAnd(view, curobj, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					keywordOr = formfilter::matchesKeywordsOr(view, curobj, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching race by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
					//logger::debug("found keywords");
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
					//logger::debug("found line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()");
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					// Already used the correct RE::FormType::Race here (unlike the
					// main path's copy of this filter, fixed above).
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Race)) {
						found = false;
					}
				}

				if (found) {
					//logger::debug("hookpatch");
					hookpatch(line, curobj);
				}
			}

			return result;
		}
	}
}
