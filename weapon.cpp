#include "weapon.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace WEAPON
{
	// =====================================================================
	// PHASE 1 -- WEAPON's own filter vocabulary, resolved once per rule.
	//
	// Everything here used to be re-derived from the rule's raw text on every
	// (weapon, rule) pair. filterAnimationTypeOr was the worst offender: its
	// upstream body calls toLowerCase(setFlag) once per if/else-if arm, i.e.
	// up to TEN std::string allocations per list entry per pair, all to
	// recompute a value fixed at load.
	//
	// Deliberately NOT done at parse time: create_patch_instructions must stay
	// free of CommonLib singletons (golden_dump runs without a game), and
	// patch_instruction's layout must not change or the golden snapshots move.
	// =====================================================================
	namespace
	{
		RE::WEAPON_TYPE* lookupAnimationType(const std::string& a_lower)
		{
			static RE::WEAPON_TYPE types[] = {
				RE::WEAPON_TYPE::kHandToHandMelee, RE::WEAPON_TYPE::kOneHandSword,
				RE::WEAPON_TYPE::kOneHandDagger, RE::WEAPON_TYPE::kOneHandAxe,
				RE::WEAPON_TYPE::kOneHandMace, RE::WEAPON_TYPE::kTwoHandSword,
				RE::WEAPON_TYPE::kTwoHandAxe, RE::WEAPON_TYPE::kBow,
				RE::WEAPON_TYPE::kStaff, RE::WEAPON_TYPE::kCrossbow
			};
			static const char* names[] = {
				"handtohandmelee", "onehandsword", "onehanddagger", "onehandaxe",
				"onehandmace", "twohandsword", "twohandaxe", "bow", "staff", "crossbow"
			};
			for (int i = 0; i < 10; ++i) {
				if (a_lower == names[i]) {
					return &types[i];
				}
			}
			return nullptr;
		}

		RE::ActorValue* lookupEmbeddedAV(const std::string& a_lower)
		{
			static RE::ActorValue avs[] = {
				RE::ActorValue::kOneHanded, RE::ActorValue::kTwoHanded,
				RE::ActorValue::kArchery, RE::ActorValue::kDestruction,
				RE::ActorValue::kIllusion, RE::ActorValue::kConjuration,
				RE::ActorValue::kAlteration, RE::ActorValue::kRestoration
			};
			static const char* names[] = {
				"onehanded", "twohanded", "marksman", "destruction",
				"illusion", "conjuration", "alteration", "restoration"
			};
			for (int i = 0; i < 8; ++i) {
				if (a_lower == names[i]) {
					return &avs[i];
				}
			}
			return nullptr;
		}

		// ==================================================================
		// PHASE 2 -- WEAPON's rule compiler.
		//
		// The filter chain that used to be ~150 lines inlined in
		// process_patch_instructions is compiled once per rule into a
		// formfilter::MatchProgram. See formfilter.h.
		//
		// WEAPON's positive disjuncts -- filterByAnimationTypeOr,
		// filterByFirstPersonModelOr, filterBySkills, the EditorID pair and
		// the keyword pair -- all only ever WRITE true, so ordering them
		// cheapest-first cannot change the answer; it just gives the three
		// integer compares the chance to reject before the EditorID substring
		// search runs.
		// ==================================================================
		formfilter::MatchProgram compileRule(const patch_instruction& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;

			// hasPlugins / hasPluginsOr consult no form at all, so they are a
			// per-RULE skip, not a per-form one.
			p.skipRule = (!line.hasPlugin.empty() && !formfilter::hasAllPlugins(line.hasPlugin)) ||
				(!line.hasPluginOr.empty() && !formfilter::hasAnyPlugin(line.hasPluginOr));

			// filterByAnimationTypeOr. The ten toLowerCase(setFlag) calls this
			// used to make per (pair, entry) are one lookup per entry, once.
			std::vector<std::uint64_t> vals;
			for (const auto& s : line.filterAnimationTypeOr) {
				if (auto* t = lookupAnimationType(toLowerCase(s))) {
					vals.push_back(static_cast<std::uint32_t>(*t));
				}
			}
			if (!line.filterAnimationTypeOr.empty()) {
				std::size_t first = p.preds.size();
				formfilter::addPred(p, Op::WeapAnimType, Mode::Or, vals);
				formfilter::pushDisjunct(p, p.positive, first);
			}

			if (!line.filterByFirstPersonModelOr.empty()) {
				std::size_t first = p.preds.size();
				formfilter::addPred(p, Op::WeapModel1st, Mode::Or,
					formfilter::idOperands(line.filterByFirstPersonModelOr, RE::FormType::Static));
				formfilter::pushDisjunct(p, p.positive, first);
			}

			// filterBySkills ("embeddedAVs").
			vals.clear();
			for (const auto& s : line.embeddedAVs) {
				if (auto* av = lookupEmbeddedAV(toLowerCase(s))) {
					vals.push_back(static_cast<std::uint32_t>(*av));
				}
			}
			if (!line.embeddedAVs.empty()) {
				std::size_t first = p.preds.size();
				formfilter::addPred(p, Op::WeapSkill, Mode::Or, vals);
				formfilter::pushDisjunct(p, p.positive, first);
			}

			std::size_t first = p.preds.size();
			if (!line.filterByEditorIdContains.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::And,
					formfilter::needleOperands(p, line.filterByEditorIdContains));
			}
			if (!line.filterByEditorIdContainsOr.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::Or,
					formfilter::needleOperands(p, line.filterByEditorIdContainsOr));
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

			p.catchAll = line.object.empty() && line.keywords.empty() &&
				line.filterByFirstPersonModelOr.empty() && line.keywordsOr.empty() &&
				line.embeddedAVs.empty() && line.filterAnimationTypeOr.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty();

			// --- negative stage, in source order ---------------------------
			if (!line.keywordsExcluded.empty()) {
				formfilter::addNegative(p, Op::Keyword, Mode::Exclude,
					formfilter::ptrOperands(line.keywordsExcluded, RE::FormType::Keyword));
			}
			if (!line.objectExcluded.empty()) {
				formfilter::addNegative(p, Op::ObjectId, Mode::Exclude,
					formfilter::idOperands(line.objectExcluded, RE::FormType::Weapon));
			}
			if (!line.filterByEditorIdContainsExcluded.empty()) {
				formfilter::addNegative(p, Op::EditorId, Mode::Exclude,
					formfilter::needleOperands(p, line.filterByEditorIdContainsExcluded));
			}
			if (!line.filterByWFlags.empty()) {
				// restrictToFlags' upstream loop writes `found = false` for
				// every entry that is not the break case, so `found` can only
				// survive if the FIRST entry is "boundweapon" and the weapon
				// carries the flag. One operand, not a list.
				formfilter::addNegative(p, Op::WeapBound, Mode::And,
					{ toLowerCase(line.filterByWFlags.front()) == "boundweapon" ? 1ull : 0ull });
			}
			if (!line.restrictToSkill.empty()) {
				// Every entry survives: StringToActorValue's answer for an
				// unrecognised name is part of the comparison upstream.
				vals.clear();
				for (const auto& s : line.restrictToSkill) {
					vals.push_back(static_cast<std::uint32_t>(StringToActorValue(toLowerCase(s))));
				}
				formfilter::addNegative(p, Op::WeapSkill, Mode::Or, vals);
			}

			return p;
		}

		// patch() writes indexed attributes, so a rule applied earlier can
		// change whether a later rule matches. Every write site in
		// WEAPON::patch to the keyword array, weaponData.animationType,
		// weaponData.skill or firstPersonModelObject is guarded by one of:
		//   keywordsToAdd / keywordsToRemove, mirrorWeapon, templateWeapon,
		//   modelFirstPerson, animationType (setAnimationType),
		//   skillType (setSkillType).
		// From the first such rule on, the index is stale and every rule scans
		// in full. Applied before the rule is classified, because the rule's
		// own `object` loop patches before its scan.
		bool weaponRuleInvalidatesIndex(const patch_instruction& line)
		{
			// mirrorWeapon / templateWeapon / modelFirstPerson come from
			// extractValueString, which writes the literal "none" when the key
			// is ABSENT -- so they are never empty, and the test has to be the
			// same `!empty() && != "none"` guard their patch() sites use.
			const auto live = [](const std::string& v) { return !v.empty() && v != "none"; };
			return !line.keywordsToAdd.empty() || !line.keywordsToRemove.empty() ||
				live(line.mirrorWeapon) || live(line.templateWeapon) ||
				live(line.modelFirstPerson) || !line.setAnimationType.empty() ||
				!line.setSkillType.empty();
		}

		void buildWeaponIndex(const RE::BSTArray<RE::TESObjectWEAP*>& a_array, formfilter::FormSignalIndex& a_index)
		{
			const std::uint32_t n = static_cast<std::uint32_t>(a_array.size());
			for (std::uint32_t i = 0; i < n; ++i) {
				RE::TESObjectWEAP* w = a_array[i];
				if (!w) {
					continue;
				}
				formfilter::addKeywordSignals(a_index, static_cast<const RE::BGSKeywordForm*>(w), i);
				a_index.add(formfilter::sigTagged(formfilter::kTagWeapAnimType,
					static_cast<std::uint32_t>(w->weaponData.animationType.underlying())), i);
				a_index.add(formfilter::sigTagged(formfilter::kTagWeapSkill,
					static_cast<std::uint32_t>(w->weaponData.skill.underlying())), i);
				if (w->firstPersonModelObject) {
					a_index.add(formfilter::sigForm(w->firstPersonModelObject->formID), i);
				}
			}
		}
	}

	// ==================================================================
	// PHASE 2 -- WEAPON's key table. One row per upstream extract*() call,
	// in the same order. See parsetable.h.
	//
	// "soundLevel" appears twice because upstream extracts it twice into the
	// same member; both rows are harmless and keeping them keeps the table a
	// literal transliteration of the call list.
	// ==================================================================
	namespace
	{
#define WEAP_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kWeaponKeys[] = {
			{ "filterByWeapons", sp::FieldKind::List, WEAP_F(object) },
			{ "filterBySkills", sp::FieldKind::ListAssign, WEAP_F(embeddedAVs) },
			{ "restrictToFlags", sp::FieldKind::ListAssign, WEAP_F(filterByWFlags) },
			{ "restrictToSkills", sp::FieldKind::ListAssign, WEAP_F(restrictToSkill) },
			{ "filterByWeaponsExcluded", sp::FieldKind::List, WEAP_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, WEAP_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, WEAP_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, WEAP_F(keywordsExcluded) },
			{ "attackDamage", sp::FieldKind::Value, WEAP_F(attackDamage) },
			{ "attackDamageToAdd", sp::FieldKind::Value, WEAP_F(attackDamageToAdd) },
			{ "critDamageToAdd", sp::FieldKind::Value, WEAP_F(critDamageToAdd) },
			{ "critDamageSetToBase", sp::FieldKind::Value, WEAP_F(critDamageSetToBase) },
			{ "critDamageMult", sp::FieldKind::Value, WEAP_F(critDamageMult) },
			{ "critPercentMult", sp::FieldKind::Value, WEAP_F(critPercentMult) },
			{ "critDamage", sp::FieldKind::Value, WEAP_F(critDamage) },
			{ "model", sp::FieldKind::Value, WEAP_F(model) },
			{ "mirrorWeapon", sp::FieldKind::Value, WEAP_F(mirrorWeapon) },
			{ "speed", sp::FieldKind::Value, WEAP_F(speed) },
			{ "speedMult", sp::FieldKind::Value, WEAP_F(speedMult) },
			{ "reach", sp::FieldKind::Value, WEAP_F(reach) },
			{ "rangeMin", sp::FieldKind::Value, WEAP_F(rangeMin) },
			{ "rangeMax", sp::FieldKind::Value, WEAP_F(rangeMax) },
			{ "stagger", sp::FieldKind::Value, WEAP_F(stagger) },
			{ "impactDataSet", sp::FieldKind::Value, WEAP_F(impactDataSet) },
			{ "equipSound", sp::FieldKind::Value, WEAP_F(equipSound) },
			{ "unequipSound", sp::FieldKind::Value, WEAP_F(unequipSound) },
			{ "attackFailSound", sp::FieldKind::Value, WEAP_F(failSound) },
			{ "soundLevel", sp::FieldKind::Value, WEAP_F(soundlevel) },
			{ "attackDamageMult", sp::FieldKind::Value, WEAP_F(attackDamageMult) },
			{ "attackDamageMatch", sp::FieldKind::Value, WEAP_F(attackDamageMatch) },
			{ "modelFirstPerson", sp::FieldKind::Value, WEAP_F(modelFirstPerson) },
			{ "value", sp::FieldKind::Value, WEAP_F(value) },
			{ "valueMult", sp::FieldKind::Value, WEAP_F(valueMult) },
			{ "weight", sp::FieldKind::Value, WEAP_F(weight) },
			{ "weightMult", sp::FieldKind::Value, WEAP_F(weightMult) },
			{ "objectEffect", sp::FieldKind::Value, WEAP_F(objectEffect) },
			{ "templateWeapon", sp::FieldKind::Value, WEAP_F(templateWeapon) },
			{ "altBlockMaterialType", sp::FieldKind::Value, WEAP_F(altBlockMaterialType) },
			{ "blockBashImpactDataSet", sp::FieldKind::Value, WEAP_F(blockBashImpactDataSet) },
			{ "equipSlot", sp::FieldKind::Value, WEAP_F(equipSlot) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, WEAP_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, WEAP_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, WEAP_F(filterByEditorIdContainsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, WEAP_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, WEAP_F(keywordsToRemove) },
			{ "filterByModNames", sp::FieldKind::ListAssign, WEAP_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, WEAP_F(fullName) },
			{ "enchantAmount", sp::FieldKind::Value, WEAP_F(enchantAmount) },
			{ "filterByAnimationTypeOr", sp::FieldKind::ListAssign, WEAP_F(filterAnimationTypeOr) },
			{ "animationType", sp::FieldKind::ListAssign, WEAP_F(setAnimationType) },
			{ "skillType", sp::FieldKind::ListAssign, WEAP_F(setSkillType) },
			{ "setFlags", sp::FieldKind::ListAssign, WEAP_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, WEAP_F(removeFlags) },
			{ "minX", sp::FieldKind::Value, WEAP_F(minX) },
			{ "minY", sp::FieldKind::Value, WEAP_F(minY) },
			{ "minZ", sp::FieldKind::Value, WEAP_F(minZ) },
			{ "maxX", sp::FieldKind::Value, WEAP_F(maxX) },
			{ "maxY", sp::FieldKind::Value, WEAP_F(maxY) },
			{ "maxZ", sp::FieldKind::Value, WEAP_F(maxZ) },
			{ "acswMatch", sp::FieldKind::Value, WEAP_F(acswMatch) },
			{ "hasPlugins", sp::FieldKind::ListAssign, WEAP_F(hasPlugin) },
			{ "hasPluginsOr", sp::FieldKind::ListAssign, WEAP_F(hasPluginOr) },
			{ "filterByFirstPersonModelOr", sp::FieldKind::List, WEAP_F(filterByFirstPersonModelOr) },
			{ "dwMatch", sp::FieldKind::Value, WEAP_F(dwMatch) },
			{ "soundLevel", sp::FieldKind::Value, WEAP_F(soundlevel) },
			{ "weaponHitType", sp::FieldKind::Value, WEAP_F(hittype) },
		};

#undef WEAP_F

		const sp::KeyTable& weaponKeyTable()
		{
			static const sp::KeyTable table(kWeaponKeys, std::size(kWeaponKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		weaponKeyTable().parse(line, &l);
		return l;
	}


	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();

		// PHASE 3: built on first use, so a config of nothing but explicitly
		// named weapons never pays for it.
		formfilter::FormSignalIndex index;
		bool                        indexBuilt = false;
		bool                        indexStale = false;
		std::vector<std::uint64_t>  keys;
		std::vector<std::uint32_t>  candidates;

		for (const auto& line : tokens) {
			indexStale = indexStale || weaponRuleInvalidatesIndex(line);

			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectWEAP* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Weapon) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESObjectWEAP*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.filterByFirstPersonModelOr.empty() && line.keywordsOr.empty() && line.filterAnimationTypeOr.empty() && line.embeddedAVs.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			// PHASE 1 + 2: the rule is compiled ONCE into a flat predicate
			// program; the form loop below is a walk over it.
			const formfilter::MatchProgram program = compileRule(line);
			if (program.skipRule) {
				continue;
			}

			// PHASE 3: restrict the scan to the weapons carrying at least one
			// of the signals this rule names, read off the program's opcodes.
			const bool keyed = !indexStale && formfilter::programSignals(program, keys);
			if (keyed) {
				if (!indexBuilt) {
					buildWeaponIndex(ObjectArray, index);
					indexBuilt = true;
					++formfilter::stats().indexBuilds;
				}
				index.gather(keys, candidates);
			}
			const std::uint32_t scanCount =
				static_cast<std::uint32_t>(keyed ? candidates.size() : ObjectArray.size());
			// Diagnostics only; see formfilter::IndexStats.
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

			// Reused across the whole form loop, so the lowered EditorID
			// allocates once rather than once per pair.
			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;
			for (std::uint32_t scanIdx = 0; scanIdx < scanCount; ++scanIdx) {
				const auto& curobj = ObjectArray[keyed ? candidates[scanIdx] : scanIdx];
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(program, curobj)) {
					continue;
				}

				target.form = curobj;
				target.kf = static_cast<const RE::BGSKeywordForm*>(curobj);
				target.weapon = curobj;
				if (formfilter::runProgram(program, target, scratch)) {
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

	void* patch(const WEAPON::patch_instruction& line, RE::TESObjectWEAP* curobj)
	{
		

		if (!line.mirrorWeapon.empty() && line.mirrorWeapon != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.mirrorWeapon;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Weapon) {
					auto weaponToCopyFrom = (RE::TESObjectWEAP*)currentform;
					if (weaponToCopyFrom) {
						curobj->SetModel(weaponToCopyFrom->GetModel());
						curobj->ReplaceModel();
						curobj->firstPersonModelObject = weaponToCopyFrom->firstPersonModelObject;
						curobj->attackFailSound = weaponToCopyFrom->attackFailSound;
						curobj->SetEquipSlot(weaponToCopyFrom->GetEquipSlot());
						curobj->impactDataSet = weaponToCopyFrom->impactDataSet;
						curobj->altBlockMaterialType = weaponToCopyFrom->altBlockMaterialType;
						curobj->equipSound = weaponToCopyFrom->equipSound;
						curobj->unequipSound = weaponToCopyFrom->unequipSound;
						curobj->icon = weaponToCopyFrom->icon;
						curobj->weaponData.animationAttackMult = weaponToCopyFrom->weaponData.animationAttackMult;
						curobj->weaponData.animationType = weaponToCopyFrom->weaponData.animationType;
						curobj->weaponData.attackAnimation = weaponToCopyFrom->weaponData.attackAnimation;
						curobj->weaponData.rangedData = weaponToCopyFrom->weaponData.rangedData;
						logger::debug(FMT_STRING("weapon formid: {:08X} {} mirrored weapon from {:08X}"), curobj->formID, curobj->fullName, weaponToCopyFrom->formID);
					}
					else {
						logger::debug(FMT_STRING("armor formid: {:08X} {} could not mirror from {}"), curobj->formID, curobj->fullName, string_form);
					}
				}
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.attackDamage.empty() && line.attackDamage != "none") {
			try {
				curobj->attackDamage = std::stof(line.attackDamage);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->attackDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.attackDamageToAdd.empty() && line.attackDamageToAdd != "none") {
			try {
				curobj->attackDamage = curobj->attackDamage + std::stof(line.attackDamageToAdd);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} added damage {} total {}"), curobj->formID, curobj->fullName, line.attackDamageToAdd, curobj->attackDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critDamageSetToBase.empty() && line.critDamageSetToBase != "none") {
			try {
				curobj->criticalData.damage = curobj->attackDamage;
				logger::debug(FMT_STRING("weapon formid: {:08X} {} set critDamage {} to weapon base damage"), curobj->formID, curobj->fullName, curobj->criticalData.damage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critDamage.empty() && line.critDamage != "none") {
			try {
				curobj->criticalData.damage = std::stof(line.critDamage);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed critDamage {}"), curobj->formID, curobj->fullName, curobj->criticalData.damage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critDamageToAdd.empty() && line.critDamageToAdd != "none") {
			try {
				curobj->criticalData.damage = curobj->criticalData.damage + std::stof(line.critDamageToAdd);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} added critDamage {} total {}"), curobj->formID, curobj->fullName, line.critDamageToAdd, curobj->criticalData.damage);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critDamageMult.empty() && line.critDamageMult != "none") {
			try {
				curobj->criticalData.damage = curobj->criticalData.damage * std::stof(line.critDamageMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied critDamage by {} total {}"), curobj->formID, curobj->fullName, line.critDamageMult, curobj->criticalData.damage);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.critPercentMult.empty() && line.critPercentMult != "none") {
			try {
				curobj->criticalData.prcntMult = std::stof(line.critPercentMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed critPercentMult {}"), curobj->formID, curobj->fullName, curobj->criticalData.prcntMult);
			} catch (const std::invalid_argument& e) {
			}
		}



		if (!line.speed.empty() && line.speed != "none") {
			try {
				curobj->weaponData.speed = std::stof(line.speed);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed speed {}"), curobj->formID, curobj->fullName, curobj->weaponData.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.speedMult.empty() && line.speedMult != "none") {
			try {
				curobj->weaponData.speed = curobj->weaponData.speed * std::stof(line.speedMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied by {} total {}"), curobj->formID, curobj->fullName, line.speedMult, curobj->weaponData.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reach.empty() && line.reach != "none") {
			try {
				curobj->weaponData.reach = std::stof(line.reach);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed reach {}"), curobj->formID, curobj->fullName, curobj->weaponData.reach);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rangeMin.empty() && line.rangeMin != "none") {
			try {
				curobj->weaponData.minRange = std::stof(line.rangeMin);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed rangeMin {}"), curobj->formID, curobj->fullName, curobj->weaponData.minRange);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rangeMax.empty() && line.rangeMax != "none") {
			try {
				curobj->weaponData.maxRange = std::stof(line.rangeMax);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed rangeMax {}"), curobj->formID, curobj->fullName, curobj->weaponData.maxRange);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.stagger.empty() && line.stagger != "none") {
			try {
				curobj->weaponData.staggerValue = std::stof(line.stagger);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed stagger {}"), curobj->formID, curobj->fullName, curobj->weaponData.staggerValue);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.attackDamageMult.empty() && line.attackDamageMult != "none") {
			try {
				curobj->attackDamage = curobj->attackDamage * std::stof(line.attackDamageMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied attackDamage to {}"), curobj->formID, curobj->fullName, curobj->attackDamage);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.impactDataSet.empty() && line.impactDataSet != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.impactDataSet;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ImpactDataSet) {
					curobj->impactDataSet = (RE::BGSImpactDataSet*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed impactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->impactDataSet->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.equipSound.empty() && line.equipSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.equipSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->equipSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed equipSound {:08X}"), curobj->formID, curobj->fullName, curobj->equipSound->formID);
				} else if (toLowerCase(line.equipSound) == "null") {
					curobj->equipSound = nullptr;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed equipSound to null"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.unequipSound.empty() && line.unequipSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.unequipSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->unequipSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed unequipSound {:08X}"), curobj->formID, curobj->fullName, curobj->unequipSound->formID);
				} else if (toLowerCase(line.unequipSound) == "null") {
					curobj->unequipSound = nullptr;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed unequipSound to null"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.failSound.empty() && line.failSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.failSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->attackFailSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed attackFailSound {:08X}"), curobj->formID, curobj->fullName, curobj->attackFailSound->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.hittype.empty() && line.hittype != "none") {
			try {
				std::string hittypeLower = line.hittype;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});
				if (hittypeLower == "normal") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "dismemberonly" || hittypeLower == "dismember") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kDismemberOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to dismember only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "explodeOnly" || hittypeLower == "explode") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kExplodeOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to explode only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "nodismemberorexplode" || hittypeLower == "no") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNoDismemberOrExplode;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to no dismember or explode"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.soundlevel.empty() && line.soundlevel != "none") {
			try {
				std::string hittypeLower = line.soundlevel;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});
				if (hittypeLower == "loud") {
					curobj->soundLevel = RE::SOUND_LEVEL::kLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to loud"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "normal") {
					curobj->soundLevel = RE::SOUND_LEVEL::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "silent") {
					curobj->soundLevel = RE::SOUND_LEVEL::kSilent;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to silent"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "veryloud") {
					curobj->soundLevel = RE::SOUND_LEVEL::kVeryLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to very load"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "quiet") {
					curobj->soundLevel = RE::SOUND_LEVEL::kQuiet;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to quiet"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("weapon formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("weapon formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.objectEffect.empty() && line.objectEffect != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.objectEffect;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform) {
				curobj->formEnchanting = (RE::EnchantmentItem*)currentform;
				logger::debug(FMT_STRING("weapon formid: {:08X} changed objectEffect {:08X} "), curobj->formID, ((RE::EnchantmentItem*)currentform)->formID);
			} else if (line.objectEffect == "null") {
				curobj->formEnchanting = nullptr;
				logger::debug(FMT_STRING("weapon formid: {:08X} changed objectEffect null (none)"), curobj->formID);
			}
		}



		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.modelFirstPerson.empty() && line.modelFirstPerson != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.modelFirstPerson;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Static) {
					curobj->firstPersonModelObject = (RE::TESObjectSTAT*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed first person model {:08X}"), curobj->formID, curobj->fullName, curobj->firstPersonModelObject->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.templateWeapon.empty() && line.templateWeapon != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.templateWeapon;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Weapon) {
					auto templateWeapon = (RE::TESObjectWEAP*)currentform;
					curobj->templateWeapon = templateWeapon;
					curobj->addons = templateWeapon->addons;
					curobj->numAddons = templateWeapon->numAddons;
					curobj->altBlockMaterialType = templateWeapon->altBlockMaterialType;
					curobj->alternateTextures = templateWeapon->alternateTextures;
					curobj->attackDamage = templateWeapon->attackDamage;
					curobj->attackFailSound = templateWeapon->attackFailSound;
					curobj->attackLoopSound = templateWeapon->attackLoopSound;
					curobj->attackSound = templateWeapon->attackSound;
					curobj->attackSound2D = templateWeapon->attackSound2D;
					curobj->blockBashImpactDataSet = templateWeapon->blockBashImpactDataSet;
					curobj->boundData = templateWeapon->boundData;
					curobj->castingType = templateWeapon->castingType;
					curobj->equipSound = templateWeapon->equipSound;
					curobj->pickupSound = templateWeapon->pickupSound;
					curobj->putdownSound = templateWeapon->putdownSound;
					curobj->idleSound = templateWeapon->idleSound;
					curobj->keywords = templateWeapon->keywords;
					curobj->numKeywords = templateWeapon->numKeywords;
					curobj->descriptionText = templateWeapon->descriptionText;
					curobj->unequipSound = templateWeapon->unequipSound;
					curobj->impactDataSet = templateWeapon->impactDataSet;
					curobj->data = templateWeapon->data;
					curobj->criticalData = templateWeapon->criticalData;
					curobj->firstPersonModelObject = templateWeapon->firstPersonModelObject;
					curobj->model = templateWeapon->model;
					curobj->ReplaceModel();
					curobj->weaponData = templateWeapon->weaponData;
					curobj->weight = templateWeapon->weight;
					curobj->soundLevel = templateWeapon->soundLevel;
					curobj->icon = templateWeapon->icon;
					curobj->equipSlot = templateWeapon->equipSlot;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed templateWeapon {:08X}"), curobj->formID, curobj->fullName, curobj->templateWeapon->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.altBlockMaterialType.empty() && line.altBlockMaterialType != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.altBlockMaterialType;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::MaterialType) {
					curobj->altBlockMaterialType = (RE::BGSMaterialType*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed altBlockMaterialType {:08X}"), curobj->formID, curobj->fullName, curobj->altBlockMaterialType->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.blockBashImpactDataSet.empty() && line.blockBashImpactDataSet != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.blockBashImpactDataSet;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ImpactDataSet) {
					curobj->blockBashImpactDataSet = (RE::BGSImpactDataSet*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed blockBashImpactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->blockBashImpactDataSet->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.equipSlot.empty() && line.equipSlot != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.equipSlot;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EquipSlot) {
					curobj->SetEquipSlot((RE::BGSEquipSlot*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed equipSlot {:08X}"), curobj->formID, curobj->fullName, curobj->equipSlot->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.enchantAmount.empty() && line.enchantAmount != "none") {
			try {
				//logger::debug(FMT_STRING("weapon formid: {:08X} {} changed old enchantAmount {}"), curobj->formID, curobj->fullName, curobj->amountofEnchantment);
				curobj->amountofEnchantment = std::stoi(line.enchantAmount);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed enchantAmount {}"), curobj->formID, curobj->fullName, curobj->amountofEnchantment);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.attackDamageMatch.empty() && line.attackDamageMatch != "none") {
			try {
				RE::TESForm*       currentform = nullptr;
				RE::TESObjectWEAP* object = nullptr;
				currentform = GetFormFromIdentifier(line.attackDamageMatch);
				if (currentform && currentform->formType == RE::FormType::Weapon) {
					object = (RE::TESObjectWEAP*)currentform;
					if (object) {
						curobj->attackDamage = object->attackDamage;
						logger::debug(FMT_STRING("weapon formid: {:08X} {} match weapon attack damage of weapon {:08X} {}"), curobj->formID, curobj->fullName, object->formID, object->fullName);
					}
				} else {
					logger::debug(FMT_STRING("weapon formid: {:08X} {} object is not an weapon {}"), curobj->formID, curobj->fullName, line.attackDamageMatch);
				}

			} catch (const std::invalid_argument& e) {
			}
		}
		
		 if (!line.setAnimationType.empty()) {
			for (const auto& setFlag : line.setAnimationType) {
				if (toLowerCase(setFlag) == "handtohandmelee") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kHandToHandMelee;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType handtohandmelee"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "onehandsword") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kOneHandSword;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType onehandsword"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "onehanddagger") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kOneHandDagger;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType onehanddagger"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "onehandaxe") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kOneHandAxe;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType onehandaxe"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "onehandmace") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kOneHandMace;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType onehandmace"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "twohandsword") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kTwoHandSword;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType twohandsword"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "twohandaxe") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kTwoHandAxe;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType twohandaxe"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "bow") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kBow;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType bow"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "staff") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kStaff;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType staff"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "crossbow") {
					curobj->weaponData.animationType = RE::WEAPON_TYPE::kCrossbow;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} set AnimationType crossbow"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					logger::warn(FMT_STRING("Invalid set flag: {}"), setFlag);
				}
			}
		}

		 if (!line.setSkillType.empty()) {
			for (const auto& setFlag : line.setSkillType) {
				if (toLowerCase(setFlag) == "onehanded") {
					curobj->weaponData.skill = RE::ActorValue::kOneHanded;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to onehanded"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "twohanded") {
					curobj->weaponData.skill = RE::ActorValue::kTwoHanded;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to twohanded"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "marksman") {
					curobj->weaponData.skill = RE::ActorValue::kArchery;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to marksman"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "block") {
					curobj->weaponData.skill = RE::ActorValue::kBlock;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to block"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "smithing") {
					curobj->weaponData.skill = RE::ActorValue::kSmithing;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to smithing"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "heavyarmor") {
					curobj->weaponData.skill = RE::ActorValue::kHeavyArmor;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to heavyarmor"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "lightarmor") {
					curobj->weaponData.skill = RE::ActorValue::kLightArmor;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to lightarmor"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "pickpocket") {
					curobj->weaponData.skill = RE::ActorValue::kPickpocket;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to pickpocket"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "lockpicking") {
					curobj->weaponData.skill = RE::ActorValue::kLockpicking;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to lockpicking"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "sneak") {
					curobj->weaponData.skill = RE::ActorValue::kSneak;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to sneak"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "alchemy") {
					curobj->weaponData.skill = RE::ActorValue::kAlchemy;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to alchemy"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "speechcraft") {
					curobj->weaponData.skill = RE::ActorValue::kSpeech;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to speechcraft"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "alteration") {
					curobj->weaponData.skill = RE::ActorValue::kAlteration;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to alteration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "conjuration") {
					curobj->weaponData.skill = RE::ActorValue::kConjuration;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to conjuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "destruction") {
					curobj->weaponData.skill = RE::ActorValue::kDestruction;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to destruction"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "illusion") {
					curobj->weaponData.skill = RE::ActorValue::kIllusion;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to illusion"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "restoration") {
					curobj->weaponData.skill = RE::ActorValue::kRestoration;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to restoration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "enchanting") {
					curobj->weaponData.skill = RE::ActorValue::kEnchanting;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} skill set to enchanting"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					logger::warn(FMT_STRING("Invalid set flag: {}"), setFlag);
				}
			}
		}
		
		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				//logger::debug(FMT_STRING("weapon formid: {:08X} {} flag {}"), curobj->formID, curobj->fullName, setFlag);
				if (toLowerCase(setFlag) == "ignoresnormalweaponresistance") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "automatic") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kAutomatic);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled automatic"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "hasscope") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kHasScope);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled hasscope"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "cantdrop") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kCantDrop);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled cantdrop"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "hidebackpack") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kHideBackpack);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled hidebackpack"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "embeddedweapon") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kEmbeddedWeapon);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled embeddedweapon"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "dontusefirstpersonisanim") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kDontUseFirstPersonISAnim);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled dontusefirstpersonisanim"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nonplayable") {
					curobj->weaponData.flags.set(RE::TESObjectWEAP::Data::Flag::kNonPlayable);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag enabled nonplayable"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "ignoresnormalweaponresistance") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "automatic") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kAutomatic);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled automatic"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hasscope") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kHasScope);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled hasscope"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "cantdrop") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kCantDrop);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled cantdrop"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hidebackpack") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kHideBackpack);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled hidebackpack"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "embeddedweapon") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kEmbeddedWeapon);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled embeddedweapon"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "dontusefirstpersonisanim") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kDontUseFirstPersonISAnim);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled dontusefirstpersonisanim"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nonplayable") {
					curobj->weaponData.flags.reset(RE::TESObjectWEAP::Data::Flag::kNonPlayable);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} flag disabled nonplayable"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}


		 if (!line.minX.empty() && line.minX != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed minX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minX);
				curobj->boundData.boundMin.x = std::stoi(line.minX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minY.empty() && line.minY != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed minY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minY);
				curobj->boundData.boundMin.y = std::stoi(line.minY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minZ.empty() && line.minZ != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed minZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minZ);
				curobj->boundData.boundMin.z = std::stoi(line.minZ);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxX.empty() && line.maxX != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed maxX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxX);
				curobj->boundData.boundMax.x = std::stoi(line.maxX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxY.empty() && line.maxY != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed maxY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxY);
				curobj->boundData.boundMax.y = std::stoi(line.maxY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxZ.empty() && line.maxZ != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed maxZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxZ);
				curobj->boundData.boundMax.z = std::stoi(line.maxZ);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.acswMatch.empty() && line.acswMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.acswMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Weapon) {
					auto templateArmor = (RE::TESObjectWEAP*)currentform;
					curobj->attackDamage = templateArmor->attackDamage;
					curobj->criticalData = templateArmor->criticalData;
					curobj->weaponData.speed = templateArmor->weaponData.speed;
					curobj->weight = templateArmor->weight;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed attackDamage, critical data, speed, weight to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.dwMatch.empty() && line.dwMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.dwMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Weapon) {
					auto templateWeapon = (RE::TESObjectWEAP*)currentform;
					curobj->attackDamage = templateWeapon->attackDamage;
					curobj->weight = templateWeapon->weight;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage and weight to {:08X} {}"), curobj->formID, curobj->fullName, templateWeapon->formID, templateWeapon->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

	}

}
