#include "reference.h"
#include "parsetable.h"
#include <utility.h>
#include <unordered_set>

namespace REFERENCE
{

	RefrData rData;

	namespace
	{
#define REFR_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kReferenceKeys[] = {
			{ "filterByRefs", sp::FieldKind::List, REFR_F(objects) },
			{ "filterByRefsExcluded", sp::FieldKind::List, REFR_F(objectExcluded) },
			{ "replaceBaseObject", sp::FieldKind::Value, REFR_F(baseobject) },
			{ "disable", sp::FieldKind::Value, REFR_F(disable) },
		};

#undef REFR_F

		const sp::KeyTable& referenceKeyTable()
		{
			static const sp::KeyTable table(kReferenceKeys, std::size(kReferenceKeys));
			return table;
		}

		bool isSafeReplacementBase(const RE::TESForm* form)
		{
			switch (form->GetFormType()) {
			case RE::FormType::Sound:
			case RE::FormType::Scroll:
			case RE::FormType::Activator:
			case RE::FormType::TalkingActivator:
			case RE::FormType::Armor:
			case RE::FormType::Book:
			case RE::FormType::Container:
			case RE::FormType::Door:
			case RE::FormType::Ingredient:
			case RE::FormType::Light:
			case RE::FormType::Misc:
			case RE::FormType::Apparatus:
			case RE::FormType::Static:
			case RE::FormType::StaticCollection:
			case RE::FormType::MovableStatic:
			case RE::FormType::Tree:
			case RE::FormType::Flora:
			case RE::FormType::Furniture:
			case RE::FormType::Weapon:
			case RE::FormType::Ammo:
			case RE::FormType::KeyMaster:
			case RE::FormType::AlchemyItem:
			case RE::FormType::IdleMarker:
			case RE::FormType::SoulGem:
			case RE::FormType::TextureSet:
				return true;
			default:
				return false;
			}
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		referenceKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> excluded;
			for (const auto& excludedString : line.objectExcluded) {
				if (const auto excludedID = BuildFormIDFromIdentifier(excludedString); excludedID != 0x0) {
					excluded.insert(excludedID);
				}
			}
			if (!line.objects.empty()) {
				for (const auto& npcstring : line.objects) {
					std::string string_form = npcstring;
					auto curFormID = BuildFormIDFromIdentifier(string_form);
					if (curFormID != 0x0 && !excluded.contains(curFormID)) {
						patch(line, curFormID);
					}
				}
			}
		}
	}

	RefrData readConfig(const std::string& folder)
	{
		rData.refrDataMap.clear();
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
		return rData;
	}

	void* patch(const REFERENCE::patch_instruction& line, RE::FormID curobj)
	{
		if (!line.baseobject.empty() && line.baseobject != "none") {

			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.baseobject;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->IsBoundObject() && isSafeReplacementBase(currentform)) {
				rData.refrDataMap[curobj].replaceWith = (RE::TESBoundObject*)currentform;
				logger::debug(FMT_STRING("refr formid to replace: {:08X} "), curobj );
			} else if (currentform && currentform->IsBoundObject()) {
				logger::warn(FMT_STRING("refr formid: {:08X} replaceBaseObject {:08X} skipped, this kind of base object can not be used for a placed object"), curobj, currentform->formID);
			}

			
		}


		if (IsSwitchOn(line.disable)) {
			rData.refrDataMap[curobj].disable = true;
			logger::debug(FMT_STRING("refr formid: {:08X} disabled"), curobj);
		}
		


		
		
		return nullptr;
	}
}
