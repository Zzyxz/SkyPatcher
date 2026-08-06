#include "soulgem.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace SOULGEM
{

	// ==================================================================
	// PHASE 2 -- SOULGEM's key table. One row per upstream (active)
	// extract*() call, in the same order. See parsetable.h.
	//
	// Several members (keywords, keywordsOr, keywordsExcluded, filterAmmos,
	// damageTypes, ...) are declared in the struct but have no active
	// extract call upstream (dead fields) -- no row is added for them,
	// preserving that (dead) behaviour exactly.
	// ==================================================================
	namespace
	{
#define SOUL_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kSoulGemKeys[] = {
			{ "filterBySoulGems", sp::FieldKind::List, SOUL_F(object) },
			{ "filterBySoulGemsExcluded", sp::FieldKind::List, SOUL_F(objectExcluded) },
			{ "value", sp::FieldKind::Value, SOUL_F(value) },
			{ "valueMult", sp::FieldKind::Value, SOUL_F(valueMult) },
			{ "weight", sp::FieldKind::Value, SOUL_F(weight) },
			{ "weightMult", sp::FieldKind::Value, SOUL_F(weightMult) },
			{ "keywordsToAdd", sp::FieldKind::List, SOUL_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, SOUL_F(keywordsToRemove) },
			{ "pickUpSound", sp::FieldKind::Value, SOUL_F(pickUpSound) },
			{ "putDownSound", sp::FieldKind::Value, SOUL_F(putDownSound) },
			{ "currentSoul", sp::FieldKind::Value, SOUL_F(currentSoul) },
			{ "soulCapacity", sp::FieldKind::Value, SOUL_F(soulCapacity) },
			{ "filterByModNames", sp::FieldKind::ListAssign, SOUL_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, SOUL_F(fullName) },
			{ "model", sp::FieldKind::Value, SOUL_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, SOUL_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, SOUL_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, SOUL_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, SOUL_F(alternateTexturesFind) },
		};

#undef SOUL_F

		const sp::KeyTable& soulGemKeyTable()
		{
			static const sp::KeyTable table(kSoulGemKeys, std::size(kSoulGemKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		soulGemKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
<<<<<<< HEAD
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESSoulGem>();
=======
		const auto& ObjectArray = dataHandler->GetFormArray<RE::TESSoulGem>();
>>>>>>> origin/new-main

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESSoulGem* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::SoulGem) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESSoulGem*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty()) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, soulgem) pair.
			// This struct has no hasPlugin/hasPluginOr/filterByEditorIdContains*
			// members, so those view slots are bound to formfilter::noStrings().
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

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
							//logger::debug("start search");
							if (findTextureSet(curobj, currentform->As<RE::BGSTextureSet>())) {
								//logger::debug("Texture found");
								found = true;
								break;
							}
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

				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.alternateTexturesFind.empty()) {
					found = true;
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::SoulGem)) {
						found = false;
						logger::info("soulgem Excluded.");
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
	}

<<<<<<< HEAD
	void* patch(const SOULGEM::patch_instruction& line, RE::TESSoulGem* curobj)
=======
	void* patch(const SOULGEM::patch_instruction& line, RE::TESSoulGem* curobj)
>>>>>>> origin/new-main
	{
		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.pickUpSound.empty() && line.pickUpSound != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.pickUpSound;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::SoundRecord) {
				curobj->pickupSound = (RE::BGSSoundDescriptorForm*)currentform;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, curobj->pickupSound->formID);
			}
		}

		if (!line.putDownSound.empty() && line.putDownSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.putDownSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->putdownSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, curobj->putdownSound->formID);
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
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("soulgem formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("soulgem formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.soulCapacity.empty() && line.soulCapacity != "none") {
			if (toLowerCase(line.soulCapacity) == "petty") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kPetty;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to Petty"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.soulCapacity) == "lesser") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kLesser;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to Lesser"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.soulCapacity) == "common") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kCommon;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to Common"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.soulCapacity) == "greater") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kGreater;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to Greater"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.soulCapacity) == "grand") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kGrand;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to Grand"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.soulCapacity) == "null") {
				curobj->soulCapacity = RE::SOUL_LEVEL::kNone;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed soul capacity to none"), curobj->formID, curobj->fullName);
			}
		}

		
		if (!line.currentSoul.empty() && line.currentSoul != "none") {
			if (toLowerCase(line.currentSoul) == "petty") {
				curobj->currentSoul = RE::SOUL_LEVEL::kPetty;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to Petty"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.currentSoul) == "lesser") {
				curobj->currentSoul = RE::SOUL_LEVEL::kLesser;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to Lesser"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.currentSoul) == "common") {
				curobj->currentSoul = RE::SOUL_LEVEL::kCommon;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to Common"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.currentSoul) == "greater") {
				curobj->currentSoul = RE::SOUL_LEVEL::kGreater;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to Greater"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.currentSoul) == "grand") {
				curobj->currentSoul = RE::SOUL_LEVEL::kGrand;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to Grand"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.currentSoul) == "null") {
				curobj->currentSoul = RE::SOUL_LEVEL::kNone;
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed current soul to none"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (!line.alternateTexturesClear.empty() && line.alternateTexturesClear != "none") {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("soulgem formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("soulgem formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("soulgem formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
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
					logger::debug(FMT_STRING("soulgem formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}

	}

}
