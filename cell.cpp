#include "cell.h"
#include <utility.h>

namespace CELL
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		// extract objects
		//std::regex  objects_regex("filterByCells\\s*=([^:]+)", regex::icase);
		//std::smatch objects_match;
		//std::regex_search(line, objects_match, objects_regex);
		//std::vector<std::string> objects;
		//if (objects_match.empty() || objects_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objects_str = objects_match[1];
		//	std::regex           objects_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
		//	std::sregex_iterator objects_end;
		//	while (objects_iterator != objects_end) {
		//		std::string tempVar = (*objects_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Weapon String: {}"), tempVar);
		//		if (tempVar != "none") {
		//			objects.push_back(tempVar);
		//		}
		//		++objects_iterator;
		//	}
		//	l.object = objects;
		//}

		extractData(line, "filterByCells\\s*=([^:]+)", l.object);

		extractValueString(line, "fogNear\\s*=([^:]+)", l.fogNear);
		extractValueString(line, "fogFar\\s*=([^:]+)", l.fogFar);
		extractValueString(line, "fogPower\\s*=([^:]+)", l.fogPower);
		extractValueString(line, "clipDist\\s*=([^:]+)", l.clipDist);

		extractValueString(line, "ambientRed\\s*=([^:]+)", l.ambientRed);
		extractValueString(line, "ambientGreen\\s*=([^:]+)", l.ambientGreen);
		extractValueString(line, "ambientBlue\\s*=([^:]+)", l.ambientBlue);

		extractValueString(line, "directionalRed\\s*=([^:]+)", l.directionalRed);
		extractValueString(line, "directionalGreen\\s*=([^:]+)", l.directionalGreen);
		extractValueString(line, "directionalBlue\\s*=([^:]+)", l.directionalBlue);

		extractValueString(line, "fogColorNearRed\\s*=([^:]+)", l.fogColorNearRed);
		extractValueString(line, "fogColorNearGreen\\s*=([^:]+)", l.fogColorNearGreen);
		extractValueString(line, "fogColorNearBlue\\s*=([^:]+)", l.fogColorNearBlue);

		// X
		extractValueString(line, "directionalAmbientXMinRed\\s*=([^:]+)", l.directionalAmbientXMinRed);
		extractValueString(line, "directionalAmbientXMinGreen\\s*=([^:]+)", l.directionalAmbientXMinGreen);
		extractValueString(line, "directionalAmbientXMinBlue\\s*=([^:]+)", l.directionalAmbientXMinBlue);
		extractValueString(line, "directionalAmbientXMaxRed\\s*=([^:]+)", l.directionalAmbientXMaxRed);
		extractValueString(line, "directionalAmbientXMaxGreen\\s*=([^:]+)", l.directionalAmbientXMaxGreen);
		extractValueString(line, "directionalAmbientXMaxBlue\\s*=([^:]+)", l.directionalAmbientXMaxBlue);

		// Y
		extractValueString(line, "directionalAmbientYMinRed\\s*=([^:]+)", l.directionalAmbientYMinRed);
		extractValueString(line, "directionalAmbientYMinGreen\\s*=([^:]+)", l.directionalAmbientYMinGreen);
		extractValueString(line, "directionalAmbientYMinBlue\\s*=([^:]+)", l.directionalAmbientYMinBlue);
		extractValueString(line, "directionalAmbientYMaxRed\\s*=([^:]+)", l.directionalAmbientYMaxRed);
		extractValueString(line, "directionalAmbientYMaxGreen\\s*=([^:]+)", l.directionalAmbientYMaxGreen);
		extractValueString(line, "directionalAmbientYMaxBlue\\s*=([^:]+)", l.directionalAmbientYMaxBlue);

		// Z
		extractValueString(line, "directionalAmbientZMinRed\\s*=([^:]+)", l.directionalAmbientZMinRed);
		extractValueString(line, "directionalAmbientZMinGreen\\s*=([^:]+)", l.directionalAmbientZMinGreen);
		extractValueString(line, "directionalAmbientZMinBlue\\s*=([^:]+)", l.directionalAmbientZMinBlue);
		extractValueString(line, "directionalAmbientZMaxRed\\s*=([^:]+)", l.directionalAmbientZMaxRed);
		extractValueString(line, "directionalAmbientZMaxGreen\\s*=([^:]+)", l.directionalAmbientZMaxGreen);
		extractValueString(line, "directionalAmbientZMaxBlue\\s*=([^:]+)", l.directionalAmbientZMaxBlue);

		extractValueString(line, "directionalAmbientSpecularRed\\s*=([^:]+)", l.directionalAmbientSpecularRed);
		extractValueString(line, "directionalAmbientSpecularGreen\\s*=([^:]+)", l.directionalAmbientSpecularGreen);
		extractValueString(line, "directionalAmbientSpecularBlue\\s*=([^:]+)", l.directionalAmbientSpecularBlue);

		extractDataStrings(line, "setInheritanceFlags\\s*=([^:]+)", l.setInheritanceFlags);
		extractDataStrings(line, "removeInheritanceFlags\\s*=([^:]+)", l.removeInheritanceFlags);

		extractDataStrings(line, "setCellFlags\\s*=([^:]+)", l.setCellFlags);
		extractDataStrings(line, "removeCellFlags\\s*=([^:]+)", l.removeCellFlags);

		extractValueString(line, "lightingTemplate\\s*=([^:]+)", l.lightingTemplate);

		extractDataStrings(line, "skipRecordByLightingTemplateFromMod\\s*=([^:]+)", l.skipLT);
		extractDataStrings(line, "skipRecordByModNameContains\\s*=([^:]+)", l.skipByModContains);

		extractValueString(line, "acousticSpace\\s*=([^:]+)", l.acousticSpace);
		extractValueString(line, "musicType\\s*=([^:]+)", l.musicType);
		extractValueString(line, "skyRegion\\s*=([^:]+)", l.skyRegion);
		extractValueString(line, "imageSpace\\s*=([^:]+)", l.imageSpace);

		extractData(line, "filterByAcousticSpace\\s*=([^:]+)", l.filterByAcousticSpace);
		extractData(line, "filterByMusicType\\s*=([^:]+)", l.filterByMusicType);
		extractData(line, "filterBySkyRegion\\s*=([^:]+)", l.filterBySkyRegion);
		extractData(line, "filterByImageSpace\\s*=([^:]+)", l.filterByImageSpace);
		extractValueString(line, "encounterZone\\s*=([^:]+)", l.encounterZone);
		extractValueString(line, "waterHeight\\s*=([^:]+)", l.waterHeight);
		extractValueString(line, "waterType\\s*=([^:]+)", l.waterType);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);

		extractValueString(line, "directionalXY\\s*=([^:]+)", l.directionalXY);
		extractValueString(line, "directionalZ\\s*=([^:]+)", l.directionalZ);

		//extractValueString(line, "waterEnvMap\\s*=([^:]+)", l.waterEnvMap);

		//// extract skipFlags
		//std::regex  skipFlagsExcluded_regex("filterByFlagsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch skipFlagsExcluded_match;
		//std::regex_search(line, skipFlagsExcluded_match, skipFlagsExcluded_regex);
		//std::vector<std::string> skipFlagsExcluded;
		//if (skipFlagsExcluded_match.empty() || skipFlagsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          skipFlagsExcluded_str = skipFlagsExcluded_match[1];
		//	std::regex           skipFlagsExcluded_list_regex("[^,]+(\\w+)", regex::icase);
		//	std::sregex_iterator skipFlagsExcluded_iterator(skipFlagsExcluded_str.begin(), skipFlagsExcluded_str.end(), skipFlagsExcluded_list_regex);
		//	std::sregex_iterator skipFlagsExcluded_end;
		//	while (skipFlagsExcluded_iterator != skipFlagsExcluded_end) {
		//		std::string keyword = (*skipFlagsExcluded_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			skipFlagsExcluded.push_back(keyword);
		//		}
		//		++skipFlagsExcluded_iterator;
		//	}
		//	l.filterByFlagsExclude = skipFlagsExcluded;
		//}

		//// extract objectsExcluded
		//std::regex  objectsExcluded_regex("filterByCellsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch objectsExcluded_match;
		//std::regex_search(line, objectsExcluded_match, objectsExcluded_regex);
		//std::vector<std::string> objectsExcluded;
		//if (objectsExcluded_match.empty() || objectsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          objectsExcluded_str = objectsExcluded_match[1];
		//	std::regex           objectsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator objectsExcluded_iterator(objectsExcluded_str.begin(), objectsExcluded_str.end(), objectsExcluded_list_regex);
		//	std::sregex_iterator objectsExcluded_end;
		//	while (objectsExcluded_iterator != objectsExcluded_end) {
		//		std::string tempVar = (*objectsExcluded_iterator)[0].str();
		//		tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
		//		tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
		//		//logger::info(FMT_STRING("Race: {}"), race);
		//		if (tempVar != "none") {
		//			objectsExcluded.push_back(tempVar);
		//		}
		//		++objectsExcluded_iterator;
		//	}
		//	l.objectExcluded = objectsExcluded;
		//}

		//// extract keywords
		//std::regex  keywords_regex("filterByKeywords\\s*=([^:]+)", regex::icase);
		//std::smatch keywords_match;
		//std::regex_search(line, keywords_match, keywords_regex);
		//std::vector<std::string> keywords;
		//if (keywords_match.empty() || keywords_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywords_str = keywords_match[1];
		//	std::regex           keywords_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
		//	std::sregex_iterator keywords_end;
		//	while (keywords_iterator != keywords_end) {
		//		std::string keyword = (*keywords_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywords.push_back(keyword);
		//		}
		//		++keywords_iterator;
		//	}
		//	l.keywords = keywords;
		//}

		//// extract keywords
		//std::regex  keywordsOr_regex("filterByKeywordsOr\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsOr_match;
		//std::regex_search(line, keywordsOr_match, keywordsOr_regex);
		//std::vector<std::string> keywordsOr;
		//if (keywordsOr_match.empty() || keywordsOr_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywordsOr_str = keywordsOr_match[1];
		//	std::regex           keywordsOr_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsOr_iterator(keywordsOr_str.begin(), keywordsOr_str.end(), keywordsOr_list_regex);
		//	std::sregex_iterator keywordsOr_end;
		//	while (keywordsOr_iterator != keywordsOr_end) {
		//		std::string keyword = (*keywordsOr_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsOr.push_back(keyword);
		//		}
		//		++keywordsOr_iterator;
		//	}
		//	l.keywordsOr = keywordsOr;
		//}

		//// extract keywords
		//std::regex  keywordsExcluded_regex("filterByKeywordsExcluded\\s*=([^:]+)", regex::icase);
		//std::smatch keywordsExcluded_match;
		//std::regex_search(line, keywordsExcluded_match, keywordsExcluded_regex);
		//std::vector<std::string> keywordsExcluded;
		//if (keywordsExcluded_match.empty() || keywordsExcluded_match[1].str().empty()) {
		//	//empty
		//} else {
		//	std::string          keywordsExcluded_str = keywordsExcluded_match[1];
		//	std::regex           keywordsExcluded_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator keywordsExcluded_iterator(keywordsExcluded_str.begin(), keywordsExcluded_str.end(), keywordsExcluded_list_regex);
		//	std::sregex_iterator keywordsExcluded_end;
		//	while (keywordsExcluded_iterator != keywordsExcluded_end) {
		//		std::string keyword = (*keywordsExcluded_iterator)[0].str();
		//		keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
		//		keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
		//		if (keyword != "none") {
		//			keywordsExcluded.push_back(keyword);
		//		}
		//		++keywordsExcluded_iterator;
		//	}
		//	l.keywordsExcluded = keywordsExcluded;
		//}

		//// extract attackDamage
		//std::regex  attackDamage_regex("attackDamage\\s*=([^:]+)", regex::icase);
		//std::smatch match;
		//std::regex_search(line, match, attackDamage_regex);
		//// extract the value after the equals sign
		//if (match.empty() || match[1].str().empty()) {
		//	l.attackDamage = "none";
		//} else {
		//	std::string value = match[1].str();
		//	value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		//	l.attackDamage = value;
		//}

		//		// extract critDamage
		//std::regex  critDamage_regex("critDamage\\s*=([^:]+)", regex::icase);
		//std::smatch matchCrit;
		//std::regex_search(line, matchCrit, critDamage_regex);
		//// extract the value after the equals sign
		//if (matchCrit.empty() || matchCrit[1].str().empty()) {
		//	l.critDamage = "none";
		//} else {
		//	std::string value = matchCrit[1].str();
		//	value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		//	l.critDamage = value;
		//}

		//// extract critPercentMult
		//std::regex  critPercentMult_regex("critPercentMult\\s*=([^:]+)", regex::icase);
		//std::smatch matchMult;
		//std::regex_search(line, matchMult, critPercentMult_regex);
		//// extract the value after the equals sign
		//if (matchMult.empty() || matchMult[1].str().empty()) {
		//	l.critPercentMult = "none";
		//} else {
		//	std::string value = matchMult[1].str();
		//	value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
		//	l.critPercentMult = value;
		//}

		//extractValueString(line, "speed\\s *= ([^:] +)", l.speed);

		//extractValueString(line, "speed\\s *= ([^:] +)", l.speed);
		//extractValueString(line, "reach\\s *= ([^:] +)", l.reach);
		//extractValueString(line, "rangeMin\\s *= ([^:] +)", l.rangeMin);
		//extractValueString(line, "rangeMax\\s *= ([^:] +)", l.rangeMax);
		//extractValueString(line, "stagger\\s *= ([^:] +)", l.stagger);

		//extractValueString(line, "impactDataSet\\s *= ([^:] +)", l.impactDataSet);
		//extractValueString(line, "equipSound\\s *= ([^:] +)", l.equipSound);

		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::TESObjectCELL*> ObjectArray = dataHandler->GetFormArray<RE::TESObjectCELL>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectCELL* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);

					if (currentform && currentform->formType == RE::FormType::Cell) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = currentform->As<RE::TESObjectCELL>();

						if (!line.skipLT.empty()) {
							bool modFound = false;
							for (const auto& modName : line.skipLT) {
								if (curobj->lightingTemplate && modName == curobj->lightingTemplate->GetFile(0)->fileName) {
									modFound = true;
								}
							}
							if (modFound == true) {
								//logger::debug("cell formid: {:08X} Lighting Template is from {} skip!", curobj->formID, curobj->lightingTemplate->GetFile(0)->fileName);
								continue;
							}
						}

						if (!line.skipByModContains.empty()) {
							const std::string& fileName = curobj->GetFile()->fileName;
							bool               allMatched = true;
							for (const auto& modName : line.skipByModContains) {
								if (toLowerCase(fileName).find(toLowerCase(modName)) == std::string::npos) {
									allMatched = false;
									break;
								}
							}
							if (allMatched) {
								////logger::debug("{:08X} Skipped by mod name contains {}", curobj->formID, fileName);
								continue;
							}
						}

						patch(line, curobj);
					}
				}
			}
			//logger::debug("before con ");
			if (!line.object.empty() && line.filterByAcousticSpace.empty() && line.filterByMusicType.empty() && line.filterBySkyRegion.empty() && line.filterByImageSpace.empty()) {
				continue;
			}
			//logger::debug("after con {}", ObjectArray.size());
			for (const auto& curobj : ObjectArray) {
								
				if (!line.skipLT.empty()) {
					bool modFound = false;
					for (const auto& modName : line.skipLT) {
						if (curobj->lightingTemplate && modName == curobj->lightingTemplate->GetFile(0)->fileName) {
							modFound = true;
						}
					}
					if (modFound == true) {
						//logger::debug("cell formid: {:08X} Lighting Template is from {} skip!", curobj->formID, curobj->lightingTemplate->GetFile(0)->fileName);
						continue;
					}
				}

				//	//logger::info("processing npc");
				bool found = false;

				// Filter: AcousticSpace
				//logger::debug("line.filterByAcousticSpace {}", line.filterByAcousticSpace.size());

				if (!found && !line.filterByAcousticSpace.empty()) {
					for (const auto& aspString : line.filterByAcousticSpace) {
						//logger::debug("line.filterByAcousticSpace aspString {}", aspString);
						auto* form = GetFormFromIdentifier(aspString);
						if (form && form->formType == RE::FormType::AcousticSpace) {
							//logger::debug("line.filterByAcousticSpace aspString1 {}", aspString);
							auto* asp = static_cast<RE::BGSAcousticSpace*>(form);
							auto* extra = curobj->extraList.GetByType<RE::ExtraCellAcousticSpace>();
							if (extra && extra->space && extra->space->formID == asp->formID) {
								found = true;
								//logger::debug("line.filterByAcousticSpace aspString2 {}", aspString);
								break;
							}
						}
					}
				}

				// Filter: MusicType
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

				// Filter: SkyRegion
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

				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::TESObjectCELL* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Cell) {
							npc = (RE::TESObjectCELL*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("Cell Excluded.");
								break;
							}
						}
					}
				}
				////Flags
				//curobj->cellFlags;
				////, LTMP
				//curobj->lightingTemplate;
				////FogNear, FogFar
				//curobj->cellData.interior->fogNear;
				//curobj->cellData.interior->fogFar;
				////, , XCCM, XCIM
				//curobj->loadedData;

				//if (!line.filterByFlagsExclude.empty()) {
				//	for (const auto& flag : line.filterByFlagsExclude) {
				//		//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude"), curobj->formID, curobj->fullName);
				//		if (toLowerCase(flag) == "notplayable" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kNotPlayable) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude nonplayable set to false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "cantdrop" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCantDrop) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "embeddedweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kEmbeddedWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingattack" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingAttack) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "criteffectondeath" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kCritEffectOnDeath) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "boltaction" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kBoltAction) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "automatic" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kAutomatic) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "secondaryweapon" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kSecondaryWeapon) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "chargingreload" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kChargingReload) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		} else if (toLowerCase(flag) == "holdinputtopower" && curobj->weaponData.flags & RE::WEAPONDATAFLAGS::kHoldInputToPower) {
				//			//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
				//			found = false;
				//		}
				//	}
				//}

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

	void* patch(CELL::patch_instruction line, RE::TESObjectCELL* curobj)
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
				curobj->waterHeight = -129.0f;  // Standardwert für "kein Wasser"
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
			line.acousticSpace != "none") {
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
			line.musicType != "none") {
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

		if (!line.skyRegion.empty() &&
			line.skyRegion != "none") {
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
			line.imageSpace != "none") {
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
			line.encounterZone != "none") {
			RE::TESForm* form = GetFormFromIdentifier(line.encounterZone);
			if (form && form->formType == RE::FormType::EncounterZone) {
				auto* imsp = static_cast<RE::BGSEncounterZone*>(form);

				if (curobj->extraList.HasType<RE::ExtraCellImageSpace>()) {
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
		} else if (toLowerCase(line.imageSpace) == "null") {
			if (curobj->extraList.HasType<RE::ExtraCellImageSpace>()) {
				curobj->extraList.RemoveByType(RE::ExtraDataType::kEncounterZone);
				logger::debug(FMT_STRING("cell formid {:08X} removed encounter zone"), curobj->formID);
			}
		}

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
			} catch (const std::invalid_argument&) {}
		}

		if (!line.fogColorNearGreen.empty() && line.fogColorNearGreen != "none") {
			try {
				curobj->cellData.interior->fogColorNear.green = static_cast<std::uint8_t>(std::stoi(line.fogColorNearGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogColorNear.green to {}"), curobj->formID, curobj->cellData.interior->fogColorNear.green);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.fogColorNearBlue.empty() && line.fogColorNearBlue != "none") {
			try {
				curobj->cellData.interior->fogColorNear.blue = static_cast<std::uint8_t>(std::stoi(line.fogColorNearBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogColorNear.blue to {}"), curobj->formID, curobj->cellData.interior->fogColorNear.blue);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.directionalRed.empty() && line.directionalRed != "none") {
			try {
				curobj->cellData.interior->directional.red = static_cast<std::uint8_t>(std::stoi(line.directionalRed));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalRed to {}"), curobj->formID, curobj->cellData.interior->directional.red);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.directionalGreen.empty() && line.directionalGreen != "none") {
			try {
				curobj->cellData.interior->directional.green = static_cast<std::uint8_t>(std::stoi(line.directionalGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalGreen to {}"), curobj->formID, curobj->cellData.interior->directional.green);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.directionalBlue.empty() && line.directionalBlue != "none") {
			try {
				curobj->cellData.interior->directional.blue = static_cast<std::uint8_t>(std::stoi(line.directionalBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed directionalBlue to {}"), curobj->formID, curobj->cellData.interior->directional.blue);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.ambientBlue.empty() && line.ambientBlue != "none") {
			try {
				curobj->cellData.interior->ambient.blue = static_cast<std::uint8_t>(std::stoi(line.ambientBlue));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientBlue to {}"), curobj->formID, curobj->cellData.interior->ambient.blue);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.ambientRed.empty() && line.ambientRed != "none") {
			try {
				curobj->cellData.interior->ambient.red = static_cast<std::uint8_t>(std::stoi(line.ambientRed));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientRed to {}"), curobj->formID, curobj->cellData.interior->ambient.red);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.ambientGreen.empty() && line.ambientGreen != "none") {
			try {
				curobj->cellData.interior->ambient.green = static_cast<std::uint8_t>(std::stoi(line.ambientGreen));
				logger::debug(FMT_STRING("cell formid: {:08X} changed ambientGreen to {}"), curobj->formID, curobj->cellData.interior->ambient.green);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.fogNear.empty() && line.fogNear != "none") {
			try {
				curobj->cellData.interior->fogNear = std::stof(line.fogNear);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogNear to {}"), curobj->formID, curobj->cellData.interior->fogNear);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.fogFar.empty() && line.fogFar != "none") {
			try {
				curobj->cellData.interior->fogFar = std::stof(line.fogFar);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogFar to {}"), curobj->formID, curobj->cellData.interior->fogFar);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.fogPower.empty() && line.fogPower != "none") {
			try {
				curobj->cellData.interior->fogPower = std::stof(line.fogPower);
				logger::debug(FMT_STRING("cell formid: {:08X} changed fogPower to {}"), curobj->formID, curobj->cellData.interior->fogPower);
			} catch (const std::invalid_argument&) {}
		}

		if (!line.clipDist.empty() && line.clipDist != "none") {
			try {
				curobj->cellData.interior->clipDist = std::stof(line.clipDist);
				logger::debug(FMT_STRING("cell formid: {:08X} changed clipDist to {}"), curobj->formID, curobj->cellData.interior->clipDist);
			} catch (const std::invalid_argument&) {}
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

		//curobj->cellData.interior->


		//logger::debug("LLight 111 {}", line.lightingTemplate);
		if (!line.lightingTemplate.empty() && line.lightingTemplate != "none") {
			RE::TESForm* currentform = nullptr;
			std::string  string_form = line.lightingTemplate;
			currentform = GetFormFromIdentifier(string_form);
			//logger::debug("LLight 1 {}", string_form);
			if (currentform && currentform->formType == RE::FormType::LightingMaster) {
				//logger::debug("LLight 2");
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
			} catch (const std::invalid_argument& e) {
			}
		}


		//logger::info("Starting Patching");
		////lightningTemplate
		//if (curobj->loadedData && curobj->loadedData->t_unk028)
		//	logger::info("curobj->loadedData t_unk028 {:08X}", curobj->loadedData->t_unk028->formID);
		//if (curobj->loadedData && curobj->loadedData->t_unk030)
		//	logger::info("curobj->loadedData t_unk030 {:08X}", curobj->loadedData->t_unk030->formID);
		//if (curobj->loadedData && curobj->loadedData->t_unk038)
		//	logger::info("curobj->loadedData t_unk038 {:08X}", curobj->loadedData->t_unk038->formID);

		//if (!line.lightningTemplate.empty() && line.lightningTemplate != "none")
		//{
		//	try {
		//		RE::TESForm* currentform = nullptr;
		//		std::string  string_form = line.lightningTemplate;
		//		currentform = GetFormFromIdentifier(string_form);
		//		if (currentform && currentform->formType == RE::FormType::LightingMaster) {
		//			curobj->lightingTemplate = (RE::BGSLightingTemplate*)currentform;
		//			logger::debug(FMT_STRING("race formid: {:08X} {} changed lightingTemplate {:08X}"), curobj->formID, curobj->fullName, curobj->lightingTemplate->formID);
		//		}
		//	} catch (const std::invalid_argument& e) {
		//	}
		//}

		//if (!line.lightningTemplate.empty() && line.lightningTemplate != "none") {
		//	try {
		//		RE::TESForm* currentform = nullptr;
		//		std::string  string_form = line.lightningTemplate;
		//		currentform = GetFormFromIdentifier(string_form);
		//		if (currentform && currentform->formType == RE::FormType::LightingMaster) {
		//			//curobj-> = (RE::BGSLightingTemplate*)currentform;
		//			logger::debug(FMT_STRING("race formid: {:08X} {} changed lightingTemplate {:08X}"), curobj->formID, curobj->fullName, curobj->lightingTemplate->formID);
		//		}
		//	} catch (const std::invalid_argument& e) {
		//	}
		//}
		return 0;
	}

}
