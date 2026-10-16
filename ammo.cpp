 #include "ammo.h"
#include "formfilter.h"
#include "parsetable.h"
namespace AMMO
{
	namespace
	{
#define AMMO_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kAmmoKeys[] = {
			{ "filterByAmmos", sp::FieldKind::List, AMMO_F(ammo) },
			{ "filterByAmmosExcluded", sp::FieldKind::List, AMMO_F(objectExcluded) },
			{ "filterByWeightLessThan", sp::FieldKind::Value, AMMO_F(weightLessThan) },
			{ "filterByKeywords", sp::FieldKind::List, AMMO_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, AMMO_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, AMMO_F(keywordsExcluded) },
			{ "weight", sp::FieldKind::Value, AMMO_F(weight) },
			{ "attackDamage", sp::FieldKind::Value, AMMO_F(damage) },
			{ "attackDamageToAdd", sp::FieldKind::Value, AMMO_F(damageToAdd) },
			{ "attackDamageMult", sp::FieldKind::Value, AMMO_F(damageMult) },
			{ "setNewProjectile", sp::FieldKind::Value, AMMO_F(projectile) },
			{ "addToFormList", sp::FieldKind::List, AMMO_F(formList) },
			{ "keywordsToAdd", sp::FieldKind::List, AMMO_F(keywordsToAdd) },
			{ "filterByModNames", sp::FieldKind::ListAssign, AMMO_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, AMMO_F(fullName) },
			{ "value", sp::FieldKind::Value, AMMO_F(value) },
			{ "valueMult", sp::FieldKind::Value, AMMO_F(valueMult) },
			{ "weightMult", sp::FieldKind::Value, AMMO_F(weightMult) },
			{ "model", sp::FieldKind::Value, AMMO_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, AMMO_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, AMMO_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, AMMO_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, AMMO_F(alternateTexturesFind) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, AMMO_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, AMMO_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, AMMO_F(filterByEditorIdContainsExcluded) },
			{ "range", sp::FieldKind::Value, AMMO_F(range) },
			{ "rangeMult", sp::FieldKind::Value, AMMO_F(rangeMult) },
			{ "speed", sp::FieldKind::Value, AMMO_F(speed) },
			{ "speedMult", sp::FieldKind::Value, AMMO_F(speedMult) },
			{ "gravity", sp::FieldKind::Value, AMMO_F(gravity) },
			{ "gravityMult", sp::FieldKind::Value, AMMO_F(gravityMult) },
			{ "restrictToBolts", sp::FieldKind::Value, AMMO_F(restrictToBolts) },
			{ "hasPlugins", sp::FieldKind::ListAssign, AMMO_F(hasPlugin) },
			{ "hasPluginsOr", sp::FieldKind::ListAssign, AMMO_F(hasPluginOr) },
			{ "setFlags", sp::FieldKind::ListAssign, AMMO_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, AMMO_F(removeFlags) },
		};

#undef AMMO_F

		const sp::KeyTable& ammoKeyTable()
		{
			static const sp::KeyTable table(kAmmoKeys, std::size(kAmmoKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction_ammo(const std::string& line)
	{
		line_content l;
		ammoKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions_ammo(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		auto& AmmoArray = dataHandler->GetFormArray<RE::TESAmmo>();
		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			std::unordered_set<RE::FormID> projectilesDone;

			if (!line.hasPlugin.empty() && !formfilter::hasAllPlugins(line.hasPlugin)) {
				continue;
			}
			if (!line.hasPluginOr.empty() && !formfilter::hasAnyPlugin(line.hasPluginOr)) {
				continue;
			}

			if (!line.ammo.empty()) {
				for (const auto& ammostring : line.ammo) {
					RE::TESForm* currentform = nullptr;
					RE::TESAmmo* ammo = nullptr;

					std::string string_form = ammostring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Ammo) {
						ammo = (RE::TESAmmo*)currentform;

						if (patched.insert(ammo->formID).second) {
							patch(line, ammo, &projectilesDone);
						}
					}
				}
			}

			if (!line.ammo.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.weightLessThan == "none" && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, line.hasPlugin, line.hasPluginOr,
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : AmmoArray) {
				bool found = false;

				bool keywordAnd = false;
				bool keywordOr = false;
				bool contains = false;
				bool containsOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!line.alternateTexturesFind.empty()) {
					for (const auto& textureString : line.alternateTexturesFind) {
						RE::TESForm* currentform = nullptr;
						currentform = GetFormFromIdentifier(textureString);
						if (currentform && currentform->formType == RE::FormType::TextureSet) {
							if (findTextureSet(curobj, currentform->As<RE::BGSTextureSet>())) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByEditorIdContains.empty()) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty()) {
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					found = true;
				}


				if (!line.keywords.empty()) {
					keywordAnd = formfilter::matchesKeywordsAnd(view, curobj->AsKeywordForm(), nullptr);
				} else {
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					keywordOr = formfilter::matchesKeywordsOr(view, curobj->AsKeywordForm(), nullptr);
				} else {
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					found = true;
				}
				if (auto* weightForm = ngcompat::AmmoWeight(curobj); weightForm && !line.weightLessThan.empty() && line.weightLessThan != "none") {
					if (const auto limit = tryParseFloat(line.weightLessThan); limit && weightForm->weight < *limit) {
						found = true;
					}
				}
				if (line.ammo.empty() && line.weightLessThan == "none" && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(view, curobj->AsKeywordForm(), nullptr)) {
						found = false;
					}
				}

				if (found && !line.restrictToBolts.empty() && line.restrictToBolts != "none") {
					const std::string bolts = toLowerCase(trim(line.restrictToBolts));
					if ((bolts == "true" || bolts == "yes") && !ngcompat::AmmoIsBolt(curobj)) {
						found = false;
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Ammo)) {
						found = false;
					}
				}

				if (found && patched.insert(curobj->formID).second) {
					patch(line, curobj, &projectilesDone);
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
		return nullptr;
	}
	void* patch(const AMMO::line_content& line, RE::TESAmmo* curobj, std::unordered_set<RE::FormID>* a_projectilesDone) {
		try {
		
		if (!line.damage.empty() && line.damage != "none") {
			try {
				ngcompat::AmmoData(curobj).damage = stof(line.damage);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).damage);
			} catch (const std::exception&) {
				logger::warn(FMT_STRING("ammo {:08X}: invalid attackDamage '{}', skipped"), curobj->formID, line.damage);
			}
		}

		if (!line.damageToAdd.empty() && line.damageToAdd != "none") {
			try {
				ngcompat::AmmoData(curobj).damage = ngcompat::AmmoData(curobj).damage + stof(line.damageToAdd);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, line.damageToAdd);
			} catch (const std::exception&) {
				logger::warn(FMT_STRING("ammo {:08X}: invalid attackDamageToAdd '{}', skipped"), curobj->formID, line.damageToAdd);
			}
		}

		if (!line.damageMult.empty() && line.damageMult != "none") {
			try {
				ngcompat::AmmoData(curobj).damage = ngcompat::AmmoData(curobj).damage * stof(line.damageMult);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed(mult) damage {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).damage);
			} catch (const std::exception&) {
				logger::warn(FMT_STRING("ammo {:08X}: invalid attackDamageMult '{}', skipped"), curobj->formID, line.damageMult);
			}
		}

		if (!line.projectile.empty() && line.projectile != "none") {
			RE::TESForm* projectileform = nullptr;
			RE::BGSProjectile* currentprojectile = nullptr;
			std::string string_form = line.projectile.c_str();
			projectileform = GetFormFromIdentifier(string_form);
			if (projectileform && projectileform->formType == RE::FormType::Projectile) {
				currentprojectile = (RE::BGSProjectile*)projectileform;
				ngcompat::AmmoData(curobj).projectile = currentprojectile;
				logger::debug(FMT_STRING("projectile set to {:08X} {}"), currentprojectile->formID, currentprojectile->fullName);
			} else {
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AsKeywordForm()->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("ammo formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.formList.empty()) {
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
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}
		auto* weightForm = ngcompat::AmmoWeight(curobj);
		if (weightForm && !line.weight.empty() && line.weight != "none") {
			try {
				weightForm->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, weightForm->weight);
			} catch (const std::exception& e) {
			}
		}
		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}
		if (weightForm && !line.weightMult.empty() && line.weightMult != "none") {
			try {
				weightForm->weight = weightForm->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, weightForm->weight);
			} catch (const std::exception& e) {
			}
		}
		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(MultRoundUp(curobj->value, std::stod(line.valueMult)));
				logger::debug(FMT_STRING("ammo formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (IsSwitchOn(line.alternateTexturesClear)) {
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
				if (objectToAdd.size() < 3) {
					logger::warn(FMT_STRING("alternateTexturesToAdd requires texture, 3D name and index; skipped invalid entry"));
					continue;
				}
				std::string  texture = objectToAdd[0];
				std::string  name = objectToAdd[1];
				int32_t      index = 0;
				try {
					index = std::stoi(objectToAdd[2]);
				} catch (const std::exception&) {
					logger::warn(FMT_STRING("alternateTexturesToAdd: invalid index '{}', skipped"), objectToAdd[2]);
					continue;
				}
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




		bool projectileOpen = ngcompat::AmmoData(curobj).projectile != nullptr;
		if (projectileOpen && a_projectilesDone) {
			const bool hasProjectileKey =
				(!line.speed.empty() && line.speed != "none") || (!line.speedMult.empty() && line.speedMult != "none") ||
				(!line.range.empty() && line.range != "none") || (!line.rangeMult.empty() && line.rangeMult != "none") ||
				(!line.gravity.empty() && line.gravity != "none") || (!line.gravityMult.empty() && line.gravityMult != "none");
			if (hasProjectileKey && !a_projectilesDone->insert(ngcompat::AmmoData(curobj).projectile->formID).second) {
				projectileOpen = false;
				logger::debug(FMT_STRING("ammo formid: {:08X} {} projectile {:08X} already changed by this line"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->formID);
			}
		}

		if (projectileOpen && !line.speed.empty() && line.speed != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.speed = std::stof(line.speed);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed speed {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.speed);
			} catch (const std::exception& e) {
			}
		}

		if (projectileOpen && !line.speedMult.empty() && line.speedMult != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.speed = ngcompat::AmmoData(curobj).projectile->data.speed * std::stof(line.speedMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied speed {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.speed);
			} catch (const std::exception& e) {
			}
		}

		if (projectileOpen && !line.range.empty() && line.range != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.range = std::stof(line.range);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed range {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.range);
			} catch (const std::exception& e) {
			}
		}

		if (projectileOpen && !line.rangeMult.empty() && line.rangeMult != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.range = ngcompat::AmmoData(curobj).projectile->data.range * std::stof(line.rangeMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied range {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.range);
			} catch (const std::exception& e) {
			}
		}

		if (projectileOpen && !line.gravity.empty() && line.gravity != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.gravity = std::stof(line.gravity);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} changed gravity {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.gravity);
			} catch (const std::exception& e) {
			}
		}

		if (projectileOpen && !line.gravityMult.empty() && line.gravityMult != "none") {
			try {
				ngcompat::AmmoData(curobj).projectile->data.gravity = ngcompat::AmmoData(curobj).projectile->data.gravity * std::stof(line.gravityMult);
				logger::debug(FMT_STRING("ammo projectile formid: {:08X} {} multiplied gravity {}"), curobj->formID, curobj->fullName, ngcompat::AmmoData(curobj).projectile->data.gravity);
			} catch (const std::exception& e) {
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				if (toLowerCase(setFlag) == "ignoresnormalweaponresistance") {
					ngcompat::AmmoData(curobj).flags.set(RE::AMMO_DATA::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nonplayable") {
					ngcompat::AmmoData(curobj).flags.set(RE::AMMO_DATA::Flag::kNonPlayable);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled nonplayable"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "nonbolt") {
					ngcompat::AmmoData(curobj).flags.set(RE::AMMO_DATA::Flag::kNonBolt);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag enabled nonbolt"), curobj->formID, curobj->fullName);
				} else {
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "ignoresnormalweaponresistance") {
					ngcompat::AmmoData(curobj).flags.reset(RE::AMMO_DATA::Flag::kIgnoresNormalWeaponResistance);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled ignoresnormalweaponresistance"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nonplayable") {
					ngcompat::AmmoData(curobj).flags.reset(RE::AMMO_DATA::Flag::kNonPlayable);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled nonplayable"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nonbolt") {
					ngcompat::AmmoData(curobj).flags.reset(RE::AMMO_DATA::Flag::kNonBolt);
					logger::debug(FMT_STRING("ammo formid: {:08X} {} flag disabled nonbolt"), curobj->formID, curobj->fullName);
				} else {
				}
			}
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("ammo {:08X}: skipped remaining operations after invalid value: {}"), curobj ? curobj->formID : 0, e.what());
		}
		return nullptr;
	}

}
