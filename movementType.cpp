#include "movementType.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <cmath>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

namespace MOVETYPE
{

	// ==================================================================
	// PHASE 2 -- MOVETYPE's key table. One row per upstream extract*()
	// call, in the same order. See parsetable.h.
	//
	// NOTE: upstream's `filterByModNames` extract call is commented out,
	// so `modNames` is never populated; no row is added for it here,
	// preserving that (dead) behaviour exactly.
	// ==================================================================
	namespace
	{
#define MOVE_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kMoveTypeKeys[] = {
			{ "filterByMovementTypes", sp::FieldKind::List, MOVE_F(object) },
			{ "filterByMovementTypesExcluded", sp::FieldKind::List, MOVE_F(objectExcluded) },
			{ "walkLeft", sp::FieldKind::Value, MOVE_F(walkLeft) },
			{ "runLeft", sp::FieldKind::Value, MOVE_F(runLeft) },
			{ "walkRight", sp::FieldKind::Value, MOVE_F(walkRight) },
			{ "runRight", sp::FieldKind::Value, MOVE_F(runRight) },
			{ "walkForward", sp::FieldKind::Value, MOVE_F(walkForward) },
			{ "runForward", sp::FieldKind::Value, MOVE_F(runForward) },
			{ "walkBack", sp::FieldKind::Value, MOVE_F(walkBack) },
			{ "runBack", sp::FieldKind::Value, MOVE_F(runBack) },
			{ "rotateInPlaceRun", sp::FieldKind::Value, MOVE_F(rotateInPlaceRun) },
			{ "rotateInPlaceWalk", sp::FieldKind::Value, MOVE_F(rotateInPlaceWalk) },
			{ "rotateWhileMovingRun", sp::FieldKind::Value, MOVE_F(rotateWhileMovingRun) },
		};

#undef MOVE_F

		const sp::KeyTable& moveTypeKeyTable()
		{
			static const sp::KeyTable table(kMoveTypeKeys, std::size(kMoveTypeKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		moveTypeKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSMovementType>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSMovementType* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::MovementType) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::BGSMovementType*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty()  ) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, movementType) pair.
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;


				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
				}

				if (!found && line.object.empty()) {
					found = true;
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::MovementType)) {
						found = false;
						//logger::info("MovementType Excluded.");
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

	void* patch(const MOVETYPE::patch_instruction& line, RE::BGSMovementType* curobj)
	{

		if (!line.walkLeft.empty() && line.walkLeft != "none") {
			try {
				//logger::debug(FMT_STRING("movementType formid: {:08X} before walkLeft {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kLeft][RE::Movement::MaxSpeeds::kWalk]);
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kLeft][RE::Movement::MaxSpeeds::kWalk] = std::stof(line.walkLeft);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed walkLeft {}"), curobj->formID, line.walkLeft);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.runLeft.empty() && line.runLeft != "none") {
			try {
				//logger::debug(FMT_STRING("movementType formid: {:08X} before runLeft {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kLeft][RE::Movement::MaxSpeeds::kRun]);
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kLeft][RE::Movement::MaxSpeeds::kRun] = std::stof(line.runLeft);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed runLeft {}"), curobj->formID, line.runLeft);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.walkRight.empty() && line.walkRight != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kRight][RE::Movement::MaxSpeeds::kWalk] = std::stof(line.walkRight);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed walkRight {}"), curobj->formID, line.walkRight);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.runRight.empty() && line.runRight != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kRight][RE::Movement::MaxSpeeds::kRun] = std::stof(line.runRight);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed runRight {}"), curobj->formID, line.runRight);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.walkForward.empty() && line.walkForward != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kForward][RE::Movement::MaxSpeeds::kWalk] = std::stof(line.walkForward);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed walkForward {}"), curobj->formID, line.walkForward);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.runForward.empty() && line.runForward != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kForward][RE::Movement::MaxSpeeds::kRun] = std::stof(line.runForward);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed runForward {}"), curobj->formID, line.runForward);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.walkBack.empty() && line.walkBack != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kBack][RE::Movement::MaxSpeeds::kWalk] = std::stof(line.walkBack);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed walkBack {}"), curobj->formID, line.walkBack);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.runBack.empty() && line.runBack != "none") {
			try {
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kBack][RE::Movement::MaxSpeeds::kRun] = std::stof(line.runBack);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed runBack {}"), curobj->formID, line.runBack);
			} catch (const std::invalid_argument& e) {
			}
		}

		//
		if (!line.rotateWhileMovingRun.empty() && line.rotateWhileMovingRun != "none") {
			try {
				//logger::debug(FMT_STRING("movementType formid: {:08X} before rotateWhileMovingRun {}"), curobj->formID, curobj->movementTypeData.defaultData.rotateWhileMovingRun);
				curobj->movementTypeData.defaultData.rotateWhileMovingRun = std::stof(line.rotateWhileMovingRun) * (M_PI / 180);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed rotateWhileMovingRun {}"), curobj->formID, curobj->movementTypeData.defaultData.rotateWhileMovingRun);
			} catch (const std::invalid_argument& e) {
			}
		}
		//
		if (!line.rotateInPlaceWalk.empty() && line.rotateInPlaceWalk != "none") {
			try {
				//logger::debug(FMT_STRING("movementType formid: {:08X} before rotateInPlaceWalk {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kWalk]);
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kWalk] = std::stof(line.rotateInPlaceWalk) * (M_PI / 180);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed rotateInPlaceWalk {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kWalk]);
			} catch (const std::invalid_argument& e) {
			}
		}
		if (!line.rotateInPlaceRun.empty() && line.rotateInPlaceRun != "none") {
			try {
				//logger::debug(FMT_STRING("movementType formid: {:08X} before rotateInPlaceRun {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kRun]);
				curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kRun] = std::stof(line.rotateInPlaceRun) * (M_PI / 180);
				logger::debug(FMT_STRING("movementType formid: {:08X} changed rotateInPlaceRun {}"), curobj->formID, curobj->movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::kRotations][RE::Movement::MaxSpeeds::kRun]);
			} catch (const std::invalid_argument& e) {
			}
		}

		


	}

}
