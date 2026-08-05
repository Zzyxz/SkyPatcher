#include "spell.h"
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
	
	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;


		extractData(line, "filterBySpells\\s*=([^:]+)", l.objects);
		extractData(line, "filterByFlags\\s*=([^:]+)", l.flagFilters);
		extractData(line, "filterByFlagsOr\\s*=([^:]+)", l.flagFiltersOr);
		extractData(line, "filterByFlagsExcluded\\s*=([^:]+)", l.flagFiltersExcluded);
		extractData(line, "filterByCastingPerk\\s*=([^:]+)", l.filterByCastingPerk);
		extractData(line, "filterBySpellsExcluded\\s*=([^:]+)", l.objectExcluded);
		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);
		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);
		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractData(line, "filterByMgefs\\s*=([^:]+)", l.mgefs);
		extractData(line, "filterByMgefsOr\\s*=([^:]+)", l.mgefsOr);
		extractData(line, "filterByMgefsExcluded\\s*=([^:]+)", l.mgefsExcluded);
		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractToArr2D(line, "mgefsToAdd\\s*=([^:]+)", l.addedObjects);
		extractToArr2D(line, "mgefsToChange\\s*=([^:]+)", l.changedObjects);
		extractToArr2D(line, "mgefsToChangeAdd\\s*=([^:]+)", l.changedAddObjects);
		extractToArr2D(line, "mgefsToChangeFirst\\s*=([^:]+)", l.changedFirstObjects);
		extractToArr2D(line, "mgefsToChangeDamageHealthEffects\\s*=([^:]+)", l.changedDamageHealthObjects);
		extractData(line, "mgefsToRemove\\s*=([^:]+)", l.removedObjects);

		extractValueString(line, "filterByType\\s*=([^:]+)", l.filterType);
		extractValueString(line, "baseCost\\s*=([^:]+)", l.baseCost);
		extractValueString(line, "halfCostPerk\\s*=([^:]+)", l.halfCostPerk);
		extractValueString(line, "chargeTime\\s*=([^:]+)", l.chargeTime);
		extractValueString(line, "castType\\s*=([^:]+)", l.castType);
		extractValueString(line, "equipSlot\\s*=([^:]+)", l.equipSlot);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractValueString(line, "clear\\s*=([^:]+)", l.clear);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractData(line, "restrictToCastingType\\s*=([^:]+)", l.restrictToCastingType);
		extractData(line, "restrictToSchool\\s*=([^:]+)", l.restrictToSchool);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		//extractData(line, "filterByHalfCostPerk\\s*=([^:]+)", l.filterByHalfCostPerk);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		//RE::BSTArray<RE::AlchemyItem*> objectArray = dataHandler->GetFormArray<RE::AlchemyItem>();
		auto& objectSpellArray = dataHandler->GetFormArray<RE::SpellItem>();
		for (const auto& line : tokens) {


			if (!line.objects.empty()) {
				for (const auto& npcstring : line.objects) {
					RE::TESForm*         currentform = nullptr;
					RE::SpellItem* curobj = nullptr;
					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Spell) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::SpellItem*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByCastingPerk.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}


			for (const auto& curobj : objectSpellArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;
				bool flagAnd = true;
				bool flagOr = false;
				bool contains = false;
				bool containsOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				if (!line.filterByCastingPerk.empty()) {
					for (const auto& npcstring : line.filterByCastingPerk) {
						RE::TESForm*   currentform = nullptr;
						std::string    string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Perk && curobj->data.castingPerk && curobj->data.castingPerk->formID == currentform->formID) {
							//logger::debug("Form {:08X} ", currentform->formID);
							found = true;
							break;
						}
					}
				}

				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd Weapon does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd Weapon true"));
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && !line.filterByEditorIdContains.empty()) {
					for (const auto& editorString : line.filterByEditorIdContains) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							contains = true;
						} else {
							contains = false;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByEditorIdContainsOr) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							containsOr = true;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.flagFilters.empty()) {
					for (const auto& flagString : line.flagFilters) {
						std::string key = toLowerCase(flagString);
						auto it = flagMap.find(key);
						if (it != flagMap.end()) {
							if (curobj->data.flags.all(it->second)) {
								flagAnd = true;
							}
							else {
								flagAnd = false;
								//logger::debug("FlagAnd Spell does not have all required flags: {}", key);
								break;
							}
						}
						else {
							logger::debug("Unknown flag in AND filter: {}", key);
						}
					}
				}
				else {
					flagAnd = true;  // No flags to check means we pass
				}

				// OR filter: at least one flag must be present
				if (!line.flagFiltersOr.empty()) {
					for (const auto& flagString : line.flagFiltersOr) {
						std::string key = toLowerCase(flagString);
						auto it = flagMap.find(key);
						if (it != flagMap.end()) {
							if (curobj->data.flags.all(it->second)) {
								flagOr = true;
								break;
							}
						}
						else {
							logger::debug("Unknown flag in OR filter: {}", key);
						}
					}
				}
				else {
					flagOr = true;  // Pass if no OR filters defined
				}

				// Final match check
				if ((!line.flagFilters.empty() || !line.flagFiltersOr.empty()) && flagAnd && flagOr) {
					//logger::debug(FMT_STRING("Spell {:08X} {} passed flag filter"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.mgefs.empty()) {
					bool foundInList = false;
					for (const auto& mgefstring : line.mgefs) {
						foundInList = false;
						//logger::debug(FMT_STRING("mgefAnd set foundInList False."));
						RE::TESForm*       currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
								
								if (effect && effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
									foundInList = true;
									//logger::debug(FMT_STRING("mgefAnd found in list."));
									break;
								}
							}
							if (!foundInList) {
								mgefAnd = false;
								//logger::debug(FMT_STRING("mgefAnd not found in list, break."));
								break;
							}
						}
					}
					if (foundInList) {
						mgefAnd = true;
					}
				} else {
					//logger::debug(FMT_STRING("mgefAnd is empty, we pass true."));
					mgefAnd = true;
				}
				if (!line.mgefsOr.empty()) {
					for (const auto& mgefstring : line.mgefsOr) {
						RE::TESForm*       currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;

							for (const auto& effect : curobj->effects) {
								if (effect && effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
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
					//logger::debug(FMT_STRING("mgef found. {} {}"), mgefAnd, mgefOr);
					found = true;
				}

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByCastingPerk.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"),keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (!line.flagFiltersExcluded.empty() && found) {
					for (const auto& flagString : line.flagFiltersExcluded) {
						std::string key = toLowerCase(flagString);
						auto it = flagMap.find(key);
						if (it != flagMap.end()) {
							if (curobj->data.flags.all(it->second)) {
								found = false;
								logger::debug(FMT_STRING("Spell {:08X} {} has excluded flag '{}', skipping"), curobj->formID, curobj->fullName, key);
								break;
							}
						}
						else {
							logger::debug("Unknown flag in excluded filter: {}", key);
						}
					}
				}


				if (!line.mgefsExcluded.empty() && found) {
					//logger::info("mgefs not empty");
					for (const auto& mgefstring : line.mgefsExcluded) {
						RE::TESForm*       currentform = nullptr;
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
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*   currentform = nullptr;
						RE::SpellItem* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Spell) {
							npc = (RE::SpellItem*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::debug(FMT_STRING("spell {:08X} {} excluded."), curobj->formID, curobj->fullName);
								break;
							}
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

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					if (curobj && !curobj->effects.empty() && curobj->effects.front() && curobj->effects.front()->baseEffect) {
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

	

	void patch(const SPELL::line_content& line, RE::SpellItem* curobj) {
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

		if ( !line.clear.empty() && line.clear != "none") {
			curobj->effects.clear();
			logger::debug(FMT_STRING("spell formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
		}

		if ( !line.removedObjects.empty()) {
			for (const auto& objectToRemove : line.removedObjects) {
				std::string        removeFormStr = objectToRemove;
				RE::EffectSetting* removeForm = (RE::EffectSetting*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->effects.begin();
					while (it != curobj->effects.end()) {
						//logger::debug(FMT_STRING("spell {:08X} {}, effect {:08X} cost {} {}"), curobj->formID, curobj->fullName, (*it)->baseEffect->formID, (*it)->cost, curobj->CalculateMagickaCost(RE::PlayerCharacter::GetSingleton()));
						
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
				logger::debug(FMT_STRING("spell {:08X} {} added effect {:08X} {} magitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
				if (objectToAdd.size() == 5) {
					moveEffectToPosition0(curobj->effects, tempEffectItem);
					logger::debug(FMT_STRING("spell {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
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
								logger::debug(FMT_STRING("spell {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("spell {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("spell {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (objectToAdd[4] != "null") {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
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
								logger::debug(FMT_STRING("spell {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (objectToAdd[2] != "null") {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("spell {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (objectToAdd[3] != "null") {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("spell {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
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
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
						if (objectToAdd[1] != "null") {
							effect->effectItem.duration = duration;
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
						}
						if (objectToAdd[2] != "null") {
							effect->effectItem.area = area;
							logger::debug(FMT_STRING("spell {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
						}
						if (objectToAdd[3] != "null") {
							effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
							logger::debug(FMT_STRING("spell {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
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

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("spell formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			curobj->data.costOverride = std::stoi(line.baseCost);
			logger::debug(FMT_STRING("spell {:08X} {} changed base cost to {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
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
			} catch (const std::invalid_argument& e) {
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
			if (currentform && currentform->formType == RE::FormType::EquipSlot)
				curobj->SetEquipSlot((RE::BGSEquipSlot*)currentform);
			logger::debug(FMT_STRING("spell formid: {:08X} {} changed equipslot to {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
		}


		//logger::debug("Spell Found True {}",line.chargeTime);
		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			curobj->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("spell formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
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
	


}

}
