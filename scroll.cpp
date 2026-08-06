#include "scroll.h"
#include "formfilter.h"
#include "parsetable.h"
namespace SCROLL
{

	// ==================================================================
	// PHASE 2 -- SCROLL's key table. One row per upstream extract*() call,
	// in the same order. See parsetable.h.
	// ==================================================================
	namespace
	{
#define SCROLL_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kScrollKeys[] = {
			{ "filterByScrolls", sp::FieldKind::List, SCROLL_F(objects) },
			{ "filterByScrollsExcluded", sp::FieldKind::List, SCROLL_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, SCROLL_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, SCROLL_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, SCROLL_F(keywordsExcluded) },
			{ "filterByMgefs", sp::FieldKind::List, SCROLL_F(mgefs) },
			{ "filterByMgefsOr", sp::FieldKind::List, SCROLL_F(mgefsOr) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, SCROLL_F(mgefsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, SCROLL_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, SCROLL_F(keywordsToRemove) },
			{ "mgefsToAdd", sp::FieldKind::Arr2D, SCROLL_F(addedObjects) },
			{ "mgefsToChange", sp::FieldKind::Arr2D, SCROLL_F(changedObjects) },
			{ "mgefsToChangeAdd", sp::FieldKind::Arr2D, SCROLL_F(changedAddObjects) },
			{ "mgefsToChangeFirst", sp::FieldKind::Arr2D, SCROLL_F(changedFirstObjects) },
			{ "mgefsToChangeDamageHealthEffects", sp::FieldKind::Arr2D, SCROLL_F(changedDamageHealthObjects) },
			{ "mgefsToRemove", sp::FieldKind::List, SCROLL_F(removedObjects) },
			{ "filterByType", sp::FieldKind::Value, SCROLL_F(filterType) },
			{ "baseCost", sp::FieldKind::Value, SCROLL_F(baseCost) },
			{ "halfCostPerk", sp::FieldKind::Value, SCROLL_F(halfCostPerk) },
			{ "chargeTime", sp::FieldKind::Value, SCROLL_F(chargeTime) },
			{ "castType", sp::FieldKind::Value, SCROLL_F(castType) },
			{ "setFlags", sp::FieldKind::ListAssign, SCROLL_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, SCROLL_F(removeFlags) },
			{ "clear", sp::FieldKind::Value, SCROLL_F(clear) },
			{ "value", sp::FieldKind::Value, SCROLL_F(value) },
			{ "valueMult", sp::FieldKind::Value, SCROLL_F(valueMult) },
			{ "weight", sp::FieldKind::Value, SCROLL_F(weight) },
			{ "weightMult", sp::FieldKind::Value, SCROLL_F(weightMult) },
			{ "filterByModNames", sp::FieldKind::ListAssign, SCROLL_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, SCROLL_F(fullName) },
			{ "model", sp::FieldKind::Value, SCROLL_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, SCROLL_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, SCROLL_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, SCROLL_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, SCROLL_F(alternateTexturesFind) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, SCROLL_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, SCROLL_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, SCROLL_F(filterByEditorIdContainsExcluded) },
			{ "filterByMinimumSkillLevel", sp::FieldKind::ListAssign, SCROLL_F(filterByMinimumSkillLevel) },
			{ "restrictToCastingType", sp::FieldKind::List, SCROLL_F(restrictToCastingType) },
			{ "restrictToSchool", sp::FieldKind::List, SCROLL_F(restrictToSchool) },
		};

#undef SCROLL_F

		const sp::KeyTable& scrollKeyTable()
		{
			static const sp::KeyTable table(kScrollKeys, std::size(kScrollKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		scrollKeyTable().parse(line, &l);
		return l;
	}

	// ==================================================================
	// PHASE 2 -- SCROLL's rule compiler.
	//
	// PARTIAL CONVERSION, deliberately, on the same terms as SPELL's: the
	// keyword pair, the EditorID pair and the catch-all are compiled into a
	// formfilter::MatchProgram; filterByAlternateTextures,
	// filterByMinimumSkillLevel, filterByMgefs{,Or,Excluded},
	// restrictToSchool and restrictToCastingType have no opcode and stay on
	// their own path. All of them are still resolved ONCE PER RULE.
	//
	// SCROLL is a copy of SPELL that has DRIFTED, and the drift is preserved
	// rather than unified:
	//   * the mgef OR and EXCLUDED loops dereference `effect->baseEffect`
	//     with no null check; the AND loop checks it. SPELL checks it in two
	//     of the three.
	//   * restrictToSchool's null guard omits SPELL's `effects.front() &&`.
	// ==================================================================
	namespace
	{
		formfilter::MatchProgram compileScrollRule(const line_content& line)
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

			// DRIFT, PRESERVED: the inert `formID != 0x000007` player guard.
			// See spell.cpp's copy for why this is exactly skipIfPlayer.
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
				line.mgefs.empty() && line.mgefsOr.empty() && line.alternateTexturesFind.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() &&
				line.filterByMinimumSkillLevel.empty();

			return p;
		}

		struct ScrollRuleExtras
		{
			// May contain nullptr, exactly as `currentform->As<BGSTextureSet>()`
			// could: findTextureSet compares pointers and handles it.
			std::vector<RE::BGSTextureSet*> textureSets;
			// Only the entries std::stoi accepted. Upstream caught the
			// invalid_argument per (pair, entry) and logged a warning; the
			// parse now happens once per rule, so the warning fires once per
			// rule instead of 4,000 times. Same call Phase 2 made for NPC's
			// restrictToFlags.
			std::vector<int>                          minimumSkillLevels;
			std::vector<RE::FormID>                   mgefsAnd;  // holes kept
			std::vector<RE::FormID>                   mgefsOr, mgefsExcluded;
			std::vector<RE::ActorValue>               schools;
			std::vector<RE::MagicSystem::CastingType> castingTypes;
			bool                                      mgefsPresent = false;
			bool                                      mgefsOrAbsent = true;
			bool                                      playerGuard = false;
		};

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

		ScrollRuleExtras compileScrollExtras(const line_content& line)
		{
			ScrollRuleExtras x;

			for (const auto& textureString : line.alternateTexturesFind) {
				RE::TESForm* currentform = GetFormFromIdentifier(textureString);
				if (currentform) {
					x.textureSets.push_back(currentform->As<RE::BGSTextureSet>());
				}
			}

			for (const auto& skillLevel : line.filterByMinimumSkillLevel) {
				try {
					x.minimumSkillLevels.push_back(std::stoi(skillLevel));
				} catch (...) {
					logger::warn("ERROR - scroll patcher - filterByMinimumSkillLevel - {}", skillLevel);
				}
			}

			x.mgefsAnd = formfilter::resolveFormIDsKeepingHoles(line.mgefs, RE::FormType::MagicEffect);
			x.mgefsOr = formfilter::resolveFormIDs(line.mgefsOr, RE::FormType::MagicEffect);
			x.mgefsExcluded = formfilter::resolveFormIDs(line.mgefsExcluded, RE::FormType::MagicEffect);
			x.mgefsPresent = !line.mgefs.empty() || !line.mgefsOr.empty();
			x.mgefsOrAbsent = line.mgefsOr.empty();

			x.schools = resolveSchools(line.restrictToSchool);
			x.castingTypes = resolveCastingTypes(line.restrictToCastingType);

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
<<<<<<< HEAD
		auto& objectSpellArray = dataHandler->GetFormArray<RE::ScrollItem>();
=======
		//RE::BSTArray<RE::AlchemyItem*> objectArray = dataHandler->GetFormArray<RE::AlchemyItem>();
		const auto& objectSpellArray = dataHandler->GetFormArray<RE::ScrollItem>();
>>>>>>> origin/new-main
		for (const auto& line : tokens) {
			
			
			if (!line.objects.empty()) {
				for (const auto& npcstring : line.objects) {
					RE::TESForm*       currentform = nullptr;
					RE::ScrollItem* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Scroll) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::ScrollItem*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty()) {
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, scroll) pair.
			// scroll's line_content has no hasPlugin/hasPluginOr fields, so
			// those two view members are bound to formfilter::noStrings().
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			// PHASE 1 + 2: the rule is resolved and compiled ONCE.
			const formfilter::MatchProgram          program = compileScrollRule(line);
			const ScrollRuleExtras                  extra = compileScrollExtras(line);
			const formfilter::ResolvedCommonFilters resolved =
				formfilter::resolve(view, RE::FormType::Scroll);

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

				// keyword pair + EditorID pair + the catch-all.
				bool found = formfilter::runProgram(program, target, scratch);

				// --- filters the opcode table does not cover ----------------
				// Each only ever writes `found = true`, so evaluating them
				// after the program (and only when it did not match) is the
				// same OR upstream computed.
				if (!found && !extra.textureSets.empty()) {
					for (RE::BGSTextureSet* const ts : extra.textureSets) {
						if (findTextureSet(curobj, ts)) {
							found = true;
							break;
						}
					}
				}

				if (!found && !extra.minimumSkillLevels.empty()) {
					for (const int levelAsInt : extra.minimumSkillLevels) {
						if (!curobj->effects.empty() &&
							curobj->effects.front() &&
							curobj->effects.front()->baseEffect &&
							static_cast<int>(curobj->effects.front()->baseEffect->data.minimumSkill) == levelAsInt) {
							found = true;
							break;
						}
					}
				}

				if (!found && extra.mgefsPresent) {
					bool mgefAnd = true;
					if (!extra.mgefsAnd.empty()) {
						// Hole-preserving AND; see spell.cpp's copy.
						bool foundInList = false;
						for (const RE::FormID id : extra.mgefsAnd) {
							foundInList = false;
							if (id) {
								for (const auto& effect : curobj->effects) {
									if (effect && effect->baseEffect && effect->baseEffect->formID == id) {
										foundInList = true;
										break;
									}
								}
								if (!foundInList) {
									break;
								}
							}
						}
						mgefAnd = foundInList;
					}
					bool mgefOr = extra.mgefsOrAbsent;
					if (!mgefOr) {
						for (const RE::FormID id : extra.mgefsOr) {
							for (const auto& effect : curobj->effects) {
								// NOTE: no `effect->baseEffect` null check.
								// Drift from the AND loop above, preserved.
								if (effect && effect->baseEffect->formID == id) {
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

				// --- negative stage, in upstream source order ---------------
				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(resolved, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.mgefsExcluded.empty() && found) {
					for (const RE::FormID id : extra.mgefsExcluded) {
						for (const auto& effect : curobj->effects) {
							// NOTE: no `effect->baseEffect` null check here
							// either. Preserved.
							if (effect && effect->baseEffect->formID == id) {
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

				if (!resolved.editorIdContainsExcluded.empty() && found) {
					const std::string& editorId = loweredEditorId(curobj, scratch);
					for (const auto& needle : resolved.editorIdContainsExcluded) {
						if (editorId.find(needle) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					// DRIFT, PRESERVED: SPELL's copy of this guard also tests
					// `effects.front() &&`. This one does not.
					if (curobj && !curobj->effects.empty() && curobj->effects.front()->baseEffect) {
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

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					// GetCastingType() is a virtual; upstream short-circuited
					// past it for every unrecognised name.
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
				//if (found) {
				//	for (const auto& effect : curobj->listOfEffects) {
				//
				//		logger::debug(FMT_STRING("effect formID: {:08X} name: {}"), effect->effectSetting->formID, effect->effectSetting->fullName);
				//		logger::debug(FMT_STRING("effect magnitude: {}"), effect->data.magnitude);
				//		logger::debug(FMT_STRING("effect duration: {}"), effect->data.duration);
				//		logger::debug(FMT_STRING("effect area: {}"), effect->data.area);
				//	}

				//}
				if (found) {
					patch(line, curobj);
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
	}

<<<<<<< HEAD
	void* patch(const SCROLL::line_content& line, RE::ScrollItem* curobj) {
=======
	void* patch(const SCROLL::line_content& line, RE::ScrollItem* curobj) {
>>>>>>> origin/new-main
		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("scroll formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("scroll formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if ( !line.clear.empty() && line.clear != "none") {
			curobj->effects.clear();
			logger::debug(FMT_STRING("scroll formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
		}

		if ( !line.removedObjects.empty()) {
			for (const auto& objectToRemove : line.removedObjects) {
				std::string        removeFormStr = objectToRemove;
				RE::EffectSetting* removeForm = (RE::EffectSetting*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->effects.begin();
					while (it != curobj->effects.end()) {
						if ((*it) && (*it)->baseEffect && (*it)->baseEffect->formID == removeForm->formID) {
							it = curobj->effects.erase(it);
							logger::debug(FMT_STRING("scroll {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if ( !line.addedObjects.empty()) {
		for (const auto& objectToAdd : line.addedObjects) {
			if (objectToAdd.size() < 4) { logger::warn("scroll {:08X}: invalid addedObjects entry, expected 4 values", curobj->formID); continue; }
				std::string        addFormStr = objectToAdd[0];
				float              magnitude = std::stof(objectToAdd[1]);
				int32_t            duration = std::stoi(objectToAdd[2]);
				int32_t            area = std::stoi(objectToAdd[3]);
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
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
				logger::debug(FMT_STRING("scroll {:08X} {} added effect {:08X} {} magitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
				if (objectToAdd.size() == 5) {
					moveEffectToPosition0(curobj->effects, tempEffectItem);
					logger::debug(FMT_STRING("scroll {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
				}
				}
			}
		}

		if ( !line.changedObjects.empty()) {
		for (auto objectToAdd : line.changedObjects) {
			if (objectToAdd.empty()) { logger::warn("scroll {:08X}: empty changedObjects entry", curobj->formID); continue; }
			objectToAdd.resize(5, "null");
				std::string        addFormStr = objectToAdd[0];
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
				if (addForm) {  // Only proceed if addForm is not null
					float magnitude = 0.0f;
					if (objectToAdd[1] != "null") {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (objectToAdd[2] != "null") {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (objectToAdd[3] != "null") {
						area = std::stoi(objectToAdd[3]);
					}

					float multiplier = 0;
					if (objectToAdd[4] != "null") {
						multiplier = multiplier + std::stof(objectToAdd[4]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (objectToAdd[1] != "null") {
								effect->effectItem.magnitude = magnitude;
								logger::debug(FMT_STRING("scroll {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("scroll {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("scroll {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (objectToAdd[4] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("scroll {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
			}
		}

		if (!line.changedAddObjects.empty()) {
		for (auto objectToAdd : line.changedAddObjects) {
			if (objectToAdd.empty()) { logger::warn("scroll {:08X}: empty changedAddObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				std::string        addFormStr = objectToAdd[0];
				RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
				if (addForm) {  // Only proceed if addForm is not null
					float magnitude = 0.0f;
					if (objectToAdd[1] != "null") {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (objectToAdd[2] != "null") {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (objectToAdd[3] != "null") {
						area = std::stoi(objectToAdd[3]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (objectToAdd[1] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude + magnitude;
								logger::debug(FMT_STRING("scroll {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("scroll {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("scroll {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
			}
		}

		if (!line.changedFirstObjects.empty()) {
		for (auto objectToAdd : line.changedFirstObjects) {
			if (objectToAdd.empty()) { logger::warn("scroll {:08X}: empty changedFirstObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				float magnitude = 0.0f;
				if (objectToAdd[0] != "null") {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (objectToAdd[1] != "null") {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (objectToAdd[2] != "null") {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (objectToAdd[3] != "null") {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				auto& effect = curobj->effects.front();
				if (effect && effect->baseEffect) {
					if (objectToAdd[0] != "null") {
						effect->effectItem.magnitude = magnitude;
						logger::debug(FMT_STRING("scroll {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
					if (objectToAdd[1] != "null") {
						effect->effectItem.duration = duration;
						logger::debug(FMT_STRING("scroll {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
					}
					if (objectToAdd[2] != "null") {
						effect->effectItem.area = area;
						logger::debug(FMT_STRING("scroll {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
					}
					if (objectToAdd[3] != "null") {
						effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
						logger::debug(FMT_STRING("scroll {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
				}
			}
		}

		if (!line.changedDamageHealthObjects.empty()) {
		for (auto objectToAdd : line.changedDamageHealthObjects) {
			if (objectToAdd.empty()) { logger::warn("scroll {:08X}: empty changedDamageHealthObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				float magnitude = 0.0f;
				if (objectToAdd[0] != "null") {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (objectToAdd[1] != "null") {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (objectToAdd[2] != "null") {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (objectToAdd[3] != "null") {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				for (const auto& effect : curobj->effects) {
					if (effect && effect->baseEffect && effect->baseEffect->IsDetrimental() && (effect->baseEffect->data.primaryAV == RE::ActorValue::kHealth || effect->baseEffect->data.secondaryAV == RE::ActorValue::kHealth)) {
						if (objectToAdd[0] != "null") {
							effect->effectItem.magnitude = magnitude;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
						if (objectToAdd[1] != "null") {
							effect->effectItem.duration = duration;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
						}
						if (objectToAdd[2] != "null") {
							effect->effectItem.area = area;
							logger::debug(FMT_STRING("spell {:08X} {} changed damage effect {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
						}
						if (objectToAdd[3] != "null") {
							effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
							logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) damage effect magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
					}
				}
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			curobj->As<RE::SpellItem>()->data.costOverride = std::stoi(line.baseCost);
			logger::debug(FMT_STRING("scroll {:08X} {} changed base cost to {}"), curobj->formID, curobj->fullName, curobj->As<RE::SpellItem>()->data.costOverride);
		}

		if ( !line.halfCostPerk.empty() && line.halfCostPerk != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.halfCostPerk;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Perk) {
					curobj->As<RE::SpellItem>()->data.castingPerk = (RE::BGSPerk*)currentform;
					logger::debug(FMT_STRING("scroll formid: {:08X} {} changed halfCostPerk {:08X}"), curobj->formID, curobj->fullName, curobj->As<RE::SpellItem>()->data.castingPerk->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if ( !line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		//logger::debug("Spell Found True {}",line.chargeTime);
		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			curobj->As<RE::SpellItem>()->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("scroll formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
		}

		if ( !line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				//logger::debug(FMT_STRING("scroll formid: {:08X} {} flag {}"), curobj->formID, curobj->fullName, setFlag);
				if (toLowerCase(setFlag) == "costoverride") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kCostOverride);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "fooditem") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kFoodItem);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "extendduration") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kExtendDuration);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled extendduration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "pcstartspell") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kPCStartSpell);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled pcstartspell"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "instantcast") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kInstantCast);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled instantcast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "ignoreloscheck") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kIgnoreLOSCheck);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled ignoreloscheck"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "ignoreresistance") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kIgnoreResistance);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled ignoreresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "noabsorb") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kNoAbsorb);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled noabsorb"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nodualcastmods") {
					curobj->As<RE::SpellItem>()->data.flags.set(RE::ScrollItem::SpellFlag::kNoDualCastMods);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag enabled nodualcastmods"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if ( !line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "costoverride") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kCostOverride);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fooditem") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kFoodItem);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "extendduration") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kExtendDuration);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled extendduration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "pcstartspell") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kPCStartSpell);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled pcstartspell"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "instantcast") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kInstantCast);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled instantcast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "ignoreloscheck") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kIgnoreLOSCheck);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled ignoreloscheck"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "ignoreresistance") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kIgnoreResistance);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled ignoreresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "noabsorb") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kNoAbsorb);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled noabsorb"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nodualcastmods") {
					curobj->As<RE::SpellItem>()->data.flags.reset(RE::ScrollItem::SpellFlag::kNoDualCastMods);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} flag disabled nodualcastmods"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}


		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("scroll formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("scroll formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("scroll formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (!line.alternateTexturesClear.empty() && line.alternateTexturesClear != "none") {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("scroll formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("scroll formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("scroll formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				}
			}
		}

		if (!line.alternateTextures.empty()) {
			for (const auto& objectToAdd : line.alternateTextures) {
				std::string  texture = objectToAdd[0];
				std::string  name = objectToAdd[1];
				int32_t      index = std::stoi(objectToAdd[2]);
				RE::TESForm* currentform = GetFormFromIdentifier(texture);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					RE::TESModelTextureSwap::AlternateTexture newTex;
					newTex.textureSet = currentform->As<RE::BGSTextureSet>();
					newTex.name3D = name;
					newTex.index3D = index;
					addAltTexture(curobj, newTex);
					logger::debug(FMT_STRING("scroll formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}


	}

}
