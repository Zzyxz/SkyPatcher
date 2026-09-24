#include "cell.h"
#include "formfilter.h"
#include "parsetable.h"
#include <utility.h>
#include <unordered_set>

namespace CELL
{

	namespace
	{
#define CELL_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction, member))

		constexpr sp::KeySpec kCellKeys[] = {
			{ "filterByCells", sp::FieldKind::List, CELL_F(object) },
			{ "filterByCellsExcluded", sp::FieldKind::List, CELL_F(objectExcluded) },
			{ "fogNear", sp::FieldKind::Value, CELL_F(fogNear) },
			{ "fogFar", sp::FieldKind::Value, CELL_F(fogFar) },
			{ "fogPower", sp::FieldKind::Value, CELL_F(fogPower) },
			{ "clipDist", sp::FieldKind::Value, CELL_F(clipDist) },
			{ "ambientRed", sp::FieldKind::Value, CELL_F(ambientRed) },
			{ "ambientGreen", sp::FieldKind::Value, CELL_F(ambientGreen) },
			{ "ambientBlue", sp::FieldKind::Value, CELL_F(ambientBlue) },
			{ "directionalRed", sp::FieldKind::Value, CELL_F(directionalRed) },
			{ "directionalGreen", sp::FieldKind::Value, CELL_F(directionalGreen) },
			{ "directionalBlue", sp::FieldKind::Value, CELL_F(directionalBlue) },
			{ "fogColorNearRed", sp::FieldKind::Value, CELL_F(fogColorNearRed) },
			{ "fogColorNearGreen", sp::FieldKind::Value, CELL_F(fogColorNearGreen) },
			{ "fogColorNearBlue", sp::FieldKind::Value, CELL_F(fogColorNearBlue) },
			{ "directionalAmbientXMinRed", sp::FieldKind::Value, CELL_F(directionalAmbientXMinRed) },
			{ "directionalAmbientXMinGreen", sp::FieldKind::Value, CELL_F(directionalAmbientXMinGreen) },
			{ "directionalAmbientXMinBlue", sp::FieldKind::Value, CELL_F(directionalAmbientXMinBlue) },
			{ "directionalAmbientXMaxRed", sp::FieldKind::Value, CELL_F(directionalAmbientXMaxRed) },
			{ "directionalAmbientXMaxGreen", sp::FieldKind::Value, CELL_F(directionalAmbientXMaxGreen) },
			{ "directionalAmbientXMaxBlue", sp::FieldKind::Value, CELL_F(directionalAmbientXMaxBlue) },
			{ "directionalAmbientYMinRed", sp::FieldKind::Value, CELL_F(directionalAmbientYMinRed) },
			{ "directionalAmbientYMinGreen", sp::FieldKind::Value, CELL_F(directionalAmbientYMinGreen) },
			{ "directionalAmbientYMinBlue", sp::FieldKind::Value, CELL_F(directionalAmbientYMinBlue) },
			{ "directionalAmbientYMaxRed", sp::FieldKind::Value, CELL_F(directionalAmbientYMaxRed) },
			{ "directionalAmbientYMaxGreen", sp::FieldKind::Value, CELL_F(directionalAmbientYMaxGreen) },
			{ "directionalAmbientYMaxBlue", sp::FieldKind::Value, CELL_F(directionalAmbientYMaxBlue) },
			{ "directionalAmbientZMinRed", sp::FieldKind::Value, CELL_F(directionalAmbientZMinRed) },
			{ "directionalAmbientZMinGreen", sp::FieldKind::Value, CELL_F(directionalAmbientZMinGreen) },
			{ "directionalAmbientZMinBlue", sp::FieldKind::Value, CELL_F(directionalAmbientZMinBlue) },
			{ "directionalAmbientZMaxRed", sp::FieldKind::Value, CELL_F(directionalAmbientZMaxRed) },
			{ "directionalAmbientZMaxGreen", sp::FieldKind::Value, CELL_F(directionalAmbientZMaxGreen) },
			{ "directionalAmbientZMaxBlue", sp::FieldKind::Value, CELL_F(directionalAmbientZMaxBlue) },
			{ "directionalAmbientSpecularRed", sp::FieldKind::Value, CELL_F(directionalAmbientSpecularRed) },
			{ "directionalAmbientSpecularGreen", sp::FieldKind::Value, CELL_F(directionalAmbientSpecularGreen) },
			{ "directionalAmbientSpecularBlue", sp::FieldKind::Value, CELL_F(directionalAmbientSpecularBlue) },
			{ "setInheritanceFlags", sp::FieldKind::ListAssign, CELL_F(setInheritanceFlags) },
			{ "removeInheritanceFlags", sp::FieldKind::ListAssign, CELL_F(removeInheritanceFlags) },
			{ "setCellFlags", sp::FieldKind::ListAssign, CELL_F(setCellFlags) },
			{ "removeCellFlags", sp::FieldKind::ListAssign, CELL_F(removeCellFlags) },
			{ "lightingTemplate", sp::FieldKind::Value, CELL_F(lightingTemplate) },
			{ "skipRecordByLightingTemplateFromMod", sp::FieldKind::ListAssign, CELL_F(skipLT) },
			{ "skipRecordByModNameContains", sp::FieldKind::ListAssign, CELL_F(skipByModContains) },
			{ "acousticSpace", sp::FieldKind::Value, CELL_F(acousticSpace) },
			{ "musicType", sp::FieldKind::Value, CELL_F(musicType) },
			{ "skyRegion", sp::FieldKind::Value, CELL_F(skyRegion) },
			{ "imageSpace", sp::FieldKind::Value, CELL_F(imageSpace) },
			{ "location", sp::FieldKind::Value, CELL_F(location) },
			{ "filterByAcousticSpace", sp::FieldKind::List, CELL_F(filterByAcousticSpace) },
			{ "filterByLocations", sp::FieldKind::List, CELL_F(filterByLocation) },
			{ "filterByMusicType", sp::FieldKind::List, CELL_F(filterByMusicType) },
			{ "filterBySkyRegion", sp::FieldKind::List, CELL_F(filterBySkyRegion) },
			{ "filterByImageSpace", sp::FieldKind::List, CELL_F(filterByImageSpace) },
			{ "encounterZone", sp::FieldKind::Value, CELL_F(encounterZone) },
			{ "waterHeight", sp::FieldKind::Value, CELL_F(waterHeight) },
			{ "waterType", sp::FieldKind::Value, CELL_F(waterType) },
			{ "fullName", sp::FieldKind::ValueTilde, CELL_F(fullName) },
			{ "directionalXY", sp::FieldKind::Value, CELL_F(directionalXY) },
			{ "directionalZ", sp::FieldKind::Value, CELL_F(directionalZ) },
		};

#undef CELL_F

		const sp::KeyTable& cellKeyTable()
		{
			static const sp::KeyTable table(kCellKeys, std::size(kCellKeys));
			return table;
		}
	}

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
		cellKeyTable().parse(line, &l);

		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		auto& ObjectArray = dataHandler->GetFormArray<RE::TESObjectCELL>();

		for (const auto& line : tokens) {
			std::unordered_set<RE::FormID> patched;
			const formfilter::CommonFilterView view{
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), formfilter::noStrings(),
				formfilter::noStrings(), line.objectExcluded
			};

			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectCELL* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);

					if (currentform && currentform->formType == RE::FormType::Cell) {
						curobj = currentform->As<RE::TESObjectCELL>();

						if (!line.skipLT.empty()) {
							bool modFound = false;
							for (const auto& modName : line.skipLT) {
							if (const auto* file = curobj->lightingTemplate ? curobj->lightingTemplate->GetFile(0) : nullptr;
								file && modName == file->fileName) {
									modFound = true;
								}
							}
							if (modFound == true) {
								continue;
							}
						}

						if (const auto* file = curobj->GetFile(); file && !line.skipByModContains.empty()) {
							const std::string& fileName = file->fileName;
							bool               allMatched = true;
							for (const auto& modName : line.skipByModContains) {
								if (toLowerCase(fileName).find(toLowerCase(modName)) == std::string::npos) {
									allMatched = false;
									break;
								}
							}
							if (allMatched) {
								continue;
							}
						}

						if (!line.objectExcluded.empty() && formfilter::isObjectExcluded(view, curobj, RE::FormType::Cell)) {
							continue;
						}

						if (patched.insert(curobj->formID).second) {
							patch(line, curobj);
						}
					}
				}
			}
			if (!line.object.empty() && line.filterByAcousticSpace.empty() && line.filterByMusicType.empty() && line.filterBySkyRegion.empty() && line.filterByImageSpace.empty() && line.filterByLocation.empty()) {
				continue;
			}
			for (const auto& curobj : ObjectArray) {
								
				if (!line.skipLT.empty()) {
					bool modFound = false;
					for (const auto& modName : line.skipLT) {
						if (const auto* file = curobj->lightingTemplate ? curobj->lightingTemplate->GetFile(0) : nullptr;
							file && modName == file->fileName) {
							modFound = true;
						}
					}
					if (modFound == true) {
						continue;
					}

				}

										if (!line.skipByModContains.empty()) {
					const auto* file = curobj->GetFile();
					if (file) {
						const std::string& fileName = file->fileName;
						bool               allMatched = true;
						for (const auto& modName : line.skipByModContains) {
							if (toLowerCase(fileName).find(toLowerCase(modName)) == std::string::npos) {
								allMatched = false;
								break;
							}
						}
						if (allMatched) {
							continue;
						}
					}
				}
				bool found = false;

				if (!found && !line.filterByAcousticSpace.empty()) {
					for (const auto& aspString : line.filterByAcousticSpace) {
						auto* form = GetFormFromIdentifier(aspString);
						if (form && form->formType == RE::FormType::AcousticSpace) {
							auto* asp = static_cast<RE::BGSAcousticSpace*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraCellAcousticSpace>();
							if (extra && extra->space && extra->space->formID == asp->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByMusicType.empty()) {
					for (const auto& musString : line.filterByMusicType) {
						auto* form = GetFormFromIdentifier(musString);
						if (form && form->formType == RE::FormType::MusicType) {
							auto* mus = static_cast<RE::BGSMusicType*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraCellMusicType>();
							if (extra && extra->type && extra->type->formID == mus->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterBySkyRegion.empty()) {
					for (const auto& regString : line.filterBySkyRegion) {
						auto* form = GetFormFromIdentifier(regString);
						if (form && form->formType == RE::FormType::Region) {
							auto* reg = static_cast<RE::TESRegion*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraCellSkyRegion>();
							if (extra && extra->skyRegion && extra->skyRegion->formID == reg->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByLocation.empty()) {
					for (const auto& locString : line.filterByLocation) {
						auto* form = GetFormFromIdentifier(locString);
						if (form && form->formType == RE::FormType::Location) {
							auto* reg = static_cast<RE::BGSLocation*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraLocation>();
							if (extra && extra->location && extra->location->formID == reg->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!found && !line.filterByImageSpace.empty()) {
					for (const auto& ispString : line.filterByImageSpace) {
						auto* form = GetFormFromIdentifier(ispString);
						if (form && form->formType == RE::FormType::ImageSpace) {
							auto* isp = static_cast<RE::TESImageSpace*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraCellImageSpace>();
							if (extra && extra->imageSpace && extra->imageSpace->formID == isp->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (found && !line.objectExcluded.empty()) {
					if (formfilter::isObjectExcluded(view, curobj, RE::FormType::Cell)) {
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
		return nullptr;
	}

	void* patch(const CELL::patch_instruction& line, RE::TESObjectCELL* curobj)
	{


		if (!line.setCellFlags.empty()) {
			static const std::unordered_map<std::string, RE::TESObjectCELL::Flag> cellFlagMap = {
				{ "isinteriorcell", RE::TESObjectCELL::Flag::kIsInteriorCell },
				{ "haswater", RE::TESObjectCELL::Flag::kHasWater },
				{ "cantravelfromhere", RE::TESObjectCELL::Flag::kCanTravelFromHere },
				{ "nolodwater", RE::TESObjectCELL::Flag::kNoLODWater },
				{ "hastempdata", RE::TESObjectCELL::Flag::kHasTempData },
				{ "publicarea", RE::TESObjectCELL::Flag::kPublicArea },
				{ "handchanged", RE::TESObjectCELL::Flag::kHandChanged },
				{ "showsky", RE::TESObjectCELL::Flag::kShowSky },
				{ "useskylighting", RE::TESObjectCELL::Flag::kUseSkyLighting },
				{ "warntoleave", RE::TESObjectCELL::Flag::kWarnToLeave }
			};

			for (const auto& setFlag : line.setCellFlags) {
				std::string lowerFlag = toLowerCase(setFlag);
				auto        it = cellFlagMap.find(lowerFlag);
				if (it != cellFlagMap.end()) {
					curobj->cellFlags.set(it->second);
					logger::debug(FMT_STRING("cell formid {:08X} enabled cellFlag: {}"), curobj->formID, lowerFlag);
				} else {
					logger::warn(FMT_STRING("cell formid {:08X} unknown cellFlag (set): {}"), curobj->formID, lowerFlag);
				}
			}
		}

		if (!line.removeCellFlags.empty()) {
			static const std::unordered_map<std::string, RE::TESObjectCELL::Flag> cellFlagMap = {
				{ "isinteriorcell", RE::TESObjectCELL::Flag::kIsInteriorCell },
				{ "haswater", RE::TESObjectCELL::Flag::kHasWater },
				{ "cantravelfromhere", RE::TESObjectCELL::Flag::kCanTravelFromHere },
				{ "nolodwater", RE::TESObjectCELL::Flag::kNoLODWater },
				{ "hastempdata", RE::TESObjectCELL::Flag::kHasTempData },
				{ "publicarea", RE::TESObjectCELL::Flag::kPublicArea },
				{ "handchanged", RE::TESObjectCELL::Flag::kHandChanged },
				{ "showsky", RE::TESObjectCELL::Flag::kShowSky },
				{ "useskylighting", RE::TESObjectCELL::Flag::kUseSkyLighting },
				{ "warntoleave", RE::TESObjectCELL::Flag::kWarnToLeave }
			};

			for (const auto& removeFlag : line.removeCellFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);
				auto        it = cellFlagMap.find(lowerFlag);
				if (it != cellFlagMap.end()) {
					curobj->cellFlags.reset(it->second);
					logger::debug(FMT_STRING("cell formid {:08X} disabled cellFlag: {}"), curobj->formID, lowerFlag);
				} else {
					logger::warn(FMT_STRING("cell formid {:08X} unknown cellFlag (remove): {}"), curobj->formID, lowerFlag);
				}
			}
		}

		if (!line.waterHeight.empty() && line.waterHeight != "none") {
			if (toLowerCase(line.waterHeight) == "null") {
				curobj->waterHeight = -129.0f;
				logger::debug(FMT_STRING("cell formid {:08X} reset waterHeight to default"), curobj->formID);
			} else {
				try {
					float value = std::stof(line.waterHeight);
					curobj->waterHeight = value;
					logger::debug(FMT_STRING("cell formid {:08X} set waterHeight to {:.6f}"), curobj->formID, value);
				} catch (...) {
					logger::warn(FMT_STRING("Invalid waterHeight '{}' for cell {:08X}"), line.waterHeight, curobj->formID);
				}
			}
		}

		if (!line.acousticSpace.empty() &&
			line.acousticSpace != "none" &&
			toLowerCase(line.acousticSpace) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.acousticSpace);
			if (form && form->formType == RE::FormType::AcousticSpace) {
				auto* asp = static_cast<RE::BGSAcousticSpace*>(form);

				if (curobj->extraList.HasType<RE::ExtraCellAcousticSpace>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraCellAcousticSpace>();
					if (extra) {
						extra->space = asp;
						logger::debug(FMT_STRING("cell formid {:08X} changed acousticSpace to {:08X}"), curobj->formID, asp->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for acousticSpace on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraCellAcousticSpace();
					newExtra->space = asp;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added acousticSpace {:08X}"), curobj->formID, asp->formID);
				}
			}
		} else if (toLowerCase(line.acousticSpace) == "null") {
			if (curobj->extraList.HasType<RE::ExtraCellAcousticSpace>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kCellAcousticSpace);
				logger::debug(FMT_STRING("cell formid {:08X} removed acousticSpace"), curobj->formID);
			}
		}

		if (!line.musicType.empty() &&
			line.musicType != "none" &&
			toLowerCase(line.musicType) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.musicType);
			if (form && form->formType == RE::FormType::MusicType) {
				auto* mus = static_cast<RE::BGSMusicType*>(form);

				if (curobj->extraList.HasType<RE::ExtraCellMusicType>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraCellMusicType>();
					if (extra) {
						extra->type = mus;
						logger::debug(FMT_STRING("cell formid {:08X} changed musicType to {:08X}"), curobj->formID, mus->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for musicType on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraCellMusicType();
					newExtra->type = mus;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added musicType {:08X}"), curobj->formID, mus->formID);
				}
			}
		} else if (toLowerCase(line.musicType) == "null") {
			if (curobj->extraList.HasType<RE::ExtraCellMusicType>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kCellMusicType);
				logger::debug(FMT_STRING("cell formid {:08X} removed musicType"), curobj->formID);
			}
		}
		
		if (!line.location.empty() &&
			line.location != "none" &&
			toLowerCase(line.location) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.location);
			if (form && form->formType == RE::FormType::Location) {
				auto* loc = static_cast<RE::BGSLocation*>(form);

				if (curobj->extraList.HasType<RE::ExtraLocation>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraLocation>();
					if (extra) {
						extra->location = loc;
						logger::debug(FMT_STRING("cell formid {:08X} changed location to {:08X}"), curobj->formID, loc->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for location on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraLocation();
					newExtra->location = loc;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added location {:08X}"), curobj->formID, loc->formID);
				}
			}
		} else if (toLowerCase(line.location) == "null") {
			if (curobj->extraList.HasType<RE::ExtraLocation>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kLocation);
				logger::debug(FMT_STRING("cell formid {:08X} removed location"), curobj->formID);
			}
		}
		
		if (!line.skyRegion.empty() &&
			line.skyRegion != "none" &&
			toLowerCase(line.skyRegion) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.skyRegion);
			if (form && form->formType == RE::FormType::Region) {
				auto* reg = static_cast<RE::TESRegion*>(form);

				if (curobj->extraList.HasType<RE::ExtraCellSkyRegion>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraCellSkyRegion>();
					if (extra) {
						extra->skyRegion = reg;
						logger::debug(FMT_STRING("cell formid {:08X} changed skyRegion to {:08X}"), curobj->formID, reg->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for skyRegion on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraCellSkyRegion();
					newExtra->skyRegion = reg;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added skyRegion {:08X}"), curobj->formID, reg->formID);
				}
			}
		} else if (toLowerCase(line.skyRegion) == "null") {
			if (curobj->extraList.HasType<RE::ExtraCellSkyRegion>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kCellSkyRegion);
				logger::debug(FMT_STRING("cell formid {:08X} removed skyRegion"), curobj->formID);
			}
		}

		if (!line.imageSpace.empty() &&
			line.imageSpace != "none" &&
			toLowerCase(line.imageSpace) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.imageSpace);
			if (form && form->formType == RE::FormType::ImageSpace) {
				auto* imsp = static_cast<RE::TESImageSpace*>(form);

				if (curobj->extraList.HasType<RE::ExtraCellImageSpace>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraCellImageSpace>();
					if (extra) {
						extra->imageSpace = imsp;
						logger::debug(FMT_STRING("cell formid {:08X} changed imageSpace to {:08X}"), curobj->formID, imsp->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for imageSpace on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraCellImageSpace();
					newExtra->imageSpace = imsp;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added imageSpace {:08X}"), curobj->formID, imsp->formID);
				}
			}
		} else if (toLowerCase(line.imageSpace) == "null") {
			if (curobj->extraList.HasType<RE::ExtraCellImageSpace>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kCellImageSpace);
				logger::debug(FMT_STRING("cell formid {:08X} removed imageSpace"), curobj->formID);
			}
		}

		if (!line.encounterZone.empty() &&
			line.encounterZone != "none" &&
			toLowerCase(line.encounterZone) != "null") {
			RE::TESForm* form = GetFormFromIdentifier(line.encounterZone);
			if (form && form->formType == RE::FormType::EncounterZone) {
				auto* imsp = static_cast<RE::BGSEncounterZone*>(form);

				if (curobj->extraList.HasType<RE::ExtraEncounterZone>()) {
					auto* extra = curobj->extraList.GetByType<RE::ExtraEncounterZone>();
					if (extra) {
						extra->zone = imsp;
						logger::debug(FMT_STRING("cell formid {:08X} changed encounter zone to {:08X}"), curobj->formID, imsp->formID);
					} else {
						logger::warn(FMT_STRING("HasType true but GetByType null for encounter zone on cell {:08X}"), curobj->formID);
					}
				} else {
					auto* newExtra = new RE::ExtraEncounterZone();
					newExtra->zone = imsp;
					curobj->extraList.Add(newExtra);
					logger::debug(FMT_STRING("cell formid {:08X} added encounter zone {:08X}"), curobj->formID, imsp->formID);
				}
			}
		} else if (toLowerCase(line.encounterZone) == "null") {
			if (curobj->extraList.HasType<RE::ExtraEncounterZone>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kEncounterZone);
				logger::debug(FMT_STRING("cell formid {:08X} removed encounter zone"), curobj->formID);
			}
		}

		const std::string* interiorValues[] = {
			&line.fogNear, &line.fogFar, &line.fogPower, &line.clipDist,
			&line.ambientRed, &line.ambientGreen, &line.ambientBlue,
			&line.directionalRed, &line.directionalGreen, &line.directionalBlue,
			&line.fogColorNearRed, &line.fogColorNearGreen, &line.fogColorNearBlue,
			&line.directionalAmbientXMinRed, &line.directionalAmbientXMinGreen, &line.directionalAmbientXMinBlue,
			&line.directionalAmbientXMaxRed, &line.directionalAmbientXMaxGreen, &line.directionalAmbientXMaxBlue,
			&line.directionalAmbientYMinRed, &line.directionalAmbientYMinGreen, &line.directionalAmbientYMinBlue,
			&line.directionalAmbientYMaxRed, &line.directionalAmbientYMaxGreen, &line.directionalAmbientYMaxBlue,
			&line.directionalAmbientZMinRed, &line.directionalAmbientZMinGreen, &line.directionalAmbientZMinBlue,
			&line.directionalAmbientZMaxRed, &line.directionalAmbientZMaxGreen, &line.directionalAmbientZMaxBlue,
			&line.directionalAmbientSpecularRed, &line.directionalAmbientSpecularGreen, &line.directionalAmbientSpecularBlue,
			&line.directionalXY, &line.directionalZ
		};
		bool hasInteriorOperation = !line.removeInheritanceFlags.empty() || !line.setInheritanceFlags.empty();
		for (const auto* value : interiorValues) {
			if (value && !value->empty() && *value != "none") {
				hasInteriorOperation = true;
				break;
			}
		}

		if (curobj->cellData.interior) {
		try {
		if (!line.removeInheritanceFlags.empty()) {
			static const std::unordered_map<std::string, RE::INTERIOR_DATA::Inherit> inheritFlagMap = {
				{ "ambientcolor", RE::INTERIOR_DATA::Inherit::kAmbientColor },
				{ "directionalcolor", RE::INTERIOR_DATA::Inherit::kDirectionalColor },
				{ "fogcolor", RE::INTERIOR_DATA::Inherit::kFogColor },
				{ "fognear", RE::INTERIOR_DATA::Inherit::kFogNear },
				{ "fogfar", RE::INTERIOR_DATA::Inherit::kFogFar },
				{ "directionalrotation", RE::INTERIOR_DATA::Inherit::kDirectionalRotation },
				{ "directionalfade", RE::INTERIOR_DATA::Inherit::kDirectionalFade },
				{ "clipdistance", RE::INTERIOR_DATA::Inherit::kClipDistance },
				{ "fogpower", RE::INTERIOR_DATA::Inherit::kFogPower },
				{ "fogmax", RE::INTERIOR_DATA::Inherit::kFogMax },
				{ "lightfadedistances", RE::INTERIOR_DATA::Inherit::kLightFadeDistances }
			};

			for (const auto& removeFlag : line.removeInheritanceFlags) {
				std::string lowerFlag = toLowerCase(removeFlag);
				auto        it = inheritFlagMap.find(lowerFlag);
				if (it != inheritFlagMap.end()) {
					curobj->cellData.interior->lightingTemplateInheritanceFlags.reset(it->second);
					logger::debug(FMT_STRING("cell formid: {:08X} disabled inheritanceFlag {}"), curobj->formID, lowerFlag);
				} else {
					logger::warn(FMT_STRING("cell formid: {:08X} invalid inheritanceFlag: {}"), curobj->formID, lowerFlag);
				}
			}
		}

		if (!line.setInheritanceFlags.empty()) {
			static const std::unordered_map<std::string, RE::INTERIOR_DATA::Inherit> inheritFlagMap = {
				{ "ambientcolor", RE::INTERIOR_DATA::Inherit::kAmbientColor },
				{ "directionalcolor", RE::INTERIOR_DATA::Inherit::kDirectionalColor },
				{ "fogcolor", RE::INTERIOR_DATA::Inherit::kFogColor },
				{ "fognear", RE::INTERIOR_DATA::Inherit::kFogNear },
				{ "fogfar", RE::INTERIOR_DATA::Inherit::kFogFar },
				{ "directionalrotation", RE::INTERIOR_DATA::Inherit::kDirectionalRotation },
				{ "directionalfade", RE::INTERIOR_DATA::Inherit::kDirectionalFade },
				{ "clipdistance", RE::INTERIOR_DATA::Inherit::kClipDistance },
				{ "fogpower", RE::INTERIOR_DATA::Inherit::kFogPower },
				{ "fogmax", RE::INTERIOR_DATA::Inherit::kFogMax },
				{ "lightfadedistances", RE::INTERIOR_DATA::Inherit::kLightFadeDistances }
			};

			for (const auto& setFlag : line.setInheritanceFlags) {
				std::string flag = toLowerCase(setFlag);
				auto        it = inheritFlagMap.find(flag);
				if (it != inheritFlagMap.end()) {
					curobj->cellData.interior->lightingTemplateInheritanceFlags.set(it->second);
					logger::debug(FMT_STRING("cell formid: {:08X} enabled inheritanceFlag {}"), curobj->formID, flag);
				} else {
					logger::warn(FMT_STRING("cell formid: {:08X} unknown inheritanceFlag: {}"), curobj->formID, flag);
				}
			}
		}



		if (!line.fogColorNearRed.empty() && line.fogColorNearRed != "none") {
			try {
				curobj->cellData.interior->fogColorNear.red = static_cast<std::uint8_t>(std::stoi(line.fogColorNearRed));
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogColorNear.red to {}"), curobj->formID, curobj->cellData.interior->fogColorNear.red);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.fogColorNearGreen.empty() && line.fogColorNearGreen != "none") {
			try {
				curobj->cellData.interior->fogColorNear.green = static_cast<std::uint8_t>(std::stoi(line.fogColorNearGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogColorNear.green to {}"), curobj->formID, curobj->cellData.interior->fogColorNear.green);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.fogColorNearBlue.empty() && line.fogColorNearBlue != "none") {
			try {
				curobj->cellData.interior->fogColorNear.blue = static_cast<std::uint8_t>(std::stoi(line.fogColorNearBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogColorNear.blue to {}"), curobj->formID, curobj->cellData.interior->fogColorNear.blue);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.directionalRed.empty() && line.directionalRed != "none") {
			try {
				curobj->cellData.interior->directional.red = static_cast<std::uint8_t>(std::stoi(line.directionalRed));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalRed to {}"), curobj->formID, curobj->cellData.interior->directional.red);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}
		if (!line.directionalGreen.empty() && line.directionalGreen != "none") {
			try {
				curobj->cellData.interior->directional.green = static_cast<std::uint8_t>(std::stoi(line.directionalGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalGreen to {}"), curobj->formID, curobj->cellData.interior->directional.green);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}
		if (!line.directionalBlue.empty() && line.directionalBlue != "none") {
			try {
				curobj->cellData.interior->directional.blue = static_cast<std::uint8_t>(std::stoi(line.directionalBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalBlue to {}"), curobj->formID, curobj->cellData.interior->directional.blue);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.ambientBlue.empty() && line.ambientBlue != "none") {
			try {
				curobj->cellData.interior->ambient.blue = static_cast<std::uint8_t>(std::stoi(line.ambientBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientBlue to {}"), curobj->formID, curobj->cellData.interior->ambient.blue);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.ambientRed.empty() && line.ambientRed != "none") {
			try {
				curobj->cellData.interior->ambient.red = static_cast<std::uint8_t>(std::stoi(line.ambientRed));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientRed to {}"), curobj->formID, curobj->cellData.interior->ambient.red);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.ambientGreen.empty() && line.ambientGreen != "none") {
			try {
				curobj->cellData.interior->ambient.green = static_cast<std::uint8_t>(std::stoi(line.ambientGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientGreen to {}"), curobj->formID, curobj->cellData.interior->ambient.green);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.fogNear.empty() && line.fogNear != "none") {
			try {
				curobj->cellData.interior->fogNear = std::stof(line.fogNear);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogNear to {}"), curobj->formID, curobj->cellData.interior->fogNear);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.fogFar.empty() && line.fogFar != "none") {
			try {
				curobj->cellData.interior->fogFar = std::stof(line.fogFar);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogFar to {}"), curobj->formID, curobj->cellData.interior->fogFar);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.fogPower.empty() && line.fogPower != "none") {
			try {
				curobj->cellData.interior->fogPower = std::stof(line.fogPower);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogPower to {}"), curobj->formID, curobj->cellData.interior->fogPower);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.clipDist.empty() && line.clipDist != "none") {
			try {
				curobj->cellData.interior->clipDist = std::stof(line.clipDist);
				logger::debug(FMT_STRING("cell formid: {:08X} changed clipDist to {}"), curobj->formID, curobj->cellData.interior->clipDist);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
			}
		}

		if (!line.directionalAmbientXMinRed.empty() && line.directionalAmbientXMinRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMinRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMinRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.red);
		}
		if (!line.directionalAmbientXMinGreen.empty() && line.directionalAmbientXMinGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMinGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMinGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.green);
		}
		if (!line.directionalAmbientXMinBlue.empty() && line.directionalAmbientXMinBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMinBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMinBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.min.blue);
		}
		if (!line.directionalAmbientXMaxRed.empty() && line.directionalAmbientXMaxRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMaxRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMaxRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.red);
		}
		if (!line.directionalAmbientXMaxGreen.empty() && line.directionalAmbientXMaxGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMaxGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMaxGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.green);
		}
		if (!line.directionalAmbientXMaxBlue.empty() && line.directionalAmbientXMaxBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientXMaxBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientXMaxBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.x.max.blue);
		}

		if (!line.directionalAmbientYMinRed.empty() && line.directionalAmbientYMinRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMinRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMinRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.red);
		}
		if (!line.directionalAmbientYMinGreen.empty() && line.directionalAmbientYMinGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMinGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMinGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.green);
		}
		if (!line.directionalAmbientYMinBlue.empty() && line.directionalAmbientYMinBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMinBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMinBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.min.blue);
		}
		if (!line.directionalAmbientYMaxRed.empty() && line.directionalAmbientYMaxRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMaxRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMaxRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.red);
		}
		if (!line.directionalAmbientYMaxGreen.empty() && line.directionalAmbientYMaxGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMaxGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMaxGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.green);
		}
		if (!line.directionalAmbientYMaxBlue.empty() && line.directionalAmbientYMaxBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientYMaxBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientYMaxBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.y.max.blue);
		}

		if (!line.directionalAmbientZMinRed.empty() && line.directionalAmbientZMinRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMinRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMinRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.red);
		}
		if (!line.directionalAmbientZMinGreen.empty() && line.directionalAmbientZMinGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMinGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMinGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.green);
		}
		if (!line.directionalAmbientZMinBlue.empty() && line.directionalAmbientZMinBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMinBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMinBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.min.blue);
		}
		if (!line.directionalAmbientZMaxRed.empty() && line.directionalAmbientZMaxRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMaxRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMaxRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.red);
		}
		if (!line.directionalAmbientZMaxGreen.empty() && line.directionalAmbientZMaxGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMaxGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMaxGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.green);
		}
		if (!line.directionalAmbientZMaxBlue.empty() && line.directionalAmbientZMaxBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientZMaxBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientZMaxBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.directional.z.max.blue);
		}

		if (!line.directionalAmbientSpecularRed.empty() && line.directionalAmbientSpecularRed != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.specular.red = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientSpecularRed));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientSpecularRed to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.specular.red);
		}

		if (!line.directionalAmbientSpecularGreen.empty() && line.directionalAmbientSpecularGreen != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.specular.green = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientSpecularGreen));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientSpecularGreen to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.specular.green);
		}

		if (!line.directionalAmbientSpecularBlue.empty() && line.directionalAmbientSpecularBlue != "none") {
			curobj->cellData.interior->directionalAmbientLightingColors.specular.blue = static_cast<std::uint8_t>(std::stoi(line.directionalAmbientSpecularBlue));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalAmbientSpecularBlue to {}"), curobj->formID, curobj->cellData.interior->directionalAmbientLightingColors.specular.blue);
		}

		if (!line.directionalXY.empty() && line.directionalXY != "none") {
			curobj->cellData.interior->directionalXY = static_cast<std::uint32_t>(std::stoi(line.directionalXY));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalXY to {}"),
				curobj->formID,
				curobj->cellData.interior->directionalXY);
		}

		if (!line.directionalZ.empty() && line.directionalZ != "none") {
			curobj->cellData.interior->directionalZ = static_cast<std::uint32_t>(std::stoi(line.directionalZ));
			logger::debug(FMT_STRING("cell formid: {:08X} changed directionalZ to {}"),
				curobj->formID,
				curobj->cellData.interior->directionalZ);
		}
		} catch (const std::exception& e) {
			logger::warn(FMT_STRING("cell formid: {:08X} skipped an invalid interior value: {}"), curobj->formID, e.what());
		}
		} else if (hasInteriorOperation) {
			logger::warn(FMT_STRING("cell formid: {:08X} is exterior; skipped interior-only operations"), curobj->formID);
		}

		if (!line.lightingTemplate.empty() && line.lightingTemplate != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.lightingTemplate;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::LightingMaster) {
				curobj->lightingTemplate = (RE::BGSLightingTemplate*)currentform;
				logger::debug(FMT_STRING("cell formid: {:08X} {} changed lightingTemplate {:08X}"), curobj->formID, curobj->fullName, curobj->lightingTemplate->formID);
			} else if (toLowerCase(line.lightingTemplate) == "null") {
				curobj->lightingTemplate = nullptr;
				logger::debug(FMT_STRING("cell formid: {:08X} {} changed lightingTemplate to null"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("cell formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}


		return 0;
	}

}
