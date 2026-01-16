#include "magiceffect.h"
#include <utility.h>

namespace MGEF
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;
			

		extractData(line, "filterByMgefs\\s*=([^:]+)", l.object);
		extractData(line, "filterBySounds\\s*=([^:]+)", l.filterBySounds);
		extractValueString(line, "filterByDamageHealthMgefs\\s*=([^:]+)", l.filterByDamageHealthMgefs);

		extractData(line, "filterByMgefsExcluded\\s*=([^:]+)", l.objectExcluded);
		
		extractData(line, "effectShadersExcluded\\s*=([^:]+)", l.effectExcluded);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		

		//extractValueString(line, "speed\\s *= ([^:] +)", l.speed);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);
		extractValueString(line, "magicItemDescription\\s*=\\s*~([^~]+)~", l.magicItemDescription);
		extractValueString(line, "enchantArt\\s*=([^:]+)", l.enchantArt);
		extractValueString(line, "enchantShader\\s*=([^:]+)", l.enchantShader);
		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);
		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		
		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractDataStrings(line, "modNamesLastOverriddenExcluded\\s*=([^:]+)", l.filterByModNamesLastOverriddenExcluded);
		extractDataStrings(line, "modNamesExcluded\\s*=([^:]+)", l.ModNamesExcluded);
		extractDataStrings(line, "hitShaderExcluded\\s*=([^:]+)", l.hitShaderExcluded);
		

		extractValueString(line, "impactDataSet\\s*=([^:]+)", l.impactDataSet);
		extractValueString(line, "baseCost\\s*=([^:]+)", l.baseCost);
		extractValueString(line, "minimumSkillLevel\\s*=([^:]+)", l.minimumSkillLevel);
		extractValueString(line, "spellmakingCastingTime\\s*=([^:]+)", l.spellmakingCastingTime);
		extractValueString(line, "spellmakingArea\\s*=([^:]+)", l.spellmakingArea);
		extractValueString(line, "projectile\\s*=([^:]+)", l.projectile);
		extractValueString(line, "castingType\\s*=([^:]+)", l.castType);
		extractValueString(line, "castingArt\\s*=([^:]+)", l.castingArt);
		extractValueString(line, "perkToApply\\s*=([^:]+)", l.perk);
		extractData(line, "restrictToCastingType\\s*=([^:]+)", l.restrictToCastingType);
		extractData(line, "restrictToDeliveryType\\s*=([^:]+)", l.restrictToDeliveryType);
		extractData(line, "restrictToMagicSkill\\s*=([^:]+)", l.restrictToMagicSkill);
		

		extractValueString(line, "removeEdgeGlow\\s*=([^:]+)", l.removeEdgeGlow);
		extractValueString(line, "skillUsageMult\\s*=([^:]+)", l.skillUsageMult);
		
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);

		extractDataStrings(line, "filterByArchetypes\\s*=([^:]+)", l.filterArchetype);
		extractDataStrings(line, "restrictToActorValue\\s*=([^:]+)", l.restrictToActorValue);
		extractValueString(line, "restrictToAreaEffects\\s*=([^:]+)", l.restrictToArea);
		extractValueString(line, "restrictToDetrimentalEffects\\s*=([^:]+)", l.restrictToDetrimental);
		extractDataStrings(line, "restrictToResistType\\s*=([^:]+)", l.restrictToResistType);
		extractValueString(line, "hitShader\\s*=([^:]+)", l.hitShader);
		extractToArr2D(line, "soundsToChange\\s*=([^:]+)", l.soundsToChange);
		
		return l;
	}  //endStruct

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::EffectSetting*> ObjectArray = dataHandler->GetFormArray<RE::EffectSetting>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::EffectSetting* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::MagicEffect) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::EffectSetting*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.filterByEditorIdContains.empty() && line.filterBySounds.empty() && line.filterByEditorIdContainsOr.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterArchetype.empty() && line.filterByDamageHealthMgefs == "none" ) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : ObjectArray) {
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool contains = false;
				bool containsOr = false;

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

				

				if (!line.ModNamesExcluded.empty())
				{
					bool modFound = false;
					for (auto const modName : line.ModNamesExcluded) {
						if (modName == curobj->GetFile(0)->fileName) {
							modFound = true;
							break;
						}
					}
					if (modFound) {
						continue;
					}
				}

				if (!line.filterByModNamesLastOverriddenExcluded.empty()) {
					bool modFound = false;
					for (auto const modName : line.filterByModNamesLastOverriddenExcluded) {
						if (modName == curobj->GetFile()->fileName) {
							modFound = true;
							break;
						}
					}
					if (modFound) {
						continue;
					}
				}

				//

				//
				if (!line.filterArchetype.empty()) {
					bool foundArchtype = false;
					for (auto string : line.filterArchetype) {
						string = toLowerCase(string);
						if (string == "absorb" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kAbsorb) {
							foundArchtype = true;
						}
						else if (string == "accumulatemagnitude" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kAccumulateMagnitude) {
							foundArchtype = true;
						}
						else if (string == "valuemodifier" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kValueModifier) {
							foundArchtype = true;
						}
						else if (string == "script" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kScript) {
							foundArchtype = true;
						}
						else if (string == "dispel" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDispel) {
							foundArchtype = true;
						}
						else if (string == "curedisease" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCureDisease) {
							foundArchtype = true;
						}
						else if (string == "dualvaluemodifier" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDualValueModifier) {
							foundArchtype = true;
						}
						else if (string == "calm" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCalm) {
							foundArchtype = true;
						}
						else if (string == "demoralize" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDemoralize) {
							foundArchtype = true;
						}
						else if (string == "frenzy" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kFrenzy) {
							foundArchtype = true;
						}
						else if (string == "disarm" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDisarm) {
							foundArchtype = true;
						}
						else if (string == "commandsummoned" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCommandSummoned) {
							foundArchtype = true;
						}
						else if (string == "invisibility" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kInvisibility) {
							foundArchtype = true;
						}
						else if (string == "light" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kLight) {
							foundArchtype = true;
						}
						else if (string == "darkness" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDarkness) {
							foundArchtype = true;
						}
						else if (string == "nighteye" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kNightEye) {
							foundArchtype = true;
						}
						else if (string == "lock" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kLock) {
							foundArchtype = true;
						}
						else if (string == "open" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kOpen) {
							foundArchtype = true;
						}
						else if (string == "boundweapon" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kBoundWeapon) {
							foundArchtype = true;
						}
						else if (string == "summoncreature" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kSummonCreature) {
							foundArchtype = true;
						}
						else if (string == "detectlife" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDetectLife) {
							foundArchtype = true;
						}
						else if (string == "telekinesis" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kTelekinesis) {
							foundArchtype = true;
						}
						else if (string == "paralysis" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kParalysis) {
							foundArchtype = true;
						}
						else if (string == "reanimate" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kReanimate) {
							foundArchtype = true;
						}
						else if (string == "soultrap" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kSoulTrap) {
							foundArchtype = true;
						}
						else if (string == "turnundead" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kTurnUndead) {
							foundArchtype = true;
						}
						else if (string == "guide" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kGuide) {
							foundArchtype = true;
						}
						else if (string == "werewolffeed" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kWerewolfFeed) {
							foundArchtype = true;
						}
						else if (string == "cureparalysis" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCureParalysis) {
							foundArchtype = true;
						}
						else if (string == "cureaddiction" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCureAddiction) {
							foundArchtype = true;
						}
						else if (string == "curepoison" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCurePoison) {
							foundArchtype = true;
						}
						else if (string == "concussion" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kConcussion) {
							foundArchtype = true;
						}
						else if (string == "valueandparts" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kValueAndParts) {
							foundArchtype = true;
						}
						else if (string == "stagger" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kStagger) {
							foundArchtype = true;
						}
						else if (string == "peakvaluemodifier" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kPeakValueModifier) {
							foundArchtype = true;
						}
						else if (string == "cloak" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kCloak) {
							foundArchtype = true;
						}
						else if (string == "werewolf" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kWerewolf) {
							foundArchtype = true;
						}
						else if (string == "slowtime" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kSlowTime) {
							foundArchtype = true;
						}
						else if (string == "rally" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kRally) {
							foundArchtype = true;
						}
						else if (string == "enhanceweapon" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kEnhanceWeapon) {
							foundArchtype = true;
						}
						else if (string == "spawnhazard" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kSpawnHazard) {
							foundArchtype = true;
						}
						else if (string == "etherealize" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kEtherealize) {
							foundArchtype = true;
						}
						else if (string == "banish" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kBanish) {
							foundArchtype = true;
						}
						else if (string == "spawnscriptedref" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kSpawnScriptedRef) {
							foundArchtype = true;
						}
						else if (string == "disguise" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDisguise) {
							foundArchtype = true;
						}
						else if (string == "grabactor" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kGrabActor) {
							foundArchtype = true;
						}
						else if (string == "vampirelord" && curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kVampireLord) {
							foundArchtype = true;
						}
					}
					if (foundArchtype)
						found = true;
				}

				if (!found && !line.filterByEditorIdContains.empty() ) {
					for (const auto& editorString : line.filterByEditorIdContains) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							contains = true;
						} else {
							contains = false;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					contains = true;
				}

				if (!found && !line.filterByEditorIdContainsOr.empty() ) {
					//logger::info("keywords not empty");
					for (const auto& editorString : line.filterByEditorIdContainsOr) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							containsOr = true;
							break;
						}
					}
				} else if (curobj->formID != 0x000007) {
					containsOr = true;
				}

				if (!found && (!line.filterByEditorIdContains.empty() || !line.filterByEditorIdContainsOr.empty()) && contains && containsOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd Weapon does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd Weapon true"));
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (line.filterByDamageHealthMgefs != "none") {
					if (curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kValueModifier || curobj->data.archetype == RE::EffectArchetypes::ArchetypeID::kDualValueModifier) {
						if (toLowerCase(line.filterByDamageHealthMgefs) == "hostile") {
							if ((curobj->data.primaryAV == RE::ActorValue::kHealth || curobj->data.secondaryAV == RE::ActorValue::kHealth) && curobj->IsHostile() && curobj->IsDetrimental()) {
								found = true;
							}
						} else {
							if ((curobj->data.primaryAV == RE::ActorValue::kHealth || curobj->data.secondaryAV == RE::ActorValue::kHealth) && curobj->IsDetrimental()) {
								found = true;
							}
						}

					}
						
				}

				if (!found && !line.filterBySounds.empty()) {
					for (const auto& soundFormStr : line.filterBySounds) {
						RE::TESForm* form = GetFormFromIdentifier(soundFormStr);
						if (!form || form->formType != RE::FormType::SoundRecord) {
							logger::warn(FMT_STRING("mgef - Invalid sound filter: {}"), soundFormStr);
							continue;
						}

						auto* targetSound = static_cast<RE::BGSSoundDescriptorForm*>(form);

						// Check if the sound is present in curobj's effectSounds
						bool match = std::any_of(
							curobj->effectSounds.begin(),
							curobj->effectSounds.end(),
							[targetSound](const RE::EffectSetting::SoundPair& pair) {
								return pair.sound == targetSound;
							}
						);

						if (match) {
							found = true;
							//logger::debug(FMT_STRING("npc {:08X} matched sound filter: {}"), curobj->formID, soundFormStr);
							break;
						}
					}
				}


				if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterBySounds.empty() && line.filterArchetype.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByDamageHealthMgefs == "none") {
					found = true;
				}



				if (!line.objectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm*       currentform = nullptr;
						RE::EffectSetting* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::MagicEffect) {
							npc = (RE::EffectSetting*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("Mgef Excluded.");
								break;
							}
						}
					}
				}

				if (!line.effectExcluded.empty() && found) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.effectExcluded) {
						RE::TESForm*       currentform = nullptr;
						//RE::EffectSetting* npc = nullptr;
						RE::TESEffectShader* effs = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							effs = (RE::TESEffectShader*)currentform;
							
							if (effs && curobj->data.effectShader) {

								if (curobj->data.effectShader->formID == effs->formID) {
									found = false;
									//logger::info("effs Excluded.");
									break;
								}
							}
						}
					}
				}

				if (!line.keywordsExcluded.empty() && found) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm*    currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::FormType::Keyword) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"),keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.filterByEditorIdContainsExcluded.empty() && found ) {
					//logger::info("factions not empty");
					for (const auto& editorString : line.filterByEditorIdContainsExcluded) {
						if (toLowerCase(clib_util::editorID::get_editorID(curobj)).find(toLowerCase(editorString)) != std::string::npos) {
							found = false;
							break;
						}
					}
				}

				if (found && !line.restrictToCastingType.empty()) {
					bool foundCastingType = false;
					for (auto string : line.restrictToCastingType) {
						string = toLowerCase(string);
						if (string == "concentration" && curobj->data.castingType == RE::MagicSystem::CastingType::kConcentration) {
							foundCastingType = true;
						} else if (string == "constanteffect" && curobj->data.castingType == RE::MagicSystem::CastingType::kConstantEffect) {
							foundCastingType = true;
						} else if (string == "fireandforget" && curobj->data.castingType == RE::MagicSystem::CastingType::kFireAndForget) {
							foundCastingType = true;
						} else if (string == "scroll" && curobj->data.castingType == RE::MagicSystem::CastingType::kScroll) {
							foundCastingType = true;
						}
					}
					if (!foundCastingType)
						found = false;
				}

				if (found && !line.restrictToDeliveryType.empty()) {
					bool foundDeliveryType = false;
					for (auto string : line.restrictToDeliveryType) {
						string = toLowerCase(string);
						if (string == "self" && curobj->data.delivery == RE::MagicSystem::Delivery::kSelf) {
							foundDeliveryType = true;
						}
						else if (string == "touch" && curobj->data.delivery == RE::MagicSystem::Delivery::kTouch) {
							foundDeliveryType = true;
						}
						else if (string == "aimed" && curobj->data.delivery == RE::MagicSystem::Delivery::kAimed) {
							foundDeliveryType = true;
						}
						else if (string == "targetactor" && curobj->data.delivery == RE::MagicSystem::Delivery::kTargetActor) {
							foundDeliveryType = true;
						}
						else if (string == "targetlocation" && curobj->data.delivery == RE::MagicSystem::Delivery::kTargetLocation) {
							foundDeliveryType = true;
						}

					}
					if (!foundDeliveryType)
						found = false;
				}


				

				//


				if (found && !line.restrictToResistType.empty()) {
					bool foundType = false;
					for (auto string : line.restrictToResistType) {
						string = toLowerCase(string);
						if (string == "resistdisease" && curobj->data.resistVariable == RE::ActorValue::kResistDisease) {
							foundType = true;
						}
						else if (string == "resistfire" && curobj->data.resistVariable == RE::ActorValue::kResistFire) {
							foundType = true;
						}
						else if (string == "resistfrost" && curobj->data.resistVariable == RE::ActorValue::kResistFrost) {
							foundType = true;
						}
						else if (string == "resistmagic" && curobj->data.resistVariable == RE::ActorValue::kResistMagic) {
							foundType = true;
						}
						else if (string == "resistshock" && curobj->data.resistVariable == RE::ActorValue::kResistShock) {
							foundType = true;
						}
						else if (string == "resistpoison" && curobj->data.resistVariable == RE::ActorValue::kPoisonResist) {
							foundType = true;
						}
					}
					if (!foundType)
						found = false;
				}

				if (found && !line.restrictToMagicSkill.empty()) {
					bool foundType = false;
					for (auto string : line.restrictToMagicSkill) {
						string = toLowerCase(string);
						if (curobj->data.associatedSkill == StringToActorValue(string)) {
							foundType = true;
						}
					}
					if (!foundType)
						found = false;
				}

				if (found && !line.restrictToArea.empty()) {
						auto string = toLowerCase(line.restrictToArea);
						if (string == "true" && curobj->data.spellmakingArea <= 0) {
							found = false;
						}
						if (string == "false" && curobj->data.spellmakingArea > 0) {
							found = false;
						}
				}

				if (found && !line.restrictToDetrimental.empty()) {
					auto string = toLowerCase(line.restrictToDetrimental);
					if (string == "true" && !curobj->IsDetrimental()) {
						found = false;
					}
					if (string == "false" && curobj->IsDetrimental()) {
						found = false;
					}
				}

				if (found && !line.restrictToActorValue.empty()) {
					bool foundType = false;
					for (auto string : line.restrictToActorValue) {
						string = toLowerCase(string); // Ensure the string is lowercase
							if(curobj->data.primaryAV == StringToActorValue(string) || curobj->data.secondaryAV == StringToActorValue(string)) {
							foundType = true;
						}
					}
					if (!foundType) {
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

	void* patch(MGEF::patch_instruction line, RE::EffectSetting* curobj)
	{
		
		if (!line.enchantArt.empty() && line.enchantArt != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.enchantArt;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ArtObject) {
					curobj->data.enchantEffectArt = (RE::BGSArtObject*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed enchantEffectArt {:08X}"), curobj->formID, curobj->fullName, curobj->data.enchantEffectArt->formID);
				} else if (line.enchantArt == "null" || line.enchantArt == "clear") {
					curobj->data.enchantEffectArt = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed enchantEffectArt null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.enchantShader.empty() && line.enchantShader != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.enchantShader;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EffectShader) {
					curobj->data.effectShader = (RE::TESEffectShader*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed enchantShader {:08X}"), curobj->formID, curobj->fullName, curobj->data.effectShader->formID);
				} else if (line.enchantShader == "null" || line.enchantShader == "clear") {
					curobj->data.effectShader = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed enchantShader null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.hitShader.empty() && line.hitShader != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.hitShader;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EffectShader) {
					curobj->data.effectShader = (RE::TESEffectShader*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed hitShader {:08X}"), curobj->formID, curobj->fullName, curobj->data.effectShader->formID);
				} else if (line.hitShader == "null" || line.hitShader == "clear") {
					curobj->data.effectShader = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} changed hitShader null (none)"), curobj->formID);
				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.setFlags.empty()) {
			for (const auto& flag : line.setFlags) {
				if (toLowerCase(flag) == "hostile") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kHostile);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Hostile"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "recover") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kRecover);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Recover"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "detrimental") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Detrimental"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "snaptomesh") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kSnapToNavMesh);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled SnapToNavMesh"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nohitevent") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoHitEvent);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoHitEvent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "dispelwithkeywords") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled DispelWithKeywords"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "noduration") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nomagnitude") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "noarea") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoArea);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoArea"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "fxpersist") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kFXPersist);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled FXPersist"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "goryvisuals") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kGoryVisuals);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled GoryVisuals"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "hideinui") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kHideInUI);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled HideInUI"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "norecast") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoRecast);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoRecast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "poweraffectsmagnitude") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled PowerAffectsMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "poweraffectsduration") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled PowerAffectsDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "painless") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kPainless);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled Painless"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nohiteffect") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoHitEffect);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoHitEffect"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(flag) == "nodeathdispel") {
					curobj->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoDeathDispel);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag enabled NoDeathDispel"), curobj->formID, curobj->fullName);
				}
				else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		// Remove Flags
		if (!line.removeFlags.empty()) {
			for (const auto& removeFlag : line.removeFlags) {
				if (toLowerCase(removeFlag) == "hostile") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kHostile);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Hostile"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "recover") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kRecover);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Recover"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "detrimental") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Detrimental"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "snaptomesh") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kSnapToNavMesh);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled SnapToNavMesh"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nohitevent") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoHitEvent);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoHitEvent"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "dispelwithkeywords") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled DispelWithKeywords"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "noduration") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nomagnitude") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "noarea") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoArea);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoArea"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "fxpersist") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kFXPersist);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled FXPersist"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "goryvisuals") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kGoryVisuals);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled GoryVisuals"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "hideinui") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kHideInUI);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled HideInUI"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "norecast") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoRecast);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoRecast"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "poweraffectsmagnitude") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsMagnitude);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled PowerAffectsMagnitude"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "poweraffectsduration") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsDuration);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled PowerAffectsDuration"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "painless") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kPainless);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled Painless"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nohiteffect") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoHitEffect);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoHitEffect"), curobj->formID, curobj->fullName);
				} else if (toLowerCase(removeFlag) == "nodeathdispel") {
					curobj->data.flags.reset(RE::EffectSetting::EffectSettingData::Flag::kNoDeathDispel);
					logger::debug(FMT_STRING("mgef setting formid: {:08X} {} flag disabled NoDeathDispel"), curobj->formID, curobj->fullName);
				} else {
					// Handle the case where an invalid flag is provided
					// You might want to add error handling or logging here
				}
			}
		}

		

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("mgef formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.magicItemDescription.empty() && line.magicItemDescription != "none") {
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed magic item description to {}"), curobj->formID, curobj->fullName, line.magicItemDescription);
				curobj->magicItemDescription = line.magicItemDescription;
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("mgef formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.impactDataSet.empty() && line.impactDataSet != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.impactDataSet;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ImpactDataSet) {
					curobj->data.impactDataSet = (RE::BGSImpactDataSet*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed impactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->data.impactDataSet->formID);
				}
				} else if (toLowerCase(string_form) == "null") {
				curobj->data.impactDataSet = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed impactDataSet to null"), curobj->formID, curobj->fullName);

				}
			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.baseCost.empty() && line.baseCost != "none") {
			curobj->data.baseCost = std::stof(line.baseCost);
			logger::debug(FMT_STRING("mgef {:08X} {} changed base cost to {}"), curobj->formID, curobj->fullName, curobj->data.baseCost);
		}

		if (!line.minimumSkillLevel.empty() && line.minimumSkillLevel != "none") {
			curobj->data.minimumSkill = std::stoi(line.minimumSkillLevel);
			logger::debug(FMT_STRING("mgef {:08X} {} changed minimum skill level to {}"), curobj->formID, curobj->fullName, curobj->data.minimumSkill);
		}

		if (!line.spellmakingCastingTime.empty() && line.spellmakingCastingTime != "none") {
			curobj->data.spellmakingChargeTime = std::stof(line.spellmakingCastingTime);
			logger::debug(FMT_STRING("mgef {:08X} {} changed spellmaking casting time to {}"), curobj->formID, curobj->fullName, curobj->data.spellmakingChargeTime);
		}

		if (!line.spellmakingArea.empty() && line.spellmakingArea != "none") {
			curobj->data.spellmakingArea = std::stoi(line.spellmakingArea);
			logger::debug(FMT_STRING("mgef {:08X} {} changed spellmaking area to {}"), curobj->formID, curobj->fullName, curobj->data.spellmakingArea);
		}

		if (!line.skillUsageMult.empty() && line.skillUsageMult != "none") {
			curobj->data.skillUsageMult = std::stof(line.skillUsageMult);
			logger::debug(FMT_STRING("mgef {:08X} {} changed skillUsageMult to {}"), curobj->formID, curobj->fullName, curobj->data.skillUsageMult);
		}

		if (!line.castType.empty() && line.castType != "none") {
			if (toLowerCase(line.castType) == "concentration") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kConcentration);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to concentration"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "constanteffect") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kConstantEffect);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to constanteffect"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "fireandforget") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kFireAndForget);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to fireandforget"), curobj->formID, curobj->fullName);
			} else if (toLowerCase(line.castType) == "scroll") {
				curobj->data.castingType = (RE::MagicSystem::CastingType::kScroll);
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingType to scroll"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.projectile.empty() && line.projectile != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.projectile;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Projectile) {
					curobj->data.projectileBase = (RE::BGSProjectile*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed projectileBase {:08X}"), curobj->formID, curobj->fullName, curobj->data.projectileBase->formID);
				}
				} else if (toLowerCase(string_form) == "null") {
					curobj->data.projectileBase = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed projectileBase to null"), curobj->formID, curobj->fullName);

				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.castingArt.empty() && line.castingArt != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.projectile;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ArtObject) {
					curobj->data.castingArt = (RE::BGSArtObject*)currentform;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingArt {:08X}"), curobj->formID, curobj->fullName, curobj->data.castingArt->formID);
				}
				} else if (toLowerCase(string_form) == "null" || toLowerCase(string_form) == "clear") {
				curobj->data.castingArt = nullptr;
				logger::debug(FMT_STRING("mgef formid: {:08X} {} changed castingArt to null"), curobj->formID, curobj->fullName);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.perk.empty() && line.perk != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.perk;
				if (toLowerCase(string_form) != "null") {
				currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Perk) {
						curobj->data.perk = (RE::BGSPerk*)currentform;
						logger::debug(FMT_STRING("mgef formid: {:08X} {} changed perk {:08X}"), curobj->formID, curobj->fullName, curobj->data.perk->formID);
					}
				} else if (toLowerCase(string_form) == "null" || toLowerCase(string_form) == "clear") {
					curobj->data.perk = nullptr;
					logger::debug(FMT_STRING("mgef formid: {:08X} {} changed perk to null"), curobj->formID, curobj->fullName);
				}

			} catch (const std::invalid_argument& e) {
			}
		}

		if (!line.removeEdgeGlow.empty() && line.removeEdgeGlow != "none") {
			RE::TESEffectShader* effectShader = curobj->data.effectShader;
			
			if (effectShader) {
				effectShader->data.fillTextureEffectPersistentAlphaRatio = 0.f;
				effectShader->data.fillTextureEffectFullAlphaRatio = 0.f;
				effectShader->data.edgeEffectPersistentAlphaRatio = 0.f;
				effectShader->data.edgeEffectFullAlphaRatio = 0.f;
				logger::debug(FMT_STRING("mgef {:08X} {} removed edge glow"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.soundsToChange.empty()) {
			for (const auto& soundEntry : line.soundsToChange) {
				if (soundEntry.size() < 2) {
					logger::warn("Invalid soundEntry: expected at least 2 elements (form + name), got {}", soundEntry.size());
					continue;
				}

				std::string spellFormStr = soundEntry[0];
				std::string soundName = toLowerCase(soundEntry[1]);

				// Convert sound name to enum
				auto soundIDOpt = [&]() -> std::optional<RE::MagicSystem::SoundID> {
					static const std::unordered_map<std::string, RE::MagicSystem::SoundID> soundNameToID = {
						{ "drawsheathe", RE::MagicSystem::SoundID::kDrawSheatheLPM },
						{ "charge",      RE::MagicSystem::SoundID::kCharge },
						{ "readyloop",   RE::MagicSystem::SoundID::kReadyLoop },
						{ "release",     RE::MagicSystem::SoundID::kRelease },
						{ "castloop",    RE::MagicSystem::SoundID::kCastLoop },
						{ "hit",         RE::MagicSystem::SoundID::kHit }
					};
					if (auto it = soundNameToID.find(soundName); it != soundNameToID.end()) {
						return it->second;
					}
					return std::nullopt;
					}();

				if (!soundIDOpt) {
					logger::debug(FMT_STRING("mgef {:08X} invalid sound name: {}"), curobj->formID, soundName);
					continue;
				}

				RE::MagicSystem::SoundID soundID = *soundIDOpt;

				if (spellFormStr == "null") {
					// Remove sound if it exists
					auto& sounds = curobj->effectSounds;
					auto it = std::remove_if(sounds.begin(), sounds.end(),
						[soundID](const RE::EffectSetting::SoundPair& pair) {
							return pair.id == soundID;
						});

					if (it != sounds.end()) {
						logger::debug(FMT_STRING("mgef {:08X} removed sound {}"), curobj->formID, soundName);
						sounds.erase(it);
					}
					else {
						logger::debug(FMT_STRING("mgef {:08X} sound {} not found for removal"), curobj->formID, soundName);
					}
					continue;
				}

				RE::BGSSoundDescriptorForm* spellSound = (RE::BGSSoundDescriptorForm*)GetFormFromIdentifier(spellFormStr);
				if (!spellSound) {
					logger::critical(FMT_STRING("mgef {:08X} spell sound not found: {}"), curobj->formID, spellFormStr);
					continue;
				}

				auto it = std::find_if(
					curobj->effectSounds.begin(),
					curobj->effectSounds.end(),
					[soundID](const RE::EffectSetting::SoundPair& pair) {
						return pair.id == soundID;
					}
				);

				if (it != curobj->effectSounds.end()) {
					// Replace the existing sound
					it->sound = spellSound;
					logger::debug(FMT_STRING("mgef {:08X} sound {} replaced on {}"), curobj->formID, soundName, spellFormStr);
				}
				else {
					// Add new sound
					logger::debug(FMT_STRING("mgef {:08X} sound add {} to {}"), curobj->formID, soundName, spellFormStr);
					curobj->effectSounds.push_back(RE::EffectSetting::SoundPair{ soundID, 0, spellSound });
				}
			}
		}





		


		//if (!line.sound.empty() && line.sound != "none") {
		//	try {
		//		RE::TESForm* currentform = nullptr;
		//		std::string  string_form = line.sound;
		//		currentform = GetFormFromIdentifier(string_form);
		//		if (currentform && currentform->formType == RE::FormType::SoundRecord) {
		//			curobj->data.so = (RE::BGSSoundDescriptorForm*)currentform;
		//			logger::debug(FMT_STRING("projectile formid: {:08X} {} changed sound {:08X}"), curobj->formID, curobj->fullName, curobj->data.activeSoundLoop->formID);
		//		}
		//	} catch (const std::invalid_argument& e) {
		//	}
		//}


	}

}
