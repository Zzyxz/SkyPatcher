#include "book.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <unordered_set>

namespace BOOK
{
	namespace
	{
#define BOOK_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kBookKeys[] = {
			{ "filterByBooks", sp::FieldKind::List, BOOK_F(object) },
			{ "filterByBooksExcluded", sp::FieldKind::List, BOOK_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, BOOK_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, BOOK_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, BOOK_F(keywordsExcluded) },
			{ "filterBySpellKeywordsOr", sp::FieldKind::List, BOOK_F(filterBySpellKeywordsOr) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, BOOK_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, BOOK_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, BOOK_F(filterByEditorIdContainsExcluded) },
			{ "filterByFlags", sp::FieldKind::ListAssign, BOOK_F(filterByFlags) },
			{ "value", sp::FieldKind::Value, BOOK_F(value) },
			{ "valueMult", sp::FieldKind::Value, BOOK_F(valueMult) },
			{ "weight", sp::FieldKind::Value, BOOK_F(weight) },
			{ "weightMult", sp::FieldKind::Value, BOOK_F(weightMult) },
			{ "teachSpell", sp::FieldKind::Value, BOOK_F(teachSpell) },
			{ "teachSkill", sp::FieldKind::Value, BOOK_F(teachSkill) },
			{ "type", sp::FieldKind::Value, BOOK_F(type) },
			{ "keywordsToAdd", sp::FieldKind::List, BOOK_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, BOOK_F(keywordsToRemove) },
			{ "setFlags", sp::FieldKind::ListAssign, BOOK_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, BOOK_F(removeFlags) },
			{ "pickUpSound", sp::FieldKind::Value, BOOK_F(pickUpSound) },
			{ "putDownSound", sp::FieldKind::Value, BOOK_F(putDownSound) },
			{ "fullName", sp::FieldKind::ValueTilde, BOOK_F(fullName) },
			{ "inventoryArt", sp::FieldKind::Value, BOOK_F(inventoryArt) },
			{ "model", sp::FieldKind::Value, BOOK_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, BOOK_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, BOOK_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, BOOK_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, BOOK_F(alternateTexturesFind) },
			{ "minX", sp::FieldKind::Value, BOOK_F(minX) },
			{ "minY", sp::FieldKind::Value, BOOK_F(minY) },
			{ "minZ", sp::FieldKind::Value, BOOK_F(minZ) },
			{ "maxX", sp::FieldKind::Value, BOOK_F(maxX) },
			{ "maxY", sp::FieldKind::Value, BOOK_F(maxY) },
			{ "maxZ", sp::FieldKind::Value, BOOK_F(maxZ) },
			{ "filterByModNames", sp::FieldKind::ListAssign, BOOK_F(modNames) },
			{ "filterByCastingPerk", sp::FieldKind::List, BOOK_F(filterByCastingPerk) },
		};

#undef BOOK_F

		const sp::KeyTable& bookKeyTable()
		{
			static const sp::KeyTable table(kBookKeys, std::size(kBookKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		bookKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESObjectBOOK>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectBOOK* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Book) {
						curobj = (RE::TESObjectBOOK*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByFlags.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterBySpellKeywordsOr.empty() && line.filterByCastingPerk.empty()) {
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

				if (!line.filterByCastingPerk.empty()) {
					for (const auto& npcstring : line.filterByCastingPerk) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Perk && curobj->TeachesSpell() && curobj->data.teaches.spell && curobj->data.teaches.spell->data.castingPerk && curobj->data.teaches.spell->data.castingPerk->formID == currentform->formID) {
							found = true;
							break;
						}
					}
				}

				if (!found && !line.filterByEditorIdContains.empty() ) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else  {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else  {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					found = true;
				}

				if (!line.alternateTexturesFind.empty()) {
					for (const auto& textureString : line.alternateTexturesFind) {
						RE::TESForm*       currentform = nullptr;
						currentform = GetFormFromIdentifier(textureString);
						if (currentform && currentform->formType == RE::FormType::TextureSet) {
							if (findTextureSet(curobj, currentform->As<RE::BGSTextureSet>())) {
								found = true;
								break;
							}
						}
					}
				}

				if (!line.filterBySpellKeywordsOr.empty()) {
					for (const auto& textureString : line.filterBySpellKeywordsOr) {
						RE::TESForm* currentform = nullptr;
						currentform = GetFormFromIdentifier(textureString);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							auto spell = curobj->data.teaches.spell;
							auto keyword = (RE::BGSKeyword*)currentform;
							if (curobj->TeachesSpell() && spell && keyword) {
								for (const auto& effect : spell->effects)
								{
									if (effect && effect->baseEffect && effect->baseEffect->HasKeyword(keyword)) {
										found = true;
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
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Book)) {
						found = false;
					}
				}

				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found && curobj->formID != 0x000007) {
					if (formfilter::hasExcludedEditorId(view, curobj)) {
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
								std::string                  line;
								std::ifstream                infile;
								std::list<patch_instruction> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									line = StripLineComment(line);
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

	void* patch(const BOOK::patch_instruction& line, RE::TESObjectBOOK* curobj)
	{

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("book formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("book formid: {:08X} {} changed value {:d}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("book formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = ClampToInt32(MultRoundUp(curobj->value, std::stof(line.valueMult)));
				logger::debug(FMT_STRING("book formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
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
			} catch (const std::exception& e) {
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				if (toLowerCase(setFlag) == "advancesactorvalue" || toLowerCase(setFlag) == "teachesspell") {
					logger::warn(FMT_STRING("book formid: {:08X} {} setFlags={} ignored, use teachSpell or teachSkill"), curobj->formID, curobj->fullName, setFlag);
				} else if (toLowerCase(setFlag) == "canttake") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kCantTake);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled CantTake"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "hasbeenread") {
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kHasBeenRead);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag enabled HasBeenRead"), curobj->formID, curobj->fullName);
				} else {
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "advancesactorvalue") {
					if (curobj->data.flags.all(RE::OBJ_BOOK::Flag::kAdvancesActorValue)) {
						if (!curobj->data.flags.all(RE::OBJ_BOOK::Flag::kTeachesSpell)) {
							curobj->data.teaches.spell = nullptr;
						}
						curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
					}
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled AdvancesActorValue"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "canttake") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kCantTake);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled CantTake"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "teachesspell") {
					if (curobj->data.flags.all(RE::OBJ_BOOK::Flag::kTeachesSpell)) {
						curobj->data.teaches.spell = nullptr;
						curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell, RE::OBJ_BOOK::Flag::kAdvancesActorValue);
					}
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled TeachesSpell"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hasbeenread") {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kHasBeenRead);
					logger::debug(FMT_STRING("book formid: {:08X} {} flag disabled HasBeenRead"), curobj->formID, curobj->fullName);
				} else {
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
					logger::debug(FMT_STRING("book formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
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
					logger::debug(FMT_STRING("book formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
				}
			} catch (const std::exception& e) {
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
						curobj->data.flags.set(RE::OBJ_BOOK::Flag::kTeachesSpell);
						curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
						logger::debug(FMT_STRING("book formid: {:08X} {} changed spell teached {:08X}"), curobj->formID, curobj->fullName, curobj->data.teaches.spell->formID);
					}
				} else if (curobj->data.flags.all(RE::OBJ_BOOK::Flag::kTeachesSpell)) {
					curobj->data.teaches.spell = nullptr;
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
					logger::debug(FMT_STRING("book formid: {:08X} {} changed spell teached to null"), curobj->formID, curobj->fullName);
				}
			} catch (const std::exception& e) {
			}
		}

		if (line.teachSkill != "none") {
			static const std::unordered_map<std::string, RE::ActorValue> skills = {
				{ "alchemy", RE::ActorValue::kAlchemy }, { "alteration", RE::ActorValue::kAlteration },
				{ "archery", RE::ActorValue::kArchery }, { "block", RE::ActorValue::kBlock },
				{ "conjuration", RE::ActorValue::kConjuration }, { "destruction", RE::ActorValue::kDestruction },
				{ "enchanting", RE::ActorValue::kEnchanting }, { "heavyarmor", RE::ActorValue::kHeavyArmor },
				{ "illusion", RE::ActorValue::kIllusion }, { "lightarmor", RE::ActorValue::kLightArmor },
				{ "lockpicking", RE::ActorValue::kLockpicking }, { "null", RE::ActorValue::kNone },
				{ "marksman", RE::ActorValue::kArchery }, { "speechcraft", RE::ActorValue::kSpeech },
				{ "onehanded", RE::ActorValue::kOneHanded }, { "pickpocket", RE::ActorValue::kPickpocket },
				{ "restoration", RE::ActorValue::kRestoration }, { "smithing", RE::ActorValue::kSmithing },
				{ "sneak", RE::ActorValue::kSneak }, { "speech", RE::ActorValue::kSpeech },
				{ "twohanded", RE::ActorValue::kTwoHanded }
			};
			if (const auto it = skills.find(toLowerCase(line.teachSkill)); it != skills.end()) {
				curobj->data.teaches.spell = nullptr;
				curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kTeachesSpell);
				if (it->second == RE::ActorValue::kNone) {
					curobj->data.flags.reset(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
				} else {
					curobj->data.teaches.actorValueToAdvance = it->second;
					curobj->data.flags.set(RE::OBJ_BOOK::Flag::kAdvancesActorValue);
				}
				logger::debug(FMT_STRING("book formid: {:08X} {} changed taught skill to {}"), curobj->formID, curobj->fullName, it->first);
			} else {
				logger::warn(FMT_STRING("book formid: {:08X}: unknown skill '{}' in teachSkill"), curobj->formID, line.teachSkill);
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("book formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (IsSwitchOn(line.alternateTexturesClear)) {
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
						logger::warn(FMT_STRING("book formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
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
				std::string texture = objectToAdd[0];
				std::string name = objectToAdd[1];
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
					logger::debug(FMT_STRING("book formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if (!line.minX.empty() && line.minX != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minX);
				curobj->boundData.boundMin.x = std::stoi(line.minX);
			} catch (const std::exception& e) {
			}
		}

		if (!line.minY.empty() && line.minY != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minY);
				curobj->boundData.boundMin.y = std::stoi(line.minY);
			} catch (const std::exception& e) {
			}
		}

		if (!line.minZ.empty() && line.minZ != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed minZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minZ);
				curobj->boundData.boundMin.z = std::stoi(line.minZ);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxX.empty() && line.maxX != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxX);
				curobj->boundData.boundMax.x = std::stoi(line.maxX);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxY.empty() && line.maxY != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxY);
				curobj->boundData.boundMax.y = std::stoi(line.maxY);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxZ.empty() && line.maxZ != "none") {
			try {
				logger::debug(FMT_STRING("book formid: {:08X} {} changed maxZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxZ);
				curobj->boundData.boundMax.z = std::stoi(line.maxZ);
			} catch (const std::exception& e) {
			}
		}
		return nullptr;
	}

}
