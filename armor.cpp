#include "armor.h"
#include "utility.h"

namespace ARMOR
{

	struct patch_instruction_armor create_patch_instruction_armor(const std::string& line)
	{
		patch_instruction_armor l;

		extractData(line, "filterByArmors\\s*=([^:]+)", l.object);

		extractData(line, "filterByArmorsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractValueString(line, "mirrorArmor\\s*=([^:]+)", l.mirrorArmor);
		extractValueString(line, "damageResist\\s*=([^:]+)", l.damageResist);
		extractValueString(line, "damageResistMatch\\s*=([^:]+)", l.damageResistMatch);

		extractValueString(line, "damageResistMultiply\\s*=([^:]+)", l.damageResistMult);

		extractValueString(line, "health\\s*=([^:]+)", l.health);

		extractValueString(line, "objectEffect\\s*=([^:]+)", l.objectEffect);

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractDataStrings(line, "filterByBipedSlots\\s*=([^:]+)", l.bipedSlot);

		extractDataStrings(line, "filterByBipedSlotsOr\\s*=([^:]+)", l.bipedSlotOr);

		extractDataStrings(line, "filterByBipedSlotsExcluded\\s*=([^:]+)", l.bipedSlotExcluded);

		extractDataStrings(line, "bipedSlotsToAdd\\s*=([^:]+)", l.setBipedSlot);

		extractDataStrings(line, "bipedSlotsToRemove\\s*=([^:]+)", l.removeBipedSlot);

		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);

		extractValueString(line, "modelFemale\\s*=([^:]+)", l.modelFemale);
		extractValueString(line, "modelMale\\s*=([^:]+)", l.modelMale);
		extractValueString(line, "value\\s*=([^:]+)", l.value);
		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);
		extractValueString(line, "armorType\\s*=([^:]+)", l.armorType);
		

		extractValueString(line, "pickUpSound\\s*=([^:]+)", l.pickUpSound);
		extractValueString(line, "putDownSound\\s*=([^:]+)", l.putDownSound);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractDataStrings(line, "filterByArmorTypes\\s*=([^:]+)", l.filterByArmorType);
		extractDataStrings(line, "filterByArmorTypesExcluded\\s*=([^:]+)", l.filterByArmorTypeExcluded);
		extractValueString(line, "altBlockMaterialType\\s*=([^:]+)", l.altBlockMaterialType);
		extractValueString(line, "blockBashImpactDataSet\\s*=([^:]+)", l.blockBashImpactDataSet);
		extractValueString(line, "equipSlot\\s*=([^:]+)", l.equipSlot);
		extractValueString(line, "enchantAmount\\s*=([^:]+)", l.enchantAmount);

		extractDataStrings(line, "filterByNameContains\\s*=([^:]+)", l.filterByNameContains);
		extractDataStrings(line, "filterByNameContainsOr\\s*=([^:]+)", l.filterByNameContainsOr);
		extractDataStrings(line, "filterByNameContainsExcluded\\s*=([^:]+)", l.filterByNameContainsExcluded);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractToArr2D(line, "reCalcArmorRating\\s*=([^:]+)", l.reCalcArmorRating);
		extractToArr2D(line, "reCalcArmorRatingv2\\s*=([^:]+)", l.reCalcArmorRatingv2);
		
		
		extractDataStrings(line, "filterByArmorAddons\\s*=([^:]+)", l.filterByArmorAddons);
		extractDataStrings(line, "filterByArmorAddonsOr\\s*=([^:]+)", l.filterByArmorAddonsOr);
		extractDataStrings(line, "filterByArmorAddonsExcluded\\s*=([^:]+)", l.filterByArmorAddonsExcluded);
		
		extractData(line, "armorAddonsToAdd\\s*=([^:]+)", l.armorAddonToAdd);
		extractData(line, "armorAddonsToRemove\\s*=([^:]+)", l.armorAddonToRemove);
		extractValueString(line, "clearArmorAddons\\s*=([^:]+)", l.clearArmorAddons);
		
		extractValueString(line, "templateArmor\\s*=([^:]+)", l.templateArmor);
		
		extractValueString(line, "minX\\s*=([^:]+)", l.minX);
		extractValueString(line, "minY\\s*=([^:]+)", l.minY);
		extractValueString(line, "minZ\\s*=([^:]+)", l.minZ);
		extractValueString(line, "maxX\\s*=([^:]+)", l.maxX);
		extractValueString(line, "maxY\\s*=([^:]+)", l.maxY);
		extractValueString(line, "maxZ\\s*=([^:]+)", l.maxZ);

		extractValueString(line, "dwMatch\\s*=([^:]+)", l.dwMatch);
		extractValueString(line, "modelMatch\\s*=([^:]+)", l.modelMatch);

		extractDataStrings(line, "hasPlugins\\s*=([^:]+)", l.hasPlugin);
		extractDataStrings(line, "hasPluginsOr\\s*=([^:]+)", l.hasPluginOr);

		extractToArr2D(line, "armaRaceToAdd\\s*=([^:]+)", l.armaRaceToAdd);

		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);

		extractData(line, "restrictToKeywords\\s*=([^:]+)", l.restrictKeywords);
		extractData(line, "restrictToBipedSlots\\s*=([^:]+)", l.restrictSlots);

		return l;
	}

	void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectARMO*> ArmorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectARMO* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Armor) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESObjectARMO*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.bipedSlot.empty() && line.bipedSlotOr.empty() && line.filterByArmorType.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByNameContains.empty() && line.filterByNameContainsOr.empty() && line.filterByArmorAddons.empty() && line.filterByArmorAddonsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ArmorArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool bipedSlotAnd = false;
				bool bipedSlotOr = false;
				bool contains = false;
				bool containsOr = false;
				bool namecontains = false;
				bool namecontainsOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;

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

				if (!line.hasPlugin.empty()) {
					bool isPluginMissing = false;
					for (auto const modName : line.hasPlugin) {
						if (!IsPluginInstalled(modName.c_str())) {
							isPluginMissing = true;
						}
					}
					if (isPluginMissing) {
						continue;
					}
				}

				if (!line.hasPluginOr.empty()) {
					bool modFound = false;
					for (auto const modName : line.hasPluginOr) {
						if (IsPluginInstalled(modName.c_str())) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
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
				} else {
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
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				//++++++++++++++++++++++++++++//
				if (!found && !line.filterByNameContains.empty()) {
					for (const auto& editorString : line.filterByNameContains) {
						if (toLowerCase(curobj->fullName.c_str()).find(toLowerCase(editorString)) != std::string::npos) {
							namecontains = true;
						} else {
							namecontains = false;
							break;
						}
					}
				} else {
					namecontains = true;
				}

				if (!found && !line.filterByNameContainsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByNameContainsOr) {
						if (toLowerCase(curobj->fullName.c_str()).find(toLowerCase(editorString)) != std::string::npos) {
							namecontainsOr = true;
							break;
						}
					}
				} else {
					namecontainsOr = true;
				}

				if (!found && (!line.filterByNameContains.empty() || !line.filterByNameContainsOr.empty()) && namecontains && namecontainsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}
				//++++++++++++++++++++++++++++//

				if (!line.filterByArmorType.empty()) {
					//logger::info("npc not empty");
					for (const auto& armorstring : line.filterByArmorType) {
						if (toLowerCase(armorstring) == "light" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kLightArmor) {
							found = true;
							//logger::info("NPC found.");
							break;
						} else if (toLowerCase(armorstring) == "heavy" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kHeavyArmor) {
							found = true;
							//logger::info("NPC found.");
							break;
						} else if (toLowerCase(armorstring) == "clothing" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kClothing) {
							found = true;
							//logger::info("NPC found.");
							break;
						}
					}
				}

				if (!line.bipedSlot.empty()) {
					//logger::info("keywords not empty");
					for (const auto& bipedSlot : line.bipedSlot) {
						int  iSlot = std::stoi(bipedSlot);
						auto slot = getBipedObjectSlot(iSlot);
						if (curobj->bipedModelData.bipedObjectSlots & slot) {
							bipedSlotAnd = true;
						} else {
							bipedSlotAnd = false;
							break;
						}
					}

				} else {
					bipedSlotAnd = true;
				}

				if (!line.bipedSlotOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& bipedSlot : line.bipedSlotOr) {
						int iSlot = std::stoi(bipedSlot);

						auto slot = getBipedObjectSlot(iSlot);
						if (curobj->bipedModelData.bipedObjectSlots & slot) {
							bipedSlotOr = true;
							break;
						}
					}
					if (found) {
						logger::debug(FMT_STRING("Armor has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
					}
				} else {
					bipedSlotOr = true;
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
								//logger::debug(FMT_STRING("KeywordAnd armor does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd armor true"));
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
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X}"), curobj->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.bipedSlot.empty() || !line.bipedSlotOr.empty()) && bipedSlotAnd && bipedSlotOr) {
					//logger::debug(FMT_STRING("Found a matching armor by bipedSlots. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching armor by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.filterByArmorAddons.empty()) {
					bool foundInList = false;
					for (const auto& mgefstring : line.filterByArmorAddons) {
						foundInList = false;
						//logger::debug(FMT_STRING("mgefAnd set foundInList False."));
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectARMA* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Armature) {
							keyword = (RE::TESObjectARMA*)currentform;
							for (const auto& addon : curobj->armorAddons) {
								if (addon->formID == keyword->formID) {
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
				if (!line.filterByArmorAddonsOr.empty()) {
					for (const auto& mgefstring : line.filterByArmorAddonsOr) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectARMA* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Armature ) {
							keyword = (RE::TESObjectARMA*)currentform;

							for (const auto& addon : curobj->armorAddons) {
								if (addon->formID == keyword->formID) {
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

				if ((!line.filterByArmorAddons.empty() || !line.filterByArmorAddonsOr.empty()) && mgefAnd && mgefOr) {
					//logger::debug(FMT_STRING("mgef found. {} {}"), mgefAnd, mgefOr);
					found = true;
				}

				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.bipedSlot.empty() && line.bipedSlotOr.empty() && line.filterByArmorType.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByNameContains.empty() && line.filterByNameContainsOr.empty() && line.filterByArmorAddons.empty() && line.filterByArmorAddonsOr.empty()) {
					//logger::debug(FMT_STRING("Found a matching armor by no filter. {:08X} {}"), curobj->formID, curobj->fullName);
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
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.bipedSlotExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& bipedSlot : line.bipedSlot) {
						int  iSlot = std::stoi(bipedSlot);
						auto slot = getBipedObjectSlot(iSlot);
						if (curobj->bipedModelData.bipedObjectSlots & slot) {
							found = false;
							logger::debug(FMT_STRING("Armor Excluded has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
							break;
						}
						//logger::debug(FMT_STRING("KeywordAnd armor true"));
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

				if (!line.filterByNameContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					for (const auto& editorString : line.filterByNameContainsExcluded) {
						if (toLowerCase(curobj->fullName.c_str()).find(toLowerCase(editorString)) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (!line.filterByArmorAddonsExcluded.empty() && found) {
					//logger::info("mgefs not empty");
					for (const auto& mgefstring : line.filterByArmorAddonsExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectARMA* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Armature) {
							keyword = (RE::TESObjectARMA*)currentform;
							for (const auto& addon : curobj->armorAddons) {
								if (addon->formID == keyword->formID) {
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
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectARMO* npc = nullptr;
						std::string        string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Armor) {
							npc = (RE::TESObjectARMO*)currentform;
							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (!line.filterByArmorTypeExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& armorstring : line.filterByArmorTypeExcluded) {
						if (toLowerCase(armorstring) == "light" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kLightArmor) {
							found = false;
							//logger::info("NPC found.");
							break;
						}
						else if (toLowerCase(armorstring) == "heavy" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kHeavyArmor) {
							found = false;
							//logger::info("NPC found.");
							break;
						}
						else if (toLowerCase(armorstring) == "clothing" && curobj->bipedModelData.armorType & RE::BIPED_MODEL::ArmorType::kClothing) {
							found = false;
							//logger::info("NPC found.");
							break;
						}
					}
				}

				if (found && !line.restrictKeywords.empty()) {
					bool hasObject = false;
					for (const auto& restrictObject : line.restrictKeywords) {
						RE::TESForm*        currentform = nullptr;
						RE::BGSKeyword*		tempForm = nullptr;
						std::string         string_form = restrictObject;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							tempForm = (RE::BGSKeyword*)currentform;
							if (curobj->HasKeyword(tempForm)) {
								hasObject = true;
								break;
							}
						}
					}
					if (!hasObject) {
						found = false;
					}
				}

				if (found && !line.restrictSlots.empty()) {
					bool hasObject = false;
					for (const auto& restrictObject : line.restrictSlots) {
						int  iSlot = std::stoi(restrictObject);
						auto slot = getBipedObjectSlot(iSlot);
						if (curobj->bipedModelData.bipedObjectSlots & slot) {
							hasObject = true;
							break;
						}
					}
					if (!hasObject) {
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
								std::string                        line;
								std::ifstream                      infile;
								std::list<patch_instruction_armor> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									tokens.push_back(create_patch_instruction_armor(line));
								}
								infile.close();
								process_patch_instructions_armor(tokens);
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

	void* patch(ARMOR::patch_instruction_armor line, RE::TESObjectARMO* curobj)
	{

		if (!line.mirrorArmor.empty() && line.mirrorArmor != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.mirrorArmor;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					if(templateArmor){
						curobj->boundData = templateArmor->boundData;
						curobj->bipedModelData = templateArmor->bipedModelData;
						curobj->armorAddons = templateArmor->armorAddons;
						curobj->constraintTemplate = templateArmor->constraintTemplate;
						curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
						curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
						curobj->ReplaceModel();
						curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
						curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
						curobj->race = templateArmor->race;
						curobj->pickupSound = templateArmor->pickupSound;
						curobj->putdownSound = templateArmor->putdownSound;
						curobj->equipSlot = templateArmor->equipSlot;
						curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
						curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
						logger::debug(FMT_STRING("armor formid: {:08X} {} mirrored armor from {:08X}"), curobj->formID, curobj->fullName, templateArmor->formID);
					}
					else {
						logger::debug(FMT_STRING("armor formid: {:08X} {} could not mirror from {}"), curobj->formID, curobj->fullName, string_form);
					}
				}
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.reCalcArmorRating.empty()) {
			for (const auto& objectToAdd : line.reCalcArmorRating) {
				float startRating = std::stof(objectToAdd[0]);
				float multiplierBase = std::stof(objectToAdd[1]);
				float multiplierSoft = std::stof(objectToAdd[2]);
				float multiplierHard = std::stof(objectToAdd[3]);
				float softCap = std::stof(objectToAdd[4]);
				float hardCap = std::stof(objectToAdd[5]);
				float gapSpreader = std::stof(objectToAdd[6]);
				float multiplier = 1;
				float armorRating = 1;
				if (curobj->armorRating > 0 && (curobj->armorRating / 100) > startRating) {
					armorRating = curobj->armorRating / 100;
					armorRating = gapSpreader + armorRating;
					if (armorRating < softCap) {
						multiplier = multiplierBase;
					} else if (armorRating >= softCap && armorRating < hardCap) {
						multiplier = multiplierSoft;
					} else if (armorRating >= hardCap) {
						multiplier = multiplierHard;
					}
					uint32_t newArmor = (armorRating + (-(armorRating - startRating)) + pow(1 + armorRating - startRating, multiplier)) * 100;
					logger::debug(FMT_STRING("armor formid: {:08X} {} recalc armorRating Before {} After {} Multiplier used {}"), curobj->formID, curobj->fullName, armorRating, (newArmor / 100), multiplier);
					curobj->armorRating = newArmor;
				}
			}
		}

		if (!line.reCalcArmorRatingv2.empty()) {
			for (const auto& p : line.reCalcArmorRatingv2) {
				// v2 Parameter-Layout (7 Werte, damit dein Parser unverändert bleiben kann):
				// [0] startRating
				// [1] maxGain
				// [2] k
				// [3] minGain (optional, meist 0)
				// [4] outSoftCap (optional, 0 = aus)
				// [5] outHardCap (optional, 0 = aus)
				// [6] gapSpreader

				if (p.size() < 7) {
					continue;
				}

				float startRating = std::stof(p[0]);
				float maxGain = std::stof(p[1]);
				float k = std::stof(p[2]);
				float minGain = std::stof(p[3]);
				float outSoftCap = std::stof(p[4]);
				float outHardCap = std::stof(p[5]);
				float gapSpreader = std::stof(p[6]);

				float base = curobj->armorRating / 100.0f;
				if (base <= 0.0f || base <= startRating) {
					continue;
				}

				float ar = base + gapSpreader;

				float x = ar - startRating;
				if (x < 0.0f) {
					x = 0.0f;
				}

				// Sättigung: gain -> nähert sich maxGain an
				// gain = minGain + (maxGain - minGain) * (1 - exp(-k * x))
				float gain = minGain + (maxGain - minGain) * (1.0f - std::exp(-k * x));
				float out = startRating + gain;

				// Optionaler Output Softcap, falls du zusätzlich noch härter bremsen willst
				if (outSoftCap > 0.0f && outHardCap > outSoftCap && out > outSoftCap) {
					float over = out - outSoftCap;
					float span = outHardCap - outSoftCap;

					// nochmal Sättigung auf den Output
					out = outSoftCap + span * (1.0f - std::exp(-over / 0.8f));
				}

				uint32_t newArmor = static_cast<uint32_t>(out * 100.0f);

				logger::debug(
					FMT_STRING("armor formid: {:08X} {} recalc v2 Before {} After {} (x={}, gain={})"),
					curobj->formID, curobj->fullName, base, (newArmor / 100.0f), x, gain);

				curobj->armorRating = newArmor;
			}
		}


		if (!line.damageResist.empty() && line.damageResist != "none") {
			try {
				curobj->armorRating = stoi(line.damageResist) * 100;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating / 100);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageResistMult.empty() && line.damageResistMult != "none") {
			try {
				curobj->armorRating = (uint32_t)(curobj->armorRating * stof(line.damageResistMult));
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed(multiplied) armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating / 100);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.damageResistMatch.empty() && line.damageResistMatch != "none") {
			try {
				RE::TESForm*    currentform = nullptr;
				RE::TESObjectARMO* object = nullptr;
				currentform = GetFormFromIdentifier(line.damageResistMatch);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					object = (RE::TESObjectARMO*)currentform;
					if (object) {
						curobj->armorRating = object->armorRating;
						logger::debug(FMT_STRING("armor formid: {:08X} {} match armor rating of armor {:08X} {}"), curobj->formID, curobj->fullName, object->formID, object->fullName);
					}
				} else {
					logger::debug(FMT_STRING("armor formid: {:08X} {} object is not an armor {}"), curobj->formID, curobj->fullName, line.damageResistMatch);
				}
				
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.objectEffect.empty() && line.objectEffect != "none") {
			RE::TESForm*         currentform = nullptr;
			RE::EnchantmentItem* obj = nullptr;
			std::string          string_form = line.objectEffect;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Enchantment) {
				obj = (RE::EnchantmentItem*)currentform;
				curobj->formEnchanting = obj;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed object effect to {:08X}"), curobj->formID, curobj->fullName, obj->formID);
			} else if (toLowerCase(line.objectEffect) == "null") {
				curobj->formEnchanting = nullptr;
				logger::debug(FMT_STRING("armor formid: {:08X} {} removed object effect"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("armor formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("armor formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.setBipedSlot.empty()) {
			for (const auto& slotString : line.setBipedSlot) {
				int slot = std::stoi(slotString);
				curobj->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
				logger::debug(FMT_STRING("armor added bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
				for (const auto& arma : curobj->armorAddons) {
					arma->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
					logger::debug(FMT_STRING("Added bipedSlot to {:08X} Slot {}"), arma->formID, slotString);
				}
			}
		}

		if (!line.removeBipedSlot.empty()) {
			for (const auto& slotString : line.removeBipedSlot) {
				int slot = std::stoi(slotString);
				curobj->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
				logger::debug(FMT_STRING("armor removed bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
				for (const auto& arma : curobj->armorAddons) {
					arma->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
					logger::debug(FMT_STRING("Removed bipedSlot from {:08X} Slot {}"), arma->formID, slotString);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.armorType.empty() && line.armorType != "none") {
			if (toLowerCase(line.armorType) == "lightarmor") {
				curobj->bipedModelData.armorType = RE::BIPED_MODEL::ArmorType::kLightArmor;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armorType to LightArmor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.armorType) == "heavyarmor") {
				curobj->bipedModelData.armorType = RE::BIPED_MODEL::ArmorType::kHeavyArmor;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armorType to HeavyArmor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.armorType) == "clothing") {
				curobj->bipedModelData.armorType = RE::BIPED_MODEL::ArmorType::kClothing;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armorType to Clothing"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.pickUpSound.empty() && line.pickUpSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.pickUpSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->pickupSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, curobj->pickupSound->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.putDownSound.empty() && line.putDownSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.putDownSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->putdownSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, curobj->putdownSound->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.modelFemale.empty() && line.modelFemale != "none") {
			try {
				curobj->worldModels[RE::SEXES::kFemale].SetModel(line.modelFemale.c_str());
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed female model path {}"), curobj->formID, curobj->fullName, curobj->worldModels[RE::SEXES::kFemale].model.c_str());
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.modelMale.empty() && line.modelMale != "none") {
			try {
				curobj->worldModels[RE::SEXES::kMale].SetModel(line.modelMale.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed male model path {}"), curobj->formID, curobj->fullName, curobj->worldModels[RE::SEXES::kMale].model.c_str());
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
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed altBlockMaterialType {:08X}"), curobj->formID, curobj->fullName, curobj->altBlockMaterialType->formID);
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
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed blockBashImpactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->blockBashImpactDataSet->formID);
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
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed equipSlot {:08X}"), curobj->formID, curobj->fullName, curobj->equipSlot->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.enchantAmount.empty() && line.enchantAmount != "none") {
			try {
				curobj->amountofEnchantment = std::stoi(line.enchantAmount);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed enchantAmount {}"), curobj->formID, curobj->fullName, curobj->amountofEnchantment);
			} catch (const std::invalid_argument& e) {
			}
		}



		if (!line.clearArmorAddons.empty() && line.clearArmorAddons != "none") {
			curobj->armorAddons.clear();
			logger::debug(FMT_STRING("armor formid: {:08X} {} cleared all armor addons"), curobj->formID, curobj->fullName);
		}

		if (!line.armorAddonToRemove.empty()) {
			for (const auto& objectToRemove : line.armorAddonToRemove) {
				std::string        removeFormStr = objectToRemove;
				RE::TESObjectARMA* removeForm = (RE::TESObjectARMA*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->armorAddons.begin();
					while (it != curobj->armorAddons.end()) {
						//logger::debug(FMT_STRING("spell {:08X} {}, effect {:08X} cost {} {}"), curobj->formID, curobj->fullName, (*it)->baseEffect->formID, (*it)->cost, curobj->CalculateMagickaCost(RE::PlayerCharacter::GetSingleton()));

						if ((*it) && (*it)->formID == removeForm->formID) {
							it = curobj->armorAddons.erase(it);
							logger::debug(FMT_STRING("armor {:08X} {}, {:08X} armor addon removed, armor addons left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->armorAddons.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if (!line.armorAddonToAdd.empty()) {
			for (size_t i = 0; i < line.armorAddonToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.armorAddonToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armature) {
					curobj->armorAddons.push_back((RE::TESObjectARMA*)currentform);
					logger::debug(FMT_STRING("armor formid: {:08X} added armor addon {:08X} "), curobj->formID, (currentform)->formID);
				}
			}
		}

		if (!line.templateArmor.empty() && line.templateArmor != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.templateArmor;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->templateArmor = templateArmor;
					curobj->boundData = templateArmor->boundData;
					curobj->bipedModelData = templateArmor->bipedModelData;
					curobj->armorAddons = templateArmor->armorAddons;
					curobj->constraintTemplate = templateArmor->constraintTemplate;
					curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
					curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
					curobj->ReplaceModel();
					curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
					curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
					curobj->race = templateArmor->race;
					curobj->pickupSound = templateArmor->pickupSound;
					curobj->putdownSound = templateArmor->putdownSound;
					curobj->equipSlot = templateArmor->equipSlot;
					curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
					curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
					curobj->keywords = templateArmor->keywords;
					curobj->numKeywords = templateArmor->numKeywords;
					curobj->armorRating = templateArmor->armorRating;
					curobj->weight = templateArmor->weight;
					curobj->descriptionText = templateArmor->descriptionText;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed templateArmor {:08X}"), curobj->formID, curobj->fullName, curobj->templateArmor->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.dwMatch.empty() && line.dwMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.dwMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->armorRating = templateArmor->armorRating;
					curobj->weight = templateArmor->weight;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed damage resist and weight to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.modelMatch.empty() && line.modelMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.modelMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->boundData = templateArmor->boundData;
					//curobj->bipedModelData = templateArmor->bipedModelData;
					curobj->bipedModelData.bipedObjectSlots = templateArmor->bipedModelData.bipedObjectSlots;
					curobj->armorAddons = templateArmor->armorAddons;
					//curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
					//curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
					if (templateArmor->worldModels[RE::SEXES::kMale].GetModel()) {
						//curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
						curobj->worldModels[RE::SEXES::kMale].SetModel(templateArmor->worldModels[RE::SEXES::kMale].GetModel());
					} else {
						curobj->worldModels[RE::SEXES::kMale].ClearDataComponent();
					}
					if (templateArmor->worldModels[RE::SEXES::kFemale].GetModel()) {
						//curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
						curobj->worldModels[RE::SEXES::kFemale].SetModel(templateArmor->worldModels[RE::SEXES::kFemale].GetModel());
					} else {
						curobj->worldModels[RE::SEXES::kFemale].ClearDataComponent();
					}
					//curobj->worldModels[RE::SEXES::kTotal] = templateArmor->worldModels[RE::SEXES::kTotal];
					curobj->ReplaceModel();
					curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
					curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
					curobj->race = templateArmor->race;
					curobj->pickupSound = templateArmor->pickupSound;
					curobj->putdownSound = templateArmor->putdownSound;
					curobj->equipSlot = templateArmor->equipSlot;
					curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
					curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed model to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		 if (!line.minX.empty() && line.minX != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minX);
				curobj->boundData.boundMin.x = std::stoi(line.minX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minY.empty() && line.minY != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minY);
				curobj->boundData.boundMin.y = std::stoi(line.minY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minZ.empty() && line.minZ != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minZ);
				curobj->boundData.boundMin.z = std::stoi(line.minZ);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxX.empty() && line.maxX != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxX);
				curobj->boundData.boundMax.x = std::stoi(line.maxX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxY.empty() && line.maxY != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxY);
				curobj->boundData.boundMax.y = std::stoi(line.maxY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxZ.empty() && line.maxZ != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxZ);
				curobj->boundData.boundMax.z = std::stoi(line.maxZ);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.armaRaceToAdd.empty()) {
			for (const auto& objectToAdd : line.armaRaceToAdd) {
				std::string  armaToAddTo = objectToAdd[0];  // Armature identifier
				RE::TESForm* currentform = GetFormFromIdentifier(armaToAddTo);

				if (currentform && currentform->formType == RE::FormType::Armature) {
					// Find the correct armorAddon by matching formID
					auto it = std::find_if(curobj->armorAddons.begin(), curobj->armorAddons.end(),
						[&](const RE::TESObjectARMA* addon) {
							return addon->formID == currentform->formID;
						});

					if (it != curobj->armorAddons.end()) {
						// Dereference the iterator to access the found armorAddon
						RE::TESObjectARMA* armorAddon = *it;

						// Loop through all races starting from index 1
						for (size_t i = 1; i < objectToAdd.size(); ++i) {
							std::string  raceToAddStr = objectToAdd[i];
							RE::TESForm* raceToAdd = GetFormFromIdentifier(raceToAddStr);

							if (raceToAdd && raceToAdd->As<RE::TESRace>()) {
								// Check if the race already exists in the armorAddon->races
								bool raceExists = std::any_of(armorAddon->additionalRaces.begin(), armorAddon->additionalRaces.end(),
									[&](RE::TESForm* existingRace) {
										return existingRace->formID == raceToAdd->formID;
									});

								if (!raceExists) {
									// Add the race if it doesn't already exist
									armorAddon->additionalRaces.push_back(raceToAdd->As<RE::TESRace>());

									logger::debug(FMT_STRING("armor formid: {:08X} {} Added race with formID: {:08X} to armorAddon with formID: {:08X}"),
										curobj->formID, curobj->fullName, raceToAdd->formID, armorAddon->formID);
								} else {
									logger::debug(FMT_STRING("armor formid: {:08X} {} Race with formID: {:08X} already exists in armorAddon with formID: {:08X}"),
										curobj->formID, curobj->fullName, raceToAdd->formID, armorAddon->formID);
								}
							}
						}
					} else {
						logger::warn(FMT_STRING("armor formid: {:08X} {} No matching armorAddon found for formID: {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} Armature form not found for identifier: {}"), curobj->formID, curobj->fullName, armaToAddTo);
				}
			}
		}

		if (!line.setFlags.empty()) {
			bool changedFlags = false;

			for (const auto& setFlagRaw : line.setFlags) {
				const auto setFlag = toLowerCase(setFlagRaw);

				if (setFlag == "nonplayable") {
					curobj->formFlags |= RE::TESObjectARMO::RecordFlags::kNonPlayable;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag enabled nonplayable"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (setFlag == "deleted") {
					curobj->formFlags |= RE::TESObjectARMO::RecordFlags::kDeleted;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag enabled deleted"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (setFlag == "shield") {
					curobj->formFlags |= RE::TESObjectARMO::RecordFlags::kShield;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag enabled shield"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (setFlag == "ignored") {
					curobj->formFlags |= RE::TESObjectARMO::RecordFlags::kIgnored;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag enabled ignored"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} unknown flag '{}' in setFlags"), curobj->formID, curobj->fullName, setFlagRaw);
				}
			}

			// Wenn mindestens ein Flag gesetzt wurde, Änderung an den Flags markieren,
			// damit es ins Save übernommen wird.
			if (changedFlags) {
				curobj->AddChange(RE::TESForm::ChangeFlags::kFlags);
			}
		}


		if (!line.removeFlags.empty()) {
			bool changedFlags = false;

			for (const auto& removeFlagRaw : line.removeFlags) {
				const auto removeFlag = toLowerCase(removeFlagRaw);

				if (removeFlag == "nonplayable") {
					curobj->formFlags &= ~RE::TESObjectARMO::RecordFlags::kNonPlayable;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag disabled nonplayable"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (removeFlag == "deleted") {
					curobj->formFlags &= ~RE::TESObjectARMO::RecordFlags::kDeleted;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag disabled deleted"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (removeFlag == "shield") {
					curobj->formFlags &= ~RE::TESObjectARMO::RecordFlags::kShield;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag disabled shield"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else if (removeFlag == "ignored") {
					curobj->formFlags &= ~RE::TESObjectARMO::RecordFlags::kIgnored;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag disabled ignored"), curobj->formID, curobj->fullName);
					changedFlags = true;

				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} unknown flag '{}' in removeFlags"),
						curobj->formID, curobj->fullName, removeFlagRaw);
				}
			}

			if (changedFlags) {
				curobj->AddChange(RE::TESForm::ChangeFlags::kFlags);
			}
		}



		

	}

}
