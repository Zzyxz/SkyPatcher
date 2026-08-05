#include "enchant.h"
#include "formfilter.h"
#include "parsetable.h"
namespace ENCH
{
	// ==================================================================
	// PHASE 2 -- ENCH's key table. One row per upstream extract*() call,
	// in the same order. See parsetable.h.
	// ==================================================================
	namespace
	{
#define ENCH_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kEnchKeys[] = {
			{ "filterByEnchs", sp::FieldKind::List, ENCH_F(objects) },
			{ "filterByEnchsExcluded", sp::FieldKind::List, ENCH_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, ENCH_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, ENCH_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, ENCH_F(keywordsExcluded) },
			{ "filterByMgefs", sp::FieldKind::List, ENCH_F(mgefs) },
			{ "filterByMgefsOr", sp::FieldKind::List, ENCH_F(mgefsOr) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, ENCH_F(mgefsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, ENCH_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, ENCH_F(keywordsToRemove) },
			{ "mgefsToAdd", sp::FieldKind::Arr2D, ENCH_F(addedObjects) },
			{ "mgefsToChange", sp::FieldKind::Arr2D, ENCH_F(changedObjects) },
			{ "mgefsToChangeAdd", sp::FieldKind::Arr2D, ENCH_F(changedAddObjects) },
			{ "mgefsToChangeFirst", sp::FieldKind::Arr2D, ENCH_F(changedFirstObjects) },
			{ "mgefsToChangeDamageHealthEffects", sp::FieldKind::Arr2D, ENCH_F(changedDamageHealthObjects) },
			{ "mgefsToRemove", sp::FieldKind::List, ENCH_F(removedObjects) },
			{ "filterByType", sp::FieldKind::Value, ENCH_F(filterType) },
			{ "baseCost", sp::FieldKind::Value, ENCH_F(baseCost) },
			{ "enchantmentAmount", sp::FieldKind::Value, ENCH_F(chargeOverride) },
			{ "chargeTime", sp::FieldKind::Value, ENCH_F(chargeTime) },
			{ "castType", sp::FieldKind::Value, ENCH_F(castType) },
			{ "setFlags", sp::FieldKind::ListAssign, ENCH_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, ENCH_F(removeFlags) },
			{ "clear", sp::FieldKind::Value, ENCH_F(clear) },
			{ "filterByModNames", sp::FieldKind::ListAssign, ENCH_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, ENCH_F(fullName) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContainsExcluded) },
			{ "filterByMinimumSkillLevel", sp::FieldKind::ListAssign, ENCH_F(filterByMinimumSkillLevel) },
			{ "restrictToCastingType", sp::FieldKind::List, ENCH_F(restrictToCastingType) },
			{ "restrictToSchool", sp::FieldKind::List, ENCH_F(restrictToSchool) },
			{ "restrictToSpellType", sp::FieldKind::List, ENCH_F(restrictToSpellType) },
		};

#undef ENCH_F

		const sp::KeyTable& enchKeyTable()
		{
			static const sp::KeyTable table(kEnchKeys, std::size(kEnchKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		enchKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		//RE::BSTArray<RE::AlchemyItem*> objectArray = dataHandler->GetFormArray<RE::AlchemyItem>();
		auto& objectSpellArray = dataHandler->GetFormArray<RE::EnchantmentItem>();
		for (const auto& line : tokens) {
			
				if (!line.objects.empty()) {
					for (const auto& npcstring : line.objects) {
						RE::TESForm*       currentform = nullptr;
					RE::EnchantmentItem* curobj = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Enchantment) {
							//logger::debug("Form {:08X} ", currentform->formID);
							curobj = (RE::EnchantmentItem*)currentform;
							patch(line, curobj);
						}
					}
				}

				if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty()) {
					continue;
				}

			// PROTOTYPE: one view per rule, not per (rule, enchantment) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : objectSpellArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;
				bool contains = false;
				bool containsOr = false;

				if ( !curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				// PROTOTYPE: preserved verbatim -- unlike weapon/book/alch,
				// these three conditions are missing a leading `!found &&`.
				// Currently inert (this runs before anything else can set
				// `found`), so behaviour is unchanged; not "fixed" here.
				if (!line.filterByEditorIdContains.empty()) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!line.filterByEditorIdContainsOr.empty()) {
					//logger::info("keywords not empty");
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if ( (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.filterByMinimumSkillLevel.empty()) {
					for (const auto& skillLevel : line.filterByMinimumSkillLevel) {
						try {
							const int levelAsInt = std::stoi(skillLevel);

							if (!curobj->effects.empty() &&
								curobj->effects.front() &&
								curobj->effects.front()->baseEffect &&
								static_cast<int>(curobj->effects.front()->baseEffect->data.minimumSkill) == levelAsInt) {
								found = true;
								break;
							}

						} catch (...) {
							logger::warn("ERROR - scroll patcher - {:08X} - filterByMinimumSkillLevel", curobj->formID);
						}
					}
				}



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
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.mgefs.empty()) {
					bool foundInList = false;
					for (const auto& mgefstring : line.mgefs) {
						foundInList = false;
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
									foundInList = true;
									break;
								}
							}
							if (!foundInList) {
								mgefAnd = false;
								break;
							}
						}
					}
					if (foundInList) {
						mgefAnd = true;
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					mgefAnd = true;
				}
				if (!line.mgefsOr.empty()) {
					for (const auto& mgefstring : line.mgefsOr) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;

							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect->formID == keyword->formID) {
									mgefOr = true;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
						if (mgefOr) {
							break;
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					mgefOr = true;
				}

				if ((!line.mgefs.empty() || !line.mgefsOr.empty()) && mgefAnd && mgefOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by mgefs. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.mgefsExcluded.empty() && found) {
					//logger::info("mgefs not empty");
					for (const auto& mgefstring : line.mgefsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect->formID == keyword->formID) {
									found = false;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					// PROTOTYPE: was RE::FormType::Spell -- objectExcluded on
					// an enchantment patcher could never match, making the
					// filter dead code. Fixed to Enchantment as part of the
					// migration.
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Enchantment)) {
						found = false;
						logger::debug(FMT_STRING("enchantment {:08X} {} excluded."), curobj->formID, curobj->fullName);
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					if (curobj && !curobj->effects.empty() && curobj->effects.front()->baseEffect) {
						const auto skill = curobj->effects.front()->baseEffect->data.associatedSkill;

						for (auto s : line.restrictToSchool) {
							s = toLowerCase(s);

							if (s == "destruction" && skill == RE::ActorValue::kDestruction) {
								foundSchool = true;
								break;
							}
							if (s == "restoration" && skill == RE::ActorValue::kRestoration) {
								foundSchool = true;
								break;
							}
							if (s == "alteration" && skill == RE::ActorValue::kAlteration) {
								foundSchool = true;
								break;
							}
							if (s == "conjuration" && skill == RE::ActorValue::kConjuration) {
								foundSchool = true;
								break;
							}
							if (s == "illusion" && skill == RE::ActorValue::kIllusion) {
								foundSchool = true;
								break;
							}
							if (s == "none" && skill == RE::ActorValue::kNone) {
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
					for (auto string : line.restrictToCastingType) {
						string = toLowerCase(string);
						if (string == "concentration" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
							foundCastingType = true;
						} else if (string == "constanteffect" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect) {
							foundCastingType = true;
						} else if (string == "fireandforget" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kFireAndForget) {
							foundCastingType = true;
						} else if (string == "scroll" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kScroll) {
							foundCastingType = true;
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToSpellType.empty()) {
					bool foundType = false;

					if (curobj) {
						const auto type = curobj->GetSpellType();

						for (auto t : line.restrictToSpellType) {
							t = toLowerCase(t);

							if (t == "spell" && type == RE::MagicSystem::SpellType::kSpell) {
								foundType = true;
								break;
							}
							if (t == "disease" && type == RE::MagicSystem::SpellType::kDisease) {
								foundType = true;
								break;
							}
							if (t == "power" && type == RE::MagicSystem::SpellType::kPower) {
								foundType = true;
								break;
							}
							if ((t == "lesserpower" || t == "lesser_power") &&
								type == RE::MagicSystem::SpellType::kLesserPower) {
								foundType = true;
								break;
							}
							if (t == "ability" && type == RE::MagicSystem::SpellType::kAbility) {
								foundType = true;
								break;
							}
							if (t == "poison" && type == RE::MagicSystem::SpellType::kPoison) {
								foundType = true;
								break;
							}
							if (t == "enchantment" && type == RE::MagicSystem::SpellType::kEnchantment) {
								foundType = true;
								break;
							}
							if ((t == "potion" || t == "alchemy") &&
								type == RE::MagicSystem::SpellType::kPotion) {
								foundType = true;
								break;
							}
							if ((t == "wordcraft" || t == "wortcraft") &&
								type == RE::MagicSystem::SpellType::kWortCraft) {
								foundType = true;
								break;
							}
							if (t == "ingredient" &&
								type == RE::MagicSystem::SpellType::kIngredient) {
								foundType = true;
								break;
							}
							if ((t == "leveledspell" || t == "leveled_spell") &&
								type == RE::MagicSystem::SpellType::kLeveledSpell) {
								foundType = true;
								break;
							}
							if (t == "addiction" &&
								type == RE::MagicSystem::SpellType::kAddiction) {
								foundType = true;
								break;
							}
							if ((t == "voicepower" || t == "voice_power" || t == "shout") &&
								type == RE::MagicSystem::SpellType::kVoicePower) {
								foundType = true;
								break;
							}
							if ((t == "staffenchantment" || t == "staff_enchantment" || t == "staff") &&
								type == RE::MagicSystem::SpellType::kStaffEnchantment) {
								foundType = true;
								break;
							}
							if (t == "scroll" &&
								type == RE::MagicSystem::SpellType::kScroll) {
								foundType = true;
								break;
							}
						}

						if (!foundType) {
							found = false;
						}
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

	void patch(const ENCH::line_content& line, RE::EnchantmentItem* curobj) {
		

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("enchantment formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if ( !line.clear.empty() && line.clear != "none") {
			curobj->effects.clear();
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
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
							logger::debug(FMT_STRING("enchantment {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if ( !line.addedObjects.empty()) {
			for (const auto& objectToAdd : line.addedObjects) {
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
					tempEffectItem->cost = addForm->data.baseCost * pow(magnitude * durationForCalc*0.1, 1.1);
					
					curobj->effects.push_back(tempEffectItem);
					logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} magitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
					if (objectToAdd.size() == 5) {
						moveEffectToPosition0(curobj->effects, tempEffectItem);
						logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
					}
				}
			}
		}

		if ( !line.changedObjects.empty()) {
			for (const auto& objectToAdd : line.changedObjects) {
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
								logger::debug(FMT_STRING("enchantment {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (objectToAdd[4] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
			}
		}

		if (!line.changedAddObjects.empty()) {
			for (const auto& objectToAdd : line.changedAddObjects) {
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
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
			}
		}

		if (!line.changedFirstObjects.empty()) {
			for (const auto& objectToAdd : line.changedFirstObjects) {
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
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
					if (objectToAdd[1] != "null") {
						effect->effectItem.duration = duration;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
					}
					if (objectToAdd[2] != "null") {
						effect->effectItem.area = area;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
					}
					if (objectToAdd[3] != "null") {
						effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
				}
			}
		}

		if (!line.changedDamageHealthObjects.empty()) {
			for (const auto& objectToAdd : line.changedDamageHealthObjects) {
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

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			try {
				curobj->data.costOverride = std::stoi(line.baseCost);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed baseCost to {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.chargeOverride.empty() && line.chargeOverride != "none") {
			try {
				curobj->data.chargeOverride = std::stoi(line.chargeOverride);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed enchantmentAmount to {}"), curobj->formID, curobj->fullName, curobj->data.chargeOverride);
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		//logger::debug("Spell Found True {}",line.chargeTime);
		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			curobj->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
		}

		if ( !line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				//logger::debug(FMT_STRING("spell formid: {:08X} {} flag {}"), curobj->formID, curobj->fullName, setFlag);
				if (toLowerCase(setFlag) == "costoverride") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "fooditem") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "extendduration") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled extendduration"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "costoverride") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fooditem") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "extendduration") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled extendduration"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

	}


}
