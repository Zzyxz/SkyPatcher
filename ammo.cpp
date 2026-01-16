 #include "ammo.h"
namespace AMMO
{

	struct line_content create_patch_instruction_ammo(const std::string& line)
	{
		line_content l;

		extractData(line, "filterByAmmos\\s*=([^:]+)", l.ammo);
		extractData(line, "filterByAmmosExcluded\\s*=([^:]+)", l.objectExcluded);

		extractValueString(line, "filterByWeightLessThan\\s*=([^:]+)", l.weightLessThan);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		extractValueString(line, "weight\\s*=([^:]+)", l.weight);

		extractValueString(line, "attackDamage\\s*=([^:]+)", l.damage);
		extractValueString(line, "attackDamageToAdd\\s*=([^:]+)", l.damageToAdd);

		extractValueString(line, "attackDamageMult\\s*=([^:]+)", l.damageMult);


		extractValueString(line, "setNewProjectile\\s*=([^:]+)", l.projectile);

		extractData(line, "addToFormList\\s*=([^:]+)", l.formList);

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		//extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractValueString(line, "value\\s*=([^:]+)", l.value);
		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);

		extractValueString(line, "model\\s*=([^:]+)", l.model);
		extractToArr2D(line, "alternateTexturesToAdd\\s*=([^:]+)", l.alternateTextures);
		extractData(line, "alternateTexturesToRemove\\s*=([^:]+)", l.alternateTexturesRemove);
		extractValueString(line, "alternateTexturesClear\\s*=([^:]+)", l.alternateTexturesClear);
		extractData(line, "filterByAlternateTextures\\s*=([^:]+)", l.alternateTexturesFind);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractValueString(line, "range\\s*=([^:]+)", l.range);
		extractValueString(line, "rangeMult\\s*=([^:]+)", l.rangeMult);
		extractValueString(line, "speed\\s*=([^:]+)", l.speed);
		extractValueString(line, "speedMult\\s*=([^:]+)", l.speedMult);
		extractValueString(line, "gravity\\s*=([^:]+)", l.gravity);
		extractValueString(line, "gravityMult\\s*=([^:]+)", l.gravityMult);
		extractValueString(line, "restrictToBolts\\s*=([^:]+)", l.restrictToBolts);
		extractDataStrings(line, "hasPlugins\\s*=([^:]+)", l.hasPlugin);
		extractDataStrings(line, "hasPluginsOr\\s*=([^:]+)", l.hasPluginOr);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);

		return l;
	}

	void process_patch_instructions_ammo(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESAmmo*> AmmoArray = dataHandler->GetFormArray<RE::TESAmmo>();
		for (const auto& line : tokens) {

			if (!line.ammo.empty()) {
				//logger::info("npc not empty");
				for (const auto& ammostring : line.ammo) {
					RE::TESForm* currentform = nullptr;
					RE::TESAmmo* ammo = nullptr;

					std::string string_form = ammostring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Ammo) {
						ammo = (RE::TESAmmo*)currentform;

						patch(line, ammo);
					}
				}
			}

			if (!line.ammo.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.weightLessThan == "none" && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}


			for (const auto& curobj : AmmoArray) {
				//logger::debug("Mod: {} || Weapon Type: {} || Damage: {} || Projectile: {} || addToFormList: {}", line.ammo.c_str(), line.type.c_str(), line.damage.c_str(), line.projectile.c_str(), line.armorPenetration.c_str());
				RE::BSTArray<RE::TESForm*> listPistol;
				RE::BSTArray<RE::TESForm*> listRifle;
				bool found = false;
				//logger::info("Bad");

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
				
				if (!line.alternateTexturesFind.empty()) {
					for (const auto& textureString : line.alternateTexturesFind) {
						RE::TESForm* currentform = nullptr;
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
				#ifndef SKYRIMVR
				if (!line.weightLessThan.empty() && line.weightLessThan != "none") {
					if (curobj->weight < stof(line.weightLessThan)) {
						found = true;
					}
				}
				#endif
				//logger::debug(FMT_STRING("{} {} {} {}"), line.ammo.size(), line.weightLessThan.size(), line.keywords.size(), line.keywordsOr.size());
				if (line.ammo.empty() && line.weightLessThan == "none" && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					
					//logger::debug(FMT_STRING("all Empty"));
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

						//proj

				if (!line.restrictToBolts.empty() && line.restrictToBolts != "none" && toLowerCase(line.restrictToBolts) == "true" && !curobj->IsBolt()) {
					try {
						found = false;
						//logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} is not a bolt."), curobj->formID, curobj->fullName);
					} catch (const std::invalid_argument& e) {
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

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESAmmo* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Ammo) {
							npc = (RE::TESAmmo*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("Projectile Excluded.");
								break;
							}
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

									tokens.push_back(create_patch_instruction_ammo(line));
								}
								infile.close();
								process_patch_instructions_ammo(tokens);
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

	void* patch(AMMO::line_content line, RE::TESAmmo* curobj) {
		
		if (!line.damage.empty() && line.damage != "none") {
				curobj->data.damage = stof(line.damage);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->data.damage);
		}

		if (!line.damageToAdd.empty() && line.damageToAdd != "none") {
				curobj->data.damage = curobj->data.damage + stof(line.damageToAdd);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, line.damageToAdd);
		}

		if (!line.damageMult.empty() && line.damageMult != "none") {
				curobj->data.damage = curobj->data.damage * stof(line.damageMult);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed(mult) damage {}"), curobj->formID, curobj->fullName, curobj->data.damage);
		}

		if (!line.projectile.empty() && line.projectile != "none") {
			RE::TESForm* projectileform = nullptr;
			RE::BGSProjectile* currentprojectile = nullptr;
			std::string string_form = line.projectile.c_str();
			projectileform = GetFormFromIdentifier(string_form);
			if (projectileform && projectileform->formType == RE::FormType::Projectile) {
				currentprojectile = (RE::BGSProjectile*)projectileform;
				curobj->data.projectile = currentprojectile;
				logger::debug(FMT_STRING("projectile set to {:08X} {}"), currentprojectile->formID, currentprojectile->fullName);
			} else {
				//logger::debug("Projectile not set.");
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("weapon formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.formList.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.formList.size(); i++) {
				RE::TESForm* currentform = nullptr;
				RE::BGSListForm* listForm = nullptr;

				std::string string_form = line.formList[i].c_str();
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::FormList) {
					listForm = (RE::BGSListForm*)currentform;
					listForm->AddForm(curobj);
					logger::debug(FMT_STRING("added ammo {:08X} {} to FormList: {:08X}"), curobj->formID, curobj->fullName, listForm->formID);
				} else {
					//logger::info("Armor Penetration not set.");
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}
		#ifndef SKYRIMVR
		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}
		#endif
		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}
		#ifndef SKYRIMVR
		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}
		#endif
		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("ammo formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::invalid_argument& e) {
			}
		}

		
		
		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::invalid_argument& e) {
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
					logger::debug(FMT_STRING("ammo formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.alternateTexturesClear.empty() && line.alternateTexturesClear != "none") {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("ammo formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("ammo formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("ammo formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
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
					logger::debug(FMT_STRING("ammo formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}




		//proj

		if (!line.speed.empty() && line.speed != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.speed = std::stof(line.speed);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed speed {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.speedMult.empty() && line.speedMult != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.speed = curobj->data.projectile->data.speed * std::stof(line.speedMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied speed {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.range.empty() && line.range != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.range = std::stof(line.range);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed range {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.range);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rangeMult.empty() && line.rangeMult != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.range = curobj->data.projectile->data.range * std::stof(line.rangeMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied range {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.range);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.gravity.empty() && line.gravity != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.gravity = std::stof(line.gravity);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed gravity {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.gravity);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.gravityMult.empty() && line.gravityMult != "none" && curobj->data.projectile) {
			try {
				curobj->data.projectile->data.gravity = curobj->data.projectile->data.gravity * std::stof(line.gravityMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied gravity {}"), curobj->formID, curobj->fullName, curobj->data.projectile->data.gravity);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				// logger::debug(FMT_STRING("ammo formid: {:08X} {} flag {}"), curobj->formID, curobj->fullName, setFlag);
				if (toLowerCase(setFlag) == "ignoresnormalweaponresistance") {
					curobj->data.flags.set(RE::AMMO_DATA::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nonplayable") {
					curobj->data.flags.set(RE::AMMO_DATA::Flag::kNonPlayable);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled nonplayable"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nonbolt") {
					curobj->data.flags.set(RE::AMMO_DATA::Flag::kNonBolt);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled nonbolt"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "ignoresnormalweaponresistance") {
					curobj->data.flags.reset(RE::AMMO_DATA::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nonplayable") {
					curobj->data.flags.reset(RE::AMMO_DATA::Flag::kNonPlayable);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled nonplayable"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nonbolt") {
					curobj->data.flags.reset(RE::AMMO_DATA::Flag::kNonBolt);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled nonbolt"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}




	}

}
