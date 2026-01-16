#include "weapon.h"
#include <utility.h>

namespace WEAPON
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;


		extractData(line, "filterByWeapons\\s*=([^:]+)", l.object);

		extractDataStrings(line, "filterBySkills\\s*=([^:]+)", l.embeddedAVs);

		extractDataStrings(line, "restrictToFlags\\s*=([^:]+)", l.filterByWFlags);
		extractDataStrings(line, "restrictToSkills\\s*=([^:]+)", l.restrictToSkill);

		extractData(line, "filterByWeaponsExcluded\\s*=([^:]+)", l.objectExcluded);


		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);


		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);


		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		extractValueString(line, "attackDamage\\s*=([^:]+)", l.attackDamage);
		extractValueString(line, "attackDamageToAdd\\s*=([^:]+)", l.attackDamageToAdd);


		extractValueString(line, "critDamageToAdd\\s*=([^:]+)", l.critDamageToAdd);
		extractValueString(line, "critDamageSetToBase\\s*=([^:]+)", l.critDamageSetToBase);
		extractValueString(line, "critDamageMult\\s*=([^:]+)", l.critDamageMult);
		extractValueString(line, "critPercentMult\\s*=([^:]+)", l.critPercentMult);
		extractValueString(line, "critDamage\\s*=([^:]+)", l.critDamage);

		//extractValueString(line, "speed\\s *= ([^:] +)", l.speed);
		extractValueString(line, "model\\s*=([^:]+)", l.model);
		extractValueString(line, "mirrorWeapon\\s*=([^:]+)", l.mirrorWeapon);
		extractValueString(line, "speed\\s*=([^:]+)", l.speed);
		extractValueString(line, "speedMult\\s*=([^:]+)", l.speedMult);
		extractValueString(line, "reach\\s*=([^:]+)", l.reach);
		extractValueString(line, "rangeMin\\s*=([^:]+)", l.rangeMin);
		extractValueString(line, "rangeMax\\s*=([^:]+)", l.rangeMax);
		extractValueString(line, "stagger\\s*=([^:]+)", l.stagger);

		extractValueString(line, "impactDataSet\\s*=([^:]+)", l.impactDataSet);
		extractValueString(line, "equipSound\\s*=([^:]+)", l.equipSound);
		extractValueString(line, "unequipSound\\s*=([^:]+)", l.unequipSound);
		extractValueString(line, "attackFailSound\\s*=([^:]+)", l.failSound);
		extractValueString(line, "soundLevel\\s*=([^:]+)", l.soundlevel);
		

		extractValueString(line, "attackDamageMult\\s*=([^:]+)", l.attackDamageMult);
		extractValueString(line, "attackDamageMatch\\s*=([^:]+)", l.attackDamageMatch);
		extractValueString(line, "modelFirstPerson\\s*=([^:]+)", l.modelFirstPerson);
		extractValueString(line, "value\\s*=([^:]+)", l.value);
		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);
		extractValueString(line, "objectEffect\\s*=([^:]+)", l.objectEffect);
		extractValueString(line, "templateWeapon\\s*=([^:]+)", l.templateWeapon);
		extractValueString(line, "altBlockMaterialType\\s*=([^:]+)", l.altBlockMaterialType);
		extractValueString(line, "blockBashImpactDataSet\\s*=([^:]+)", l.blockBashImpactDataSet);
		extractValueString(line, "equipSlot\\s*=([^:]+)", l.equipSlot);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractValueString(line, "enchantAmount\\s*=([^:]+)", l.enchantAmount);
		extractDataStrings(line, "filterByAnimationTypeOr\\s*=([^:]+)", l.filterAnimationTypeOr);
		extractDataStrings(line, "animationType\\s*=([^:]+)", l.setAnimationType);
		extractDataStrings(line, "skillType\\s*=([^:]+)", l.setSkillType);

		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);

		extractValueString(line, "minX\\s*=([^:]+)", l.minX);
		extractValueString(line, "minY\\s*=([^:]+)", l.minY);
		extractValueString(line, "minZ\\s*=([^:]+)", l.minZ);
		extractValueString(line, "maxX\\s*=([^:]+)", l.maxX);
		extractValueString(line, "maxY\\s*=([^:]+)", l.maxY);
		extractValueString(line, "maxZ\\s*=([^:]+)", l.maxZ);

		extractValueString(line, "acswMatch\\s*=([^:]+)", l.acswMatch);

		extractDataStrings(line, "hasPlugins\\s*=([^:]+)", l.hasPlugin);
		extractDataStrings(line, "hasPluginsOr\\s*=([^:]+)", l.hasPluginOr);

		extractData(line, "filterByFirstPersonModelOr\\s*=([^:]+)", l.filterByFirstPersonModelOr);
		extractValueString(line, "dwMatch\\s*=([^:]+)", l.dwMatch);

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectWEAP*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();

		for (const auto& line : tokens) {
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


			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool contains = false;
				bool containsOr = false;
				//logger::info(FMT_STRING("Its a form! {:08X}"), curobj->formID);
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

				

				if (!line.filterAnimationTypeOr.empty()) {
					for (const auto& setFlag : line.filterAnimationTypeOr) {
						if (toLowerCase(setFlag) == "handtohandmelee" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kHandToHandMelee) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "onehandsword" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kOneHandSword) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "onehanddagger" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kOneHandDagger) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "onehandaxe" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kOneHandAxe) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "onehandmace" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kOneHandMace) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "twohandsword" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kTwoHandSword) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "twohandaxe" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kTwoHandAxe) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "bow" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kBow) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "staff" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kStaff) {
							found = true;
							break;
						} else if (toLowerCase(setFlag) == "crossbow" && curobj->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow) {
							found = true;
							break;
						} else {
							// Handle the case where an invalid flag is provided
							//logger::warn(FMT_STRING("Invalid set flag: {}"), setFlag);
						}
					}
				}

				//filterByFirstPersonModelOr
				for (const auto& keywordstring : line.filterByFirstPersonModelOr) {
					RE::TESForm*    currentform = nullptr;
					RE::TESObjectSTAT* fpmodel = nullptr;

					std::string string_form = keywordstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Static) {
						fpmodel = (RE::TESObjectSTAT*)currentform;

						if (curobj->firstPersonModelObject && curobj->firstPersonModelObject->formID == fpmodel->formID) {
							found = true;
							//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
							//logger::info("Keyword found.");
							break;
						}
					}
				}

				if (!line.embeddedAVs.empty()) {
					//logger::debug("skill {}", curobj->weaponData.skill.underlying());
					for (auto string : line.embeddedAVs) {
						string = toLowerCase(string);
						if (string == "onehanded" && curobj->weaponData.skill == RE::ActorValue::kOneHanded) {
							found = true;
							break;
						} else if (string == "twohanded" && curobj->weaponData.skill == RE::ActorValue::kTwoHanded) {
							found = true;
							break;
						} else if (string == "marksman" && curobj->weaponData.skill == RE::ActorValue::kArchery) {
							found = true;
							break;
						} else if (string == "destruction" && curobj->weaponData.skill == RE::ActorValue::kDestruction) {
							found = true;
							break;
						} else if (string == "illusion" && curobj->weaponData.skill == RE::ActorValue::kIllusion) {
							found = true;
							break;
						} else if (string == "conjuration" && curobj->weaponData.skill == RE::ActorValue::kConjuration) {
							found = true;
							break;
						} else if (string == "alteration" && curobj->weaponData.skill == RE::ActorValue::kAlteration) {
							found = true;
							break;
						} else if (string == "restoration" && curobj->weaponData.skill == RE::ActorValue::kRestoration) {
							found = true;
							break;
						}
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



				if (line.object.empty() && line.keywords.empty() && line.filterByFirstPersonModelOr.empty() && line.keywordsOr.empty() && line.embeddedAVs.empty() && line.filterAnimationTypeOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found ) {
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

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectWEAP* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Weapon) {
							npc = (RE::TESObjectWEAP*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::info("Weapon Excluded.");
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

				if (found && !line.filterByWFlags.empty()) {
					//logger::debug("skill {}", curobj->weaponData.skill.underlying());
					for (auto string : line.filterByWFlags) {
						string = toLowerCase(string);
						if (string == "boundweapon" && curobj->weaponData.flags2 & RE::TESObjectWEAP::Data::Flag2::kBoundWeapon) {
							break;
						} else {
							found = false;
						}
					}
				}

				if (found && !line.restrictToSkill.empty()) {
					bool foundType = false;
					for (auto string : line.restrictToSkill) {
						string = toLowerCase(string);
						if (curobj->weaponData.skill == StringToActorValue(string)) {
							foundType = true;
						}
					}
					if (!foundType)
						found = false;
				}



				//if (!line.filterByFlagsExclude.empty()) {
				//	for (const auto& flag : line.filterByFlagsExclude) {
				//		//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude"), curobj->formID, curobj->fullName);
				//		if (toLowerCase(flag) == "notplayable" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kNotPlayable) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude nonplayable set to false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "cantdrop" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCantDrop) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "embeddedweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kEmbeddedWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingattack" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingAttack) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "criteffectondeath" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCritEffectOnDeath) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "boltaction" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kBoltAction) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "automatic" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kAutomatic) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "secondaryweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kSecondaryWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingreload" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingReload) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "holdinputtopower" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kHoldInputToPower) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		}
				//	}
				//}

				//		const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				//if (!vm) {
				//	return false;
				//}

				//const auto policy = vm->GetObjectHandlePolicy();
				//if (!policy) {
				//	return false;
				//}

				//const auto handle = policy->GetHandleForObject(GetFormType(), this);
				//return handle != policy->EmptyHandle();

				
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

	void* patch(WEAPON::patch_instruction line, RE::TESObjectWEAP* curobj)
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
				logger::debug(FMT_STRING("weapon formid: {:08X} {} added damage {} total "), curobj->formID, curobj->fullName, line.attackDamageToAdd, curobj->attackDamage);
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
				} else if (hittypeLower == "veryload") {
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
				if (currentform && currentform->formType == RE::FormType::Armor) {
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
