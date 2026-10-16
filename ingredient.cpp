#include "ingredient.h"
#include "formfilter.h"
#include "parsetable.h"
#include <sstream>
#include <unordered_set>
namespace INGR
{
	namespace
	{
#define ING_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kIngredientKeys[] = {
			{ "filterByIngs", sp::FieldKind::List, ING_F(objects) },
			{ "filterByIngsExcluded", sp::FieldKind::List, ING_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, ING_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, ING_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, ING_F(keywordsExcluded) },
			{ "filterByMgefs", sp::FieldKind::List, ING_F(mgefs) },
			{ "filterByMgefsOr", sp::FieldKind::List, ING_F(mgefsOr) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, ING_F(mgefsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, ING_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, ING_F(keywordsToRemove) },
			{ "mgefsToAdd", sp::FieldKind::Arr2D, ING_F(addedObjects) },
			{ "mgefsToChange", sp::FieldKind::Arr2D, ING_F(changedObjects) },
			{ "mgefsToChangeAdd", sp::FieldKind::Arr2D, ING_F(changedAddObjects) },
			{ "mgefsToRemove", sp::FieldKind::List, ING_F(removedObjects) },
			{ "filterByType", sp::FieldKind::Value, ING_F(filterType) },
			{ "value", sp::FieldKind::Value, ING_F(capsvalue) },
			{ "valueMult", sp::FieldKind::Value, ING_F(valueMult) },
			{ "weight", sp::FieldKind::Value, ING_F(weight) },
			{ "weightMult", sp::FieldKind::Value, ING_F(weightMult) },
			{ "clear", sp::FieldKind::Value, ING_F(clear) },
			{ "fullName", sp::FieldKind::ValueTilde, ING_F(fullName) },
			{ "filterByModNames", sp::FieldKind::ListAssign, ING_F(modNames) },
			{ "model", sp::FieldKind::Value, ING_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, ING_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, ING_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, ING_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, ING_F(alternateTexturesFind) },
		};

#undef ING_F

		const sp::KeyTable& ingredientKeyTable()
		{
			static const sp::KeyTable table(kIngredientKeys, std::size(kIngredientKeys));
			return table;
		}

		std::string standaloneClearValue(const std::string& a_line)
		{
			const std::size_t n = a_line.size();
			std::size_t       tokStart = 0;
			while (tokStart <= n) {
				std::size_t tokEnd = a_line.find(':', tokStart);
				if (tokEnd == std::string::npos) {
					tokEnd = n;
				}
				const std::size_t e = a_line.find('=', tokStart);
				if (e != std::string::npos && e < tokEnd) {
					const auto key = sp::detail::trimWs(std::string_view(a_line).substr(tokStart, e - tokStart));
					if (key == "clear" && e + 1 < tokEnd) {
						return sp::detail::trimWs(std::string_view(a_line).substr(e + 1, tokEnd - e - 1));
					}
				}
				if (tokEnd == n) {
					break;
				}
				tokStart = tokEnd + 1;
			}
			return "none";
		}

		bool resolveTypeMatches(const std::string& a_value)
		{
			bool              matches = false;
			std::stringstream ss(a_value);
			std::string       piece;
			while (std::getline(ss, piece, ',')) {
				const std::string name = toLowerCase(trim(piece));
				if (name.empty()) {
					continue;
				}
				if (name == "ingredient" || name == "wortcraft" || name == "wordcraft") {
					matches = true;
				} else {
					logger::warn("ingredient - unknown type in filterByType: {}", name);
				}
			}
			return matches;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		ingredientKeyTable().parse(line, &l);
		if (l.clear != "none") {
			l.clear = standaloneClearValue(line);
		}
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		auto& objectIngredientArray = dataHandler->GetFormArray<RE::IngredientItem>();
		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			
			if (!line.objects.empty()) {
				for (const auto& npcstring : line.objects) {
					RE::TESForm*     currentform = nullptr;
					RE::IngredientItem* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Ingredient) {
						curobj = (RE::IngredientItem*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.alternateTexturesFind.empty() && line.filterType == "none") {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), line.objectExcluded
			};

			std::vector<RE::BGSTextureSet*> textureSets;
			for (const auto& textureString : line.alternateTexturesFind) {
				RE::TESForm* currentform = GetFormFromIdentifier(textureString);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					textureSets.push_back(currentform->As<RE::BGSTextureSet>());
				}
			}
			const std::vector<RE::FormID> mgefsAnd = formfilter::resolveFormIDs(line.mgefs, RE::FormType::MagicEffect);
			const bool typePresent = line.filterType != "none";
			const bool typeMatches = typePresent && resolveTypeMatches(line.filterType);

			for (const auto& curobj : objectIngredientArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}


				if (!textureSets.empty()) {
					for (RE::BGSTextureSet* const ts : textureSets) {
						if (findTextureSet(curobj, ts)) {
							found = true;
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

				if (!line.mgefs.empty()) {
					mgefAnd = !mgefsAnd.empty();
					for (const RE::FormID id : mgefsAnd) {
						bool foundInList = false;
						for (const auto& effect : curobj->effects) {
							if (effect && effect->baseEffect && effect->baseEffect->formID == id) {
								foundInList = true;
								break;
							}
						}
						if (!foundInList) {
							mgefAnd = false;
							break;
						}
					}
				} else {
					mgefAnd = true;
				}
				if (!line.mgefsOr.empty()) {
					for (const auto& mgefstring : line.mgefsOr) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;

							for (const auto& effect : curobj->effects) {
							if (effect && effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
									mgefOr = true;
									break;
								}
							}
						}
						if (mgefOr) {
							break;
						}
					}
				} else {
					mgefOr = true;
				}

				if ((!line.mgefs.empty() || !line.mgefsOr.empty()) && mgefAnd && mgefOr) {
					found = true;
				}

				if (!found && typeMatches) {
					found = true;
				}

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.alternateTexturesFind.empty() && line.filterType == "none") {
					found = true;
				}

				if (!line.keywordsExcluded.empty() && found) {
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
						found = false;
					}
				}

				if (!line.mgefsExcluded.empty() && found) {
					for (const auto& mgefstring : line.mgefsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							keyword = (RE::EffectSetting*)currentform;
							for (const auto& effect : curobj->effects) {
							if (effect && effect->baseEffect && effect->baseEffect->formID == keyword->formID) {
									found = false;
									break;
								}
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Ingredient)) {
						found = false;
						logger::debug(FMT_STRING("ingredient {:08X} {} excluded."), curobj->formID, curobj->fullName);
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
		return nullptr;
	}
	void patch(const INGR::line_content& line, RE::IngredientItem* curobj) {
		try {
		

		if ( !line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("ingredient formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("ingredient formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (IsSwitchOn(line.clear)) {
			curobj->effects.clear();
			logger::debug(FMT_STRING("ingredient formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
		}

		if ( !line.removedObjects.empty()) {
			for (const auto& objectToRemove : line.removedObjects) {
				std::string        removeFormStr = objectToRemove;
				auto* resolvedRemoveForm = GetFormFromIdentifier(removeFormStr);
				auto* removeForm = resolvedRemoveForm ? resolvedRemoveForm->As<RE::EffectSetting>() : nullptr;
				if (removeForm) {
					auto it = curobj->effects.begin();
					while (it != curobj->effects.end()) {
						if ((*it) && (*it)->baseEffect && (*it)->baseEffect->formID == removeForm->formID) {
							it = curobj->effects.erase(it);
							logger::debug(FMT_STRING("ingredient {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if ( !line.addedObjects.empty()) {
			for (const auto& objectToAdd : line.addedObjects) {
				try {
				if (objectToAdd.size() < 4) { logger::warn("ingredient {:08X}: invalid addedObjects entry, expected 4 values", curobj->formID); continue; }
				std::string        addFormStr = objectToAdd[0];
				float              magnitude = std::stof(objectToAdd[1]);
				int32_t            duration = std::stoi(objectToAdd[2]);
				int32_t            area = std::stoi(objectToAdd[3]);
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
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
					logger::debug(FMT_STRING("ingredient {:08X} {} added effect {:08X} {} magnitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
				}
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("ingredient {:08X}: invalid mgefsToAdd entry skipped: {}"), curobj->formID, e.what());
				}
			}
		}

		if ( !line.changedObjects.empty()) {
			for (auto objectToAdd : line.changedObjects) {
				try {
				if (objectToAdd.empty()) { logger::warn("ingredient {:08X}: empty changedObjects entry", curobj->formID); continue; }
				objectToAdd.resize(5, "null");
				std::string        addFormStr = objectToAdd[0];
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
				if (addForm) {
					float magnitude = 0.0f;
					if (!IsNullValue(objectToAdd[1])) {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (!IsNullValue(objectToAdd[2])) {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (!IsNullValue(objectToAdd[3])) {
						area = std::stoi(objectToAdd[3]);
					}

					float multiplier = 0;
					if (!IsNullValue(objectToAdd[4])) {
						multiplier = multiplier + std::stof(objectToAdd[4]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (!IsNullValue(objectToAdd[1])) {
								effect->effectItem.magnitude = magnitude;
								logger::debug(FMT_STRING("ingredient {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("ingredient {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("ingredient {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (!IsNullValue(objectToAdd[4])) {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("ingredient {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("ingredient {:08X}: invalid mgefsToChange entry skipped: {}"), curobj->formID, e.what());
				}
			}
		}

		if (!line.changedAddObjects.empty()) {
			for (auto objectToAdd : line.changedAddObjects) {
				try {
				if (objectToAdd.empty()) { logger::warn("ingredient {:08X}: empty changedAddObjects entry", curobj->formID); continue; }
				objectToAdd.resize(4, "null");
				std::string        addFormStr = objectToAdd[0];
				auto* resolvedAddForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedAddForm ? resolvedAddForm->As<RE::EffectSetting>() : nullptr;
				if (addForm) {
					float magnitude = 0.0f;
					if (!IsNullValue(objectToAdd[1])) {
						magnitude = std::stof(objectToAdd[1]);
					}

					int32_t duration = 0;
					if (!IsNullValue(objectToAdd[2])) {
						duration = std::stoi(objectToAdd[2]);
					}

					int32_t area = 0;
					if (!IsNullValue(objectToAdd[3])) {
						area = std::stoi(objectToAdd[3]);
					}

					for (const auto& effect : curobj->effects) {
						if (effect && effect->baseEffect && effect->baseEffect->formID == addForm->formID) {
							if (!IsNullValue(objectToAdd[1])) {
								effect->effectItem.magnitude += magnitude;
								logger::debug(FMT_STRING("ingredient {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration += duration;
								logger::debug(FMT_STRING("ingredient {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area += area;
								logger::debug(FMT_STRING("ingredient {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("ingredient {:08X}: invalid mgefsToChangeAdd entry skipped: {}"), curobj->formID, e.what());
				}
			}
		}

		if ( !line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("ingredient {:08X}: invalid weight skipped: {}"), curobj->formID, e.what());
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("ingredient {:08X}: invalid weightMult skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.capsvalue.empty() && line.capsvalue != "none") {
			try {
				curobj->value = std::stoi(line.capsvalue);
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("ingredient {:08X}: invalid value skipped: {}"), curobj->formID, e.what());
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = ClampToInt32(MultRoundUp(curobj->value, std::stof(line.valueMult)));
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("ingredient {:08X}: invalid valueMult skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if (!line.model.empty() && line.model != "none") {
			try {
				curobj->SetModel(line.model.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("ingredient formid: {:08X} {} changed model path {}"), curobj->formID, curobj->fullName, curobj->model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (IsSwitchOn(line.alternateTexturesClear)) {
			clearAltTextures(curobj);
			logger::debug(FMT_STRING("ingredient formid: {:08X} {} cleared all alternate textures"), curobj->formID, curobj->fullName);
		}

		if (!line.alternateTexturesRemove.empty()) {
			for (size_t i = 0; i < line.alternateTexturesRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.alternateTexturesRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::TextureSet) {
					auto removed = removeAltTextureByTextureSet(curobj, currentform->As<RE::BGSTextureSet>());
					if (removed) {
						logger::debug(FMT_STRING("ingredient formid: {:08X} {} alternateTexture removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					} else {
						logger::critical(FMT_STRING("ingredient formid: {:08X} {} alternateTexture not removed {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				}
			}
		}

		if (!line.alternateTextures.empty()) {
			for (const auto& objectToAdd : line.alternateTextures) {
				try {
				if (objectToAdd.size() < 3) {
					logger::warn(FMT_STRING("alternateTexturesToAdd requires texture, 3D name and index; skipped invalid entry"));
					continue;
				}
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
					logger::debug(FMT_STRING("ingredient formid: {:08X} {} changed alternate textures {:08X} name3d {} index3d {}"), curobj->formID, curobj->fullName, newTex.textureSet->formID, newTex.name3D.c_str(), newTex.index3D);
				}
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("ingredient {:08X}: invalid alternateTexturesToAdd entry skipped: {}"), curobj->formID, e.what());
				}
			}
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("ingredient {:08X}: skipped remaining operations after invalid value: {}"), curobj ? curobj->formID : 0, e.what());
		}
	}

}
