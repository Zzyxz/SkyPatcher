#include "projectile.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>

namespace PROJECTILE
{

	// ==================================================================
	// PHASE 2 -- PROJECTILE's key table. One row per upstream (active)
	// extract*() call, in the same order. See parsetable.h.
	//
	// Several members (keywords, keywordsOr, keywordsExcluded, filterAmmos,
	// damageTypes, ...) are declared in the struct but have no active
	// extract call upstream (dead fields) -- no row is added for them,
	// preserving that (dead) behaviour exactly.
	// ==================================================================
	namespace
	{
#define PROJ_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kProjectileKeys[] = {
			{ "filterByProjectiles", sp::FieldKind::List, PROJ_F(object) },
			{ "filterByProjectilesExcluded", sp::FieldKind::List, PROJ_F(objectExcluded) },
			{ "range", sp::FieldKind::Value, PROJ_F(range) },
			{ "rangeMult", sp::FieldKind::Value, PROJ_F(rangeMult) },
			{ "speed", sp::FieldKind::Value, PROJ_F(speed) },
			{ "speedMult", sp::FieldKind::Value, PROJ_F(speedMult) },
			{ "gravity", sp::FieldKind::Value, PROJ_F(gravity) },
			{ "gravityMult", sp::FieldKind::Value, PROJ_F(gravityMult) },
			{ "sound", sp::FieldKind::Value, PROJ_F(sound) },
			{ "type", sp::FieldKind::Value, PROJ_F(type) },
			{ "explosion", sp::FieldKind::Value, PROJ_F(explosion) },
			{ "setFlags", sp::FieldKind::ListAssign, PROJ_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, PROJ_F(removeFlags) },
			{ "filterByModNames", sp::FieldKind::ListAssign, PROJ_F(modNames) },
			{ "model", sp::FieldKind::Value, PROJ_F(model) },
			{ "alternateTexturesToAdd", sp::FieldKind::Arr2D, PROJ_F(alternateTextures) },
			{ "alternateTexturesToRemove", sp::FieldKind::List, PROJ_F(alternateTexturesRemove) },
			{ "alternateTexturesClear", sp::FieldKind::Value, PROJ_F(alternateTexturesClear) },
			{ "filterByAlternateTextures", sp::FieldKind::List, PROJ_F(alternateTexturesFind) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, PROJ_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, PROJ_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, PROJ_F(filterByEditorIdContainsExcluded) },
		};

#undef PROJ_F

		const sp::KeyTable& projectileKeyTable()
		{
			static const sp::KeyTable table(kProjectileKeys, std::size(kProjectileKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		projectileKeyTable().parse(line, &l);
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::BGSProjectile>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::BGSProjectile* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Projectile) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::BGSProjectile*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			// PROTOTYPE: one view per rule, not per (rule, projectile) pair.
			// This struct has no hasPlugin/hasPluginOr members, so those two
			// view slots are bound to noStrings().
			const formfilter::CommonFilterView view{
				line.modNames, formfilter::noStrings(), formfilter::noStrings(),
				line.keywords, line.keywordsOr, line.keywordsExcluded,
				line.filterByEditorIdContains, line.filterByEditorIdContainsOr,
				line.filterByEditorIdContainsExcluded, line.objectExcluded
			};

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool contains = false;
				bool containsOr = false;

				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(view, curobj)) {
					continue;
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

				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (!found && line.object.empty() && line.alternateTexturesFind.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty()) {
					found = true;
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found) {
					//logger::info("factions not empty");
					if (formfilter::hasExcludedEditorId(view, curobj)) {
						found = false;
					}
				}

				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Projectile)) {
						found = false;
					}
				}

				if (found) {
					patch(line, curobj);
				}
			}
		}
	}

	void patch(const PROJECTILE::patch_instruction& line, RE::BGSProjectile* curobj)
	{

		if (!line.speed.empty() && line.speed != "none") {
			try {
				curobj->data.speed = std::stof(line.speed);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed speed {}"), curobj->formID, curobj->fullName, curobj->data.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.speedMult.empty() && line.speedMult != "none") {
			try {
				curobj->data.speed = curobj->data.speed * std::stof(line.speedMult);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} multiplied speed {}"), curobj->formID, curobj->fullName, curobj->data.speed);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.range.empty() && line.range != "none") {
			try {
				curobj->data.range = std::stof(line.range);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed range {}"), curobj->formID, curobj->fullName, curobj->data.range);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.rangeMult.empty() && line.rangeMult != "none") {
			try {
				curobj->data.range = curobj->data.range * std::stof(line.rangeMult);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} multiplied range {}"), curobj->formID, curobj->fullName, curobj->data.range);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.gravity.empty() && line.gravity != "none") {
			try {
				curobj->data.gravity = std::stof(line.gravity);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed gravity {}"), curobj->formID, curobj->fullName, curobj->data.gravity);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.gravityMult.empty() && line.gravityMult != "none") {
			try {
				curobj->data.gravity = curobj->data.gravity * std::stof(line.gravityMult);
				logger::debug(FMT_STRING("projectile formid: {:08X} {} multiplied gravity {}"), curobj->formID, curobj->fullName, curobj->data.gravity);
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.sound.empty() && line.sound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.sound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->data.activeSoundLoop = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("projectile formid: {:08X} {} changed sound {:08X}"), curobj->formID, curobj->fullName, curobj->data.activeSoundLoop->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.type.empty() && line.type != "none") {
			if (toLowerCase(line.type) == "missile") {
				curobj->data.types = RE::BGSProjectileData::Type::kMissile;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to missile"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "grenade") {
				curobj->data.types = RE::BGSProjectileData::Type::kGrenade;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to grenade"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "beam") {
				curobj->data.types = RE::BGSProjectileData::Type::kBeam;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to beam"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "flamethrower") {
				curobj->data.types = RE::BGSProjectileData::Type::kFlamethrower;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to flamethrower"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "cone") {
				curobj->data.types = RE::BGSProjectileData::Type::kCone;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to cone"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "barrier") {
				curobj->data.types = RE::BGSProjectileData::Type::kBarrier;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to barrier"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.type) == "arrow") {
				curobj->data.types = RE::BGSProjectileData::Type::kArrow;
				logger::debug(FMT_STRING("projectile formid: {:08X} {} changed types to arrow"), curobj->formID, curobj->fullName);
			}
		}

		

		if (!line.setFlags.empty()) {
			for (const auto& setFlag : line.setFlags) {
				if (toLowerCase(setFlag) == "hitscan") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kHitScan);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled hitscan"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "explosion") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kExplosion);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled explosion"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "explosionalttrigger") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kExplosionAltTrigger);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled explosionalttrigger"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "muzzleflash") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kMuzzleFlash);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled muzzleflash"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "canturnoff") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kCanTurnOff);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled canturnoff"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "canpickup") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kCanPickUp);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled canpickup"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "supersonic") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kSupersonic);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled supersonic"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "pinslimbs") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kPinsLimbs);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled pinslimbs"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "passsmtransparent") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kPassSMTransparent);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled passsmtransparent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "disablecombataimcorrection") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kDisableCombatAimCorrection);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled disablecombataimcorrection"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(setFlag) == "continuousupdate") {
					curobj->data.flags.set(RE::BGSProjectileData::BGSProjectileFlags::kContinuousUpdate);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag enabled continuousupdate"), curobj->formID, curobj->fullName);
				}
				// Add similar blocks for any additional flags in RE::BGSProjectileData::BGSProjectileFlags
				else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		if (line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "hitscan") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kHitScan);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled hitscan"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "explosion") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kExplosion);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled explosion"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "explosionalttrigger") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kExplosionAltTrigger);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled explosionalttrigger"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "muzzleflash") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kMuzzleFlash);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled muzzleflash"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "canturnoff") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kCanTurnOff);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled canturnoff"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "canpickup") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kCanPickUp);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled canpickup"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "supersonic") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kSupersonic);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled supersonic"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "pinslimbs") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kPinsLimbs);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled pinslimbs"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "passsmtransparent") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kPassSMTransparent);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled passsmtransparent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "disablecombataimcorrection") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kDisableCombatAimCorrection);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled disablecombataimcorrection"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "continuousupdate") {
					curobj->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kContinuousUpdate);
					logger::debug(FMT_STRING("projectile formid: {:08X} {} flag disabled continuousupdate"), curobj->formID, curobj->fullName);
				}
				// Add similar blocks for any additional flags in RE::BGSProjectileData::BGSProjectileFlags
				else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}


		if (!line.explosion.empty() && line.explosion != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.explosion;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Explosion) {
				curobj->data.explosionType = (RE::BGSExplosion*)currentform;
				logger::debug(FMT_STRING("npc formid: {:08X} changed explosion to {:08X} "), curobj->formID, currentform->formID);
			} else if (line.explosion == "null") {
				curobj->data.explosionType = nullptr;
				logger::debug(FMT_STRING("npc formid: {:08X} changed explosion to null (none) "), curobj->formID);
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



}
