 #include "ammo.h"
namespace AMMO
{

	struct line_content create_patch_instruction_ammo(const std::string& line)
	{
		line_content l;

		//// extract ammos
		//std::regex ammos_regex("filterByAmmos\\s*=([^:]+)", regex::icase);
		//std::smatch ammos_match;
		//std::regex_search(line, ammos_match, ammos_regex);
		//std::vector<std::string> ammos;
		//if (ammos_match.empty() || ammos_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string ammos_str = ammos_match[1];
		//	std::regex ammos_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator ammos_iterator(ammos_str.begin(), ammos_str.end(), ammos_list_regex);
		//	std::sregex_iterator ammos_end;
		//	while (ammos_iterator != ammos_end) {
		//		std::string tempVar = (*ammos_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			ammos.push_back(tempVar);
		//		}
		//		++ammos_iterator;
		//	}
		//	l.ammo = ammos;
		//}

		extractData(line, "filterByAmmos\\s*=([^:]+)", l.ammo);

		// extract weight
		std::regex weightLess_regex("filterByWeightLessThan\\s*=([^:]+)", regex::icase);
		std::smatch weightLessmatch;
		std::regex_search(line, weightLessmatch, weightLess_regex);
		// extract the value after the equals sign
		if (weightLessmatch.empty() || weightLessmatch[1].str().empty()) {
			l.weightLessThan = "none";
		} else {
			std::string value = weightLessmatch[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.weightLessThan = value;
		}

		//// extract keywords
		//std::regex  keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
		//std::smatch keywords_match;
		//std::regex_search(line, keywords_match, keywords_regex);
		//std::vector<std::string> keywords;
		//if (keywords_match.empty() || keywords_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywords_str = keywords_match[1];
		//	std::regex           keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
		//	std::sregex_iterator keywords_end;
		//	while (keywords_iterator != keywords_end) {
		//		std::string keyword = (*keywords_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywords.push_back(keyword);
		//		}
		//		++keywords_iterator;
		//	}
		//	l.keywords = keywords;
		//}

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		//// extract keywords
		//std::regex  keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsOr_match;
		//std::regex_search(line, keywordsOr_match, keywordsOr_regex);
		//std::vector<std::string> keywordsOr;
		//if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywordsOr_str = keywordsOr_match[1];
		//	std::regex           keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
		//	std::sregex_iterator keywordsOr_end;
		//	while (keywordsOr_iterator != keywordsOr_end) {
		//		std::string keyword = (*keywordsOr_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsOr.push_back(keyword);
		//		}
		//		++keywordsOr_iterator;
		//	}
		//	l.keywordsOr = keywordsOr;
		//}

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		//// extract keywords
		//std::regex  keywordsExcluded_regex("filterByKeywordsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsExcluded_match;
		//std::regex_search(line, keywordsExcluded_match, keywordsExcluded_regex);
		//std::vector<std::string> keywordsExcluded;
		//if (keywordsExcluded_match.empty() || keywordsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywordsExcluded_str = keywordsExcluded_match[1];
		//	std::regex           keywordsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsExcluded_iterator(keywordsExcluded_str.begin(), keywordsExcluded_str.end(), keywordsExcluded_list_regex);
		//	std::sregex_iterator keywordsExcluded_end;
		//	while (keywordsExcluded_iterator != keywordsExcluded_end) {
		//		std::string keyword = (*keywordsExcluded_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsExcluded.push_back(keyword);
		//		}
		//		++keywordsExcluded_iterator;
		//	}
		//	l.keywordsExcluded = keywordsExcluded;
		//}

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

						// extract weight
		std::regex weight_regex("weight\\s*=([^:]+)", regex::icase);
		std::smatch weightmatch;
		std::regex_search(line, weightmatch, weight_regex);
		// extract the value after the equals sign
		if (weightmatch.empty() || weightmatch[1].str().empty()) {
			l.weight = "none";
		} else {
			std::string value = weightmatch[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.weight = value;
		}

		// extract damage
		std::regex damage_regex("attackDamage\\s*=([^:]+)", regex::icase);
		std::smatch damagematch;
		std::regex_search(line, damagematch, damage_regex);
		// extract the value after the equals sign
		if (damagematch.empty() || damagematch[1].str().empty()) {
			l.damage = "none";
		} else {
			std::string damagevalue = damagematch[1].str();
			damagevalue.erase(std::remove_if(damagevalue.begin(), damagevalue.end(), ::isspace), damagevalue.end());
			l.damage = damagevalue;
		}

		// extract damageMult
		std::regex  damageMult_regex("attackDamageMult\\s*=([^:]+)", regex::icase);
		std::smatch damageMultmatch;
		std::regex_search(line, damageMultmatch, damageMult_regex);
		// extract the value after the equals sign
		if (damageMultmatch.empty() || damageMultmatch[1].str().empty()) {
			l.damageMult = "none";
		} else {
			std::string damageMultvalue = damageMultmatch[1].str();
			damageMultvalue.erase(std::remove_if(damageMultvalue.begin(), damageMultvalue.end(), ::isspace), damageMultvalue.end());
			l.damageMult = damageMultvalue;
		}

		// extract projectile
		std::regex setNewProjectile_regex("setNewProjectile\\s*=([^:]+)", regex::icase);
		std::smatch setNewProjectilematch;
		std::regex_search(line, setNewProjectilematch, setNewProjectile_regex);
		// extract the value after the equals sign
		if (setNewProjectilematch.empty() || setNewProjectilematch[1].str().empty()) {
			l.projectile = "none";
		} else {
			std::string setNewProjectilevalue = setNewProjectilematch[1].str();
			setNewProjectilevalue.erase(setNewProjectilevalue.begin(), std::find_if_not(setNewProjectilevalue.begin(), setNewProjectilevalue.end(), ::isspace));
			setNewProjectilevalue.erase(std::find_if_not(setNewProjectilevalue.rbegin(), setNewProjectilevalue.rend(), ::isspace).base(), setNewProjectilevalue.end());
			l.projectile = setNewProjectilevalue;
		}

		//// extract formList
		//std::regex formList_regex("addToFormList\\s*=([^:]+)", regex::icase);
		//std::smatch formList_match;
		//std::regex_search(line, formList_match, formList_regex);
		//std::vector<std::string> formList;
		//if (formList_match.empty() || formList_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string formList_str = formList_match[1];
		//	std::regex formList_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator formList_iterator(formList_str.begin(), formList_str.end(), formList_list_regex);
		//	std::sregex_iterator formList_end;
		//	while (formList_iterator != formList_end) {
		//		std::string tempVar = (*formList_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			formList.push_back(tempVar);
		//		}
		//		++formList_iterator;
		//	}
		//	l.formList = formList;
		//}

		extractData(line, "addToFormList\\s*=([^:]+)", l.formList);

		//// extract fullName
		//std::regex fullName_regex("fullName\\s*=\\s*~([^~]+?)\\s*~");
		//std::smatch namematch;
		//std::regex_search(line, namematch, fullName_regex);
		//// extract the value after the equals sign
		//if (namematch.empty() || namematch[1].str().empty()) {
		//	l.fullName = "none";
		//} else {
		//	std::string namevalue = namematch[1].str();
		//	namevalue.erase(namevalue.begin(), std::find_if_not(namevalue.begin(), namevalue.end(), ::isspace));
		//	namevalue.erase(std::find_if_not(namevalue.rbegin(), namevalue.rend(), ::isspace).base(), namevalue.end());
		//	l.fullName = namevalue;
		//}

				// extract keywordsToAdd
		std::regex keywordsToAdd_regex("keywordsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch keywordsToAdd_match;
		std::regex_search(line, keywordsToAdd_match, keywordsToAdd_regex);
		std::vector<std::string> keywordsToAdd;
		if (keywordsToAdd_match.empty() || keywordsToAdd_match[1].str().empty()) {
			//empty
		} else {
			std::string keywordsToAdd_str = keywordsToAdd_match[1];
			std::regex keywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator keywordsToAdd_iterator(keywordsToAdd_str.begin(), keywordsToAdd_str.end(), keywordsToAdd_list_regex);
			std::sregex_iterator keywordsToAdd_end;
			while (keywordsToAdd_iterator != keywordsToAdd_end) {
				std::string keywordToAdd = (*keywordsToAdd_iterator)[0].str();
				keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
				keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
				if (keywordToAdd != "none") {
					//logger::info(FMT_STRING("keywordsToAdd: {}"), keywordToAdd);
					keywordsToAdd.push_back(keywordToAdd);
				}
				++keywordsToAdd_iterator;
			}
			l.keywordsToAdd = keywordsToAdd;
		}

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

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
				//logger::info(FMT_STRING("Its a form! {:08X}"), curobj->formID);

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

				if (!line.weightLessThan.empty() && line.weightLessThan != "none") {
					if (curobj->weight < stof(line.weightLessThan)) {
						found = true;
					}
				}

				if (line.ammo.empty() && line.weightLessThan.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
				}

				if (!line.keywordsExcluded.empty()) {
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

				if (found) {
					patch(line, curobj);
				}

				if (found && !line.damage.empty() && line.damage != "none") {
					try {
						curobj->data.damage = stof(line.damage);
						logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->data.damage);
					} catch (const std::invalid_argument& e) {
					}
				}

				if (found && !line.projectile.empty() && line.projectile != "none") {
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

				if (found && !line.formList.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.formList.size(); i++) {
						RE::TESForm* currentform = nullptr;
						RE::BGSListForm* listForm = nullptr;
						std::string string_form = line.formList[i].c_str();
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::FormList) {
							listForm = (RE::BGSListForm*)currentform;
							listForm->forms.push_back(curobj);
							logger::debug(FMT_STRING("added ammo {:08X} {} to FormList: {:08X}"), curobj->formID, curobj->fullName, listForm->formID);
						} else {
							//logger::info("Armor Penetration not set.");
						}
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
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
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

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::invalid_argument& e) {
			}
		}
	}

}
