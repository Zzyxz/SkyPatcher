#include "encounterzone.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace ENCOUNTERZONE
{
	namespace
	{
#define EZ_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kEncounterZoneKeys[] = {
			{ "filterByEncounterZones", sp::FieldKind::List, EZ_F(object) },
			{ "filterByEncounterZonesExcluded", sp::FieldKind::List, EZ_F(objectExcluded) },
			{ "minLevel", sp::FieldKind::Value, EZ_F(minLevel) },
			{ "minLevelAdd", sp::FieldKind::Value, EZ_F(minLevelAdd) },
			{ "minLevelMult", sp::FieldKind::Value, EZ_F(minLevelMult) },
			{ "maxLevel", sp::FieldKind::Value, EZ_F(maxLevel) },
			{ "maxLevelAdd", sp::FieldKind::Value, EZ_F(maxLevelAdd) },
			{ "maxLevelMult", sp::FieldKind::Value, EZ_F(maxLevelMult) },
			{ "location", sp::FieldKind::Value, EZ_F(location) },
			{ "filterByModNames", sp::FieldKind::ListAssign, EZ_F(modNames) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, EZ_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, EZ_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, EZ_F(filterByEditorIdContainsExcluded) },
		};

#undef EZ_F

		const sp::KeyTable& encounterZoneKeyTable()
		{
			static const sp::KeyTable table(kEncounterZoneKeys, std::size(kEncounterZoneKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		encounterZoneKeyTable().parse(line, &l);
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSEncounterZone* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::EncounterZone) {
						curobj = (RE::BGSEncounterZone*)currentform;
						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}

			if (!line.object.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				continue;
			}

			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				bool found = false;
				bool contains = false;
				bool containsOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!found && !line.filterByEditorIdContains.empty() ) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					found = true;
				}

				

				if (line.object.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}



				if (!line.objectExcluded.empty() && found) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::EncounterZone)) {
						found = false;
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found ) {
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

	void* patch(const ENCOUNTERZONE::patch_instruction& line, RE::BGSEncounterZone* curobj)
	{


		if (!line.minLevel.empty() && line.minLevel != "none") {
			try {
				int val = std::stoi(line.minLevel);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevel out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					curobj->data.minLevel = static_cast<int8_t>(val);
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed minLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.minLevel);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.minLevelAdd.empty() && line.minLevelAdd != "none") {
			try {
				int val = std::stoi(line.minLevelAdd);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevelAdd value out of bounds"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.minLevel + val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.minLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(added) minLevel {}"),
						curobj->formID,
						clib_util::editorID::get_editorID(curobj),
						curobj->data.minLevel);
				}
			} catch (const std::exception&) {
			}
		}


		if (!line.minLevelMult.empty() && line.minLevelMult != "none") {
			const auto mult = tryParseFloat(line.minLevelMult);
			if (!mult || !std::isfinite(*mult)) {
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} minLevelMult invalid"), curobj->formID, clib_util::editorID::get_editorID(curobj));
			} else {
				const double product = static_cast<double>(curobj->data.minLevel) * static_cast<double>(*mult);
				const double result = std::clamp(std::round(product), static_cast<double>(INT8_MIN), static_cast<double>(INT8_MAX));
				curobj->data.minLevel = static_cast<int8_t>(result);

				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(mult) minLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.minLevel);
			}
		}

		if (!line.maxLevel.empty() && line.maxLevel != "none") {
			try {
				int val = std::stoi(line.maxLevel);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevel out of bound"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					curobj->data.maxLevel = static_cast<int8_t>(val);
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed maxLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.maxLevel);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxLevelAdd.empty() && line.maxLevelAdd != "none") {
			try {
				int val = std::stoi(line.maxLevelAdd);
				if (val < INT8_MIN || val > INT8_MAX) {
					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevelAdd value out of bounds"), curobj->formID, clib_util::editorID::get_editorID(curobj));
				} else {
					int result = curobj->data.maxLevel + val;
					result = clamp(result, static_cast<int>(INT8_MIN), static_cast<int>(INT8_MAX));
					curobj->data.maxLevel = static_cast<int8_t>(result);

					logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(added) maxLevel {}"),
						curobj->formID,
						clib_util::editorID::get_editorID(curobj),
						curobj->data.maxLevel);
				}
			} catch (const std::exception&) {
			}
		}

		if (!line.maxLevelMult.empty() && line.maxLevelMult != "none") {
			const auto mult = tryParseFloat(line.maxLevelMult);
			if (!mult || !std::isfinite(*mult)) {
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} maxLevelMult invalid"), curobj->formID, clib_util::editorID::get_editorID(curobj));
			} else {
				const double product = static_cast<double>(curobj->data.maxLevel) * static_cast<double>(*mult);
				double       result = std::clamp(std::round(product), static_cast<double>(INT8_MIN), static_cast<double>(INT8_MAX));
				if (curobj->data.maxLevel > 0 && *mult > 0.0f && result < 1.0) {
					result = 1.0;
				}
				curobj->data.maxLevel = static_cast<int8_t>(result);

				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} changed(mult) maxLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.maxLevel);
			}
		}

		const auto isSet = [](const std::string& value) { return !value.empty() && value != "none"; };
		const bool minChanged = isSet(line.minLevel) || isSet(line.minLevelAdd) || isSet(line.minLevelMult);
		const bool maxChanged = isSet(line.maxLevel) || isSet(line.maxLevelAdd) || isSet(line.maxLevelMult);
		if ((minChanged || maxChanged) && curobj->data.maxLevel > 0 && curobj->data.minLevel > curobj->data.maxLevel) {
			logger::warn(FMT_STRING("encounterzone formid: {:08X} {} minLevel {} is above maxLevel {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.minLevel, curobj->data.maxLevel);
		}

		if (!line.location.empty() && line.location != "none") {
			RE::TESForm*		currentform = nullptr;
			std::string			string_form = line.location;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Location) {
				curobj->data.location = (RE::BGSLocation*)currentform;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} set location {:08X} {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), curobj->data.location->formID, clib_util::editorID::get_editorID(curobj->data.location));
			} else if (toLowerCase(line.location) == "null") {
				curobj->data.location = nullptr;
				logger::debug(FMT_STRING("encounterzone formid: {:08X} {} removed location"), curobj->formID, clib_util::editorID::get_editorID(curobj) );
			}
		}

		return nullptr;
	}

}
