#include "location.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace LOCATION
{

	// ==================================================================
	// PHASE 2 -- LOCATION's key table. One row per upstream extract*()
	// call, in the same order. See parsetable.h and npc.cpp/weapon.cpp
	// for the pattern this follows.
	// ==================================================================
	namespace
	{
#define LOC_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kLocationKeys[] = {
			{ "filterByLocations", sp::FieldKind::List, LOC_F(object) },
			{ "filterByLocationsExcluded", sp::FieldKind::List, LOC_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, LOC_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, LOC_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, LOC_F(keywordsExcluded) },
			{ "fullName", sp::FieldKind::ValueTilde, LOC_F(fullName) },
			{ "keywordsToAdd", sp::FieldKind::List, LOC_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, LOC_F(keywordsToRemove) },
			{ "filterByModNames", sp::FieldKind::ListAssign, LOC_F(modNames) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, LOC_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, LOC_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, LOC_F(filterByEditorIdContainsExcluded) },
			{ "filterByMusicType", sp::FieldKind::ListAssign, LOC_F(filterByMusicType) },
			{ "filterByParentLocation", sp::FieldKind::ListAssign, LOC_F(filterByParentLoc) },
			{ "filterByUnreportedCrimeFaction", sp::FieldKind::ListAssign, LOC_F(filterByUnreportedCrimeFaction) },
			{ "musicType", sp::FieldKind::Value, LOC_F(musicType) },
			{ "unreportedCrimeFaction", sp::FieldKind::Value, LOC_F(unreportedCrimeFaction) },
			{ "parentLocation", sp::FieldKind::Value, LOC_F(parentLoc) },
		};

#undef LOC_F

		const sp::KeyTable& locationKeyTable()
		{
			static const sp::KeyTable table(kLocationKeys, std::size(kLocationKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		locationKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSLocation>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSLocation* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Location) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::BGSLocation*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByParentLoc.empty()) {
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, location) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				//logger::debug("{:08X} editorID {}", curobj->formID, clib_util::editorID::get_editorID(curobj));
				//logger::debug("{:08X} editorID {}", RE::TESForm::LookupByEditorID("Saw01")->formID, "Saw01");
				//logger::info("processing npc");
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

				if (!found && !line.filterByUnreportedCrimeFaction.empty()) {
					for (const auto& factionString : line.filterByUnreportedCrimeFaction) {
						RE::TESForm* currentform = GetFormFromIdentifier(factionString);
						if (currentform && currentform->formType == RE::FormType::Faction) {
							auto* faction = static_cast<RE::TESFaction*>(currentform);
							if (curobj->unreportedCrimeFaction && curobj->unreportedCrimeFaction->formID == faction->formID) {
								found = true;
								break;
							}
						}
					}
				}


				if (!found && !line.filterByParentLoc.empty()) {
					for (const auto& locString : line.filterByParentLoc) {
						RE::TESForm* currentform = GetFormFromIdentifier(locString);
						if (currentform && currentform->formType == RE::FormType::Location) {
							auto* parent = static_cast<RE::BGSLocation*>(currentform);
							if (curobj->parentLoc && curobj->parentLoc->formID == parent->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByMusicType.empty()) {
					for (const auto& musicString : line.filterByMusicType) {
						RE::TESForm* currentform = GetFormFromIdentifier(musicString);
						if (currentform && currentform->formType == RE::FormType::MusicType) {
							auto* music = static_cast<RE::BGSMusicType*>(currentform);
							if (curobj->musicType && curobj->musicType->formID == music->formID) {
								found = true;
								break;
							}
						}
					}
				}

				

				if (!found && !line.filterByEditorIdContains.empty() ) {
					contains = formfilter::matchesEditorIdAnd(view, curobj);
				} else {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					//logger::info("keywords not empty");
					containsOr = formfilter::matchesEditorIdOr(view, curobj);
				} else {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					keywordAnd = formfilter::matchesKeywordsAnd(view, curobj, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					keywordOr = formfilter::matchesKeywordsOr(view, curobj, nullptr);
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByParentLoc.empty()) {
					found = true;
				}



				if (found && !line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Location)) {
						found = false;
						//logger::info("Misc Excluded.");
					}
				}

				// DRIFT (preserved, not fixed): upstream tests `found` twice
				// here (`found && ... && found`); functionally identical to
				// testing it once, kept verbatim. See task report.
				if (found && !line.filterByEditorIdContainsExcluded.empty() && found ) {
					//logger::info("factions not empty");
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (found && !line.keywordsExcluded.empty()) {
					//logger::info("keywords not empty");
					if (formfilter::hasExcludedKeyword(view, curobj, nullptr)) {
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

	void* patch(const LOCATION::patch_instruction& line, RE::BGSLocation* curobj)
	{

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("location formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("location formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}



		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("location formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.musicType.empty() && line.musicType != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.musicType);
			if (currentform && currentform->formType == RE::FormType::MusicType) {
				curobj->musicType = static_cast<RE::BGSMusicType*>(currentform);
				logger::debug(FMT_STRING("location formid: {:08X} changed musicType to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.musicType == "null") {
				curobj->musicType = nullptr;
				logger::debug(FMT_STRING("location {:08X} changed musicType to null"), curobj->formID);
			}
		}

		if (!line.parentLoc.empty() && line.parentLoc != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.parentLoc);
			if (currentform && currentform->formType == RE::FormType::Location) {
				curobj->parentLoc = static_cast<RE::BGSLocation*>(currentform);
				logger::debug(FMT_STRING("location formid: {:08X} changed parentLoc to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.parentLoc == "null") {
				curobj->parentLoc = nullptr;
				logger::debug(FMT_STRING("location formid: {:08X} changed parentLoc to null"), curobj->formID);
			}
		}

		if (!line.unreportedCrimeFaction.empty() && line.unreportedCrimeFaction != "none") {
			RE::TESForm* currentform = GetFormFromIdentifier(line.unreportedCrimeFaction);
			if (currentform && currentform->formType == RE::FormType::Faction) {
				curobj->unreportedCrimeFaction = static_cast<RE::TESFaction*>(currentform);
				logger::debug(FMT_STRING("location {:08X} changed unreportedCrimeFaction to {:08X}"), curobj->formID, currentform->formID);
			} else if (line.unreportedCrimeFaction == "null") {
				curobj->unreportedCrimeFaction = nullptr;
				logger::debug(FMT_STRING("location formid: {:08X} cleared unreportedCrimeFaction (set to null)"), curobj->formID);
			}
		}




	}

}
