#include "enchant.h"
#include "formfilter.h"
#include "parsetable.h"
#include <sstream>
#include <unordered_set>
namespace ENCH
{
	namespace
	{
#define ENCH_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kEnchKeys[] = {
			{ "filterByEnchs", sp::FieldKind::List, ENCH_F(objects) },
			{ "filterByEnchsExcluded", sp::FieldKind::List, ENCH_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, ENCH_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, ENCH_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, ENCH_F(keywordsExcluded) },
			{ "filterByMgefs", sp::FieldKind::List, ENCH_F(mgefs) },
			{ "filterByMgefsOr", sp::FieldKind::List, ENCH_F(mgefsOr) },
			{ "filterByMgefsExcluded", sp::FieldKind::List, ENCH_F(mgefsExcluded) },
			{ "keywordsToAdd", sp::FieldKind::List, ENCH_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, ENCH_F(keywordsToRemove) },
			{ "mgefsToAdd", sp::FieldKind::Arr2D, ENCH_F(addedObjects) },
			{ "mgefsToChange", sp::FieldKind::Arr2D, ENCH_F(changedObjects) },
			{ "mgefsToChangeAdd", sp::FieldKind::Arr2D, ENCH_F(changedAddObjects) },
			{ "mgefsToChangeFirst", sp::FieldKind::Arr2D, ENCH_F(changedFirstObjects) },
			{ "mgefsToChangeDamageHealthEffects", sp::FieldKind::Arr2D, ENCH_F(changedDamageHealthObjects) },
			{ "mgefsToRemove", sp::FieldKind::List, ENCH_F(removedObjects) },
			{ "filterByType", sp::FieldKind::Value, ENCH_F(filterType) },
			{ "baseCost", sp::FieldKind::Value, ENCH_F(baseCost) },
			{ "enchantmentAmount", sp::FieldKind::Value, ENCH_F(chargeOverride) },
			{ "chargeTime", sp::FieldKind::Value, ENCH_F(chargeTime) },
			{ "castType", sp::FieldKind::Value, ENCH_F(castType) },
			{ "setFlags", sp::FieldKind::ListAssign, ENCH_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, ENCH_F(removeFlags) },
			{ "clear", sp::FieldKind::Value, ENCH_F(clear) },
			{ "filterByModNames", sp::FieldKind::ListAssign, ENCH_F(modNames) },
			{ "fullName", sp::FieldKind::ValueTilde, ENCH_F(fullName) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, ENCH_F(filterByEditorIdContainsExcluded) },
			{ "filterByMinimumSkillLevel", sp::FieldKind::ListAssign, ENCH_F(filterByMinimumSkillLevel) },
			{ "restrictToCastingType", sp::FieldKind::List, ENCH_F(restrictToCastingType) },
			{ "restrictToSchool", sp::FieldKind::List, ENCH_F(restrictToSchool) },
			{ "restrictToSpellType", sp::FieldKind::List, ENCH_F(restrictToSpellType) },
		};

#undef ENCH_F

		const sp::KeyTable& enchKeyTable()
		{
			static const sp::KeyTable table(kEnchKeys, std::size(kEnchKeys));
			return table;
		}

		std::vector<RE::MagicSystem::SpellType> resolveSpellTypes(const std::string& a_value)
		{
			using T = RE::MagicSystem::SpellType;
			static const std::unordered_map<std::string, T> map{
				{ "spell", T::kSpell }, { "disease", T::kDisease }, { "power", T::kPower },
				{ "lesserpower", T::kLesserPower }, { "lesser_power", T::kLesserPower },
				{ "ability", T::kAbility }, { "poison", T::kPoison }, { "enchantment", T::kEnchantment },
				{ "potion", T::kPotion }, { "alchemy", T::kPotion },
				{ "wortcraft", T::kWortCraft }, { "wordcraft", T::kWortCraft }, { "ingredient", T::kIngredient },
				{ "leveledspell", T::kLeveledSpell }, { "leveled_spell", T::kLeveledSpell },
				{ "addiction", T::kAddiction },
				{ "voicepower", T::kVoicePower }, { "voice_power", T::kVoicePower }, { "shout", T::kVoicePower },
				{ "staffenchantment", T::kStaffEnchantment }, { "staff_enchantment", T::kStaffEnchantment },
				{ "staff", T::kStaffEnchantment }, { "scroll", T::kScroll },
			};
			std::vector<T>    out;
			std::stringstream ss(a_value);
			std::string       piece;
			while (std::getline(ss, piece, ',')) {
				const std::string name = toLowerCase(trim(piece));
				if (name.empty()) {
					continue;
				}
				const auto it = map.find(name);
				if (it != map.end()) {
					out.push_back(it->second);
				} else {
					logger::warn("enchantment - unknown type in filterByType: {}", name);
				}
			}
			return out;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		enchKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		auto& objectSpellArray = dataHandler->GetFormArray<RE::EnchantmentItem>();
		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			
				if (!line.objects.empty()) {
					for (const auto& npcstring : line.objects) {
						RE::TESForm*       currentform = nullptr;
					RE::EnchantmentItem* curobj = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Enchantment) {
							curobj = (RE::EnchantmentItem*)currentform;
							if (patched.insert(curobj->formID).second) {
								patch(line, curobj);
							}
						}
					}
				}

				if (!line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty() && line.filterType == "none") {
					continue;
				}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			std::vector<int> minimumSkillLevels;
			for (const auto& skillLevel : line.filterByMinimumSkillLevel) {
				try {
					minimumSkillLevels.push_back(std::stoi(skillLevel));
				} catch (...) {
					logger::warn("enchantment - invalid filterByMinimumSkillLevel value: {}", skillLevel);
				}
			}
			const std::vector<RE::FormID> mgefsAnd = formfilter::resolveFormIDs(line.mgefs, RE::FormType::MagicEffect);
			const bool                    typePresent = line.filterType != "none";
			const std::vector<RE::MagicSystem::SpellType> spellTypes =
				typePresent ? resolveSpellTypes(line.filterType) : std::vector<RE::MagicSystem::SpellType>{};

			for (const auto& curobj : objectSpellArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;
				bool contains = false;
				bool containsOr = false;

				if ( !curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!line.filterByEditorIdContains.empty()) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!line.filterByEditorIdContainsOr.empty()) {
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if ( (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					found = true;
				}

				if (!minimumSkillLevels.empty() &&
					!curobj->effects.empty() &&
					curobj->effects.front() &&
					curobj->effects.front()->baseEffect) {
					const int minimumSkill = static_cast<int>(curobj->effects.front()->baseEffect->data.minimumSkill);
					for (const int levelAsInt : minimumSkillLevels) {
						if (minimumSkill == levelAsInt) {
							found = true;
							break;
						}
					}
				}

				if (!found && typePresent) {
					for (const auto t : spellTypes) {
						if (curobj->data.spellType == t) {
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

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByMinimumSkillLevel.empty() && line.filterType == "none") {
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
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Enchantment)) {
						found = false;
						logger::debug(FMT_STRING("enchantment {:08X} {} excluded."), curobj->formID, curobj->fullName);
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (found && !line.restrictToSchool.empty()) {
					bool foundSchool = false;

					if (curobj && !curobj->effects.empty() && curobj->effects.front() && curobj->effects.front()->baseEffect) {
						const auto skill = curobj->effects.front()->baseEffect->data.associatedSkill;

						for (auto s : line.restrictToSchool) {
							s = toLowerCase(s);

							if (s == "destruction" && skill == RE::ActorValue::kDestruction) {
								foundSchool = true;
								break;
							}
							if (s == "restoration" && skill == RE::ActorValue::kRestoration) {
								foundSchool = true;
								break;
							}
							if (s == "alteration" && skill == RE::ActorValue::kAlteration) {
								foundSchool = true;
								break;
							}
							if (s == "conjuration" && skill == RE::ActorValue::kConjuration) {
								foundSchool = true;
								break;
							}
							if (s == "illusion" && skill == RE::ActorValue::kIllusion) {
								foundSchool = true;
								break;
							}
							if (s == "none" && skill == RE::ActorValue::kNone) {
								foundSchool = true;
								break;
							}
						}

						if (!foundSchool) {
							found = false;
						}
					}
				}

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					for (auto string : line.restrictToCastingType) {
						string = toLowerCase(string);
						if (string == "concentration" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
							foundCastingType = true;
						} else if (string == "constanteffect" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect) {
							foundCastingType = true;
						} else if (string == "fireandforget" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kFireAndForget) {
							foundCastingType = true;
						} else if (string == "scroll" && curobj->GetCastingType() == RE::MagicSystem::CastingType::kScroll) {
							foundCastingType = true;
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToSpellType.empty()) {
					bool foundType = false;

					if (curobj) {
						const auto type = curobj->GetSpellType();

						for (auto t : line.restrictToSpellType) {
							t = toLowerCase(t);

							if (t == "spell" && type == RE::MagicSystem::SpellType::kSpell) {
								foundType = true;
								break;
							}
							if (t == "disease" && type == RE::MagicSystem::SpellType::kDisease) {
								foundType = true;
								break;
							}
							if (t == "power" && type == RE::MagicSystem::SpellType::kPower) {
								foundType = true;
								break;
							}
							if ((t == "lesserpower" || t == "lesser_power") &&
								type == RE::MagicSystem::SpellType::kLesserPower) {
								foundType = true;
								break;
							}
							if (t == "ability" && type == RE::MagicSystem::SpellType::kAbility) {
								foundType = true;
								break;
							}
							if (t == "poison" && type == RE::MagicSystem::SpellType::kPoison) {
								foundType = true;
								break;
							}
							if (t == "enchantment" && type == RE::MagicSystem::SpellType::kEnchantment) {
								foundType = true;
								break;
							}
							if ((t == "potion" || t == "alchemy") &&
								type == RE::MagicSystem::SpellType::kPotion) {
								foundType = true;
								break;
							}
							if ((t == "wordcraft" || t == "wortcraft") &&
								type == RE::MagicSystem::SpellType::kWortCraft) {
								foundType = true;
								break;
							}
							if (t == "ingredient" &&
								type == RE::MagicSystem::SpellType::kIngredient) {
								foundType = true;
								break;
							}
							if ((t == "leveledspell" || t == "leveled_spell") &&
								type == RE::MagicSystem::SpellType::kLeveledSpell) {
								foundType = true;
								break;
							}
							if (t == "addiction" &&
								type == RE::MagicSystem::SpellType::kAddiction) {
								foundType = true;
								break;
							}
							if ((t == "voicepower" || t == "voice_power" || t == "shout") &&
								type == RE::MagicSystem::SpellType::kVoicePower) {
								foundType = true;
								break;
							}
							if ((t == "staffenchantment" || t == "staff_enchantment" || t == "staff") &&
								type == RE::MagicSystem::SpellType::kStaffEnchantment) {
								foundType = true;
								break;
							}
							if (t == "scroll" &&
								type == RE::MagicSystem::SpellType::kScroll) {
								foundType = true;
								break;
							}
						}

						if (!foundType) {
							found = false;
						}
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
	void patch(const ENCH::line_content& line, RE::EnchantmentItem* curobj) {
		try {
		

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
					logger::debug(FMT_STRING("enchantment formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (IsSwitchOn(line.clear)) {
			curobj->effects.clear();
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} cleared all mgefs"), curobj->formID, curobj->fullName);
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
							logger::debug(FMT_STRING("enchantment {:08X} {}, {:08X} effect removed, effects left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->effects.size());
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
			if (objectToAdd.size() < 4) { logger::warn("enchant {:08X}: invalid addedObjects entry, expected 4 values", curobj->formID); continue; }
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
					tempEffectItem->cost = addForm->data.baseCost * pow(magnitude * durationForCalc*0.1, 1.1);
					
					curobj->effects.push_back(tempEffectItem);
					logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} magnitude {} duration {} area {} cost {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3], tempEffectItem->cost);
					if (objectToAdd.size() >= 5) {
						moveEffectToPosition0(curobj->effects, tempEffectItem);
						logger::debug(FMT_STRING("enchantment {:08X} {} added effect {:08X} {} moved to first position"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName);
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid mgefsToAdd entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if ( !line.changedObjects.empty()) {
		for (auto objectToAdd : line.changedObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("enchant {:08X}: empty changedObjects entry", curobj->formID); continue; }
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
								logger::debug(FMT_STRING("enchantment {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration = duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area = area;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
							if (!IsNullValue(objectToAdd[4])) {
								effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
								logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid mgefsToChange entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if (!line.changedAddObjects.empty()) {
		for (auto objectToAdd : line.changedAddObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("enchant {:08X}: empty changedAddObjects entry", curobj->formID); continue; }
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
								effect->effectItem.magnitude = effect->effectItem.magnitude + magnitude;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, addForm->formID, addForm->fullName, effect->effectItem.magnitude);
							}
							if (!IsNullValue(objectToAdd[2])) {
								effect->effectItem.duration = effect->effectItem.duration + duration;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, addForm->formID, addForm->fullName, effect->effectItem.duration);
							}
							if (!IsNullValue(objectToAdd[3])) {
								effect->effectItem.area = effect->effectItem.area + area;
								logger::debug(FMT_STRING("enchantment {:08X} {} added {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, addForm->formID, addForm->fullName, effect->effectItem.area);
							}
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid mgefsToChangeAdd entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if (!line.changedFirstObjects.empty()) {
		for (auto objectToAdd : line.changedFirstObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("enchant {:08X}: empty changedFirstObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				float magnitude = 0.0f;
				if (!IsNullValue(objectToAdd[0])) {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (!IsNullValue(objectToAdd[1])) {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (!IsNullValue(objectToAdd[2])) {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (!IsNullValue(objectToAdd[3])) {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				if (!curobj->effects.empty()) {
					auto& effect = curobj->effects.front();
					if (effect && effect->baseEffect) {
					if (!IsNullValue(objectToAdd[0])) {
						effect->effectItem.magnitude = magnitude;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
					if (!IsNullValue(objectToAdd[1])) {
						effect->effectItem.duration = duration;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
					}
					if (!IsNullValue(objectToAdd[2])) {
						effect->effectItem.area = area;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed first {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
					}
					if (!IsNullValue(objectToAdd[3])) {
						effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
						logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) first magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
					}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid mgefsToChangeFirst entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if (!line.changedDamageHealthObjects.empty()) {
		for (auto objectToAdd : line.changedDamageHealthObjects) {
			try {
			if (objectToAdd.empty()) { logger::warn("enchant {:08X}: empty changedDamageHealthObjects entry", curobj->formID); continue; }
			objectToAdd.resize(4, "null");
				float magnitude = 0.0f;
				if (!IsNullValue(objectToAdd[0])) {
					magnitude = std::stof(objectToAdd[0]);
				}

				int32_t duration = 0;
				if (!IsNullValue(objectToAdd[1])) {
					duration = std::stoi(objectToAdd[1]);
				}

				int32_t area = 0;
				if (!IsNullValue(objectToAdd[2])) {
					area = std::stoi(objectToAdd[2]);
				}

				float multiplier = 0;
				if (!IsNullValue(objectToAdd[3])) {
					multiplier = multiplier + std::stof(objectToAdd[3]);
				}

				for (const auto& effect : curobj->effects) {
					if (effect && effect->baseEffect && effect->baseEffect->IsDetrimental() && (effect->baseEffect->data.primaryAV == RE::ActorValue::kHealth || effect->baseEffect->data.secondaryAV == RE::ActorValue::kHealth)) {
						if (!IsNullValue(objectToAdd[0])) {
							effect->effectItem.magnitude = magnitude;
							logger::debug(FMT_STRING("enchantment {:08X} {} changed damage effect {} to magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, magnitude, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
						if (!IsNullValue(objectToAdd[1])) {
							effect->effectItem.duration = duration;
							logger::debug(FMT_STRING("enchantment {:08X} {} changed damage effect {} to duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, duration, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.duration);
						}
						if (!IsNullValue(objectToAdd[2])) {
							effect->effectItem.area = area;
							logger::debug(FMT_STRING("enchantment {:08X} {} changed damage effect {} to area of {:08X} {} to {}"), curobj->formID, curobj->fullName, area, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.area);
						}
						if (!IsNullValue(objectToAdd[3])) {
							effect->effectItem.magnitude = effect->effectItem.magnitude * multiplier;
							logger::debug(FMT_STRING("enchantment {:08X} {} changed(multiplied) damage effect magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, effect->baseEffect->formID, effect->baseEffect->fullName, effect->effectItem.magnitude);
						}
					}
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid mgefsToChangeDamageHealthEffects entry skipped: {}"), curobj->formID, e.what());
			}
			}
		}

		if ( !line.baseCost.empty() && line.baseCost != "none") {
			try {
				curobj->data.costOverride = std::stoi(line.baseCost);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed baseCost to {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid baseCost skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.chargeOverride.empty() && line.chargeOverride != "none") {
			try {
				curobj->data.chargeOverride = std::stoi(line.chargeOverride);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed enchantmentAmount to {}"), curobj->formID, curobj->fullName, curobj->data.chargeOverride);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid enchantmentAmount skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if ( !line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->SetCastingType(RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		if ( !line.chargeTime.empty() && line.chargeTime != "none") {
			try {
			curobj->data.chargeTime = std::stof(line.chargeTime);
			logger::debug(FMT_STRING("enchantment formid: {:08X} {} changed chargeTime to {}"), curobj->formID, curobj->fullName, line.chargeTime);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("enchantment {:08X}: invalid chargeTime skipped: {}"), curobj->formID, e.what());
			}
		}

		if ( !line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				if (toLowerCase(setFlag) == "costoverride") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "fooditem") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "extendduration") {
					curobj->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag enabled extendduration"), curobj->formID, curobj->fullName);
				} else {
				}
			}
		}

		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "costoverride") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kCostOverride);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled costoverride"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fooditem") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kFoodItem);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled fooditem"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "extendduration") {
					curobj->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kExtendDuration);
					logger::debug(FMT_STRING("enchantment formid: {:08X} {} flag disabled extendduration"), curobj->formID, curobj->fullName);
				} else {
				}
			}
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("enchantment {:08X}: skipped remaining operations after invalid value: {}"), curobj ? curobj->formID : 0, e.what());
		}
	}


}
