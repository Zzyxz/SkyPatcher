#include "spell.h"
#include "formfilter.h"
#include "parsetable.h"
#include <sstream>
#include <unordered_set>
namespace SPELL
{

	static const std::unordered_map<std::string, RE::SpellItem::SpellFlag> flagMap{
	{"costoverride", RE::SpellItem::SpellFlag::kCostOverride},
	{"fooditem", RE::SpellItem::SpellFlag::kFoodItem},
	{"extendduration", RE::SpellItem::SpellFlag::kExtendDuration},
	{"pcstartspell", RE::SpellItem::SpellFlag::kPCStartSpell},
	{"instantcast", RE::SpellItem::SpellFlag::kInstantCast},
	{"ignoreloscheck", RE::SpellItem::SpellFlag::kIgnoreLOSCheck},
	{"ignoreresistance", RE::SpellItem::SpellFlag::kIgnoreResistance},
	{"noabsorb", RE::SpellItem::SpellFlag::kNoAbsorb},
	{"nodualcastmods", RE::SpellItem::SpellFlag::kNoDualCastMods}
	};

	namespace
	{
#define SPELL_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kSpellKeys[] = {
			{ "filterBySpells", sp::FieldKind::List, SPELL_F(objects) },
			{ "filterByFlags", sp::FieldKind::List, SPELL_F(flagFilters) },
			{ "filterByFlagsOr", sp::FieldKind::List, SPELL_F(flagFiltersOr) },
			{ "filterByFlagsExcluded", sp::FieldKind::List, SPELL_F(flagFiltersExcluded) },
			{ "filterByCastingPerk", sp::FieldKind::List, SPELL_F(filterByCastingPerk) },
			{ "filterBySpellsExcluded", sp::FieldKind::List, SPELL_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, SPELL_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, SPELL_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, SPELL_F(keywordsExcluded) },
			{ "filterByMgefs", sp::FieldKind::List, SPELL_F(mgefs) },
			{ "filterByMgefsOr", sp::FieldKind::List, SPELL_F(mgefsOr) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, SPELL_F(mgefsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, SPELL_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, SPELL_F(keywordsToRemove) },
			{ "mgefsToAdd", sp::FieldKind::Arr2D, SPELL_F(addedObjects) },
			{ "mgefsToChange", sp::FieldKind::Arr2D, SPELL_F(changedObjects) },
			{ "mgefsToChangeAdd", sp::FieldKind::Arr2D, SPELL_F(changedAddObjects) },
			{ "mgefsToChangeFirst", sp::FieldKind::Arr2D, SPELL_F(changedFirstObjects) },
			{ "mgefsToChangeDamageHealthEffects", sp::FieldKind::Arr2D, SPELL_F(changedDamageHealthObjects) },
			{ "mgefsToRemove", sp::FieldKind::List, SPELL_F(removedObjects) },
			{ "filterByType", sp::FieldKind::Value, SPELL_F(filterType) },
			{ "baseCost", sp::FieldKind::Value, SPELL_F(baseCost) },
			{ "halfCostPerk", sp::FieldKind::Value, SPELL_F(halfCostPerk) },
			{ "chargeTime", sp::FieldKind::Value, SPELL_F(chargeTime) },
			{ "castType", sp::FieldKind::Value, SPELL_F(castType) },
			{ "equipSlot", sp::FieldKind::Value, SPELL_F(equipSlot) },
			{ "setFlags", sp::FieldKind::ListAssign, SPELL_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, SPELL_F(removeFlags) },
			{ "clear", sp::FieldKind::Value, SPELL_F(clear) },
			{ "filterByModNames", sp::FieldKind::ListAssign, SPELL_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, SPELL_F(fullName) },
			{ "restrictToCastingType", sp::FieldKind::List, SPELL_F(restrictToCastingType) },
			{ "restrictToSchool", sp::FieldKind::List, SPELL_F(restrictToSchool) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, SPELL_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, SPELL_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, SPELL_F(filterByEditorIdContainsExcluded) },
		};

#undef SPELL_F

		const sp::KeyTable& spellKeyTable()
		{
			static const sp::KeyTable table(kSpellKeys, std::size(kSpellKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		spellKeyTable().parse(line, &l);

		return l;
	}

	namespace
	{
		formfilter::MatchProgram compileSpellRule(const line_content& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;

			std::size_t first = p.preds.size();
			if (!line.keywords.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::And,
					formfilter::ptrOperands(line.keywords, RE::FormType::Keyword));
			}
			if (!line.keywordsOr.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::Or,
					formfilter::ptrOperands(line.keywordsOr, RE::FormType::Keyword));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			const bool playerGuard =
				line.filterByEditorIdContains.empty() != line.filterByEditorIdContainsOr.empty();
			first = p.preds.size();
			if (!line.filterByEditorIdContains.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::And,
					formfilter::needleOperands(p, line.filterByEditorIdContains), playerGuard);
			}
			if (!line.filterByEditorIdContainsOr.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::Or,
					formfilter::needleOperands(p, line.filterByEditorIdContainsOr), playerGuard);
			}
			formfilter::pushDisjunct(p, p.positive, first);

			formfilter::sortCheapestFirst(p.positive);

			p.catchAll = line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() &&
				line.mgefs.empty() && line.mgefsOr.empty() && line.filterByCastingPerk.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() &&
				line.flagFilters.empty() && line.flagFiltersOr.empty() && line.filterType == "none";

			return p;
		}

		struct SpellRuleExtras
		{
			std::vector<RE::FormID>               castingPerks;
			std::vector<RE::SpellItem::SpellFlag> flagsAnd, flagsOr;
			std::vector<std::pair<RE::SpellItem::SpellFlag, std::string>> flagsExcluded;
			std::vector<RE::FormID>                   mgefsAnd;
			std::vector<RE::FormID>                   mgefsOr, mgefsExcluded;
			std::vector<RE::MagicSystem::CastingType> castingTypes;
			std::vector<RE::ActorValue>               schools;
			std::vector<RE::MagicSystem::SpellType>   spellTypes;
			bool                                      typePresent = false;
			bool                                      flagsPresent = false;
			bool                                      flagsAndUnresolved = false;
			bool                                      flagsOrAbsent = true;
			bool                                      mgefsPresent = false;
			bool                                      mgefsOrAbsent = true;
			bool                                      playerGuard = false;
		};

		std::vector<RE::SpellItem::SpellFlag> resolveSpellFlags(const std::vector<std::string>& a_in,
			const char* a_kind)
		{
			std::vector<RE::SpellItem::SpellFlag> out;
			out.reserve(a_in.size());
			for (const auto& s : a_in) {
				const std::string key = toLowerCase(s);
				const auto        it = flagMap.find(key);
				if (it != flagMap.end()) {
					out.push_back(it->second);
				} else {
					logger::warn("spell - unknown flag in {} filter: {}", a_kind, key);
				}
			}
			return out;
		}

		std::vector<RE::MagicSystem::SpellType> resolveSpellTypes(const std::string& a_value)
		{
			using T = RE::MagicSystem::SpellType;
			static const std::unordered_map<std::string, T> map{
				{ "spell", T::kSpell }, { "disease", T::kDisease }, { "power", T::kPower },
				{ "lesserpower", T::kLesserPower }, { "lesser_power", T::kLesserPower },
				{ "ability", T::kAbility }, { "poison", T::kPoison }, { "enchantment", T::kEnchantment },
				{ "potion", T::kPotion }, { "alchemy", T::kPotion },
				{ "wortcraft", T::kWortCraft }, { "wordcraft", T::kWortCraft }, { "ingredient", T::kIngredient },
				{ "leveledspell", T::kLeveledSpell }, { "leveled_spell", T::kLeveledSpell },
				{ "addiction", T::kAddiction },
				{ "voicepower", T::kVoicePower }, { "voice_power", T::kVoicePower }, { "shout", T::kVoicePower },
				{ "staffenchantment", T::kStaffEnchantment }, { "staff_enchantment", T::kStaffEnchantment },
				{ "scroll", T::kScroll },
			};
			std::vector<T>    out;
			std::stringstream ss(a_value);
			std::string       piece;
			while (std::getline(ss, piece, ',')) {
				const std::string name = toLowerCase(trim(piece));
				if (name.empty()) {
					continue;
				}
				const auto it = map.find(name);
				if (it != map.end()) {
					out.push_back(it->second);
				} else {
					logger::warn("spell - unknown type in filterByType: {}", name);
				}
			}
			return out;
		}

		std::vector<std::pair<RE::SpellItem::SpellFlag, std::string>> resolveSpellFlagsNamed(
			const std::vector<std::string>& a_in, const char* a_kind)
		{
			std::vector<std::pair<RE::SpellItem::SpellFlag, std::string>> out;
			out.reserve(a_in.size());
			for (const auto& s : a_in) {
				std::string key = toLowerCase(s);
				const auto  it = flagMap.find(key);
				if (it != flagMap.end()) {
					out.emplace_back(it->second, std::move(key));
				} else {
					logger::debug("Unknown flag in {} filter: {}", a_kind, key);
				}
			}
			return out;
		}

		std::vector<RE::MagicSystem::CastingType> resolveCastingTypes(const std::vector<std::string>& a_in)
		{
			std::vector<RE::MagicSystem::CastingType> out;
			for (const auto& raw : a_in) {
				const std::string s = toLowerCase(raw);
				if (s == "concentration") {
					out.push_back(RE::MagicSystem::CastingType::kConcentration);
				} else if (s == "constanteffect") {
					out.push_back(RE::MagicSystem::CastingType::kConstantEffect);
				} else if (s == "fireandforget") {
					out.push_back(RE::MagicSystem::CastingType::kFireAndForget);
				} else if (s == "scroll") {
					out.push_back(RE::MagicSystem::CastingType::kScroll);
				}
			}
			return out;
		}

		std::vector<RE::ActorValue> resolveSchools(const std::vector<std::string>& a_in)
		{
			std::vector<RE::ActorValue> out;
			for (const auto& raw : a_in) {
				const std::string s = toLowerCase(raw);
				if (s == "destruction") {
					out.push_back(RE::ActorValue::kDestruction);
				} else if (s == "restoration") {
					out.push_back(RE::ActorValue::kRestoration);
				} else if (s == "alteration") {
					out.push_back(RE::ActorValue::kAlteration);
				} else if (s == "conjuration") {
					out.push_back(RE::ActorValue::kConjuration);
				} else if (s == "illusion") {
					out.push_back(RE::ActorValue::kIllusion);
				} else if (s == "none") {
					out.push_back(RE::ActorValue::kNone);
				}
			}
			return out;
		}

		SpellRuleExtras compileSpellExtras(const line_content& line)
		{
			SpellRuleExtras x;
			x.castingPerks = formfilter::resolveFormIDs(line.filterByCastingPerk, RE::FormType::Perk);

			x.flagsAnd = resolveSpellFlags(line.flagFilters, "AND");
			x.flagsOr = resolveSpellFlags(line.flagFiltersOr, "OR");
			x.flagsExcluded = resolveSpellFlagsNamed(line.flagFiltersExcluded, "excluded");
			x.flagsPresent = !line.flagFilters.empty() || !line.flagFiltersOr.empty();
			x.flagsAndUnresolved = !line.flagFilters.empty() && x.flagsAnd.empty();
			x.flagsOrAbsent = line.flagFiltersOr.empty();

			x.typePresent = line.filterType != "none";
			if (x.typePresent) {
				x.spellTypes = resolveSpellTypes(line.filterType);
			}

			x.mgefsAnd = formfilter::resolveFormIDsKeepingHoles(line.mgefs, RE::FormType::MagicEffect);
			x.mgefsOr = formfilter::resolveFormIDs(line.mgefsOr, RE::FormType::MagicEffect);
			x.mgefsExcluded = formfilter::resolveFormIDs(line.mgefsExcluded, RE::FormType::MagicEffect);
			x.mgefsPresent = !line.mgefs.empty() || !line.mgefsOr.empty();
			x.mgefsOrAbsent = line.mgefsOr.empty();

			x.castingTypes = resolveCastingTypes(line.restrictToCastingType);
			x.schools = resolveSchools(line.restrictToSchool);

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

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		auto& objectSpellArray = dataHandler->GetFormArray<RE::SpellItem>();
		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;


			if (!line.objects.empty()) {
				for (const auto& npcstring : line.objects) {
					RE::TESForm*         currentform = nullptr;
					RE::SpellItem* curobj = nullptr;
					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Spell) {
						curobj = (RE::SpellItem*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByCastingPerk.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.flagFilters.empty() && line.flagFiltersOr.empty() && line.filterType == "none") {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			const formfilter::MatchProgram          program = compileSpellRule(line);
			const SpellRuleExtras                   extra = compileSpellExtras(line);
			const formfilter::ResolvedCommonFilters resolved =
				formfilter::resolve(view, RE::FormType::Spell);

			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;

			for (const auto& curobj : objectSpellArray) {
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

				if (!found && !extra.castingPerks.empty() && curobj->data.castingPerk) {
					for (const RE::FormID id : extra.castingPerks) {
						if (curobj->data.castingPerk->formID == id) {
							found = true;
							break;
						}
					}
				}

				if (!found && extra.flagsPresent) {
					bool flagAnd = !extra.flagsAndUnresolved;
					for (const auto f : extra.flagsAnd) {
						if (!curobj->data.flags.all(f)) {
							flagAnd = false;
							break;
						}
					}
					bool flagOr = extra.flagsOrAbsent;
					if (!flagOr) {
						for (const auto f : extra.flagsOr) {
							if (curobj->data.flags.all(f)) {
								flagOr = true;
								break;
							}
						}
					}
					if (flagAnd && flagOr) {
						found = true;
					}
				}

				if (!found && extra.mgefsPresent) {
					bool mgefAnd = true;
					if (!extra.mgefsAnd.empty()) {
						bool anyResolved = false;
						for (const RE::FormID id : extra.mgefsAnd) {
							if (!id) {
								continue;
							}
							anyResolved = true;
							bool foundInList = false;
							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect && effect->baseEffect->formID == id) {
									foundInList = true;
									break;
								}
							}
							if (!foundInList) {
								mgefAnd = false;
								break;
							}
						}
						if (!anyResolved) {
							mgefAnd = false;
						}
					}
					bool mgefOr = extra.mgefsOrAbsent;
					if (!mgefOr) {
						for (const RE::FormID id : extra.mgefsOr) {
							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect && effect->baseEffect->formID == id) {
									mgefOr = true;
									break;
								}
							}
							if (mgefOr) {
								break;
							}
						}
					}
					if (mgefAnd && mgefOr) {
						found = true;
					}
				}

				if (!found && extra.typePresent) {
					for (const auto t : extra.spellTypes) {
						if (curobj->data.spellType == t) {
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

				if (!resolved.editorIdContainsExcluded.empty() && found) {
					const std::string& editorId = loweredEditorId(curobj, scratch);
					for (const auto& needle : resolved.editorIdContainsExcluded) {
						if (editorId.find(needle) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (!line.flagFiltersExcluded.empty() && found) {
					for (const auto& fe : extra.flagsExcluded) {
						if (curobj->data.flags.all(fe.first)) {
							found = false;
							logger::debug(FMT_STRING("Spell {:08X} {} has excluded flag '{}', skipping"), curobj->formID, curobj->fullName, fe.second);
							break;
						}
					}
				}

				if (!line.mgefsExcluded.empty() && found) {
					for (const RE::FormID id : extra.mgefsExcluded) {
						for (const auto& effect : curobj->effects) {
							if (effect && effect->baseEffect && effect->baseEffect->formID == id) {
								found = false;
								break;
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(resolved, curobj)) {
						found = false;
					}
				}

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					if (!extra.castingTypes.empty()) {
						const auto castingType = curobj->GetCastingType();
						for (const auto t : extra.castingTypes) {
							if (castingType == t) {
								foundCastingType = true;
								break;
							}
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					if (curobj && !curobj->effects.empty() && curobj->effects.front() && curobj->effects.front()->baseEffect) {
						const auto skill = curobj->effects.front()->baseEffect->data.associatedSkill;

						for (const auto s : extra.schools) {
							if (skill == s) {
								foundSchool = true;
								break;
							}
						}

						if (!foundSchool) {
							found = false;
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

	void* readConfig(const std::string& folder)
	{
		char skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						} else {
							std::string fileName = ent->d_name;
							size_t pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();

								const std::string lowerName = toLowerCase(fileName);
								if (lowerName.find(".esp") != std::string::npos || lowerName.find(".esl") != std::string::npos || lowerName.find(".esm") != std::string::npos) {
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
								std::string line;
								std::ifstream infile;
								std::list<line_content> tokens;
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

	

	void patch(const SPELL::line_content& line, RE::SpellItem* curobj) {
		try {
		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("spell formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if ( !line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("spell formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (IsSwitchOn(line.clear)) {
			curobj->effects.clear();
			logger::debug(FMT_STRING("spell formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
		}

		if ( !line.removedObjects.empty()) {
			for (const auto& objectToRemove : line.removedObjects) {
				std::string        removeFormStr = objectToRemove;
				auto* resolvedRemoveForm = GetFormFromIdentifier(removeFormStr);
				auto* removeForm = resolvedRemoveForm ? resolvedRemoveForm->As<RE::EffectSetting>() : nullptr;
				if (removeForm) {
					auto it = curobj->effects.begin();
					while (it != curobj->effects.end()) {
						if ((*it) && (*it)->baseEffect && (*it)->baseEffect->formID == removeForm->formID) {
							it = curobj->effects.erase(it);
							logger::debug(FMT_STRING("spell {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if ( !line.addedObjects.empty()) {
		for (const auto& objectToAdd : line.addedObjects) {
			try {
			if (objectToAdd.size() < 4) { logger::warn("spell {:08X}: invalid addedObjects entry, expected 4 values", curobj->formID); continue; }
				std::string        addFormStr = objectToAdd[0];
				float              magnitude = std::stof(objectToAdd[1]);
				int32_t            duration = std::stoi(objectToAdd[2]);
				int32_t            area = std::stoi(objectToAdd[3]);
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
				if (addForm) {
					RE::Effect* tempEffectItem = new RE::Effect;
					tempEffectItem->baseEffect = addForm;
					tempEffectItem->effectItem.magnitude = magnitude;
					tempEffectItem->effectItem.duration = duration;
					tempEffectItem->effectItem.area = area;
					float durationForCalc = duration;
					if (durationForCalc == 0) {
						durationForCalc = 10;
					}
					tempEffectItem->cost = addForm->data.baseCost * pow(magnitude * durationForCalc * 0.1, 1.1);
					
				curobj->effects.push_back(tempEffectItem);
				logger::debug(FMT_STRING("spell {:08X} {} added effect {:08X} {} magnitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
				if (objectToAdd.size() >= 5) {
					moveEffectToPosition0(curobj->effects, tempEffectItem);
					logger::debug(FMT_STRING("spell {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
				}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid mgefsToAdd entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if ( !line.changedObjects.empty()) {
		for (auto objectToAdd : line.changedObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("spell {:08X}: empty changedObjects entry", curobj->formID); continue; }
			objectToAdd.resize(5, "null");
				std::string        addFormStr = objectToAdd[0];
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
				if (addForm) {
					float magnitude = 0.0f;
					if (!IsNullValue(objectToAdd[1])) {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (!IsNullValue(objectToAdd[2])) {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (!IsNullValue(objectToAdd[3])) {
						area = std::stoi(objectToAdd[3]);
					}

					float multiplier = 0;

					if (!IsNullValue(objectToAdd[4])) {
						multiplier = multiplier + std::stof(objectToAdd[4]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (!IsNullValue(objectToAdd[1])) {
								effect->effectItem.magnitude = magnitude;
								logger::debug(FMT_STRING("spell {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("spell {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("spell {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (!IsNullValue(objectToAdd[4])) {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid mgefsToChange entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if (!line.changedAddObjects.empty()) {
		for (auto objectToAdd : line.changedAddObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("spell {:08X}: empty changedAddObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				std::string        addFormStr = objectToAdd[0];
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
				if (addForm) {
					float magnitude = 0.0f;
					if (!IsNullValue(objectToAdd[1])) {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (!IsNullValue(objectToAdd[2])) {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (!IsNullValue(objectToAdd[3])) {
						area = std::stoi(objectToAdd[3]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (!IsNullValue(objectToAdd[1])) {
								effect->effectItem.magnitude = effect->effectItem.magnitude + magnitude;
								logger::debug(FMT_STRING("spell {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("spell {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("spell {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid mgefsToChangeAdd entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if (!line.changedFirstObjects.empty()) {
		for (auto objectToAdd : line.changedFirstObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("spell {:08X}: empty changedFirstObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");

					float magnitude = 0.0f;
					if (!IsNullValue(objectToAdd[0])) {
						magnitude = std::stof(objectToAdd[0]);
					}

					int32_t duration = 0;
					if (!IsNullValue(objectToAdd[1])) {
						duration = std::stoi(objectToAdd[1]);
					}

					int32_t area = 0;
					if (!IsNullValue(objectToAdd[2])) {
						area = std::stoi(objectToAdd[2]);
					}

					float multiplier = 0;
					if (!IsNullValue(objectToAdd[3])) {
						multiplier = multiplier + std::stof(objectToAdd[3]);
					}

				if (!curobj->effects.empty()) {
					auto& effect = curobj->effects.front();
					if (effect && effect->baseEffect) {
						if (!IsNullValue(objectToAdd[0])) {
							effect->effectItem.magnitude = magnitude;
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
						if (!IsNullValue(objectToAdd[1])) {
							effect->effectItem.duration = duration;
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
						}
						if (!IsNullValue(objectToAdd[2])) {
							effect->effectItem.area = area;
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
						}
						if (!IsNullValue(objectToAdd[3])) {
							effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
							logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
					}

			}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid mgefsToChangeFirst entry skipped: {}"), curobj->formID, e.what());
			}
		}
		}

		if (!line.changedDamageHealthObjects.empty()) {
		for (auto objectToAdd : line.changedDamageHealthObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("spell {:08X}: empty changedDamageHealthObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				float magnitude = 0.0f;
				if (!IsNullValue(objectToAdd[0])) {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (!IsNullValue(objectToAdd[1])) {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (!IsNullValue(objectToAdd[2])) {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (!IsNullValue(objectToAdd[3])) {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				for (const auto& effect : curobj->effects) {
					if (effect && effect->baseEffect && effect->baseEffect->IsDetrimental() && (effect->baseEffect->data.primaryAV == RE::ActorValue::kHealth || effect->baseEffect->data.secondaryAV == RE::ActorValue::kHealth)) {
						if (!IsNullValue(objectToAdd[0])) {
							effect->effectItem.magnitude = magnitude;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
						if (!IsNullValue(objectToAdd[1])) {
							effect->effectItem.duration = duration;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
						}
						if (!IsNullValue(objectToAdd[2])) {
							effect->effectItem.area = area;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
						}
						if (!IsNullValue(objectToAdd[3])) {
							effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
							logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) damage effect magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid mgefsToChangeDamageHealthEffects entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			try {
			curobj->data.costOverride = std::stoi(line.baseCost);
			logger::debug(FMT_STRING("spell {:08X} {} changed base cost to {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid baseCost skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.halfCostPerk.empty() && line.halfCostPerk != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.halfCostPerk;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Perk) {
					curobj->data.castingPerk = (RE::BGSPerk*)currentform;
					logger::debug(FMT_STRING("spell formid: {:08X} {} changed halfCostPerk {:08X}"), curobj->formID, curobj->fullName, curobj->data.castingPerk->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if ( !line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.equipSlot.empty() && line.equipSlot != "none") {
			RE::TESForm* currentform = nullptr;
			currentform = GetFormFromIdentifier(line.equipSlot);
			if (currentform && currentform->formType == RE::FormType::EquipSlot) {
				curobj->SetEquipSlot((RE::BGSEquipSlot*)currentform);
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed equipslot to {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
			}
		}


		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			try {
			curobj->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("spell formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("spell {:08X}: invalid chargeTime skipped: {}"), curobj->formID, e.what());
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				std::string key = toLowerCase(setFlag);
				auto it = flagMap.find(key);
				if (it != flagMap.end()) {
					curobj->data.flags.set(it->second);
					logger::debug(FMT_STRING("spell formid: {:08X} {} flag enabled {}"), curobj->formID, curobj->fullName, key);
				}
				else {
					logger::debug(FMT_STRING("spell formid: {:08X} {} unknown flag '{}', skipping"), curobj->formID, curobj->fullName, key);
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				std::string key = toLowerCase(removeFlag);
				auto it = flagMap.find(key);
				if (it != flagMap.end()) {
					curobj->data.flags.reset(it->second);
					logger::debug(FMT_STRING("spell formid: {:08X} {} flag disabled {}"), curobj->formID, curobj->fullName, key);
				}
				else {
					logger::debug(FMT_STRING("spell formid: {:08X} {} unknown flag '{}' for removal, skipping"), curobj->formID, curobj->fullName, key);
				}
			}
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("spell {:08X}: skipped remaining operations after invalid value: {}"), curobj ? curobj->formID : 0, e.what());
		}
}

}
