#include "leveledlist.h"
#include "enginelists.h"
#include "formfilter.h"
#include "parsetable.h"
#include <algorithm>
#include <cmath>
#include <limits>
namespace LEVELEDLIST
{
	namespace
	{
#define LL_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))
		constexpr sp::KeySpec kLeveledListKeys[] = {
			{ "filterByLLs", sp::FieldKind::List, LL_F(objects) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, LL_F(filterByEditorIdContainsExcluded) },
			{ "filterByLLNPCs", sp::FieldKind::List, LL_F(containerObjects) },
			{ "addToLLs", sp::FieldKind::Arr2D, LL_F(addedObjects) },
			{ "addOnceToLLs", sp::FieldKind::Arr2D, LL_F(addedObjectsOnce) },
			{ "removeFromLLs", sp::FieldKind::Arr2D, LL_F(removedObjects) },
			{ "clear", sp::FieldKind::Value, LL_F(clear) },
			{ "calcForLevel", sp::FieldKind::Value, LL_F(calcForLevel) },
			{ "calcEachItem", sp::FieldKind::Value, LL_F(calcEachItem) },
			{ "calcLevelAndEachItem", sp::FieldKind::Value, LL_F(calcForLevelAndEachItem) },
			{ "calcUseAll", sp::FieldKind::Value, LL_F(calcUseAll) },
			{ "clearFlags", sp::FieldKind::Value, LL_F(clearFlags) },
			{ "addTemplateKeywords", sp::FieldKind::Arr2D, LL_F(templateKeyword) },
			{ "removeObjectsByKeyword", sp::FieldKind::List, LL_F(removeItemsByKeyword) },
			{ "chanceGlobal", sp::FieldKind::Value, LL_F(llglobal) },
			{ "chanceNone", sp::FieldKind::Value, LL_F(chanceNone) },
			{ "formsToReplace", sp::FieldKind::Arr2D, LL_F(replaceObjects) },
			{ "objectMultCount", sp::FieldKind::Arr2D, LL_F(objectMultCount) },
		};
#undef LL_F

		const sp::KeyTable& leveledListKeyTable()
		{
			static const sp::KeyTable table(kLeveledListKeys, std::size(kLeveledListKeys));
			return table;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		leveledListKeyTable().parse(line, &l);

		std::regex  noFilterLL_regex("noFilterLL\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLmatch;
		std::regex_search(line, noFilterLLmatch, noFilterLL_regex);
		if (noFilterLLmatch.empty() || noFilterLLmatch[1].str().empty()) {
			l.noFilterLL = "none";
		}
		else {
			std::string noFilterLLvalue = noFilterLLmatch[1].str();
			noFilterLLvalue.erase(noFilterLLvalue.begin(), std::find_if_not(noFilterLLvalue.begin(), noFilterLLvalue.end(), ::isspace));
			noFilterLLvalue.erase(std::find_if_not(noFilterLLvalue.rbegin(), noFilterLLvalue.rend(), ::isspace).base(), noFilterLLvalue.end());
			l.noFilterLL = noFilterLLvalue;
		}

		std::regex  noFilterLLNPC_regex("noFilterLLNPC\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLNPCmatch;
		std::regex_search(line, noFilterLLNPCmatch, noFilterLLNPC_regex);
		if (noFilterLLNPCmatch.empty() || noFilterLLNPCmatch[1].str().empty()) {
			l.noFilterLLNPC = "none";
		}
		else {
			std::string noFilterLLNPCvalue = noFilterLLNPCmatch[1].str();
			noFilterLLNPCvalue.erase(noFilterLLNPCvalue.begin(), std::find_if_not(noFilterLLNPCvalue.begin(), noFilterLLNPCvalue.end(), ::isspace));
			noFilterLLNPCvalue.erase(std::find_if_not(noFilterLLNPCvalue.rbegin(), noFilterLLNPCvalue.rend(), ::isspace).base(), noFilterLLNPCvalue.end());
			l.noFilterLLNPC = noFilterLLNPCvalue;
		}

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                    dataHandler = RE::TESDataHandler::GetSingleton();




		for (const auto& line : tokens) {
			if (!line.objects.empty()) {
				for (const auto& objectstring : line.objects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevItem* object = nullptr;

					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::LeveledItem) {
						object = (RE::TESLevItem*)currentform;
						if (static_cast<uint32_t>(object->numEntries) > 120) {
							logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), object->formID);
							continue;
						}
						patch(line, object);
					}
				}
			}


			if (!line.containerObjects.empty()) {
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevCharacter* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::LeveledNPC) {
						object = (RE::TESLevCharacter*)currentform;
						patch(line, object);
					}
				}
			}

			if (!line.noFilterLL.empty() && toLowerCase(line.noFilterLL) == "true") {
				auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
				for (const auto& curobj : objectArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					patch(line, curobj);
				}
			}

			if (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty() || !line.filterByEditorIdContainsExcluded.empty()) {
				const formfilter::CommonFilterView view{
					formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
					formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
					line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
					line.filterByEditorIdContainsExcluded, formfilter::noStrings()
				};

				auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();
				for (const auto& curobj : objectArray) {

					bool contains = false;
					bool containsOr = false;
					bool found = false;

					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					if (!line.filterByEditorIdContains.empty()) {
						contains = formfilter::matchesEditorIdAnd(view, curobj);
					}
					else {
						contains = true;
					}

					if (!line.filterByEditorIdContainsOr.empty()) {
						containsOr = formfilter::matchesEditorIdOr(view, curobj);
					}
					else {
						containsOr = true;
					}

					if ((!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
						found = true;
					}

					if (!line.filterByEditorIdContainsExcluded.empty() && found) {
						if (formfilter::hasExcludedEditorId(view, curobj)) {
							found = false;
						}
					}

					if (found) {
						patch(line, curobj);
					}
				}
			}


			if (!line.noFilterLLNPC.empty() && toLowerCase(line.noFilterLLNPC) == "true") {
				auto& tesLevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();
				for (const auto& curobj : tesLevCharArray) {
					if (static_cast<uint32_t>(curobj->numEntries) > 120) {
						logger::debug(FMT_STRING("leveled lists {:08X} skipped. Too huge progress. This Leveled List also may provide performance issues ingame."), curobj->formID);
						continue;
					}

					patch(line, curobj);
				}
			}



		}
	}

	void* readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
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
						}
						else {
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
								std::string             line;
								std::ifstream           infile;
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
			}
			else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return nullptr;
	}
	template <typename T>
	void patch(const LEVELEDLIST::line_content& line, T* curobj)
	{
		auto* list = static_cast<RE::TESLeveledList*>(curobj);

		const auto parseUInt16 = [curobj](const std::string& text, std::string_view operation, std::uint16_t& result) {
			try {
				const auto value = std::stoul(text);
				if (value > std::numeric_limits<std::uint16_t>::max()) {
					throw std::out_of_range("value exceeds uint16 range");
				}
				result = static_cast<std::uint16_t>(value);
				return true;
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("leveled list {:08X}: invalid {} value '{}': {}"), curobj->formID, operation, text, e.what());
				return false;
			}
		};

		const auto parseFloat = [curobj](const std::string& text, std::string_view operation, float& result) {
			try {
				result = std::stof(text);
				return true;
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("leveled list {:08X}: invalid {} value '{}': {}"), curobj->formID, operation, text, e.what());
				return false;
			}
		};

		const auto matchesRelation = [](std::uint16_t value, const std::string& relation, std::uint16_t number) {
			if (relation == "<") {
				return value < number;
			}
			if (relation == ">") {
				return value > number;
			}
			if (relation == ">=") {
				return value >= number;
			}
			if (relation == "<=") {
				return value <= number;
			}
			if (relation.empty()) {
				return value == number;
			}
			return false;
		};

		if (!line.replaceObjects.empty()) {
			for (const auto& objectToreplace : line.replaceObjects) {
				if (objectToreplace.size() < 2) {
					logger::warn(FMT_STRING("leveled list {:08X}: invalid replacement entry, expected 2 forms"), curobj->formID);
					continue;
				}
				const std::string& formToReplaceString = objectToreplace[0];
				const std::string& formToReplaceWithString = objectToreplace[1];
				RE::TESForm*       formToReplace = GetFormFromIdentifier(formToReplaceString);
				RE::TESForm*       formToReplaceWith = GetFormFromIdentifier(formToReplaceWithString);
				if (!formToReplace || !formToReplaceWith) {
					logger::warn(FMT_STRING("leveled list {:08X}: invalid replacement form '{}' -> '{}', skipping"), curobj->formID, formToReplaceString, formToReplaceWithString);
					continue;
				}
				std::string reason;
				if (!EngineLists::CanHoldForm(curobj, list, formToReplaceWith, reason)) {
					logger::warn(FMT_STRING("leveled list {:08X}: cannot replace with {:08X}: {}"), curobj->formID, formToReplaceWith->formID, reason);
					continue;
				}
				int count = 0;
				EngineLists::ForEachLeveledObject(list, [&](RE::LEVELED_OBJECT& obj) {
					if (obj.form == formToReplace) {
						obj.form = formToReplaceWith;
						count++;
					}
				});
				logger::debug(FMT_STRING("leveled Lists {:08X} all forms of {:08X} have been replaced by {:08X}. Total replaced: {}"), curobj->formID, formToReplace->formID, formToReplaceWith->formID, count);
			}
		}

		if (!line.removedObjects.empty()) {
			for (const auto& objectsToRemove : line.removedObjects) {
				if (objectsToRemove.empty()) {
					logger::warn(FMT_STRING("leveled list {:08X}: empty remove entry"), curobj->formID);
					continue;
				}
				RE::TESForm* delForm = GetFormFromIdentifier(objectsToRemove[0]);
				if (!delForm) {
					logger::debug(FMT_STRING("Form not found {}, skipping."), objectsToRemove[0]);
					continue;
				}

				std::string relationSign = "";
				uint16_t    number = 0;
				bool        hasNumberLvl = false;
				bool        hasNumberCount = false;

				if (objectsToRemove.size() >= 2) {
					std::vector<std::string> templvl = splitRelationNumber(objectsToRemove[1]);
					if (templvl.size() == 2 && templvl[0] != "none") {
						if (!parseUInt16(templvl[0], "remove level", number)) { continue; }
						relationSign = templvl[1];
						hasNumberLvl = true;
					}
					else if (templvl.size() == 1 && templvl[0] != "none") {
						if (!parseUInt16(templvl[0], "remove level", number)) { continue; }
						hasNumberLvl = true;
					}

					if (hasNumberLvl) {
						const auto removed = EngineLists::RemoveLeveledObjects(curobj, list, [&](const RE::LEVELED_OBJECT& x) {
							return x.form == delForm && matchesRelation(x.level, relationSign, number);
						});
						logger::debug(FMT_STRING("leveled Lists {:08X} level {}{} removed Form {:08X} ({} entries)"), curobj->formID, relationSign.empty() ? "==" : relationSign, number, delForm->formID, removed);
					}
				}

				if (objectsToRemove.size() >= 3) {
					std::vector<std::string> tempcount = splitRelationNumber(objectsToRemove[2]);

					relationSign = "";

					if (tempcount.size() == 2 && tempcount[0] != "none") {
						if (!parseUInt16(tempcount[0], "remove count", number)) { continue; }
						relationSign = tempcount[1];
						hasNumberCount = true;
					}
					else if (tempcount.size() == 1 && tempcount[0] != "none") {
						if (!parseUInt16(tempcount[0], "remove count", number)) { continue; }
						hasNumberCount = true;
					}

					if (hasNumberCount) {
						const auto removed = EngineLists::RemoveLeveledObjects(curobj, list, [&](const RE::LEVELED_OBJECT& x) {
							return x.form == delForm && matchesRelation(x.count, relationSign, number);
						});
						logger::debug(FMT_STRING("leveled Lists {:08X} count {}{} removed Form {:08X} ({} entries)"), curobj->formID, relationSign.empty() ? "==" : relationSign, number, delForm->formID, removed);
					}
				}

				if (!hasNumberLvl && !hasNumberCount) {
					const auto removed = EngineLists::RemoveLeveledObjects(curobj, list, [&](const RE::LEVELED_OBJECT& x) {
						return x.form == delForm;
					});
					logger::debug(FMT_STRING("leveled Lists {:08X} removed all Forms {:08X} ({} entries)"), curobj->formID, delForm->formID, removed);
				}
			}
		}

		if (!line.removeItemsByKeyword.empty()) {
			for (const auto& string_form : line.removeItemsByKeyword) {
				RE::TESForm* currentform = GetFormFromIdentifier(string_form);
				auto*        keyword = currentform ? currentform->As<RE::BGSKeyword>() : nullptr;
				if (!keyword) {
					continue;
				}
				const auto removed = EngineLists::RemoveLeveledObjects(curobj, list, [&](const RE::LEVELED_OBJECT& x) {
					const auto* keyForm = ngcompat::KeywordForm(x.form);
					return keyForm && keyForm->HasKeyword(keyword);
				});
				logger::debug(FMT_STRING("leveled Lists {:08X} removed {} items with keyword {:08X} {}"), curobj->formID, removed, keyword->formID, keyword->formEditorID);
			}
		}

		if (!line.clear.empty() && line.clear != "none") {
			if (line.clear == "yes" || line.clear == "true") {
				EngineLists::ClearLeveledList(curobj, list);
				logger::debug(FMT_STRING("leveled lists {:08X} cleared"), curobj->formID);
			}
		}

		if (!line.calcForLevel.empty() && line.calcForLevel != "none") {
			if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
				curobj->llFlags = RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
			}
		}

		if (!line.calcEachItem.empty() && line.calcEachItem != "none") {
			if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
				curobj->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
			}
		}

		if (!line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
			if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
				curobj->llFlags = static_cast<RE::TESLeveledList::Flag>(
					RE::TESLeveledList::Flag::kCalculateFromAllLevelsLTOrEqPCLevel |
					RE::TESLeveledList::Flag::kCalculateForEachItemInCount);
				logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
			}
		}

		if (!line.calcUseAll.empty() && line.calcUseAll != "none") {
			curobj->llFlags = RE::TESLeveledList::kUseAll;
			logger::debug(FMT_STRING("leveled lists {:08X} set to Use All"), curobj->formID);
		}

		if (!line.clearFlags.empty() && line.clearFlags != "none") {
			curobj->llFlags = resetAllFlags(curobj->llFlags);
			logger::debug(FMT_STRING("leveled lists {:08X} cleared all flags"), curobj->formID);
		}

		if (!line.addedObjects.empty()) {
			for (const auto& objectToAdd : line.addedObjects) {
				if (objectToAdd.empty()) { logger::warn("leveled list {:08X}: empty addedObjects entry", curobj->formID); continue; }
				uint16_t level = 1;
				uint16_t count = 1;

				const std::string& addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					if (!parseUInt16(objectToAdd[1], "add level", level)) { continue; }
				}
				if (objectToAdd.size() > 2) {
					if (!parseUInt16(objectToAdd[2], "add count", count)) { continue; }
				}
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);
				if (!addForm) {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
					continue;
				}
				if (EngineLists::AddLeveledObject(curobj, list, addForm, level, count)) {
					logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				}
			}
		}

		if (!line.addedObjectsOnce.empty()) {
			for (const auto& objectToAdd : line.addedObjectsOnce) {
				if (objectToAdd.empty()) { logger::warn("leveled list {:08X}: empty addedObjectsOnce entry", curobj->formID); continue; }
				uint16_t level = 1;
				uint16_t count = 1;

				const std::string& addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					if (!parseUInt16(objectToAdd[1], "add-once level", level)) { continue; }
				}
				if (objectToAdd.size() > 2) {
					if (!parseUInt16(objectToAdd[2], "add-once count", count)) { continue; }
				}
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);
				if (!addForm) {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
					continue;
				}

				bool exists = false;
				EngineLists::ForEachLeveledObject(list, [&](RE::LEVELED_OBJECT& obj) {
					exists = exists || obj.form == addForm;
				});
				if (exists) {
					logger::debug(FMT_STRING("leveled Lists {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
				}
				else if (EngineLists::AddLeveledObject(curobj, list, addForm, level, count)) {
					logger::debug(FMT_STRING("leveled Lists {:08X} added once Form {:08X}"), curobj->formID, addForm->formID);
				}
			}
		}

		if (!line.llglobal.empty() && line.llglobal != "none") {
			if (toLowerCase(line.llglobal) != "null") {
				RE::TESForm* addForm = GetFormFromIdentifier(line.llglobal);
				if (addForm && addForm->As<RE::TESGlobal>()) {
					curobj->chanceGlobal = addForm->As<RE::TESGlobal>();
					logger::debug(FMT_STRING("leveled lists {:08X} added global variable chance {:08X}"), curobj->formID, addForm->formID);
				}
			}
			else {
				curobj->chanceGlobal = nullptr;
				logger::debug(FMT_STRING("leveled lists {:08X} removed global variable chance"), curobj->formID);
			}
		}

		if (!line.chanceNone.empty() && line.chanceNone != "none") {
			std::uint16_t chance = 0;
			if (parseUInt16(line.chanceNone, "chanceNone", chance)) {
				curobj->chanceNone = static_cast<int8_t>(chance);
				logger::debug(FMT_STRING("leveled Lists {:08X} set chance of none to {}"), curobj->formID, curobj->chanceNone);
			}
		}

		if (!line.objectMultCount.empty() && curobj->numEntries > 0) {
			for (const auto& objectToreplace : line.objectMultCount) {
				if (objectToreplace.size() < 2) {
					logger::warn(FMT_STRING("leveled list {:08X}: invalid objectMultCount entry, expected form and multiplier"), curobj->formID);
					continue;
				}
				float multiplier = 1.0f;
				if (!parseFloat(objectToreplace[1], "objectMultCount multiplier", multiplier)) { continue; }
				RE::TESForm* formToReplace = GetFormFromIdentifier(objectToreplace[0]);
				if (!formToReplace) {
					continue;
				}
				int count = 0;
				EngineLists::ForEachLeveledObject(list, [&](RE::LEVELED_OBJECT& obj) {
					if (obj.form == formToReplace) {
						const auto scaled = std::ceil(static_cast<double>(obj.count) * multiplier);
						obj.count = static_cast<std::uint16_t>(std::clamp(scaled, 0.0, static_cast<double>(std::numeric_limits<std::uint16_t>::max())));
						count++;
					}
				});
				if (count > 0) {
					logger::debug(FMT_STRING("leveled Lists {:08X} all matching forms of {:08X} have their count multiplied by {}. Total changes: {}"), curobj->formID, formToReplace->formID, multiplier, count);
				}
			}
		}
	}

}
