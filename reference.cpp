#include "reference.h"
#include <utility.h>

namespace REFERENCE
{

	RefrData rData;

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		

		extractData(line, "filterByRefs\\s*=([^:]+)", l.objects);
		extractData(line, "filterByRefsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractValueString(line, "replaceBaseObject\\s*=([^:]+)", l.baseobject);
		extractValueString(line, "disable\\s*=([^:]+)", l.disable);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		//RE::BSTArray<RE::TESObjectWEAP*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectREFR>();

		for (const auto& line : tokens) {
			//logger::debug("REFR 1 ");
			if (!line.objects.empty()) {
				//logger::debug("REFR 2 ");
				for (const auto& npcstring : line.objects) {
					//logger::debug("REFR 3 ");
					RE::TESForm*       currentform = nullptr;
					//RE::FormID curobj = nullptr;

					std::string string_form = npcstring;
					auto curFormID = BuildFormIDFromIdentifier(string_form);
					if (curFormID != 0x0) {
						//logger::debug("REFR {:08X} ", currentform->formID);
						//curobj = (RE::TESObjectREFR*)currentform;
						patch(line, curFormID);
					}
				}
			}

			//if (!line.objects.empty()  ) {
			//	//logger::info("continue");
			//	continue;
			//}

			
		}
	}

	RefrData readConfig(const std::string& folder)
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
		return rData;
	}

	void* patch(REFERENCE::patch_instruction line, RE::FormID curobj)
	{
		//logger::debug(FMT_STRING("refr 11 formid: {:08X} "), curobj);
		if (!line.baseobject.empty() && line.baseobject != "none") {

			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.baseobject;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->IsBoundObject()) {
				//curobj->data.objectReference = (RE::TESBoundObject*)currentform;
				rData.refrDataMap[curobj].replaceWith = (RE::TESBoundObject*)currentform;
				logger::debug(FMT_STRING("refr formid to replace: {:08X} "), curobj );
			}

			
		}


		if(line.disable == "true"){
			rData.refrDataMap[curobj].disable = true;
			logger::debug(FMT_STRING("refr formid: {:08X} disabled"), curobj);
		}
		


		


	}
}
