#include "faction.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace FACTION
{
	// ==================================================================
	// PHASE 2 -- FACTION's key table. One row per upstream extract*()
	// call, in the same order. See parsetable.h.
	// ==================================================================
	namespace
	{
#define FACTION_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kFactionKeys[] = {
			{ "filterByFactions", sp::FieldKind::List, FACTION_F(object) },
			{ "filterByFactionsExcluded", sp::FieldKind::List, FACTION_F(objectExcluded) },
			{ "setFlags", sp::FieldKind::ListAssign, FACTION_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, FACTION_F(removeFlags) },
			{ "filterByModNames", sp::FieldKind::ListAssign, FACTION_F(modNames) },
		};

#undef FACTION_F

		const sp::KeyTable& factionKeyTable()
		{
			static const sp::KeyTable table(kFactionKeys, std::size(kFactionKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
<<<<<<< HEAD
		factionKeyTable().parse(line, &l);
=======

		extractData(line, "filterByFactions\\s*=([^:]+)", l.object);

		extractData(line, "filterByFactionsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		extractToArr2D(line, "relationshipToAdd\\s*=([^:]+)", l.rsAdd);
		extractToArr2D(line, "relationshipToChange\\s*=([^:]+)", l.rsChange);
		extractData(line, "relationshipToRemove\\s*=([^:]+)", l.rsRemove);

>>>>>>> origin/new-main
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
<<<<<<< HEAD
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESFaction>();
=======
		const auto& ObjectArray = dataHandler->GetFormArray<RE::TESFaction>();
>>>>>>> origin/new-main

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESFaction* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Faction) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESFaction*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty()  ) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, faction) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;



				if (!curobj ||curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}



				if (!found && line.object.empty()  && line.keywords.empty() && line.keywordsOr.empty()) {
					found = true;
				}


				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Faction)) {
						found = false;
						logger::info("Projectile Excluded.");
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
	}

<<<<<<< HEAD
	void* patch(const FACTION::patch_instruction& line, RE::TESFaction* curobj)
=======
	void* patch(const FACTION::patch_instruction& line, RE::TESFaction* curobj)
>>>>>>> origin/new-main
	{


		// Remove Flags
		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				auto lowerRemoveFlag = toLowerCase(std::string(removeFlag));
				if (lowerRemoveFlag == "hiddenfromnpc") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kHiddenFromNPC);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled HiddenFromNPC"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "specialcombat") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kSpecialCombat);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled SpecialCombat"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "playerisexpelled") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kPlayerIsExpelled);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled PlayerIsExpelled"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "playerisenemy") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kPlayerIsEnemy);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled PlayerIsEnemy"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "trackcrime") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kTrackCrime);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled TrackCrime"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_murder") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Murder);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Murder"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_assult") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Assult);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Assult"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_stealing") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Stealing);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Stealing"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ingorescrimes_trespass") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IngoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "donotreportcrimesagainstmembers") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kDoNotReportCrimesAgainstMembers);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled DoNotReportCrimesAgainstMembers"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "crimegold_usedefaults") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kCrimeGold_UseDefaults);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled CrimeGold_UseDefaults"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_pickpocket") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Pickpocket);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Pickpocket"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "vendor") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kVendor);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled Vendor"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "canbeowner") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kCanBeOwner);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled CanBeOwner"), curobj->formID, curobj->fullName);
				} else if (lowerRemoveFlag == "ignorescrimes_werewolf") {
					curobj->data.flags.reset(RE::FACTION_DATA::Flag::kIgnoresCrimes_Werewolf);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag disabled IgnoresCrimes_Werewolf"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		// Set Flags
		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				auto lowerSetFlag = toLowerCase(std::string(setFlag));
				if (lowerSetFlag == "hiddenfromnpc") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kHiddenFromNPC);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled HiddenFromNPC"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "specialcombat") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kSpecialCombat);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled SpecialCombat"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "playerisexpelled") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kPlayerIsExpelled);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled PlayerIsExpelled"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "playerisenemy") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kPlayerIsEnemy);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled PlayerIsEnemy"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "trackcrime") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kTrackCrime);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled TrackCrime"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_murder") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Murder);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Murder"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_assult") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Assult);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Assult"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_stealing") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Stealing);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Stealing"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ingorescrimes_trespass") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_trespass") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIngoresCrimes_Trespass);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Trespass"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "donotreportcrimesagainstmembers") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kDoNotReportCrimesAgainstMembers);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled DoNotReportCrimesAgainstMembers"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "crimegold_usedefaults") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kCrimeGold_UseDefaults);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled CrimeGold_UseDefaults"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_pickpocket") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Pickpocket);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Pickpocket"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "vendor") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kVendor);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled Vendor"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "canbeowner") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kCanBeOwner);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled CanBeOwner"), curobj->formID, curobj->fullName);
				} else if (lowerSetFlag == "ignorescrimes_werewolf") {
					curobj->data.flags.set(RE::FACTION_DATA::Flag::kIgnoresCrimes_Werewolf);
					logger::debug(FMT_STRING("faction formid: {:08X} {} flag enabled IgnoresCrimes_Werewolf"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		
		if (!line.rsAdd.empty()) {
			for (const auto& rs : line.rsAdd) {
				if (rs.size() < 3) {
					continue;
				}

				std::string        factionStr = rs[0];
				int32_t            modifier = std::stoi(rs[1]);
				RE::FIGHT_REACTION fightReaction = GetFightReaction(rs[2]);

				RE::TESForm* form = GetFormFromIdentifier(factionStr);

				if (!form || form->formType != RE::FormType::Faction) {
					continue;
				}

				auto* reaction = new RE::GROUP_REACTION();
				reaction->form = form;
				reaction->reaction = modifier;
				reaction->fightReaction = fightReaction;

				curobj->reactions.push_front(reaction);

				logger::debug(
					FMT_STRING("faction {:08X} {} added relationship to {:08X}, modifier {}, reaction {}"),
					curobj->formID,
					curobj->fullName,
					form->formID,
					modifier,
					static_cast<int>(fightReaction));
			}
		}
		
		if (!line.rsChange.empty()) {
			for (const auto& rs : line.rsChange) {
				if (rs.size() < 3) {
					continue;
				}

				std::string        factionStr = rs[0];
				int32_t            modifier = std::stoi(rs[1]);
				RE::FIGHT_REACTION fightReaction = GetFightReaction(rs[2]);

				RE::TESForm* form = GetFormFromIdentifier(factionStr);

				if (!form || form->formType != RE::FormType::Faction) {
					continue;
				}

				bool found = false;

				for (auto* reaction : curobj->reactions) {
					if (!reaction || reaction->form != form) {
						continue;
					}

					reaction->reaction = modifier;
					reaction->fightReaction = fightReaction;
					found = true;

					logger::debug(
						FMT_STRING("faction {:08X} {} changed relationship to {:08X}, modifier {}, reaction {}"),
						curobj->formID,
						curobj->fullName,
						form->formID,
						modifier,
						static_cast<int>(fightReaction));

					break;
				}

				if (!found) {
					logger::debug(
						FMT_STRING("faction {:08X} {} relationship to {:08X} not found, change skipped"),
						curobj->formID,
						curobj->fullName,
						form->formID);
				}
			}
		}

		if (!line.rsRemove.empty()) {
			for (const auto& factionStr : line.rsRemove) {
				RE::TESForm* form = GetFormFromIdentifier(factionStr);

				if (!form || form->formType != RE::FormType::Faction) {
					continue;
				}

				bool                                  found = false;
				RE::BSSimpleList<RE::GROUP_REACTION*> kept;

				while (!curobj->reactions.empty()) {
					auto* reaction = curobj->reactions.front();
					curobj->reactions.pop_front();

					if (reaction && reaction->form == form) {
						found = true;
						delete reaction;
						continue;
					}

					kept.push_front(reaction);
				}

				while (!kept.empty()) {
					auto* reaction = kept.front();
					kept.pop_front();
					curobj->reactions.push_front(reaction);
				}

				if (found) {
					logger::debug(
						FMT_STRING("faction {:08X} {} removed relationship to {:08X}"),
						curobj->formID,
						curobj->fullName,
						form->formID);
				} else {
					logger::debug(
						FMT_STRING("faction {:08X} {} relationship to {:08X} not found, remove skipped"),
						curobj->formID,
						curobj->fullName,
						form->formID);
				}
			}
		}

	}

}
