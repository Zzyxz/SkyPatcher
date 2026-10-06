#include "misc.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <unordered_set>

namespace MISC
{

	namespace
	{
#define MISC_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kMiscKeys[] = {
			{ "filterByMiscs", sp::FieldKind::List, MISC_F(object) },
			{ "filterByMiscsExcluded", sp::FieldKind::List, MISC_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, MISC_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, MISC_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, MISC_F(keywordsExcluded) },
			{ "fullName", sp::FieldKind::ValueTilde, MISC_F(fullName) },
			{ "value", sp::FieldKind::Value, MISC_F(value) },
			{ "valueMult", sp::FieldKind::Value, MISC_F(valueMult) },
			{ "weight", sp::FieldKind::Value, MISC_F(weight) },
			{ "weightMult", sp::FieldKind::Value, MISC_F(weightMult) },
			{ "keywordsToAdd", sp::FieldKind::List, MISC_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, MISC_F(keywordsToRemove) },
			{ "pickUpSound", sp::FieldKind::Value, MISC_F(pickUpSound) },
			{ "putDownSound", sp::FieldKind::Value, MISC_F(putDownSound) },
			{ "filterByModNames", sp::FieldKind::ListAssign, MISC_F(modNames) },
			{ "model", sp::FieldKind::Value, MISC_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, MISC_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, MISC_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, MISC_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, MISC_F(alternateTexturesFind) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, MISC_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, MISC_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, MISC_F(filterByEditorIdContainsExcluded) },
		};

#undef MISC_F

		const sp::KeyTable& miscKeyTable()
		{
			static const sp::KeyTable table(kMiscKeys, std::size(kMiscKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		miscKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESObjectMISC>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectMISC* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Misc) {
						curobj = (RE::TESObjectMISC*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
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
						if (currentform) {
							if (findTextureSet(curobj, currentform->As<RE::BGSTextureSet>())) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByEditorIdContains.empty() ) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					found = true;
				}

				if (!line.keywords.empty()) {
					keywordAnd = formfilter::matchesKeywordsAnd(view, curobj, nullptr);
				} else {
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					keywordOr = formfilter::matchesKeywordsOr(view, curobj, nullptr);
				} else {
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					found = true;
				}

				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}



				if (!line.objectExcluded.empty()) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Misc)) {
						found = false;
						logger::info("Misc Excluded.");
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found ) {
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (!line.keywordsExcluded.empty()) {
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
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
									if (line.empty() || line[0] == skipChar) {
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
		return nullptr;
	}

	void* patch(const MISC::patch_instruction& line, RE::TESObjectMISC* curobj)
	{
		
		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("misc formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("misc formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.pickUpSound.empty() && line.pickUpSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.pickUpSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->pickupSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("misc formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, curobj->pickupSound->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.putDownSound.empty() && line.putDownSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.putDownSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->putdownSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("misc formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, curobj->putdownSound->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("misc formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("misc formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (!line.alternateTexturesClear.empty() && line.alternateTexturesClear != "none") {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("misc formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("misc formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("misc formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
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
				int32_t index = 0;
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
					logger::debug(FMT_STRING("misc formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}
		return nullptr;
	}

}
