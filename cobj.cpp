#include "cobj.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace COBJ
{
	// ==================================================================
	// PHASE 2 -- COBJ's key table. One row per upstream extract*() call,
	// in the same order. See parsetable.h.
	// ==================================================================
	namespace
	{
#define COBJ_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kCobjKeys[] = {
			{ "filterByCobjs", sp::FieldKind::List, COBJ_F(object) },
			{ "filterByWorkBenchKeywords", sp::FieldKind::List, COBJ_F(filterWorkbenchKeywords) },
			{ "filterWorkbenchKeywordsExcluded", sp::FieldKind::List, COBJ_F(filterWorkbenchKeywordsExcluded) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, COBJ_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, COBJ_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, COBJ_F(filterByEditorIdContainsExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, COBJ_F(keywordsNew) },
			{ "filterByKeywordsOr", sp::FieldKind::List, COBJ_F(keywordsNewOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, COBJ_F(keywordsNewExcluded) },
			{ "filterByCobjsExcluded", sp::FieldKind::List, COBJ_F(objectExcluded) },
			{ "filterByIngredients", sp::FieldKind::List, COBJ_F(keywords) },
			{ "filterByIngredientsOr", sp::FieldKind::List, COBJ_F(keywordsOr) },
			{ "filterByIngredientsExcluded", sp::FieldKind::List, COBJ_F(filterByRecipeIngredientsExcluded) },
			{ "filterByModNames", sp::FieldKind::ListAssign, COBJ_F(modNames) },
			{ "addToCobjs", sp::FieldKind::Arr2D, COBJ_F(addedContainerObjects) },
			{ "addToCobjsAdjusted", sp::FieldKind::Arr2D, COBJ_F(addedContainerObjectsAdjusted) },
			{ "replaceInCobjs", sp::FieldKind::Arr2D, COBJ_F(replaceObjectInContainer) },
			{ "changeCobjsCountByMult", sp::FieldKind::Arr2D, COBJ_F(changeCobjsCountByMult) },
			{ "changeCobjsCount", sp::FieldKind::Arr2D, COBJ_F(changeCobjsCount) },
			{ "removeFromCobjs", sp::FieldKind::List, COBJ_F(removeInstancesContainerObjects) },
			{ "removeFromCobjsByCount", sp::FieldKind::Arr2D, COBJ_F(removeContainerObjects) },
			{ "clear", sp::FieldKind::Value, COBJ_F(clearIngredients) },
			{ "createdObject", sp::FieldKind::Value, COBJ_F(createdObject) },
			{ "count", sp::FieldKind::Value, COBJ_F(count) },
			{ "countMult", sp::FieldKind::Value, COBJ_F(countMult) },
			{ "workbenchKeyword", sp::FieldKind::Value, COBJ_F(workbenchKeyword) },
			{ "keywordsToAdd", sp::FieldKind::List, COBJ_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, COBJ_F(keywordsToRemove) },
			{ "restrictToKeywords", sp::FieldKind::List, COBJ_F(restrictToKeywords) },
		};

#undef COBJ_F

		const sp::KeyTable& cobjKeyTable()
		{
			static const sp::KeyTable table(kCobjKeys, std::size(kCobjKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		cobjKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                                dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSConstructibleObject>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*                currentform = nullptr;
					RE::BGSConstructibleObject* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::ConstructibleObject) {
						curobj = (RE::BGSConstructibleObject*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterWorkbenchKeywords.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.keywordsNew.empty() && line.keywordsNewOr.empty()) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, cobj) pair.
			// NOTE: cobj's `filterByKeywords` maps to l.keywordsNew (tested
			// against curobj->createdItem), not l.keywords -- l.keywords is
			// `filterByIngredients` and tests container contents, a
			// different semantic the shared helpers do not cover. The view
			// below intentionally binds keywords/keywordsOr/keywordsExcluded
			// to the "New" fields; l.keywords/keywordsOr/
			// filterByRecipeIngredientsExcluded stay on their own inline
			// container-contents code path further down.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywordsNew, line.keywordsNewOr, line.keywordsNewExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool keywordNewAnd = false;
				bool keywordNewOr = false;
				bool contains = false;
				bool containsOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				RE::BGSKeywordForm* createdItemKw = curobj->createdItem ? curobj->createdItem->As<RE::BGSKeywordForm>() : nullptr;

				if (!line.keywordsNew.empty() && createdItemKw) {
					//logger::info("keywords not empty");
					keywordNewAnd = formfilter::matchesKeywordsAnd(view, createdItemKw, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordNewAnd = true;
				}
				if (!line.keywordsNewOr.empty() && createdItemKw) {
					//logger::info("keywords not empty");
					keywordNewOr = formfilter::matchesKeywordsOr(view, createdItemKw, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordNewOr = true;
				}

				if ((!line.keywordsNew.empty() || !line.keywordsNewOr.empty()) && keywordNewAnd && keywordNewOr) {
					//logger::debug(FMT_STRING("Found a matching armor by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.filterWorkbenchKeywords.empty()) {
					for (const auto& keywordstring : line.filterWorkbenchKeywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->benchKeyword && curobj->benchKeyword->formID == keyword->formID) {
								found = true;

								break;
							}
						}
					}
				}

				if (!line.keywords.empty()) {
					//logger::info("filterByRecipeIngredients 1");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm*        currentform = nullptr;
						RE::TESBoundObject* keyword = nullptr;
						//logger::info("filterByRecipeIngredients 2");
						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->IsBoundObject()) {
							//logger::info("filterByRecipeIngredients 3");

							keyword = (RE::TESBoundObject*)currentform;
							if (curobj->requiredItems.CountObjectsInContainer(keyword) > 0) {
								keywordAnd = true;
								//logger::info("filterByRecipeIngredients 4");

							} else {
								keywordAnd = false;
								//	logger::info("filterByRecipeIngredients 5");

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
						RE::TESForm*        currentform = nullptr;
						RE::TESBoundObject* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->IsBoundObject()) {
							keyword = (RE::TESBoundObject*)currentform;

							if (curobj->requiredItems.CountObjectsInContainer(keyword) > 0) {
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

				if (!found && !line.filterByEditorIdContains.empty()) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty()) {
					//logger::info("keywords not empty");
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterWorkbenchKeywords.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.keywordsNew.empty() && line.keywordsNewOr.empty()) {
					found = true;
				}

				if (!line.restrictToKeywords.empty()) {
					bool hasRestrictedKeyword = false;
					for (const auto& npcstring : line.restrictToKeywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;
						std::string     string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (curobj->createdItem && currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;
							if (auto boundObject = curobj->createdItem->As<RE::BGSKeywordForm>(); boundObject && boundObject->HasKeyword(keyword)) {
								hasRestrictedKeyword = true;
							}
						}
					}
					if (hasRestrictedKeyword) {
						//found = true;
					} else {
						found = false;
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (!line.keywordsNewExcluded.empty() && createdItemKw && found) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(view, createdItemKw, nullptr)) {
						found = false;
					}
				}

				if (!line.filterWorkbenchKeywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.filterWorkbenchKeywordsExcluded) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->benchKeyword && curobj->benchKeyword->formID == keyword->formID) {
								found = false;

								break;
							}
						}
					}
				}

				if (!line.filterByRecipeIngredientsExcluded.empty() && found) {
					for (const auto& keywordstring : line.filterByRecipeIngredientsExcluded) {
						RE::TESForm*        currentform = nullptr;
						RE::TESBoundObject* keyword = nullptr;
						std::string         string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->IsBoundObject()) {
							keyword = (RE::TESBoundObject*)currentform;
							if (curobj->requiredItems.CountObjectsInContainer(keyword) > 0) {
								found = false;

								break;
							}
						}
					}
				}

				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::ConstructibleObject)) {
						found = false;
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

	void* patch(const COBJ::patch_instruction& line, RE::BGSConstructibleObject* curobj)
	{


		if (!line.clearIngredients.empty() && line.clearIngredients != "none") {
			try {
				curobj->requiredItems.RemoveAllObjectFromContainer();
				logger::debug(FMT_STRING("cobj formid: {:08X} cleared all ingredients"), curobj->formID);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.removeContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd[0];
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					curobj->requiredItems.RemoveObjectFromContainer(addForm, count);
					logger::debug(FMT_STRING("cobj formid: {:08X} removed Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("cobj formid: {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.removeInstancesContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removeInstancesContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd;
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					auto removedObjectsCount = curobj->requiredItems.RemoveObjectInstancesFromContainer(addForm);
					logger::debug(FMT_STRING("cobj formid: {:08X} removed Form {:08X} count {}"), curobj->formID, addForm->formID, removedObjectsCount);
				} else {
					logger::critical(FMT_STRING("cobj formid: {:08X} Form not found: {}"), curobj->formID, objectToAdd);
				}
			}
		}

		if (!line.addedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd[0];
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					curobj->requiredItems.AddObjectToContainer(addForm, count, nullptr);
					logger::debug(FMT_STRING("cobj formid: {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("cobj formid: {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.addedContainerObjectsAdjusted.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjectsAdjusted) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd[0];
				uint32_t            count = std::stoi(objectToAdd[1]);
				uint32_t            maxAmount = std::stoi(objectToAdd[2]);
				uint32_t            createdItemCount = curobj->data.numConstructed;
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					double proportion = static_cast<double>(createdItemCount) / maxAmount;
					int    finalAmount = std::ceil(count * proportion);
					curobj->requiredItems.AddObjectToContainer(addForm, finalAmount, nullptr);
					logger::debug(FMT_STRING("cobj formid: {:08X} added Form {:08X} with amount {} of {}"), curobj->formID, addForm->formID, finalAmount, maxAmount);
				} else {
					logger::critical(FMT_STRING("cobj formid: {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.replaceObjectInContainer.empty()) {
			for (const auto& objectToAdd : line.replaceObjectInContainer) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         object1 = objectToAdd[0];
				std::string         object2 = (objectToAdd[1]);
				RE::TESBoundObject* oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(object1);
				RE::TESBoundObject* newObject = (RE::TESBoundObject*)GetFormFromIdentifier(object2);

				if (oldObject && newObject) {
					curobj->requiredItems.ReplaceObjectInContainer(oldObject, newObject);
					logger::debug(FMT_STRING("cobj formid: {:08X} replaced Form {:08X} with Form {:08X} "), curobj->formID, oldObject->formID, newObject->formID);
				}
			}
		}

		for (const auto& entry : line.changeCobjsCount) {
			std::string identifier = entry[0];  // "null" oder "Skyrim.esm|12345678"
			std::string multStr = entry[1];     // z. B. "0.01"

			RE::TESBoundObject* oldObject = nullptr;
			if (!identifier.empty() && identifier != "null") {
				oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(identifier);
				if (!oldObject) {
					logger::warn(FMT_STRING("Invalid object identifier: {}"), identifier);
					continue;
				}
			}

			int32_t newCount = 1;
			try {
				newCount = std::stoi(multStr);
			} catch (...) {
				logger::warn(FMT_STRING("Invalid count for cobj change: {}"), multStr);
				continue;
			}

			if (curobj->requiredItems.ChangeCOBJsCount(oldObject, newCount)) {
				if (oldObject) {
					logger::debug(FMT_STRING("cobj formid: {:08X} changed count for {:08X} to {}"),
						curobj->formID, oldObject->formID, newCount);
				} else {
					logger::debug(FMT_STRING("cobj formid: {:08X} changed count for ALL objects by {} {}"),
						curobj->formID, newCount, clib_util::editorID::get_editorID(curobj));
				}
			}
		}

		for (const auto& entry : line.changeCobjsCountByMult) {
			std::string identifier = entry[0];  // "null" oder "Skyrim.esm|12345678"
			std::string multStr = entry[1];     // z. B. "0.01"

			RE::TESBoundObject* oldObject = nullptr;
			if (!identifier.empty() && identifier != "null") {
				oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(identifier);
				if (!oldObject) {
					logger::warn(FMT_STRING("Invalid object identifier: {}"), identifier);
					continue;
				}
			}

			float multiplier = 1.0f;
			try {
				multiplier = std::stof(multStr);
			} catch (...) {
				logger::warn(FMT_STRING("Invalid multiplier: {}"), multStr);
				continue;
			}

			if (multiplier <= 0.0f) {
				logger::warn(FMT_STRING("Multiplier must be > 0: {}"), multStr);
				continue;
			}

			if (curobj->requiredItems.ChangeCOBJsCountByMult(oldObject, multiplier)) {
				if (oldObject) {
					logger::debug(FMT_STRING("cobj formid: {:08X} changed/multiplied count for {:08X} by {}"),
						curobj->formID, oldObject->formID, multiplier);
				} else {
					logger::debug(FMT_STRING("cobj formid: {:08X} changed/multiplied count for ALL objects to {} {}"),
						curobj->formID, multiplier, clib_util::editorID::get_editorID(curobj));
				}
			}
		}

		if (!line.workbenchKeyword.empty() && line.workbenchKeyword != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.workbenchKeyword;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				curobj->benchKeyword = (RE::BGSKeyword*)currentform;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			} else if (line.workbenchKeyword == "null") {
				curobj->benchKeyword = nullptr;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to null (none)"), curobj->formID);
			}
		}

		if (!line.createdObject.empty() && line.createdObject != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.createdObject;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->IsBoundObject()) {
				curobj->createdItem = (RE::TESBoundObject*)currentform;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed createdObject {:08X} "), curobj->formID, (currentform)->formID);
			} else if (line.createdObject == "null") {
				curobj->createdItem = nullptr;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed createdObject null (none)"), curobj->formID);
			}
		}

		if (!line.count.empty() && line.count != "none") {
			try {
				curobj->data.numConstructed = std::stoi(line.count);
				logger::debug(FMT_STRING("cobj formid: {:08X} changed count {}"), curobj->formID, curobj->data.numConstructed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.countMult.empty() && line.countMult != "none") {
			try {
				curobj->data.numConstructed = curobj->data.numConstructed * std::stof(line.countMult);
				logger::debug(FMT_STRING("cobj formid: {:08X} multiplied count {}"), curobj->formID, curobj->data.numConstructed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (curobj->createdItem && currentform && currentform->formType == RE::FormType::Keyword && curobj->createdItem->As<RE::BGSKeywordForm>()) {
					curobj->createdItem->As<RE::BGSKeywordForm>()->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("cobj formid: {:08X} {} object {} added keyword {:08X} {} "), curobj->formID, clib_util::editorID::get_editorID(curobj), clib_util::editorID::get_editorID(curobj->createdItem), ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (curobj->createdItem && currentform && currentform->formType == RE::FormType::Keyword && curobj->createdItem->As<RE::BGSKeywordForm>()) {
					curobj->createdItem->As<RE::BGSKeywordForm>()->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("cobj formid: {:08X} {} object {} removed keyword {:08X} {} "), curobj->formID, clib_util::editorID::get_editorID(curobj), clib_util::editorID::get_editorID(curobj->createdItem), ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}
	}

}
