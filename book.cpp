#include "book.h"
#include <utility.h>

namespace BOOK
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		extractData(line, "filterByBooks\\s*=([^:]+)", l.object);

		extractData(line, "filterByBooksExcluded\\s*=([^:]+)", l.objectExcluded);
		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);
		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);
		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractData(line, "filterBySpellKeywordsOr\\s*=([^:]+)", l.filterBySpellKeywordsOr);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractDataStrings(line, "filterByFlags\\s*=([^:]+)", l.filterByFlags);
		extractValueString(line, "value\\s*=([^:]+)", l.value);
		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);
		extractValueString(line, "teachSpell\\s*=([^:]+)", l.teachSpell);
		extractValueString(line, "teachSkill\\s*=([^:]+)", l.teachSkill);
		extractValueString(line, "type\\s*=([^:]+)", l.type);
		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractValueString(line, "pickUpSound\\s*=([^:]+)", l.pickUpSound);
		extractValueString(line, "putDownSound\\s*=([^:]+)", l.putDownSound);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		
		extractValueString(line, "inventoryArt\\s*=([^:]+)", l.inventoryArt);
		extractValueString(line, "model\\s*=([^:]+)", l.model);
		extractToArr2D(line, "alternateTexturesToAdd\\s*=([^:]+)", l.alternateTextures);
		extractData(line, "alternateTexturesToRemove\\s*=([^:]+)", l.alternateTexturesRemove);
		extractValueString(line, "alternateTexturesClear\\s*=([^:]+)", l.alternateTexturesClear);
		extractData(line, "filterByAlternateTextures\\s*=([^:]+)", l.alternateTexturesFind);

		extractValueString(line, "minX\\s*=([^:]+)", l.minX);
		extractValueString(line, "minY\\s*=([^:]+)", l.minY);
		extractValueString(line, "minZ\\s*=([^:]+)", l.minZ);
		extractValueString(line, "maxX\\s*=([^:]+)", l.maxX);
		extractValueString(line, "maxY\\s*=([^:]+)", l.maxY);
		extractValueString(line, "maxZ\\s*=([^:]+)", l.maxZ);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractData(line, "filterByCastingPerk\\s*=([^:]+)", l.filterByCastingPerk);

		//extractValueString(line, "speed\\s *= ([^:] +)", l.speed);

		//extractValueString(line, "speed\\s*=([^:]+)", l.speed);
		//extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectBOOK*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectBOOK>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectBOOK* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Book) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESObjectBOOK*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByFlags.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterBySpellKeywordsOr.empty() && line.filterByCastingPerk.empty()) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
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
						RE::TESForm* currentform = nullptr;
						std::string  string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Book && curobj->TeachesSpell() && curobj->data.teaches.spell && curobj->data.teaches.spell->data.castingPerk && curobj->data.teaches.spell->data.castingPerk->formID == currentform->formID) {
							//logger::debug("Form {:08X} ", currentform->formID);
							found = true;
							break;
						}
					}
				}

				if (!found && !line.filterByEditorIdContains.empty() ) {
					for (const auto& editorString : line.filterByEditorIdContains) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							contains = true;
						} else {
							contains = false;
							break;
						}
					}
				} else  {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByEditorIdContainsOr) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							containsOr = true;
							break;
						}
					}
				} else  {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.alternateTexturesFind.empty()) {
					for (const auto& textureString : line.alternateTexturesFind) {
						RE::TESForm*       currentform = nullptr;
						currentform = GetFormFromIdentifier(textureString);
						if (currentform) {
							//logger::debug("start search");
							if (findTextureSet(curobj, currentform->As<RE::BGSTextureSet>())) {
								//logger::debug("Texture found");
								found = true;
								break;
							}
						}
					}
				}

				if (!line.filterBySpellKeywordsOr.empty()) {
					for (const auto& textureString : line.filterBySpellKeywordsOr) {
						bool foundInList = false;
						RE::TESForm* currentform = nullptr;
						currentform = GetFormFromIdentifier(textureString);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							auto spell = curobj->data.teaches.spell;
							auto keyword = (RE::BGSKeyword*)currentform;
							//logger::info("spellformid {}", spell->formID);
							if (curobj->TeachesSpell() && spell && keyword) {
								for (const auto& effect : spell->effects)
								{
									if (effect->baseEffect->HasKeyword(keyword)) {
										found = true;
										//logger::debug(FMT_STRING("mgefAnd found in list."));
										break;
									}
								}
							}
						}

						if (found) {
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


				if (!line.filterByFlags.empty()) {
					for (const auto& flag : line.filterByFlags) {
						if (toLowerCase(flag) == "advancesactorvalue" && curobj->data.flags & RE::OBJ_BOOK::Flag::kAdvancesActorValue) {
							found = true;
						} else if (toLowerCase(flag) == "canttake" && curobj->data.flags & RE::OBJ_BOOK::Flag::kCantTake) {
							found = true;
						} else if (toLowerCase(flag) == "teachesspell" && curobj->data.flags & RE::OBJ_BOOK::Flag::kTeachesSpell) {
							found = true;
						} else if (toLowerCase(flag) == "hasbeenread" && curobj->data.flags & RE::OBJ_BOOK::Flag::kHasBeenRead) {
							found = true;
						}
					}
				}



				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByFlags.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterBySpellKeywordsOr.empty() && line.filterByCastingPerk.empty()) {
					found = true;
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectWEAP* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Projectile) {
							npc = (RE::TESObjectWEAP*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								break;
							}
						}
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found && curobj->formID != 0x000007) {
					//logger::info("factions not empty");
					for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							found = false;
							break;
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

	void* patch(BOOK::patch_instruction line, RE::TESObjectBOOK* curobj)
	{

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("book formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("book formid: {:08X} {} changed value {:d}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("book formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("book formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
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
						logger::debug(FMT_STRING("book formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("book formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.inventoryArt.empty() && line.inventoryArt != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.inventoryArt;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Static) {
					curobj->inventoryModel = (RE::TESObjectSTAT*)currentform;
					logger::debug(FMT_STRING("book formid: {:08X} {} changed inventory art {:08X}"), curobj->formID, curobj->fullName, curobj->inventoryModel->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		// Set Flags
		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				if (toLowerCase(setFlag) == "advancesactorvalue") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled AdvancesActorValue"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "canttake") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kCantTake);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled CantTake"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "teachesspell") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kTeachesSpell);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled TeachesSpell"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "hasbeenread") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kHasBeenRead);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled HasBeenRead"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		// Reset Flags
		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "advancesactorvalue") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled AdvancesActorValue"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "canttake") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kCantTake);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled CantTake"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "teachesspell") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled TeachesSpell"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hasbeenread") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kHasBeenRead);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled HasBeenRead"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (line.type != "none") {
			if (toLowerCase(line.type) == "booktome") {
				curobj->data.type = RE::OBJ_BOOK::Type::kBookTome;
				logger::debug(FMT_STRING("book formid: {:08X} {} set type to book/tome"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "notescroll") {
				curobj->data.type = RE::OBJ_BOOK::Type::kNoteScroll;
				logger::debug(FMT_STRING("book formid: {:08X} {} set type to note/scroll"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.pickUpSound.empty() && line.pickUpSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.pickUpSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->pickupSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("book formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, curobj->pickupSound->formID);
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
					logger::debug(FMT_STRING("book formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, curobj->putdownSound->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (line.teachSpell != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.teachSpell;
				if (toLowerCase(string_form) != "null") {
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Spell) {
						curobj->data.teaches.spell = (RE::SpellItem*)currentform;
						logger::debug(FMT_STRING("book formid: {:08X} {} changed spell teached {:08X}"), curobj->formID, curobj->fullName, curobj->data.teaches.spell->formID);
					}
				} else if (toLowerCase(string_form) == "null") {
					curobj->data.teaches.spell = nullptr;
					logger::debug(FMT_STRING("book formid: {:08X} {} changed spell teached to null"), curobj->formID, curobj->fullName);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (line.teachSkill != "none") {
			if (toLowerCase(line.teachSkill) == "alchemy") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kAlchemy;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to alchemy"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "alteration") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kAlteration;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to alteration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "archery") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kArchery;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to archery"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "block") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kBlock;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to block"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "conjuration") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kConjuration;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to conjuration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "destruction") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kDestruction;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to destruction"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "enchanting") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kEnchanting;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to enchanting"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "heavyarmor") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kHeavyArmor;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to heavy armor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "illusion") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kIllusion;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to illusion"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "lightarmor") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kLightArmor;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to light armor"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "lockpicking") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kLockpicking;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to lockpicking"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "null") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kNone;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to none"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "onehanded") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kOneHanded;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to one-handed"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "pickpocket") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kPickpocket;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to pickpocket"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "restoration") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kRestoration;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to restoration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "smithing") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kSmithing;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to smithing"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "sneak") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kSneak;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to sneak"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "speech") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kSpeech;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to speech"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.teachSkill) == "twohanded") {
				curobj->data.teaches.actorValueToAdvance = RE::ActorValue::kTwoHanded;
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to two-handed"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("book formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.alternateTexturesClear.empty() && line.alternateTexturesClear != "none") {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("book formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("book formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("book formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				}
			}
		}

		if (!line.alternateTextures.empty()) {
			for (const auto& objectToAdd : line.alternateTextures) {
				std::string texture = objectToAdd[0];
				std::string name = objectToAdd[1];
				int32_t     index = std::stoi(objectToAdd[2]);
				RE::TESForm* currentform = GetFormFromIdentifier(texture);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					RE::TESModelTextureSwap::AlternateTexture newTex;
					newTex.textureSet = currentform->As<RE::BGSTextureSet>();
					newTex.name3D = name;
					newTex.index3D = index;
					addAltTexture(curobj, newTex);
					logger::debug(FMT_STRING("book formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minX.empty() && line.minX != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minX);
				curobj->boundData.boundMin.x = std::stoi(line.minX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minY.empty() && line.minY != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minY);
				curobj->boundData.boundMin.y = std::stoi(line.minY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.minZ.empty() && line.minZ != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minZ);
				curobj->boundData.boundMin.z = std::stoi(line.minZ);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxX.empty() && line.maxX != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxX);
				curobj->boundData.boundMax.x = std::stoi(line.maxX);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxY.empty() && line.maxY != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxY);
				curobj->boundData.boundMax.y = std::stoi(line.maxY);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.maxZ.empty() && line.maxZ != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxZ);
				curobj->boundData.boundMax.z = std::stoi(line.maxZ);
			} catch (const std::invalid_argument& e) {
			}
		}





	}

}
