#include "magiceffect.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace MGEF
{
	// ==================================================================
	// PHASE 2 -- MagicEffect's key table.
	//
	// This replaces 44 extract*() calls, each of which ran its own
	// std::regex_search over the whole line. See parsetable.h for the
	// equivalence argument; the table below is a transliteration of the old
	// call list, one row per call, IN THE SAME ORDER, so the two can be
	// diffed by eye. (A commented-out "speed" extractValueString call
	// predates this change and was already dead code; it is not carried
	// forward.)
	// ==================================================================
	namespace
	{
#define ME_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kMagicEffectKeys[] = {
			{ "filterByMgefs", sp::FieldKind::List, ME_F(object) },
			{ "filterBySounds", sp::FieldKind::List, ME_F(filterBySounds) },
			{ "filterByDamageHealthMgefs", sp::FieldKind::Value, ME_F(filterByDamageHealthMgefs) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, ME_F(objectExcluded) },
			{ "effectShadersExcluded", sp::FieldKind::List, ME_F(effectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, ME_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, ME_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, ME_F(keywordsExcluded) },
			{ "fullName", sp::FieldKind::ValueTilde, ME_F(fullName) },
			{ "magicItemDescription", sp::FieldKind::ValueTilde, ME_F(magicItemDescription) },
			{ "enchantArt", sp::FieldKind::Value, ME_F(enchantArt) },
			{ "enchantShader", sp::FieldKind::Value, ME_F(enchantShader) },
			{ "setFlags", sp::FieldKind::ListAssign, ME_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, ME_F(removeFlags) },
			{ "keywordsToAdd", sp::FieldKind::List, ME_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, ME_F(keywordsToRemove) },
			{ "filterByModNames", sp::FieldKind::ListAssign, ME_F(modNames) },
			{ "modNamesLastOverriddenExcluded", sp::FieldKind::ListAssign, ME_F(filterByModNamesLastOverriddenExcluded) },
			{ "modNamesExcluded", sp::FieldKind::ListAssign, ME_F(ModNamesExcluded) },
			{ "hitShaderExcluded", sp::FieldKind::ListAssign, ME_F(hitShaderExcluded) },
			{ "impactDataSet", sp::FieldKind::Value, ME_F(impactDataSet) },
			{ "baseCost", sp::FieldKind::Value, ME_F(baseCost) },
			{ "minimumSkillLevel", sp::FieldKind::Value, ME_F(minimumSkillLevel) },
			{ "spellmakingCastingTime", sp::FieldKind::Value, ME_F(spellmakingCastingTime) },
			{ "spellmakingArea", sp::FieldKind::Value, ME_F(spellmakingArea) },
			{ "projectile", sp::FieldKind::Value, ME_F(projectile) },
			{ "castingType", sp::FieldKind::Value, ME_F(castType) },
			{ "castingArt", sp::FieldKind::Value, ME_F(castingArt) },
			{ "perkToApply", sp::FieldKind::Value, ME_F(perk) },
			{ "restrictToCastingType", sp::FieldKind::List, ME_F(restrictToCastingType) },
			{ "restrictToDeliveryType", sp::FieldKind::List, ME_F(restrictToDeliveryType) },
			{ "restrictToMagicSkill", sp::FieldKind::List, ME_F(restrictToMagicSkill) },
			{ "removeEdgeGlow", sp::FieldKind::Value, ME_F(removeEdgeGlow) },
			{ "skillUsageMult", sp::FieldKind::Value, ME_F(skillUsageMult) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, ME_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, ME_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, ME_F(filterByEditorIdContainsExcluded) },
			{ "filterByArchetypes", sp::FieldKind::ListAssign, ME_F(filterArchetype) },
			{ "restrictToActorValue", sp::FieldKind::ListAssign, ME_F(restrictToActorValue) },
			{ "restrictToAreaEffects", sp::FieldKind::Value, ME_F(restrictToArea) },
			{ "restrictToDetrimentalEffects", sp::FieldKind::Value, ME_F(restrictToDetrimental) },
			{ "restrictToResistType", sp::FieldKind::ListAssign, ME_F(restrictToResistType) },
			{ "hitShader", sp::FieldKind::Value, ME_F(hitShader) },
			{ "soundsToChange", sp::FieldKind::Arr2D, ME_F(soundsToChange) },
		};

#undef ME_F

		const sp::KeyTable& magicEffectKeyTable()
		{
			static const sp::KeyTable table(kMagicEffectKeys, std::size(kMagicEffectKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		magicEffectKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	// ==================================================================
	// PHASE 2 -- MGEF's rule compiler.
	//
	// PARTIAL CONVERSION, deliberately. Only the filters the shared opcode
	// table already covers are compiled into a formfilter::MatchProgram: the
	// keyword pair, the EditorID pair and the catch-all. MGEF's other filter
	// blocks -- filterByArchetypes, filterByDamageHealthMgefs, filterBySounds,
	// effectShadersExcluded and the seven restrictTo* gates -- have no opcode,
	// and per formfilter.h's rule they stay on their own path rather than
	// growing an engine three already-verified types depend on.
	//
	// They are all still hoisted out of the (form, rule) loop. This is the
	// type where that matters most: filterByArchetypes was a 47-rung
	// `toLowerCase(name) == "..."` ladder run per (pair, entry), and
	// restrictToMagicSkill/restrictToActorValue called StringToActorValue per
	// (pair, entry). Every one of those is now one lookup per rule.
	//
	// NOT converted, and staying verbatim: modNamesExcluded and
	// filterByModNamesLastOverriddenExcluded. They compare raw config text to
	// GetFile(0)->fileName / GetFile()->fileName, which is the same
	// case-sensitive per-pair string compare formfilter.h deliberately keeps
	// for filterByModNames (see ResolvedCommonFilters::modNames).
	// ==================================================================
	namespace
	{
		formfilter::MatchProgram compileMgefRule(const patch_instruction& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;

			// DRIFT, PRESERVED: the `else if (formID != 0x000007)` guard is a
			// copy-pasted actor check, inert here because EffectSetting forms
			// never carry the player's formID. It is only reached when the
			// corresponding list is EMPTY, so it can only matter when exactly
			// one of the two is present -- which is exactly skipIfPlayer.
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
				line.filterBySounds.empty() && line.filterArchetype.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() &&
				line.filterByDamageHealthMgefs == "none";

			return p;
		}

		// Every one of MGEF's `if (name == "x" && value == kX) hit = true;`
		// ladders reduces to "the form's value is one of the named ones": an
		// unrecognised name can never make the conjunction true, so it
		// contributes nothing, and no rung writes false.
		const RE::EffectArchetypes::ArchetypeID* lookupArchetype(const std::string& a_lower)
		{
			using A = RE::EffectArchetypes::ArchetypeID;
			static const std::unordered_map<std::string, A> map{
				{ "absorb", A::kAbsorb }, { "accumulatemagnitude", A::kAccumulateMagnitude },
				{ "valuemodifier", A::kValueModifier }, { "script", A::kScript },
				{ "dispel", A::kDispel }, { "curedisease", A::kCureDisease },
				{ "dualvaluemodifier", A::kDualValueModifier }, { "calm", A::kCalm },
				{ "demoralize", A::kDemoralize }, { "frenzy", A::kFrenzy },
				{ "disarm", A::kDisarm }, { "commandsummoned", A::kCommandSummoned },
				{ "invisibility", A::kInvisibility }, { "light", A::kLight },
				{ "darkness", A::kDarkness }, { "nighteye", A::kNightEye },
				{ "lock", A::kLock }, { "open", A::kOpen },
				{ "boundweapon", A::kBoundWeapon }, { "summoncreature", A::kSummonCreature },
				{ "detectlife", A::kDetectLife }, { "telekinesis", A::kTelekinesis },
				{ "paralysis", A::kParalysis }, { "reanimate", A::kReanimate },
				{ "soultrap", A::kSoulTrap }, { "turnundead", A::kTurnUndead },
				{ "guide", A::kGuide }, { "werewolffeed", A::kWerewolfFeed },
				{ "cureparalysis", A::kCureParalysis }, { "cureaddiction", A::kCureAddiction },
				{ "curepoison", A::kCurePoison }, { "concussion", A::kConcussion },
				{ "valueandparts", A::kValueAndParts }, { "stagger", A::kStagger },
				{ "peakvaluemodifier", A::kPeakValueModifier }, { "cloak", A::kCloak },
				{ "werewolf", A::kWerewolf }, { "slowtime", A::kSlowTime },
				{ "rally", A::kRally }, { "enhanceweapon", A::kEnhanceWeapon },
				{ "spawnhazard", A::kSpawnHazard }, { "etherealize", A::kEtherealize },
				{ "banish", A::kBanish }, { "spawnscriptedref", A::kSpawnScriptedRef },
				{ "disguise", A::kDisguise }, { "grabactor", A::kGrabActor },
				{ "vampirelord", A::kVampireLord },
			};
			const auto it = map.find(a_lower);
			return it == map.end() ? nullptr : &it->second;
		}

		struct MgefRuleExtras
		{
			std::vector<RE::EffectArchetypes::ArchetypeID> archetypes;
			std::vector<RE::BGSSoundDescriptorForm*>       sounds;
			std::vector<RE::FormID>                        effectShadersExcluded;
			std::vector<RE::MagicSystem::CastingType>      castingTypes;
			std::vector<RE::MagicSystem::Delivery>         deliveryTypes;
			std::vector<RE::ActorValue>                    resistTypes;
			// StringToActorValue's answer for an unrecognised name is part of
			// the comparison upstream, so every entry survives (same call
			// weapon.cpp made for restrictToSkills).
			std::vector<RE::ActorValue> magicSkills;
			std::vector<RE::ActorValue> actorValues;
			std::string                 area;          // toLowerCase'd once
			std::string                 detrimental;   // toLowerCase'd once
			bool                        damageHealthActive = false;
			bool                        damageHealthHostile = false;
			bool                        playerGuard = false;
		};

		MgefRuleExtras compileMgefExtras(const patch_instruction& line)
		{
			MgefRuleExtras x;

			for (const auto& raw : line.filterArchetype) {
				if (const auto* a = lookupArchetype(toLowerCase(raw))) {
					x.archetypes.push_back(*a);
				}
			}

			// Upstream warned once per (pair, invalid identifier); resolving
			// once per rule makes that one warning per rule.
			for (const auto& soundFormStr : line.filterBySounds) {
				RE::TESForm* form = GetFormFromIdentifier(soundFormStr);
				if (!form || form->formType != RE::FormType::SoundRecord) {
					logger::warn(FMT_STRING("mgef - Invalid sound filter: {}"), soundFormStr);
					continue;
				}
				x.sounds.push_back(static_cast<RE::BGSSoundDescriptorForm*>(form));
			}

			// NOTE: no formType check upstream -- any identifier that resolves
			// is C-cast to TESEffectShader and only its formID is read.
			for (const auto& npcstring : line.effectExcluded) {
				std::string  string_form = npcstring;
				RE::TESForm* currentform = GetFormFromIdentifier(string_form);
				if (currentform) {
					x.effectShadersExcluded.push_back(currentform->formID);
				}
			}

			for (const auto& raw : line.restrictToCastingType) {
				const std::string s = toLowerCase(raw);
				if (s == "concentration") {
					x.castingTypes.push_back(RE::MagicSystem::CastingType::kConcentration);
				} else if (s == "constanteffect") {
					x.castingTypes.push_back(RE::MagicSystem::CastingType::kConstantEffect);
				} else if (s == "fireandforget") {
					x.castingTypes.push_back(RE::MagicSystem::CastingType::kFireAndForget);
				} else if (s == "scroll") {
					x.castingTypes.push_back(RE::MagicSystem::CastingType::kScroll);
				}
			}

			for (const auto& raw : line.restrictToDeliveryType) {
				const std::string s = toLowerCase(raw);
				if (s == "self") {
					x.deliveryTypes.push_back(RE::MagicSystem::Delivery::kSelf);
				} else if (s == "touch") {
					x.deliveryTypes.push_back(RE::MagicSystem::Delivery::kTouch);
				} else if (s == "aimed") {
					x.deliveryTypes.push_back(RE::MagicSystem::Delivery::kAimed);
				} else if (s == "targetactor") {
					x.deliveryTypes.push_back(RE::MagicSystem::Delivery::kTargetActor);
				} else if (s == "targetlocation") {
					x.deliveryTypes.push_back(RE::MagicSystem::Delivery::kTargetLocation);
				}
			}

			for (const auto& raw : line.restrictToResistType) {
				const std::string s = toLowerCase(raw);
				if (s == "resistdisease") {
					x.resistTypes.push_back(RE::ActorValue::kResistDisease);
				} else if (s == "resistfire") {
					x.resistTypes.push_back(RE::ActorValue::kResistFire);
				} else if (s == "resistfrost") {
					x.resistTypes.push_back(RE::ActorValue::kResistFrost);
				} else if (s == "resistmagic") {
					x.resistTypes.push_back(RE::ActorValue::kResistMagic);
				} else if (s == "resistshock") {
					x.resistTypes.push_back(RE::ActorValue::kResistShock);
				} else if (s == "resistpoison") {
					x.resistTypes.push_back(RE::ActorValue::kPoisonResist);
				}
			}

			for (const auto& raw : line.restrictToMagicSkill) {
				x.magicSkills.push_back(StringToActorValue(toLowerCase(raw)));
			}
			for (const auto& raw : line.restrictToActorValue) {
				x.actorValues.push_back(StringToActorValue(toLowerCase(raw)));
			}

			x.area = toLowerCase(line.restrictToArea);
			x.detrimental = toLowerCase(line.restrictToDetrimental);

			x.damageHealthActive = line.filterByDamageHealthMgefs != "none";
			x.damageHealthHostile = toLowerCase(line.filterByDamageHealthMgefs) == "hostile";

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
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::EffectSetting>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::EffectSetting* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::MagicEffect) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::EffectSetting*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.filterByEditorIdContains.empty() && line.filterBySounds.empty() && line.filterByEditorIdContainsOr.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterArchetype.empty() && line.filterByDamageHealthMgefs == "none" ) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, magiceffect) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			// PHASE 1 + 2: the rule is resolved and compiled ONCE.
			const formfilter::MatchProgram          program = compileMgefRule(line);
			const MgefRuleExtras                    extra = compileMgefExtras(line);
			const formfilter::ResolvedCommonFilters resolved =
				formfilter::resolve(view, RE::FormType::MagicEffect);

			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(program, curobj)) {
					continue;
				}

				if (!line.ModNamesExcluded.empty())
				{
					bool modFound = false;
					for (auto const modName : line.ModNamesExcluded) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
							break;
						}
					}
					if (modFound) {
						continue;
					}
				}

				if (!line.filterByModNamesLastOverriddenExcluded.empty()) {
					bool modFound = false;
					for (auto const modName : line.filterByModNamesLastOverriddenExcluded) {
						if (modName == curobj->GetFile()->fileName) {
							modFound = true;
							break;
						}
					}
					if (modFound) {
						continue;
					}
				}

				target.form = curobj;
				target.kf = curobj;
				target.isPlayer = extra.playerGuard && curobj->formID == 0x000007;

				// EditorID pair + keyword pair + the catch-all.
				bool found = formfilter::runProgram(program, target, scratch);

				// --- filters the opcode table does not cover ----------------
				// Every one of these only ever writes `found = true`, so
				// running them after the program -- and only when it did not
				// already match -- computes the same OR upstream did.
				if (!found && !extra.archetypes.empty()) {
					for (const auto a : extra.archetypes) {
						if (curobj->data.archetype == a) {
							found = true;
							break;
						}
					}
				}

				if (!found && extra.damageHealthActive) {
					if (curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kValueModifier ||
						curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDualValueModifier) {
						if ((curobj->data.primaryAV == RE::ActorValue::kHealth ||
								curobj->data.secondaryAV == RE::ActorValue::kHealth) &&
							curobj->IsDetrimental() &&
							(!extra.damageHealthHostile || curobj->IsHostile())) {
							found = true;
						}
					}
				}

				if (!found && !extra.sounds.empty()) {
					for (RE::BGSSoundDescriptorForm* const targetSound : extra.sounds) {
						// Check if the sound is present in curobj's effectSounds
						const bool match = std::any_of(
							curobj->effectSounds.begin(),
							curobj->effectSounds.end(),
							[targetSound](const RE::EffectSetting::SoundPair& pair) {
								return pair.sound == targetSound;
							});

						if (match) {
							found = true;
							break;
						}
					}
				}

				// --- negative stage, in upstream source order ---------------
				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(resolved, curobj)) {
						found = false;
						//logger::info("Mgef Excluded.");
					}
				}

				if (!line.effectExcluded.empty() && found) {
					for (const RE::FormID id : extra.effectShadersExcluded) {
						if (curobj->data.effectShader && curobj->data.effectShader->formID == id) {
							found = false;
							//logger::info("effs Excluded.");
							break;
						}
					}
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(resolved, curobj, nullptr)) {
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

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					for (const auto t : extra.castingTypes) {
						if (curobj->data.castingType == t) {
							foundCastingType = true;
							break;
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToDeliveryType.empty()) {
					bool foundDeliveryType = false;
					for (const auto d : extra.deliveryTypes) {
						if (curobj->data.delivery == d) {
							foundDeliveryType = true;
							break;
						}
					}
					if (!foundDeliveryType)
						found = false;
				}

				if (found && !line.restrictToResistType.empty()) {
					bool foundType = false;
					for (const auto r : extra.resistTypes) {
						if (curobj->data.resistVariable == r) {
							foundType = true;
							break;
						}
					}
					if (!foundType)
						found = false;
				}

				if (found && !line.restrictToMagicSkill.empty()) {
					bool foundType = false;
					for (const auto av : extra.magicSkills) {
						if (curobj->data.associatedSkill == av) {
							foundType = true;
							break;
						}
					}
					if (!foundType)
						found = false;
				}

				if (found && !line.restrictToArea.empty()) {
					if (extra.area == "true" && curobj->data.spellmakingArea <= 0) {
						found = false;
					}
					if (extra.area == "false" && curobj->data.spellmakingArea > 0) {
						found = false;
					}
				}

				if (found && !line.restrictToDetrimental.empty()) {
					if (extra.detrimental == "true" && !curobj->IsDetrimental()) {
						found = false;
					}
					if (extra.detrimental == "false" && curobj->IsDetrimental()) {
						found = false;
					}
				}

				if (found && !line.restrictToActorValue.empty()) {
					bool foundType = false;
					for (const auto av : extra.actorValues) {
						if (curobj->data.primaryAV == av || curobj->data.secondaryAV == av) {
							foundType = true;
							break;
						}
					}
					if (!foundType) {
						found = false;
					}
				}



				if (found) {
					patch(line, curobj);
				}
			}
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

									tokens.push_back(create_patch_instructions(line));
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

	void* patch(const MGEF::patch_instruction& line, RE::EffectSetting* curobj)
	{
		
		if (!line.enchantArt.empty() && line.enchantArt != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.enchantArt;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ArtObject) {
					curobj->data.enchantEffectArt = (RE::BGSArtObject*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed enchantEffectArt {:08X}"), curobj->formID, curobj->fullName, curobj->data.enchantEffectArt->formID);
				} else if (line.enchantArt == "null" || line.enchantArt == "clear") {
					curobj->data.enchantEffectArt = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed enchantEffectArt null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.enchantShader.empty() && line.enchantShader != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.enchantShader;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EffectShader ) {
					curobj->data.enchantShader = (RE::TESEffectShader*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed enchantShader {:08X}"), curobj->formID, curobj->fullName, curobj->data.enchantShader->formID);
				} else if (line.enchantShader == "null" || line.enchantShader == "clear") {
					curobj->data.enchantShader = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed enchantShader null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.hitShader.empty() && line.hitShader != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.hitShader;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EffectShader) {
					curobj->data.effectShader = (RE::TESEffectShader*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed hitShader {:08X}"), curobj->formID, curobj->fullName, curobj->data.effectShader->formID);
				} else if (line.hitShader == "null" || line.hitShader == "clear") {
					curobj->data.effectShader = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed hitShader null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& flag : line.setFlags) {
				if (toLowerCase(flag) == "hostile") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kHostile);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Hostile"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "recover") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kRecover);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Recover"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "detrimental") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Detrimental"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "snaptomesh") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kSnapToNavMesh);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled SnapToNavMesh"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nohitevent") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoHitEvent);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoHitEvent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "dispelwithkeywords") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled DispelWithKeywords"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "noduration") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nomagnitude") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "noarea") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoArea);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoArea"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "fxpersist") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kFXPersist);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled FXPersist"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "goryvisuals") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kGoryVisuals);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled GoryVisuals"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "hideinui") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kHideInUI);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled HideInUI"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "norecast") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoRecast);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoRecast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "poweraffectsmagnitude") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled PowerAffectsMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "poweraffectsduration") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled PowerAffectsDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "painless") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPainless);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Painless"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nohiteffect") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoHitEffect);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoHitEffect"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nodeathdispel") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoDeathDispel);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoDeathDispel"), curobj->formID, curobj->fullName);
				}
				else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		// Remove Flags
		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "hostile") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kHostile);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Hostile"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "recover") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kRecover);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Recover"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "detrimental") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Detrimental"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "snaptomesh") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kSnapToNavMesh);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled SnapToNavMesh"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nohitevent") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoHitEvent);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoHitEvent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "dispelwithkeywords") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled DispelWithKeywords"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "noduration") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nomagnitude") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "noarea") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoArea);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoArea"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fxpersist") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kFXPersist);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled FXPersist"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "goryvisuals") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kGoryVisuals);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled GoryVisuals"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hideinui") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kHideInUI);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled HideInUI"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "norecast") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoRecast);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoRecast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "poweraffectsmagnitude") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled PowerAffectsMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "poweraffectsduration") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled PowerAffectsDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "painless") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPainless);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Painless"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nohiteffect") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoHitEffect);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoHitEffect"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nodeathdispel") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoDeathDispel);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoDeathDispel"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
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
						logger::debug(FMT_STRING("mgef formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.magicItemDescription.empty() && line.magicItemDescription != "none") {
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed magic item description to {}"), curobj->formID, curobj->fullName, line.magicItemDescription);
				curobj->magicItemDescription = line.magicItemDescription;
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("mgef formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.impactDataSet.empty() && line.impactDataSet != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.impactDataSet;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ImpactDataSet) {
					curobj->data.impactDataSet = (RE::BGSImpactDataSet*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed impactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->data.impactDataSet->formID);
				}
				} else if (toLowerCase(string_form) == "null") {
				curobj->data.impactDataSet = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed impactDataSet to null"), curobj->formID, curobj->fullName);

				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCost.empty() && line.baseCost != "none") {
			curobj->data.baseCost = std::stof(line.baseCost);
			logger::debug(FMT_STRING("mgef {:08X} {} changed base cost to {}"), curobj->formID, curobj->fullName, curobj->data.baseCost);
		}

		if (!line.minimumSkillLevel.empty() && line.minimumSkillLevel != "none") {
			curobj->data.minimumSkill = std::stoi(line.minimumSkillLevel);
			logger::debug(FMT_STRING("mgef {:08X} {} changed minimum skill level to {}"), curobj->formID, curobj->fullName, curobj->data.minimumSkill);
		}

		if (!line.spellmakingCastingTime.empty() && line.spellmakingCastingTime != "none") {
			curobj->data.spellmakingChargeTime = std::stof(line.spellmakingCastingTime);
			logger::debug(FMT_STRING("mgef {:08X} {} changed spellmaking casting time to {}"), curobj->formID, curobj->fullName, curobj->data.spellmakingChargeTime);
		}

		if (!line.spellmakingArea.empty() && line.spellmakingArea != "none") {
			curobj->data.spellmakingArea = std::stoi(line.spellmakingArea);
			logger::debug(FMT_STRING("mgef {:08X} {} changed spellmaking area to {}"), curobj->formID, curobj->fullName, curobj->data.spellmakingArea);
		}

		if (!line.skillUsageMult.empty() && line.skillUsageMult != "none") {
			curobj->data.skillUsageMult = std::stof(line.skillUsageMult);
			logger::debug(FMT_STRING("mgef {:08X} {} changed skillUsageMult to {}"), curobj->formID, curobj->fullName, curobj->data.skillUsageMult);
		}

		if (!line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.projectile.empty() && line.projectile != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.projectile;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Projectile) {
					curobj->data.projectileBase = (RE::BGSProjectile*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed projectileBase {:08X}"), curobj->formID, curobj->fullName, curobj->data.projectileBase->formID);
				}
				} else if (toLowerCase(string_form) == "null") {
					curobj->data.projectileBase = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed projectileBase to null"), curobj->formID, curobj->fullName);

				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.castingArt.empty() && line.castingArt != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.castingArt;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ArtObject) {
					curobj->data.castingArt = (RE::BGSArtObject*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingArt {:08X}"), curobj->formID, curobj->fullName, curobj->data.castingArt->formID);
				}
				} else if (toLowerCase(string_form) == "null" || toLowerCase(string_form) == "clear") {
				curobj->data.castingArt = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingArt to null"), curobj->formID, curobj->fullName);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.perk.empty() && line.perk != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.perk;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Perk) {
						curobj->data.perk = (RE::BGSPerk*)currentform;
						logger::debug(FMT_STRING("mgef formid: {:08X} {} changed perk {:08X}"), curobj->formID, curobj->fullName, curobj->data.perk->formID);
					}
				} else if (toLowerCase(string_form) == "null" || toLowerCase(string_form) == "clear") {
					curobj->data.perk = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed perk to null"), curobj->formID, curobj->fullName);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.removeEdgeGlow.empty() && line.removeEdgeGlow != "none") {
			RE::TESEffectShader* effectShader = curobj->data.effectShader;
			
			if (effectShader) {
				effectShader->data.fillTextureEffectPersistentAlphaRatio = 0.f;
				effectShader->data.fillTextureEffectFullAlphaRatio = 0.f;
				effectShader->data.edgeEffectPersistentAlphaRatio = 0.f;
				effectShader->data.edgeEffectFullAlphaRatio = 0.f;
				logger::debug(FMT_STRING("mgef {:08X} {} removed edge glow"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.soundsToChange.empty()) {
			for (const auto& soundEntry : line.soundsToChange) {
				if (soundEntry.size() < 2) {
					logger::warn("Invalid soundEntry: expected at least 2 elements (form + name), got {}", soundEntry.size());
					continue;
				}

				std::string spellFormStr = soundEntry[0];
				std::string soundName = toLowerCase(soundEntry[1]);

				// Convert sound name to enum
				auto soundIDOpt = [&]() -> std::optional<RE::MagicSystem::SoundID> {
					static const std::unordered_map<std::string, RE::MagicSystem::SoundID> soundNameToID = {
						{ "drawsheathe", RE::MagicSystem::SoundID::kDrawSheatheLPM },
						{ "charge",      RE::MagicSystem::SoundID::kCharge },
						{ "readyloop",   RE::MagicSystem::SoundID::kReadyLoop },
						{ "release",     RE::MagicSystem::SoundID::kRelease },
						{ "castloop",    RE::MagicSystem::SoundID::kCastLoop },
						{ "hit",         RE::MagicSystem::SoundID::kHit }
					};
					if (auto it = soundNameToID.find(soundName); it != soundNameToID.end()) {
						return it->second;
					}
					return std::nullopt;
					}();

				if (!soundIDOpt) {
					logger::debug(FMT_STRING("mgef {:08X} invalid sound name: {}"), curobj->formID, soundName);
					continue;
				}

				RE::MagicSystem::SoundID soundID = *soundIDOpt;

				if (spellFormStr == "null") {
					// Remove sound if it exists
					auto& sounds = curobj->effectSounds;
					auto it = std::remove_if(sounds.begin(), sounds.end(),
						[soundID](const RE::EffectSetting::SoundPair& pair) {
							return pair.id == soundID;
						});

					if (it != sounds.end()) {
						logger::debug(FMT_STRING("mgef {:08X} removed sound {}"), curobj->formID, soundName);
						sounds.erase(it);
					}
					else {
						logger::debug(FMT_STRING("mgef {:08X} sound {} not found for removal"), curobj->formID, soundName);
					}
					continue;
				}

				RE::BGSSoundDescriptorForm* spellSound = (RE::BGSSoundDescriptorForm*)GetFormFromIdentifier(spellFormStr);
				if (!spellSound) {
					logger::critical(FMT_STRING("mgef {:08X} spell sound not found: {}"), curobj->formID, spellFormStr);
					continue;
				}

				auto it = std::find_if(
					curobj->effectSounds.begin(),
					curobj->effectSounds.end(),
					[soundID](const RE::EffectSetting::SoundPair& pair) {
						return pair.id == soundID;
					}
				);

				if (it != curobj->effectSounds.end()) {
					// Replace the existing sound
					it->sound = spellSound;
					logger::debug(FMT_STRING("mgef {:08X} sound {} replaced on {}"), curobj->formID, soundName, spellFormStr);
				}
				else {
					// Add new sound
					logger::debug(FMT_STRING("mgef {:08X} sound add {} to {}"), curobj->formID, soundName, spellFormStr);
					curobj->effectSounds.push_back(RE::EffectSetting::SoundPair{ soundID, 0, spellSound });
				}
			}
		}





		


		//if (!line.sound.empty() && line.sound != "none") {
		//	try {
		//		RE::TESForm* currentform = nullptr;
		//		std::string  string_form = line.sound;
		//		currentform = GetFormFromIdentifier(string_form);
		//		if (currentform && currentform->formType == RE::FormType::SoundRecord) {
		//			curobj->data.so = (RE::BGSSoundDescriptorForm*)currentform;
		//			logger::debug(FMT_STRING("projectile formid: {:08X} {} changed sound {:08X}"), curobj->formID, curobj->fullName, curobj->data.activeSoundLoop->formID);
		//		}
		//	} catch (const std::invalid_argument& e) {
		//	}
		//}


	}

}
