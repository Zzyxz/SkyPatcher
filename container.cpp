#include "container.h"
#include "enginelists.h"
#include "formfilter.h"
#include "parsetable.h"

#include <unordered_set>
namespace CONTAINER
{

	std::list<line_content> hooktokens;
	int                     iAllowLeveledListsAddedToContainers = 1;

	namespace
	{
#define CONT_F(member) static_cast<std::uint16_t>(offsetof(line_content, member))

		constexpr sp::KeySpec kContainerKeys[] = {
			{ "filterByContainers", sp::FieldKind::List, CONT_F(containerObjects) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, CONT_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, CONT_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, CONT_F(filterByEditorIdContainsExcluded) },
			{ "addToContainers", sp::FieldKind::Arr2D, CONT_F(addedContainerObjects) },
			{ "replaceInContainers", sp::FieldKind::Arr2D, CONT_F(objectsToReplace) },
			{ "addOnceToContainers", sp::FieldKind::Arr2D, CONT_F(addedContainerObjectsOnce) },
			{ "removeFromContainers", sp::FieldKind::List, CONT_F(removedContainerObjects) },
			{ "removeContainerObjectsByCount", sp::FieldKind::Arr2D, CONT_F(removeContainerObjectsByCount) },
			{ "clear", sp::FieldKind::Value, CONT_F(clear) },
			{ "removeContainerObjectsByKeywords", sp::FieldKind::List, CONT_F(removeContainerObjectsByKeyword) },
			{ "objectMultCount", sp::FieldKind::Arr2D, CONT_F(objectMultCount) },
			{ "filterByModNames", sp::FieldKind::ListAssign, CONT_F(modNames) },
		};

#undef CONT_F

		const sp::KeyTable& containerKeyTable()
		{
			static const sp::KeyTable table(kContainerKeys, std::size(kContainerKeys));
			return table;
		}

		bool NamesPlugin(const std::string& a_name)
		{
			const auto lower = toLowerCase(a_name);
			return lower.find(".esp") != std::string::npos || lower.find(".esm") != std::string::npos || lower.find(".esl") != std::string::npos;
		}

		bool ParseAddCount(const std::string& a_text, std::string_view a_key, std::int32_t& a_count)
		{
			try {
				const auto value = std::stoi(a_text);
				if (value <= 0) {
					logger::warn(FMT_STRING("Count for {} must be above 0, skipping entry: {}"), a_key, a_text);
					return false;
				}
				a_count = value;
				return true;
			} catch (...) {
				logger::warn(FMT_STRING("Invalid count for {}: {}"), a_key, a_text);
				return false;
			}
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;
		containerKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESObjectCONT>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;

			if (!line.containerObjects.empty()) {
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectCONT* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Container) {
						object = (RE::TESObjectCONT*)currentform;
						if (patched.insert(object->formID).second) {
							patchContainer(line, object);
						}
					}
				}
			}

			if (!line.containerObjects.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, formfilter::noStrings()
			};

			for (const auto& curobj : ObjectArray) {
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!patched.contains(curobj->formID) && findObject(line, view, curobj)) {
					patchContainer(line, curobj);
				}

			}





		}
	}

	void* readConfigHook(const std::string& folder)
	{
		hooktokens.clear();
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
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string   line;
								std::ifstream infile;

								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction(line));
								}
								infile.close();
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

								if (NamesPlugin(fileName)) {
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



	void patchContainer(const CONTAINER::line_content& line, RE::TESObjectCONT* curobj)
	{


		if (IsSwitchOn(line.clear)) {
			try {
				EngineLists::ClearContainer(*curobj);
				logger::debug(FMT_STRING("container formid: {:08X} cleared all objects"), curobj->formID);
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.removedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removedContainerObjects) {
				std::string         addFormStr = objectToAdd;
				auto* resolvedForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedForm ? resolvedForm->As<RE::TESBoundObject>() : nullptr;
				if (addForm) {
					EngineLists::RemoveContainerObjectInstances(*curobj, addForm);
					logger::debug(FMT_STRING("container {:08X} removed Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container {:08X} Form not found: {}"), curobj->formID, objectToAdd);
				}
			}
		}

		if (!line.removeContainerObjectsByCount.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByCount) {
				if (objectToAdd.size() < 2) { logger::warn("container {:08X}: invalid removeContainerObjectsByCount entry, expected 2 values", curobj->formID); continue; }
				std::string addFormStr = objectToAdd[0];
				if (objectToAdd.size() < 2) {
					logger::warn(FMT_STRING("removeContainerObjectsByCount is missing its count, skipping entry: {}"), addFormStr);
					continue;
				}
				std::int32_t count = 0;
				try {
					count = std::stoi(objectToAdd[1]);
				} catch (...) {
					logger::warn(FMT_STRING("Invalid count for removeContainerObjectsByCount: {}"), objectToAdd[1]);
					continue;
				}
				auto* resolvedForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedForm ? resolvedForm->As<RE::TESBoundObject>() : nullptr;

				if (addForm) {
					const auto removed = EngineLists::RemoveContainerObjectWithCount(*curobj, addForm, count);
					logger::debug(FMT_STRING("container formid: {:08X} removed by count Form {:08X} ({} entries)"), curobj->formID, addForm->formID, removed);
				}
				else {
					logger::critical(FMT_STRING("container formid: {:08X} Form not found: {} (removeContainerObjectsByCount)"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.removeContainerObjectsByKeyword.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByKeyword) {
				std::string         addFormStr = objectToAdd;
				auto* resolvedForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedForm ? resolvedForm->As<RE::BGSKeyword>() : nullptr;
				if (addForm) {
					EngineLists::RemoveContainerObjectsByKeyword(*curobj, addForm);
					logger::debug(FMT_STRING("container formid: {:08X} removed by keyword Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container formid: {:08X} Form not found: {} (remove by keyword)"), curobj->formID, objectToAdd);
				}
			}
		}

		if (!line.addedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjects) {
				if (objectToAdd.empty()) { logger::warn("container {:08X}: empty addedContainerObjects entry", curobj->formID); continue; }
				std::int32_t        count = 1;
				std::string         addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1 && !ParseAddCount(objectToAdd[1], "addToContainers", count)) {
					continue;
				}
				auto* resolvedForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedForm ? resolvedForm->As<RE::TESBoundObject>() : nullptr;
				if (addForm) {
					if (addForm->As<RE::TESLevItem>() && iAllowLeveledListsAddedToContainers == 0) {
						logger::debug(FMT_STRING("container {:08X} add container objects skipped Leveled Item {:08X}"), curobj->formID, addForm->formID);
						continue;
					}
					EngineLists::AddContainerObject(*curobj, addForm, count);
					logger::debug(FMT_STRING("container {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.addedContainerObjectsOnce.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjectsOnce) {
				if (objectToAdd.empty()) { logger::warn("container {:08X}: empty addedContainerObjectsOnce entry", curobj->formID); continue; }
				std::int32_t        count = 1;
				std::string         addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1 && !ParseAddCount(objectToAdd[1], "addOnceToContainers", count)) {
					continue;
				}
				auto* resolvedForm = GetFormFromIdentifier(addFormStr);
				auto* addForm = resolvedForm ? resolvedForm->As<RE::TESBoundObject>() : nullptr;
				if (addForm) {
					if (addForm->As<RE::TESLevItem>() && iAllowLeveledListsAddedToContainers == 0) {
						logger::debug(FMT_STRING("container {:08X} add container objects once skipped Leveled Item {:08X}"), curobj->formID, addForm->formID);
						continue;
					}
					bool formExists = curobj->CountObjectsInContainer(addForm) != 0;
					if (!formExists) {
						EngineLists::AddContainerObject(*curobj, addForm, count);
						logger::debug(FMT_STRING("container {:08X} added once Form  {:08X}"), curobj->formID, addForm->formID);
					}
					else {
						logger::debug(FMT_STRING("container {:08X} Form {:08X} already exists"), curobj->formID, addForm->formID);
					}
				}
				else {
					logger::critical(FMT_STRING("container {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.objectMultCount.empty()) {
			for (const auto& objectToreplace : line.objectMultCount) {
				if (objectToreplace.empty()) {
					logger::warn(FMT_STRING("container {:08X}: empty objectMultCount entry"), curobj->formID);
					continue;
				}
				std::string formToReplaceString = objectToreplace[0];
				if (objectToreplace.size() < 2) {
					logger::warn(FMT_STRING("objectMultCount is missing its multiplier, skipping entry: {}"), formToReplaceString);
					continue;
				}
				float multiplier = 1;
				try {
					multiplier = std::stof(objectToreplace[1]);
				} catch (...) {
					logger::warn(FMT_STRING("Invalid multiplier for objectMultCount: {}"), objectToreplace[1]);
					continue;
				}
				if (!(multiplier >= 0.0f)) {
					logger::warn(FMT_STRING("Multiplier for objectMultCount must be 0 or more, skipping entry: {}"), objectToreplace[1]);
					continue;
				}
				std::size_t  count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				if (formToReplace && formToReplace->IsBoundObject() && curobj->CountObjectsInContainer(formToReplace->As<RE::TESBoundObject>()) > 0) {
					count = EngineLists::MultiplyContainerObjectCounts(*curobj, formToReplace->As<RE::TESBoundObject>(), multiplier);
					logger::debug(FMT_STRING("container {:08X} all matching forms of {:08X} have their count multiplied by {} ({} entries)"), curobj->formID, formToReplace->formID, multiplier, count);
				}
			}
		}

		if (!line.objectsToReplace.empty()) {
			for (const auto& objectToAdd : line.objectsToReplace) {
				if (objectToAdd.size() < 2) { logger::warn("container {:08X}: invalid objectsToReplace entry, expected 2 values", curobj->formID); continue; }
				if (objectToAdd.size() < 2) {
					logger::warn(FMT_STRING("replaceInContainers needs <oldForm>~<newForm>, skipping entry: {}"), objectToAdd[0]);
					continue;
				}
				std::string         object1 = objectToAdd[0];
				std::string         object2 = objectToAdd[1];
				auto* resolvedOldObject = GetFormFromIdentifier(object1);
				auto* resolvedNewObject = GetFormFromIdentifier(object2);
				auto* oldObject = resolvedOldObject ? resolvedOldObject->As<RE::TESBoundObject>() : nullptr;
				auto* newObject = resolvedNewObject ? resolvedNewObject->As<RE::TESBoundObject>() : nullptr;

				if (oldObject && newObject && newObject->As<RE::TESLevItem>() && iAllowLeveledListsAddedToContainers == 0) {
					logger::debug(FMT_STRING("container {:08X} replace skipped Leveled Item {:08X}"), curobj->formID, newObject->formID);
					continue;
				}

				if (oldObject && newObject) {
					replaceObjectInContainer(*curobj, oldObject, newObject);
					logger::debug(FMT_STRING("container formid: {:08X} replaced Form {:08X} with Form {:08X} "), curobj->formID, oldObject->formID, newObject->formID);
				}
			}
		}





	}


	bool findObject(const line_content& line, const formfilter::CommonFilterView& view, RE::TESObjectCONT* curobj)
	{
		bool found = false;
		bool contains = false;
		bool containsOr = false;

		if (!found && !line.filterByEditorIdContains.empty()) {
			contains = formfilter::matchesEditorIdAnd(view, curobj);
		}
		else {
			contains = true;
		}

		if (!found && !line.filterByEditorIdContainsOr.empty()) {
			containsOr = formfilter::matchesEditorIdOr(view, curobj);
		}
		else {
			containsOr = true;
		}

		if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
			found = true;
		}

		if (!found && line.containerObjects.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
			found = true;
		}

		if (!line.filterByEditorIdContainsExcluded.empty() && found) {
			if (formfilter::hasExcludedEditorId(view, curobj)) {
				found = false;
			}
		}

		return found;

	}

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x13, &Hook_InitItemImpl);
	}

	void LoadRaceDataHook::Hook_InitItemImpl(RE::TESObjectCONT* curobj)
	{
		const bool wasInitialized = curobj->IsInitialized();
		func(curobj);
		if (!wasInitialized) {
			logger::debug("CONT {:08X} {} {}", curobj->formID, curobj->fullName, curobj->GetFormEditorID());

			for (const auto& line : hooktokens) {
				bool found = false;
				if (curobj->IsDeleted()) {
					return;
				}

				const formfilter::CommonFilterView view{
					line.modNames, formfilter::noStrings(), formfilter::noStrings(),
					formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
					line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
					line.filterByEditorIdContainsExcluded, formfilter::noStrings()
				};

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!line.containerObjects.empty()) {
					for (const auto& npcstring : line.containerObjects) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							if (curobj->formID == currentform->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (found || findObject(line, view, curobj))
					patchContainer(line, curobj);
			}

		}
	}


}
