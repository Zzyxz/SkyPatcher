#include "container.h"
namespace CONTAINER
{

	std::list<line_content> hooktokens;
	int                     iAllowLeveledListsAddedToContainers = 0;


	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;


		extractData(line, "filterByContainers\\s*=([^:]+)", l.containerObjects);

		//std::regex  addContainer_regex("addToContainers\\s*=([^:]+)", regex::icase);
		//std::smatch addContainer_match;
		//std::regex_search(line, addContainer_match, addContainer_regex);
		//std::vector<std::string> addContainer;
		//if (addContainer_match.empty() || addContainer_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string              valueLine = addContainer_match[1].str();
		//	std::vector<std::string> arr;

		//	// exclude the addToLL= part from the first string
		//	size_t      startPos = valueLine.find("=") + 1;
		//	size_t      pos = 0;
		//	std::string token;
		//	while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
		//		token = valueLine.substr(startPos, pos - startPos);
		//		token = trim(token);  // remove leading and trailing white spaces
		//		arr.push_back(token);
		//		startPos = pos + 1;
		//	}
		//	token = valueLine.substr(startPos);
		//	token = trim(token);  // remove leading and trailing white spaces
		//	arr.push_back(token);

		//	std::vector<std::vector<std::string>> arr2D(arr.size());

		//	for (int i = 0; i < arr.size(); i++) {
		//		std::vector<std::string> splitArr;
		//		size_t                   innerPos = 0;
		//		std::string              innerToken;
		//		while ((innerPos = arr[i].find("~")) != std::string::npos) {
		//			innerToken = arr[i].substr(0, innerPos);
		//			innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//			splitArr.push_back(innerToken);
		//			arr[i].erase(0, innerPos + 1);
		//		}
		//		innerToken = arr[i];
		//		innerToken = trim(innerToken);  // remove leading and trailing white spaces
		//		splitArr.push_back(innerToken);
		//		arr2D[i] = splitArr;
		//	}
		//	l.addedContainerObjects = arr2D;
		//}
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractToArr2D(line, "addToContainers\\s*=([^:]+)", l.addedContainerObjects);
		extractToArr2D(line, "replaceInContainers\\s*=([^:]+)", l.objectsToReplace);
		extractToArr2D(line, "addOnceToContainers\\s*=([^:]+)", l.addedContainerObjectsOnce);

		extractData(line, "removeFromContainers\\s*=([^:]+)", l.removedContainerObjects);
		extractToArr2D(line, "removeContainerObjectsByCount\\s*=([^:]+)", l.removeContainerObjectsByCount);
		extractValueString(line, "clear\\s*=([^:]+)", l.clear);
		extractData(line, "removeContainerObjectsByKeywords\\s*=([^:]+)", l.removeContainerObjectsByKeyword);
		extractToArr2D(line, "objectMultCount\\s*=([^:]+)", l.objectMultCount);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectCONT*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectCONT>();

		for (const auto& line : tokens) {

			if (!line.containerObjects.empty()) {
				//logger::info("npc not empty");
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectCONT* object = nullptr;
					std::string        string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Container) {
						object = (RE::TESObjectCONT*)currentform;
						patchContainer(line, object);
					}
				}
			}

			if (!line.containerObjects.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}



			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");

					//logger::info(FMT_STRING("Its a form! {:08X}"), curobj->formID);
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				if (line.containerObjects.empty() && findObject(line, curobj)) {
					patchContainer(line, curobj);
				}

			}





		}
	}

	void* readConfigHook(const std::string& folder)
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
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string   line;
								std::ifstream infile;

								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									hooktokens.push_back(create_patch_instruction(line));
								}
								infile.close();
								//process_patch_instructions(tokens);
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
									if (line[0] == skipChar) {
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
	}



	void patchContainer(CONTAINER::line_content line, RE::TESObjectCONT* curobj)
	{


		if (!line.clear.empty() && line.clear != "none") {
			try {
				curobj->RemoveAllObjectFromContainer();
				logger::debug(FMT_STRING("container formid: {:08X} cleared all objects"), curobj->formID);
			}
			catch (const std::invalid_argument& e) {
			}
		}

		if (!line.removedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd;
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);
				if (addForm) {
					curobj->RemoveObjectInstancesFromContainer(addForm);
					logger::debug(FMT_STRING("container {:08X} removed Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container {:08X} Form not found: {}"), curobj->formID, objectToAdd);
				}
			}
		}

		if (!line.removeContainerObjectsByCount.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByCount) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd[0];
				uint32_t            count = std::stoi(objectToAdd[1]);
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);

				if (addForm) {
					curobj->RemoveObjectFromContainer(addForm, count);
					logger::debug(FMT_STRING("container formid: {:08X} removed by count Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container formid: {:08X} Form not found: {} (removeContainerObjectsByCount)"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.removeContainerObjectsByKeyword.empty()) {
			for (const auto& objectToAdd : line.removeContainerObjectsByKeyword) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         addFormStr = objectToAdd;
				RE::BGSKeyword* addForm = (RE::BGSKeyword*)GetFormFromIdentifier(addFormStr);
				if (addForm) {
					curobj->RemoveObjectFromContainerByKeyword(addForm);
					logger::debug(FMT_STRING("container formid: {:08X} removed by keyword Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container formid: {:08X} Form not found: {} (remove by keyword)"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.addedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				uint32_t            count = 1;
				std::string         addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					count = std::stoi(objectToAdd[1]);
				}
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);
				if (addForm) {
					if (addForm->As<RE::TESLevItem>() && iAllowLeveledListsAddedToContainers == 0) {
						logger::debug(FMT_STRING("container {:08X} add container objects skipped Leveled Item {:08X}"), curobj->formID, addForm->formID);
						continue;
					}
					curobj->AddObjectToContainer(addForm, count, nullptr);
					logger::debug(FMT_STRING("container {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				}
				else {
					logger::critical(FMT_STRING("container {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
		}

		if (!line.addedContainerObjectsOnce.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjectsOnce) {
				uint32_t            count = 1;
				std::string         addFormStr = objectToAdd[0];
				if (objectToAdd.size() > 1) {
					count = std::stoi(objectToAdd[1]);
				}
				RE::TESBoundObject* addForm = (RE::TESBoundObject*)GetFormFromIdentifier(addFormStr);
				if (addForm) {
					if (addForm->As<RE::TESLevItem>() && iAllowLeveledListsAddedToContainers == 0) {
						logger::debug(FMT_STRING("container {:08X} add container objects once skipped Leveled Item {:08X}"), curobj->formID, addForm->formID);
						continue;
					}
					// Check if addForm already exists in the container
					bool formExists = curobj->CountObjectsInContainer(addForm) != 0;
					if (!formExists) {
						// addForm not found in the container, add it
						curobj->AddObjectToContainer(addForm, count, nullptr);
						logger::debug(FMT_STRING("container {:08X} added once Form  {:08X}"), curobj->formID, addForm->formID);
					}
					else {
						// addForm already exists in the container, log a message or handle it accordingly
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
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string formToReplaceString = objectToreplace[0];
				float multiplier = 1;
				if (objectToreplace.size() > 1) {
					multiplier = std::stof(objectToreplace[1]);
				}
				int          count = 0;
				RE::TESForm* formToReplace = GetFormFromIdentifier(formToReplaceString);
				// Replace logic for the entire vector
				if (formToReplace && formToReplace->IsBoundObject() && curobj->CountObjectsInContainer(formToReplace->As<RE::TESBoundObject>()) > 0) {
					count = curobj->MultObjectCountInContainer(formToReplace->As<RE::TESBoundObject>(), multiplier);
					logger::debug(FMT_STRING("container {:08X} all matching forms of {:08X} have their count multiplied by {}"), curobj->formID, formToReplace->formID, multiplier, count);
				}
			}
		}

		if (!line.objectsToReplace.empty()) {
			for (const auto& objectToAdd : line.objectsToReplace) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string         object1 = objectToAdd[0];
				std::string         object2 = objectToAdd[1];
				RE::TESBoundObject* oldObject = (RE::TESBoundObject*)GetFormFromIdentifier(object1);
				RE::TESBoundObject* newObject = (RE::TESBoundObject*)GetFormFromIdentifier(object2);

				if (oldObject && newObject) {
					curobj->ReplaceObjectInContainer(oldObject, newObject);
					logger::debug(FMT_STRING("container formid: {:08X} replaced Form {:08X} with Form {:08X} "), curobj->formID, oldObject->formID, newObject->formID);
				}
			}
		}





	}


	bool findObject(line_content line, RE::TESObjectCONT* curobj)
	{
		bool found = false;
		bool keywordAnd = false;
		bool keywordOr = false;
		bool contains = false;
		bool containsOr = false;


		if (!found && !line.filterByEditorIdContains.empty()) {
			for (const auto& editorString : line.filterByEditorIdContains) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					contains = true;
				}
				else {
					contains = false;
					break;
				}
			}
		}
		else {
			contains = true;
		}

		if (!found && !line.filterByEditorIdContainsOr.empty()) {
			//logger::info("keywords not empty");
			for (const auto& editorString : line.filterByEditorIdContainsOr) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					containsOr = true;
					break;
				}
			}
		}
		else {
			containsOr = true;
		}

		if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
			//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
			found = true;
		}

		if (!found && line.containerObjects.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
			found = true;
		}

		if (!line.filterByEditorIdContainsExcluded.empty() && found) {
			//logger::info("factions not empty");
			for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
				if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
					found = false;
					break;
				}
			}
		}



		return found;

	}

	REL::Relocation<decltype(&LoadRaceDataHook::Hook_InitItemImpl)> func;

	void LoadRaceDataHook::InstallHook()
	{
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE[0] };
		func = Vtbl.write_vfunc(0x13, &Hook_InitItemImpl);
		logger::debug("{} Done!", __FUNCTION__);
	}

	bool LoadRaceDataHook::Hook_InitItemImpl(RE::TESObjectCONT* curobj)
	{
		auto result = func(curobj);
		if (result) {
			logger::debug("CONT {:08X} {} {}", curobj->formID, curobj->fullName, curobj->GetFormEditorID());

			for (const auto& line : hooktokens) {
				bool found = false;
				if (curobj->IsDeleted()) {
					return result;
				}

				if (!line.modNames.empty()) {
					bool modFound = false;
					for (auto const& modName : line.modNames) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == false) {
						continue;
					}
				}

				//logger::info("looking up npc {:08X} {} {}", curobj->formID, clib_util::editorID::get_editorID(curobj), string_form);
				if (!line.containerObjects.empty()) {
					//logger::debug("objects size {}", line.object.size());
					for (const auto& npcstring : line.containerObjects) {
						RE::TESForm* currentform = nullptr;
						std::string  string_form = npcstring;
						//logger::info("looking up npc {:08X} {} {}", curobj->formID, clib_util::editorID::get_editorID(curobj), string_form);
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							if (curobj->formID == currentform->formID) {
								found = true;
								break;
							}
						}
					}
				}

				//logger::debug("CONT {:08X} {} {} -> patching {}", curobj->formID, curobj->fullName, curobj->GetFormEditorID(), found);

				if (found || findObject(line, curobj))
					patchContainer(line, curobj);
			}

		}
		return result;
	}


}
