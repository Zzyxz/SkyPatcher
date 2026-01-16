#include "dirent.h"
#include <alch.h>
#include <ammo.h>
#include <armor.h>
#include <book.h>
#include <cell.h>
#include <cobj.h>
#include <container.h>
#include <enchant.h>
#include <encounterzone.h>
#include <faction.h>
#include <formlist.h>
#include <future>
#include <ingredient.h>
#include <leveledlist.h>
#include <magiceffect.h>
#include <misc.h>
#include <movementType.h>
#include <npc.h>
#include <outfit.h>
#include <projectile.h>
#include <race.h>
#include <reference.h>
#include <scroll.h>
#include <soulgem.h>
#include <spell.h>
#include <unordered_map>
#include <weapon.h>
#include <location.h>
#include <unordered_map>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

const char* configFile = ".\\Data\\SKSE\\Plugins\\SkyPatcher.ini";
const char* settingsSection = "Patcher";
const char* settingsSectionLog = "Log";
const char* settingsSectionFeatures = "Features";
NPC::ActorData                        actData;
REFERENCE::RefrData                   refData;
bool                                  statRefreshEnabled = false;
bool                                  npcPatchingEnabled = false;
bool                                  RefPatchingEnabled = false;
bool                                  statRefreshExcludeEnabled = false;
std::string                           excludeNPCs = "";
std::vector<std::vector<std::string>> excludedNPCsArr;
std::vector<RE::FormID>               excludedNPCsFormIDs = {};
bool                                  loadOnlyOnce = true;
int									  iUpdateNPCVisualsOnLoad = 1;
RE::BGSKeyword* SkyPatcher_VisualStyleApply = nullptr;
bool								  loadStylesAfterLoad = false;
std::unordered_map<RE::FormID, int> npcStyles;
inline constexpr std::array headSlots = { RE::BIPED_OBJECT::kHair, RE::BIPED_OBJECT::kLongHair, RE::BIPED_OBJECT::kCirclet };
inline constexpr auto HAIR_TINT = "ZZYXZZ_HAIRTINT"sv;

void LoadIniFilesRecursive(const std::string& folderPath)
{
	DIR* dir = opendir(folderPath.c_str());
	if (!dir) {
		logger::error("Failed to open directory: {}", folderPath);
		return;
	}

	dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		std::string name = entry->d_name;

		// Skip "." and ".."
		if (name == "." || name == "..") {
			continue;
		}

		std::string fullPath = folderPath + "\\" + name;

		struct stat s;
		if (stat(fullPath.c_str(), &s) == 0) {
			if (s.st_mode & S_IFDIR) {
				// It's a directory → recurse
				LoadIniFilesRecursive(fullPath);
			} else if (s.st_mode & S_IFREG) {
				// It's a file → check extension
				if (name.size() > 4 && _stricmp(name.substr(name.size() - 4).c_str(), ".ini") == 0) {
					std::string basePath = ".\\Data\\SKSE\\Plugins\\SkyPatcher\\";
					std::string relativePath = fullPath;

					// Kürze den Pfad, wenn er mit basePath beginnt
					if (relativePath.rfind(basePath, 0) == 0) {
						relativePath = relativePath.substr(basePath.length());
					}

					logger::info("Loaded ini files: {}", relativePath);

				}
			}
		}
	}

	closedir(dir);
}

void update_color_data(RE::NiAVObject* a_root, const RE::BSFixedString& a_name, const RE::NiColor& a_color)
{
	if (a_root) {
		if (const auto data = a_root->GetExtraData<RE::NiIntegerExtraData>(a_name); data) {
			if (a_color != RE::NiColor(data->value)) {
				data->value = a_color.ToInt();
			}
		}
		else {
			if (const auto newData = RE::NiIntegerExtraData::Create(a_name, a_color.ToInt()); newData) {
				a_root->AddExtraData(newData);
			}
		}
	}
}

void Enable(RE::TESObjectREFR* obref, bool abFadeIn)
{
	using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, bool);
	RE::VMStackID           frame = 0;
	REL::Relocation<func_t> func{ RELOCATION_ID(55629, 56158) };
	auto                    vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
	return func(vm, frame, obref, abFadeIn);
}

bool read_formID(SKSE::SerializationInterface* a_intfc, RE::FormID& a_formID)
{
	a_intfc->ReadRecordData(a_formID);
	if (a_formID != 0) {
		return a_intfc->ResolveFormID(a_formID, a_formID);
	}
	return true;
}

class UpdateClass
{
public:
	void updateNPCVisuals(RE::Actor* a_this)
	{
		if (a_this && a_this->currentProcess && a_this->currentProcess->InHighProcess()) {
			int count = 0;
			while (!a_this->Get3D(false) && count < 10) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				if (a_this->data.objectReference) {
					logger::debug("3DUpdate: Actor: {:08X} NPC: {:08X} wait for 3D... times waited {}", a_this->formID, a_this->data.objectReference->formID, count);
				}
				else {
					logger::debug("3DUpdate: Actor: {:08X} wait for 3D... times waited {}", a_this->formID, count);

				}
				count++;
			}  // After enabling an actor their 3D model isn't accessible immediately (?). A hook would do the same job but would be more complex

			SKSE::GetTaskInterface()->AddTask([this, a_this] {
				a_this->DoReset3D(true);
				a_this->UpdateSkinColor();
				});

			if (a_this->data.objectReference) {
				logger::debug("3DUpdate: Actor: {:08X} NPC: {:08X} 3D has been reset.", a_this->formID, a_this->data.objectReference->formID);
			}
			else {
				logger::debug("3DUpdate: Actor: {:08X} 3D has been reset.", a_this->formID);

			}

		}
		else {
			if (a_this && a_this->data.objectReference) {
				SKSE::GetTaskInterface()->AddTask([this, a_this] {
					a_this->DoReset3D(true);
					a_this->UpdateSkinColor();
					});
				logger::debug("3DUpdate: Actor: {:08X} NPC: {:08X} is currently not a high process, 3D has been reset without wait.", a_this->formID, a_this->data.objectReference->formID);
			}

		}
	}

	void updateNPCVisualsByEnable(RE::Actor* a_this)
	{
		//while (!a_this->Get3D(false))  // After enabling an actor their 3D model isn't accessible immediately (?). A hook would do the same job but would be more complex
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (a_this) {
			a_this->Disable();
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			Enable(a_this, true);
		}
	}

	void updateREFRAndDisable(RE::TESObjectREFR* a_this)
	{
		//while (!a_this->Get3D(false))  // After enabling an actor their 3D model isn't accessible immediately (?). A hook would do the same job but would be more complex
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (a_this) {
			a_this->Disable();

		}
	}

	void updateNPCVisualsController(RE::Actor* a_this)
	{
		std::future<void> futureA = std::async(std::launch::deferred, &UpdateClass::updateNPCVisuals, this, a_this);
		futureA.get();
	}

	void updateNPCVisualsControllerEnable(RE::Actor* a_this)
	{
		std::future<void> futureA = std::async(std::launch::deferred, &UpdateClass::updateNPCVisualsByEnable, this, a_this);
		futureA.get();
	}

	void disableREFR(RE::TESObjectREFR* a_this)
	{
		std::future<void> futureA = std::async(std::launch::deferred, &UpdateClass::updateREFRAndDisable, this, a_this);
		futureA.get();
	}

	void startUpdate(RE::Actor* a_this)
	{
		SKSE::GetTaskInterface()->AddTask([this, a_this]() {
			//UpdateClass::updateNPCVisualsController(a_this);
			std::jthread t(&UpdateClass::updateNPCVisualsController, this, a_this);
			t.detach();
			});
		// No need to call t.detach() as std::jthread will join on destruction
	}

	void disableAndUpdate(RE::TESObjectREFR* a_this)
	{
		SKSE::GetTaskInterface()->AddTask([this, a_this]() {
			//UpdateClass::updateNPCVisualsController(a_this);
			std::jthread t(&UpdateClass::disableREFR, this, a_this);
			t.detach();
			});
		// No need to call t.detach() as std::jthread will join on destruction
	}

	void startUpdateByEnable(RE::Actor* a_this)
	{
		SKSE::GetTaskInterface()->AddTask([this, a_this]() {
			//UpdateClass::updateNPCVisualsController(a_this);
			std::jthread t(&UpdateClass::updateNPCVisualsControllerEnable, this, a_this);
			t.detach();
			});
		// No need to call t.detach() as std::jthread will join on destruction
	}
};

UpdateClass updater;

class FormDeleteEventSink : public RE::BSTEventSink<RE::TESFormDeleteEvent>
{
public:
	RE::BSEventNotifyControl ProcessEvent(
		RE::TESFormDeleteEvent const* a_event,
		RE::BSTEventSource<RE::TESFormDeleteEvent>* a_eventSource) override
	{
		if (!a_event) {
			return RE::BSEventNotifyControl::kContinue;
		}

		// Extract the deleted FormID
		RE::FormID deletedFormID = a_event->formID;
		//logger::debug("Form with FormID {:X} has been deleted.", deletedFormID);

		npcStyles.erase(deletedFormID);
		//logger::debug("Form with FormID {:X} has been deleted.", deletedFormID);


		return RE::BSEventNotifyControl::kContinue;
	}

	static void Init()
	{
		auto* source = RE::ScriptEventSourceHolder::GetSingleton();
		if (source) {
			source->AddEventSink<RE::TESFormDeleteEvent>(&formDeleteEventSink);
			logger::debug("Registered TESFormDeleteEvent listener.");
		}
		else {
			logger::error("Failed to register TESFormDeleteEvent listener! Event source is null.");
		}
	}

	static FormDeleteEventSink formDeleteEventSink;
};

// **Define the static instance outside the class**
FormDeleteEventSink FormDeleteEventSink::formDeleteEventSink;

//void FormDeleteEventSink::Init()
//{
//	auto* source = RE::ScriptEventSourceHolder::GetSingleton();
//	if (source) {
//		source->AddEventSink(&formDeleteEventSink);
//		logger::info("Registered TESFormDeleteEvent listener.");
//	}
//	else {
//		logger::error("Failed to register TESFormDeleteEvent listener! Event source is null.");
//	}
//}



// Create a singleton instance of the event sink
//FormDeleteEventSink formDeleteEventSink;

void SaveCallback(SKSE::SerializationInterface* serde) {
	constexpr std::uint32_t SAVE_VERSION = 1;
	const auto start_time{ std::chrono::system_clock::now() };


	// Write the size of the map first
	std::size_t mapSize = 0; // We will count valid entries here
	for (const auto& pair : npcStyles) {
		// Check if the NPC (FormID) is still valid
		if (RE::TESForm::LookupByID(pair.first)) {
			mapSize++; // Only count valid FormIDs
		}
	}
	// Write the size of the map first
	serde->WriteRecord('STYL', SAVE_VERSION, mapSize);
	// Write each valid FormID and style pair
	for (const auto& pair : npcStyles) {
		// Only save if the FormID is valid

		if (auto npc = RE::TESForm::LookupByID(pair.first)) {
			// Write the FormID (key)
			serde->WriteRecordData(&pair.first, sizeof(pair.first));
			// Write the style (value)
			serde->WriteRecordData(&pair.second, sizeof(pair.second));
			logger::debug("SaveCallback FormID {:08X} saved.", pair.first);
		}
		else {
			npcStyles.erase(pair.first);
			logger::debug("SaveCallback FormID {:08X} failed to look up and is not saved.", pair.first);
		}
	}
	// Log how long the saving process took
	const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
	double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
	logger::debug(">>>>>>>>>>>>>>>>>>>>>>>>>>> saved {} npc styles in {} seconds", npcStyles.size(), seconds);
}



void OnFormDeleted(RE::TESFormDeleteEvent* a_event) {
	if (!a_event) return;

	RE::FormID deletedFormID = a_event->formID;

	logger::debug("Form with FormID {:X} has been deleted.", deletedFormID);

}

//void RegisterEventListeners() {
//	auto* source = RE::ScriptEventSourceHolder::GetSingleton();
//	if (source) {
//		source->AddEventSink<RE::TESFormDeleteEvent>(&OnFormDeleted);
//		logger::info("Registered TESFormDeleteEvent listener.");
//	}
//}

void LoadCallback(SKSE::SerializationInterface* serde) {
	std::uint32_t type, version, length;
	const auto start_time{ std::chrono::system_clock::now() };

	std::unordered_map<RE::FormID, int> tempNpcStyles = npcStyles; // Backup current styles

	while (serde->GetNextRecordInfo(type, version, length)) {
		if (type == 'STYL' && version == 1) {
			std::size_t npcCount;
			serde->ReadRecordData(&npcCount, sizeof(npcCount));
			npcStyles.clear(); // Clear existing map

			for (std::size_t i = 0; i < npcCount; ++i) {
				RE::FormID formID;
				int style;
				serde->ReadRecordData(&formID, sizeof(formID));
				serde->ReadRecordData(&style, sizeof(style));

				if (auto npc = RE::TESForm::LookupByID(formID)) {
					npcStyles[formID] = style;  // Load saved styles
				}
				else {
					logger::debug("NPC with FormID {:X} is no longer valid and was not loaded into npcStyles.", formID);
				}
			}
		}
	}

	// Merge newly assigned styles that weren’t in the save
	for (const auto& [formID, style] : tempNpcStyles) {
		if (!npcStyles.contains(formID)) {
			npcStyles[formID] = style;
			//logger::debug("Merged new style for NPC {:X} assigned during Load3D.", formID);
		}
	}

	const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
	double seconds = static_cast<double>(elapsed.count()) / 1000000.0;
	logger::debug(">>>>>>>>>>>>>>>>>>>>>>>>>>> loaded {} npc styles in {} seconds", npcStyles.size(), seconds);
}



void RegisterSerialization() {
	// Cast away constness to use the serialization interface
	const auto* serde = SKSE::GetSerializationInterface();

	serde->SetUniqueID('STYL');
	serde->SetSaveCallback(SaveCallback);  // Now it's a non-const pointer
	serde->SetLoadCallback(LoadCallback);  // Now it's a non-const pointer

}

bool CopyVisual(RE::TESNPC* npcToCopyFrom, RE::Actor* a_this, NPC::NPCData npcData) {
	bool shouldCopy = false;
	auto npc = a_this->GetActorBase();

	if (npcToCopyFrom) {

		auto data = (RE::ExtraLeveledCreature*)a_this->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);

		if (data) {
			//logger::debug("copyVisual: actor {:08X} original: {:08X} tempate: {:08X} base {:08X}", a_this->formID, data->originalBase->formID, data->templateBase->formID, npc->formID);
			if (data->originalBase) {
				auto it = actData.npcData.find(data->originalBase->formID);
				if (it != actData.npcData.end()) {
					logger::debug("CopyVisual: actor {:08X} matched by original: {:08X}", a_this->formID, data->originalBase->formID);
					shouldCopy = true;
				}

			}

			if (!shouldCopy && data->templateBase) {
				auto it = actData.npcData.find(data->templateBase->formID);
				if (it != actData.npcData.end()) {
					logger::debug("CopyVisual: actor {:08X} matched by template {:08X}", a_this->formID, data->templateBase->formID);
					shouldCopy = true;
				}

			}


			if (npcData.restrictToTraits == true && shouldCopy) {
				if (data->originalBase && !data->originalBase->actorData.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits)) {
					shouldCopy = false;
					return false;
				}
			}

			if (shouldCopy && npc) {
				RE::TESNPC* npcToChange = npc;

				npcToChange->race = npcToCopyFrom->race;
				if (npcToCopyFrom->actorData.actorBaseFlags) {
					if (npcToCopyFrom->actorData.actorBaseFlags.all(RE::ACTOR_BASE_DATA::Flag::kFemale)) {
						npcToChange->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
					}
					else {
						npcToChange->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
					}

					if (npcToCopyFrom->actorData.actorBaseFlags.all(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims)) {
						npcToChange->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims);
					}
					else {
						npcToChange->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kOppositeGenderAnims);
					}
					npcToChange->RemoveChange(RE::TESNPC::ChangeFlags::kGender);
				}
				npcToChange->skin = npcToCopyFrom->skin;
				npcToChange->height = npcToCopyFrom->height;
				npcToChange->weight = npcToCopyFrom->weight;
				npcToChange->faceNPC = npcToCopyFrom;
				//npcToChange->tintLayers = npcToCopyFrom->tintLayers;
				npcToChange->bodyTintColor = npcToCopyFrom->bodyTintColor;
				//npcToChange->headRelatedData = npcToCopyFrom->headRelatedData;
				if (npcData.changeVoice) {
					npcToChange->voiceType = npcToCopyFrom->voiceType;
					//logger::debug("changed voice in template base: Actor {:08X}", a_this->formID );
				}
				if (npcToCopyFrom->headParts) {
					for (int i = 0; i < npcToCopyFrom->numHeadParts; i++) {
						if (npcToCopyFrom->headParts[i]) {
							//logger::debug("3d load template base: NPC {:08X} {} Change Head Part by random {:08X} {}", npcToChange->formID, npcToChange->fullName, npcToCopyFrom->headParts[i]->formID, npcToCopyFrom->headParts[i]->fullName);
							npcToChange->ChangeHeadPart(npcToCopyFrom->headParts[i]);
						}
					}
				}

				updater.startUpdate(a_this);
				return true;
			}
		}

	}
	logger::debug("Failed to apply rolled style to NPC. Form was not valid");
	return false;

}





struct Load3D
{
	static RE::NiAVObject* thunk(RE::Character* a_this, bool a_arg1)
	{
		//RE::NiAVObject* const root;
		const auto root = func(a_this, a_arg1);
		//return root;

		if (a_this == nullptr) {
			return root;
		}

		//if (a_this && a_this->GetParentCell() && a_this->GetParentCell()->cellData.interior)
		//	logger::debug("color {} {} {} ", a_this->GetParentCell()->cellData.interior->ambient.red, a_this->GetParentCell()->cellData.interior->ambient.green, a_this->GetParentCell()->cellData.interior->ambient.blue);
		//logger::debug("TESObjectREFR {:08X}", a_this->formID);
		if (statRefreshExcludeEnabled) {
			for (RE::FormID formid : excludedNPCsFormIDs) {
				auto base = a_this->GetActorBase();
				if (base && base->formID == formid) {
					//logger::debug("NPC excluded {:08X} {}", base->formID, base->fullName);
					const auto root = func(a_this, a_arg1);
					return root;
				}
			}
		}
		//if (auto style = GetFormFromIdentifier("skyai_csHumanMeleeLvl1")->As<RE::TESCombatStyle>(); a_this->combatController)
		//	a_this->combatController->combatStyle = style;
		bool refreshHealth = false;
		bool refreshStamina = false;
		bool refreshMagicka = false;

		//auto itActor = actData.npcData.find(a_this->formID);
		//if (itActor != actData.npcData.end()) {
		//	
		//	if (a_this && itActor->second.baseObject) {
		//		a_this->data.objectReference = itActor->second.baseObject;
		//		updater.startUpdate(a_this);
		//		logger::debug("Load3D: {:08X} updated actor base object to {}", a_this->formID, itActor->second.baseObject->formID);
		//	}
		//	//itActor->second.baseObject
		//}

		if (const auto npc = a_this->GetActorBase()) {
			auto data = (RE::ExtraLeveledCreature*)a_this->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
			auto it = actData.npcData.find(a_this->data.objectReference->formID);
			bool originalBaseHasStyleList = false;

			if (data) {
				//Get Original Base Data
				if (data->originalBase) {
					auto itBase = actData.npcData.find(data->originalBase->formID);
					if (itBase != actData.npcData.end()) {
						
						if (itBase->second.npcVisualStyleList) {
							originalBaseHasStyleList = true;
						}

						if (itBase->second.updateFlags) {
							RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(itBase->first);
							if (npcForm) {
								npc->actorData.actorBaseFlags = npcForm->actorData.actorBaseFlags;
								logger::debug("Load3D: {:08X} updated actor flags (original Base)", itBase->first);
							}
							else {
								logger::debug("Load3D: Failed to lookup NPC with ID {:08X} (original Base)", itBase->first);
							}
						}

						if (itBase->second.updateTemplateFlags) {
							RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(itBase->first);
							if (npcForm) {
								npc->actorData.templateUseFlags = npcForm->actorData.templateUseFlags;
								logger::debug("Load3D: {:08X} updated template flags (original Base)", itBase->first);
							}
							else {
								logger::debug("Load3D: Failed to lookup NPC with ID {:08X} (original Base)", itBase->first);
							}
						}
					}

				}


				if (data->templateBase) {
					it = actData.npcData.find(data->templateBase->formID);
					if (it != actData.npcData.end()) {
						if (statRefreshEnabled && !(npc->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats && npc->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kPCLevelMult)) {
							if (it->second.health >= 0) {
								data->templateBase->actorData.healthOffset = it->second.health;
								data->originalBase->actorData.healthOffset = data->templateBase->actorData.healthOffset;
								a_this->data.objectReference->As<RE::TESNPC>()->actorData.healthOffset = data->templateBase->actorData.healthOffset;
								refreshHealth = true;
							}
							if (it->second.stamina >= 0) {
								data->templateBase->actorData.staminaOffset = it->second.stamina;
								data->originalBase->actorData.staminaOffset = data->templateBase->actorData.staminaOffset;
								a_this->data.objectReference->As<RE::TESNPC>()->actorData.staminaOffset = data->templateBase->actorData.staminaOffset;
								refreshStamina = true;
							}
							if (it->second.magicka >= 0) {
								data->templateBase->actorData.magickaOffset = it->second.magicka;
								data->originalBase->actorData.magickaOffset = data->templateBase->actorData.magickaOffset;
								a_this->data.objectReference->As<RE::TESNPC>()->actorData.magickaOffset = data->templateBase->actorData.magickaOffset;
								refreshMagicka = true;
							}
						}
						for (auto perk : it->second.perks) {
							npc->AddPerk(perk, 0);
						}
						for (auto spell : it->second.spellItems) {
							if (!a_this->HasSpell(spell))
								a_this->AddSpell(spell);
						}
					}

				}
			}
			else {
				if (it != actData.npcData.end()) {
					//logger::debug("set stats normal");
					if (statRefreshEnabled  && !(npc->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats && npc->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kPCLevelMult)) {
						if (it->second.health >= 0) {
							npc->actorData.healthOffset = it->second.health;
							refreshHealth = true;
						}
						if (it->second.stamina >= 0) {
							npc->actorData.staminaOffset = it->second.stamina;
							refreshStamina = true;
						}
						if (it->second.magicka >= 0) {
							npc->actorData.magickaOffset = it->second.magicka;
							refreshMagicka = true;
						}
					}



					for (auto perk : it->second.perks) {
						npc->AddPerk(perk, 0);
					}
					for (auto spell : it->second.spellItems) {
						if (!a_this->HasSpell(spell))
							a_this->AddSpell(spell);
					}

				}

			}

			if (refreshHealth) {
				a_this->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
			}
			if (refreshStamina) {
				a_this->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
			}
			if (refreshMagicka) {
				a_this->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
			}



			if (it->second.updateFlags) {
				RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
				if (npcForm) {
					npc->actorData.actorBaseFlags = npcForm->actorData.actorBaseFlags;
					logger::debug("Load3D: {:08X} updated actor flags", it->first);
				}
			}

			if (it->second.updateTemplateFlags) {
				RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
				if (npcForm) {
					npc->actorData.templateUseFlags = npcForm->actorData.templateUseFlags;
					logger::debug("Load3D: {:08X} updated template flags", it->first);
				}
			}

			if (it->second.minLevelWasSet || it->second.maxLevelWasSet) {  // Check conditions first
				RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
				if (npcForm) {  // Check if the form is a valid NPC
					if (it->second.minLevelWasSet) {
						npcForm->actorData.calcLevelMin = it->second.minLevel;
						logger::debug("Load3D: {:08X} setminlevel {}", it->first, it->second.minLevel);
					}

					if (it->second.maxLevelWasSet) {
						npcForm->actorData.calcLevelMax = it->second.maxLevel;
						logger::debug("Load3D: {:08X} setmaxlevel {}", it->first, it->second.maxLevel);
					}
				}
			}

			//a_this->extraList
			//if (!it->second.overlaysToAdd.empty()) {
			//	logger::debug("Load3D {:08X} - OverlayToAdd size {} npcid: {:08X} tintlayers size {} ", npc->formID, it->second.overlaysToAdd.size(), it->first, npc->faceNPC->tintLayers->size());
			//	
			//	if (!npc->tintLayers) {
			//		npc->tintLayers = new RE::BSTArray<RE::TESNPC::Layer*>();
			//		if (npc->tintLayers) {
			//			std::construct_at(npc->tintLayers);
			//			logger::debug("Load3D {:08X} - tintLayers was null, allocated new BSTArray", npc->formID);
			//		}
			//		else {
			//			logger::critical("Load3D {:08X} - Failed to allocate tintLayers", npc->formID);
			//			//return;
			//		}
			//	}

			//	for (const auto& overlay : it->second.overlaysToAdd) {
			//		bool exists = std::any_of(npc->tintLayers->begin(), npc->tintLayers->end(),
			//			[&](RE::TESNPC::Layer* layer) {
			//				return layer && layer->tintIndex == overlay.tintIndex && layer->preset == overlay.preset;
			//			});

			//		if (!exists) {
			//			auto* newLayer = new RE::TESNPC::Layer;
			//			if (!newLayer) {
			//				logger::critical("Load3D {:08X} - Failed to allocate new overlay layer", npc->formID);
			//				continue;
			//			}
			//			//std::memset(newLayer, 0, sizeof(RE::TESNPC::Layer));

			//			newLayer->tintIndex = overlay.tintIndex;   
			//			newLayer->tintColor = overlay.color;
			//			newLayer->interpolationValue = overlay.interpolationValue;
			//			newLayer->preset = overlay.preset;

			//			npc->tintLayers->push_back(newLayer);

			//			logger::debug("Load3D {:08X} - Applied overlay index {} with color {} preset {}",
			//				npc->formID, overlay.tintIndex, overlay.color.ToHex(), overlay.preset);
			//		}
			//		else {
			//			logger::debug("Load3D {:08X} - Skipped duplicate overlay index {} preset {}",
			//				npc->formID, overlay.tintIndex, overlay.preset);
			//		}
			//	}

			//	updater.startUpdate(a_this);

			//	if (npc->tintLayers) {
			//		logger::info("Load3D {:08X} - Final Tint Layers:", npc->formID);
			//		for (const auto* layer : *npc->tintLayers) {
			//			if (!layer) continue;

			//			logger::info("  - TINT INDEX: {}, COLOR: {}, ALPHA: {}, PRESET: {}",
			//				layer->tintIndex,
			//				layer->tintColor.ToHex(),
			//				layer->interpolationValue,
			//				layer->preset);
			//		}
			//	}
			//}




			//auto collo = it->second.haircolor;
			//collo->color
			
			//if ( it->second.haircolor) {
			//	//it->second.haircolor->color
			//	//npc->SetHairColor(it->second.haircolor);
			//	auto root = a_this->Get3D(false);
			//	root->UpdateHairColor(it->second.haircolor->color);
			//	auto haircolor = it->second.haircolor;
			//	//if (const auto& biped = a_this->GetCurrentBiped(); biped) {
			//	//	for (auto& slot : headSlots) {
			//	//		const auto node = biped->objects[slot].partClone;
			//	//		if (node && node->HasShaderType(RE::BSShaderMaterial::Feature::kHairTint)) {
			//	//			node->UpdateHairColor(it->second.haircolor->color);
			//	//		}
			//	//	}
			//	//}
			//	//update_color_data(root, HAIR_TINT, it->second.haircolor->color);
			//	SKSE::GetTaskInterface()->AddTask([root, a_this, haircolor]() {
			//		root->UpdateHairColor(haircolor->color);
			//		if (const auto& biped = a_this->GetCurrentBiped(); biped) {
			//			for (auto& slot : headSlots) {
			//				const auto node = biped->objects[slot].partClone;
			//				if (node && node->HasShaderType(RE::BSShaderMaterial::Feature::kHairTint)) {
			//					node->UpdateHairColor(haircolor->color);
			//				}
			//			}
			//		}
			//		update_color_data(root, HAIR_TINT, haircolor->color);
			//		});
			//	logger::debug("3DLoaded: Updated hair color.");
			//	//updater.startUpdate(a_this);
			//}

			if ((it != actData.npcData.end() && it->second.npcVisualStyleList || originalBaseHasStyleList) ) {
				//logger::debug("NPC assign rd vs: {:08X} {:08X} style: {:08X}", a_this->formID, npc->formID, it->second.npcVisualStyleList->formID);

				NPC::NPCData& aNPCData = it->second;
				auto actorData = (RE::ExtraLeveledCreature*)a_this->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
				if (actorData && actorData->originalBase) {
					if (auto itr = actData.npcData.find(actorData->originalBase->formID); itr != actData.npcData.end() && itr->second.npcVisualStyleList) {
						//forms = itr->second.npcVisualStyleList->forms;
						aNPCData = itr->second;
						//logger::debug("Use alternative");
						//logger::debug("CopyVisual: actor {:08X} list {:08X} base {:08X}", a_this->formID, aNPCData.npcVisualStyleList->formID, actorData->originalBase->formID);
					}
				}

				if (aNPCData.npcVisualStyleList \
					&& aNPCData.npcVisualStyleList->formID \
					&& !aNPCData.npcVisualStyleList->forms.empty()) {

					auto& forms = aNPCData.npcVisualStyleList->forms;

					// Check if the NPC already has an assigned style
					auto styleIt = npcStyles.find(a_this->formID);
					int styleIndex = -1;

					if (styleIt != npcStyles.end()) {
						// NPC already has a style, use the existing one
						styleIndex = styleIt->second;

						// Validate stored style index
						if (styleIndex == -1) {
							logger::debug("Load3D: NPC with FormID {:X} previously failed style roll and will not receive a style.", a_this->formID);
						}
						else if (styleIndex >= forms.size()) {
							logger::debug("Load3D: NPC with FormID {:X} had an invalid style index {} (out of range). Reassigning a new style.", a_this->formID, styleIndex);
							styleIt = npcStyles.end(); // Force reassigning a new style
						}
						else {
							logger::debug("Load3D: NPC with FormID {:X} using stored style index {}.", a_this->formID, styleIndex);
						}
					}

					// If no valid style was found, roll for a new one
					if (styleIt == npcStyles.end()) {
						std::random_device randomDevice;
						std::mt19937 generator(randomDevice());
						std::uniform_int_distribution<int> chanceDist(1, 100);
						int rolledChance = chanceDist(generator);
						if (rolledChance > aNPCData.styleChance) {
							// Failed chance roll, mark NPC as having no style
							npcStyles[a_this->formID] = -1;
							logger::debug("Load3D: NPC with FormID {:X} failed style chance {} rolled {} and will not receive a style.", a_this->formID, aNPCData.styleChance, rolledChance);
							styleIndex = -1;
						}
						else if (!forms.empty()) {
							// Assign a new random style
							std::uniform_int_distribution<size_t> distribution(0, forms.size() - 1);
							styleIndex = distribution(generator);
							logger::debug("Load3D: Rolled new random style index {} to Actor with FormID {:X} from {} available styles ", styleIndex, a_this->formID, forms.size());
						}
						else {
							// No styles available
							npcStyles[a_this->formID] = -1;
							logger::debug("Load3D: No styles available in the list! NPC with FormID {:X} will not receive a style.", a_this->formID);
							styleIndex = -1;
						}
					}

					RE::TESNPC* npcToCopyFrom = nullptr;
					if (styleIndex >= 0 && styleIndex < forms.size()) {
						if (auto form = forms[styleIndex]; form) {
							npcToCopyFrom = form->As<RE::TESNPC>();
							if (!npcToCopyFrom) {
								logger::debug("Load3D: Form {:08X} is not an NPC.", form->formID);
							}
						}
						else {
							logger::debug("Load3D: Form at index {} is null!", styleIndex);
						}
					}
					else {
						logger::debug("Load3D: Invalid styleIndex {} (out of range).", styleIndex);
					}

					// Wende den Stil nur an, wenn npcToCopyFrom gültig ist und CopyVisual erfolgreich war
					if (npcToCopyFrom && CopyVisual(npcToCopyFrom, a_this, aNPCData)) {
						npcStyles[a_this->formID] = styleIndex;
						logger::debug("--> Load3D: Applied style index {} to Actor {:X} from list {:08X}, NPC source {:08X}.", styleIndex, a_this->formID, aNPCData.npcVisualStyleList->formID, npcToCopyFrom->formID);
					}
				}
			}

			if (it != actData.npcData.end() && it->second.npcCombatStyleList) {
				// Generate a random value between 1 and 100 (inclusive)
				std::random_device rd;                      // Obtain a random number from hardware
				std::mt19937 gen(rd());                     // Seed the generator
				std::uniform_int_distribution<int> dis(1, 100); // Define the range from 0 to 100
				int randomValueThatNeedsTobeGen = dis(gen);  // Generate the random number

				if (randomValueThatNeedsTobeGen <= it->second.npcCombatStyleChance) {
					auto& forms = it->second.npcCombatStyleList->forms;  // Reference to the forms array
					if (!forms.empty()) {  // Ensure the array is not empty
						// Generate a random index from the forms array
						std::uniform_int_distribution<size_t> disForms(0, forms.size() - 1);
						size_t randomIndex = disForms(gen);  // Generate a random index
						// Access the random form
						auto randomForm = forms[randomIndex]->As<RE::TESCombatStyle>();
						if (randomForm) {
							auto dataCS = (RE::ExtraCombatStyle*)a_this->extraList.GetByType(RE::ExtraDataType::kCombatStyle);
							if (dataCS) {
								dataCS->combatStyle = randomForm; // Assign the new combat style
							}
							npc->combatStyle = randomForm; // Set NPC combat style
						}
					}
				}
			}

			if (it->second.voiceType) {
				RE::TESNPC* npc = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
				if (npc) {
					npc->voiceType = it->second.voiceType;
					logger::debug("Load3D: {:08X} updated voiceType to {:08X}", npc->formID, it->second.voiceType->formID);
				}
			}

		}
		//const auto root = func(a_this, true);
		//const auto root = func(a_this, a_arg1);
		return root;
	}


	static inline REL::Relocation<decltype(thunk)> func;

	static inline constexpr std::size_t index{ 0 };
	static inline constexpr std::size_t size{ 0x6A };
};

struct Load3DREFR
{
	static RE::NiAVObject* thunk(RE::TESObjectREFR* a_this, bool a_arg1)
	{
		//RE::NiAVObject* const root;
		const auto root = func(a_this, a_arg1);

		if (a_this == nullptr) {
			return root;
		}

		//logger::debug("TESObjectREFR {:08X}", a_this->formID);
		auto it = refData.refrDataMap.find(a_this->formID);
		if (it != refData.refrDataMap.end()) {
			if (it->second.disable) {
				logger::debug("TESObjectREFR {:08X} disable ref  ", a_this->formID);
				SKSE::GetTaskInterface()->AddTask([a_this]() {
					if (a_this) {
						a_this->Disable();
					}
					});
			}

			if (it->second.replaceWith) {

				RE::TESBoundObject* boundobj = it->second.replaceWith;
				//logger::debug("TESObjectREFR {:08X} replace ref", a_this->formID);
				if (boundobj) {
					SKSE::GetTaskInterface()->AddTask([a_this, boundobj]() {
						if (a_this) {
							a_this->SetObjectReference(boundobj);
							a_this->Enable(false);
							
							//a_this->data.objectReference = boundobj;
						}
						});
				}
			}

		}

		return root;
	}


	static inline REL::Relocation<decltype(thunk)> func;

	static inline constexpr std::size_t index{ 0 };
	static inline constexpr std::size_t size{ 0x6A };
};

void InstallActorHook()
{
	stl::write_vfunc<RE::Character, Load3D>();
	//logger::debug("iUpdateNPC is enabled");
}

void InstallRefHook()
{
	stl::write_vfunc<RE::TESObjectREFR, Load3DREFR>();
	//logger::debug("iUpdateRefs is enabled");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kPreLoadGame:
	{
		npcStyles.clear();
		logger::debug(">>> Cleared Random NPC Styles Map.");
		//SKSE::SerializationInterface* serde = skse->GetSerializationInterface();
		//serde->SetSaveCallback(SaveCallback);
		break;
	}
	case SKSE::MessagingInterface::kPostLoadGame:
	{
		const auto start_time{ std::chrono::system_clock::now() };
		//TriggerLoad();
		//loadStylesAfterLoad = false;
		//if (GetPrivateProfileInt(settingsSection, "iEnableNPCPatching", 0, configFile)) {
		//	const auto start_time{ std::chrono::system_clock::now() };
		//	if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableSetLevelDirectlyByPCMult", 0, configFile)) {
		//		NPC::setbEnableSetLevelDirectlyByPCMult();
		//	}
		//	NPC::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\npc");
		//	const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
		//	double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
		//	logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Load Game NPC Patcher finished in {} seconds", seconds);
		//}

		//auto a_this = RE::PlayerCharacter::GetSingleton();
		//if (a_this->GetActorBase()->attackDataMap && a_this->race && a_this->race->attackDataMap) {
		//	//a_this->GetActorBase()->attackDataMap = a_this->race->attackDataMap;
		//	//a_this->data.objectReference->As<RE::TESNPC>()->attackDataMap = a_this->race->attackDataMap;
		//
		//	for (auto& map : a_this->race->attackDataMap->attackDataMap) {
		//		logger::debug("RACE map {} {}",  map.second.get()->event, map.second.get()->data.damageMult);
		//		if (map.second.get()->event.c_str() == "attackPowerStartDualWield")
		//			map.second.get()->data.damageMult = 0.36;
		//		logger::debug("RACE map updated: {} {}", map.second.get()->event, map.second.get()->data.damageMult);
		//	}
		//	for (auto&  map : a_this->GetActorBase()->attackDataMap->attackDataMap) {
		//		logger::debug("NPC map {} {}",  map.second.get()->event, map.second.get()->data.damageMult);
		//		if (map.second.get()->event.c_str() == "attackPowerStartDualWield")
		//			map.second.get()->data.damageMult = 0.36;
		//		logger::debug("NPC map updated: {} {}", map.second.get()->event, map.second.get()->data.damageMult);
		//	}
		//	//RE::free(a_this->sourceFiles.array);
		//}
		const auto processLists = RE::ProcessLists::GetSingleton();
		for (auto actorHandles : processLists->allProcesses) {
			for (const auto actorHandle : *actorHandles) {
				const auto& actorPtr = actorHandle;
				const auto  actorHand = actorPtr.get();
				auto        actor = actorHand.get();
				if (actor) {
					if (actor->data.objectReference) {
						//logger::debug("actor has objectRef -> {:08X}", actor->data.objectReference->As<RE::TESNPC>()->formID);
						auto it = actData.npcData.find(actor->data.objectReference->formID);
						auto data = (RE::ExtraLeveledCreature*)actor->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
						bool originalBaseHasStyleList = false;
						//NPC::NPCData npcOriginalBaseData = {};

						if (data) {

							if (data->originalBase) {
								auto itBase = actData.npcData.find(data->originalBase->formID);
								if (itBase != actData.npcData.end() && itBase->second.npcCombatStyleList) {
									originalBaseHasStyleList = true;
								}
							}

							//logger::debug(FMT_STRING("Actor {:08X}  orginalBase {:08X} templateBAse {:08X}"), a_this->formID, data->originalBase->formID, data->templateBase->formID);
							if (data->templateBase) {
								it = actData.npcData.find(data->templateBase->formID);
							}
						}

						if (it != actData.npcData.end()) {
							if (it->second.npcClass) {
								actor->data.objectReference->As<RE::TESNPC>()->npcClass = it->second.npcClass;
							}

							if (it->second.npcCombatStyleList) {
								// Generate a random value between 1 and 100 (inclusive)
								std::random_device rd;                      // Obtain a random number from hardware
								std::mt19937 gen(rd());                     // Seed the generator
								std::uniform_int_distribution<int> dis(1, 100); // Define the range from 1 to 100
								int randomValueThatNeedsTobeGen = dis(gen);  // Generate the random number

								// Check if the random value is less than or equal to npcCombatStyleChance
								if (randomValueThatNeedsTobeGen <= it->second.npcCombatStyleChance) {
									auto& forms = it->second.npcCombatStyleList->forms;
									if (!forms.empty()) {  // Ensure the array is not empty
										// Generate a random index from the forms array
										std::uniform_int_distribution<size_t> disForms(0, forms.size() - 1);
										size_t randomIndex = disForms(gen);  // Generate a random index

										// Access the random form
										auto randomForm = forms[randomIndex]->As<RE::TESCombatStyle>();
										if (randomForm) {
											auto dataCS = (RE::ExtraCombatStyle*)actor->extraList.GetByType(RE::ExtraDataType::kCombatStyle);
											if (dataCS) {
												// You can log or check previous combat style if needed
												dataCS->combatStyle = randomForm;  // Assign the new combat style
											}

											// Set the combat style if the actor is a TESNPC
											if (actor->data.objectReference->As<RE::TESNPC>()) {
												actor->data.objectReference->As<RE::TESNPC>()->combatStyle = randomForm;
												// Optionally log the style change
												// logger::debug("Actor: {:08X} new style {:08X}", actor->formID, randomForm->formID);
											}
										}
									}
								}
							}


							////
							//
							//
							if (it->second.skinWasSet) {
								//auto skinArmor = it->second.skin;  // Reference to the forms array
								//logger::debug("Load Save Actor set new visuals");
								if (actor->data.objectReference->As<RE::TESNPC>()) {  // Ensure the array is not empty
									actor->data.objectReference->As<RE::TESNPC>()->skin = it->second.skin;
								}
							}

							//if (it->second.haircolor) {
							//	actor->data.objectReference->As<RE::TESNPC>()->SetHairColor(it->second.haircolor);
							//	actor->UpdateHairColor();
							//	auto root = actor->Get3D(false);
							//	root->UpdateHairColor(it->second.haircolor->color);
							//	auto haircolor = it->second.haircolor;
							//	SKSE::GetTaskInterface()->AddTask([root, actor, haircolor]() {
							//		root->UpdateHairColor(haircolor->color);
							//		if (const auto& biped = actor->GetCurrentBiped(); biped) {
							//			for (auto& slot : headSlots) {
							//				const auto node = biped->objects[slot].partClone;
							//				if (node && node->HasShaderType(RE::BSShaderMaterial::Feature::kHairTint)) {
							//					node->UpdateHairColor(haircolor->color);
							//				}
							//			}
							//		}
							//		update_color_data(root, HAIR_TINT, haircolor->color);
							//		});
							//	//logger::debug("PostLoadGame: actor {:08X} {} hair color {:08X}", actor->data.objectReference->As<RE::TESNPC>()->formID, actor->data.objectReference->As<RE::TESNPC>()->fullName, it->second.haircolor->formID);
							//}

							if (it->second.npcRace) {
								//auto npcRace = it->second.npcRace;  // Reference to the forms array
								//logger::debug("Load Save Actor set new visuals");
								if (actor->data.objectReference->As<RE::TESNPC>()) {  // Ensure the array is not empty
									actor->data.objectReference->As<RE::TESNPC>()->race = it->second.npcRace;
								}
							}



							if (it->second.updateFlags) {
								RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
								if (npcForm) {  // Check if LookupByID succeeded
									RE::TESNPC* actorNPC = actor->data.objectReference->As<RE::TESNPC>();
									if (actorNPC) {  // Check if the cast to TESNPC succeeded
										actorNPC->actorData.actorBaseFlags = npcForm->actorData.actorBaseFlags;
										logger::debug("PostLoadGame: {:08X} updated actor flags", it->first);
									}
									else {
										logger::debug("PostLoadGame: Failed to cast objectReference to TESNPC for actor with ID {:08X}", it->first);
									}
								}
								else {
									logger::debug("PostLoadGame: Failed to lookup NPC with ID {:08X}", it->first);
								}
							}

							if (it->second.updateTemplateFlags) {
								RE::TESNPC* npcForm = RE::TESForm::LookupByID<RE::TESNPC>(it->first);
								if (npcForm) {  // Check if LookupByID succeeded
									RE::TESNPC* actorNPC = actor->data.objectReference->As<RE::TESNPC>();
									if (actorNPC) {  // Check if the cast to TESNPC succeeded
										actorNPC->actorData.templateUseFlags = npcForm->actorData.templateUseFlags;
										logger::debug("PostLoadGame: {:08X} updated template flags", it->first);
									}
									else {
										logger::debug("PostLoadGame: Failed to cast objectReference to TESNPC for actor with ID {:08X}", it->first);
									}
								}
								else {
									logger::debug("PostLoadGame: Failed to lookup NPC with ID {:08X}", it->first);
								}
							}

							if (it->second.minLevelWasSet || it->second.maxLevelWasSet) {  // Check bools first
								RE::TESNPC* npc = actor->data.objectReference->As<RE::TESNPC>();
								if (npc) {  // Check if the cast to TESNPC succeeded
									if (it->second.minLevelWasSet) {
										npc->actorData.calcLevelMin = it->second.minLevel;
										logger::debug("PostLoadGame: {:08X} setminlevel {}", it->first, it->second.minLevel);
									}

									if (it->second.maxLevelWasSet) {
										npc->actorData.calcLevelMax = it->second.maxLevel;
										logger::debug("PostLoadGame: {:08X} setmaxlevel {}", it->first, it->second.maxLevel);
									}
								}
								else {
									logger::debug("Failed to cast objectReference to TESNPC for actor with ID {:08X}", it->first);
								}
							}

							//

							if (it->second.npcVisual) {
								auto npcToCopyFrom = it->second.npcVisual;  // Reference to the forms array
								//logger::debug("Load Save Actor set new visuals");
								if (npcToCopyFrom && actor->data.objectReference->As<RE::TESNPC>()) {  // Ensure the array is not empty
									RE::TESNPC* npcToChange = actor->data.objectReference->As<RE::TESNPC>();
									npcToChange->height = npcToCopyFrom->height;
									npcToChange->weight = npcToCopyFrom->weight;
									npcToChange->faceNPC = npcToCopyFrom;
									npcToChange->tintLayers = npcToCopyFrom->tintLayers;
									npcToChange->bodyTintColor = npcToCopyFrom->bodyTintColor;
									//npcToChange->headRelatedData = npcToCopyFrom->headRelatedData;
									if (npcToChange && npcToCopyFrom) {
										for (int i = 0; i < npcToCopyFrom->numHeadParts; i++) {
											if (npcToCopyFrom->headParts && npcToCopyFrom->headParts[i]) {
												//logger::debug("PostLoadGame: Change Head Part {:08X} {}", npcToCopyFrom->headParts[i]->formID, npcToCopyFrom->headParts[i]->fullName);
												npcToChange->ChangeHeadPart(npcToCopyFrom->headParts[i]);
											}
										}
									}
									logger::debug("PostLoadGame: applied Visual Style to Actor {:08X} {}.", actor->formID, actor->GetDisplayFullName());
								}
							}

							/////////////////////////

							if (it != actData.npcData.end() && it->second.npcVisualStyleList || originalBaseHasStyleList) {

								NPC::NPCData& aNPCData = it->second;
								auto actorData = (RE::ExtraLeveledCreature*)actor->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
								if (actorData && actorData->originalBase) {
									if (auto itr = actData.npcData.find(actorData->originalBase->formID); itr != actData.npcData.end() && itr->second.npcVisualStyleList) {
										//forms = itr->second.npcVisualStyleList->forms;
										aNPCData = itr->second;
										//logger::debug("Use alternative");
										//logger::debug("CopyVisual: actor {:08X} list {:08X} base {:08X}", a_this->formID, aNPCData.npcVisualStyleList->formID, actorData->originalBase->formID);
									}
								}

								if (aNPCData.npcVisualStyleList && !aNPCData.npcVisualStyleList->forms.empty()) {

									auto& forms = aNPCData.npcVisualStyleList->forms;

									// Check if the NPC already has an assigned style
									auto styleIt = npcStyles.find(actor->formID);

									// Check if the NPC already has an assigned style
									int styleIndex = -1;

									if (styleIt != npcStyles.end()) {
										// NPC already has a style, use the existing one
										styleIndex = styleIt->second;

										// Validate stored style index
										if (styleIndex == -1) {
											logger::debug("PostLoadGame: NPC with FormID {:X} previously failed style roll and will not receive a style.", actor->formID);
										}
										else if (styleIndex >= forms.size()) {
											logger::debug("PostLoadGame: NPC with FormID {:X} had an invalid style index {} (out of range). Reassigning a new style.",
												actor->formID, styleIndex);
											styleIt = npcStyles.end(); // Force reassigning a new style
										}
										else {
											logger::debug("PostLoadGame: NPC with FormID {:X} using stored style index {}.", actor->formID, styleIndex);
										}
									}

									// If no valid style was found, roll for a new one
									if (styleIt == npcStyles.end()) {
										std::random_device randomDevice;
										std::mt19937 generator(randomDevice());
										std::uniform_int_distribution<int> chanceDist(1, 100);

										int rolledChance = chanceDist(generator);
										if (rolledChance > aNPCData.styleChance) {
											// Failed chance roll, mark NPC as having no style
											npcStyles[actor->formID] = -1;
											logger::debug("PostLoadGame: NPC with FormID {:X} failed style chance {} rolled {} and will not receive a style.",
												actor->formID, aNPCData.styleChance, rolledChance);
											styleIndex = -1;
										}
										else if (!forms.empty()) {
											// Assign a new random style
											std::uniform_int_distribution<size_t> distribution(0, forms.size() - 1);
											styleIndex = distribution(generator);
											npcStyles[actor->formID] = styleIndex;
											logger::debug("PostLoadGame: Assigned new random style index {} to Actor with FormID {:X}", styleIndex, actor->formID);
										}
										else {
											// No styles available
											npcStyles[actor->formID] = -1;
											logger::debug("PostLoadGame: No styles available in the list! NPC with FormID {:X} will not receive a style.", actor->formID);
											styleIndex = -1;
										}
									}

									RE::TESNPC* npcToCopyFrom = nullptr;
									if (styleIndex >= 0 && styleIndex < forms.size()) {
										if (auto form = forms[styleIndex]; form) {
											npcToCopyFrom = form->As<RE::TESNPC>();
											if (!npcToCopyFrom) {
												logger::debug("PostLoadGame: Form {:08X} is not an NPC.", form->formID);
											}
										}
										else {
											logger::debug("PostLoadGame: Form at index {} is null!", styleIndex);
										}
									}
									else {
										logger::debug("PostLoadGame: Invalid styleIndex {} (out of range).", styleIndex);
									}

									// Wende den Stil nur an, wenn npcToCopyFrom gültig ist und CopyVisual erfolgreich war
									if (npcToCopyFrom && CopyVisual(npcToCopyFrom, actor, aNPCData)) {
										npcStyles[actor->formID] = styleIndex;
										logger::debug("--> PostLoadGame: Applied style index {} to Actor {:X} from list {:08X}, NPC source {:08X}.",
											styleIndex, actor->formID, aNPCData.npcVisualStyleList->formID, npcToCopyFrom->formID);
									}


								}
								else {
									logger::debug("PostLoadGame: Can't apply random style, formList empty or not valid.");
								}

							}

							///////////////////////////


							if ((it->second.npcVisual || it->second.skinWasSet) || it->second.haircolor) {
								if (iUpdateNPCVisualsOnLoad == 1) {

									if (actor->data.objectReference) {
										logger::debug("PostLoadGame: Actor: {:08X} NPC: {:08X} Name: {} has been marked for 3DReset.", actor->formID, actor->data.objectReference->formID, actor->data.objectReference->GetName());
									}
									else {
										logger::debug("PostLoadGame: Actor: {:08X} 3D has been marked for 3DReset.", actor->formID);
									}

									if (actor->GetActorBase()) {
										logger::debug("PostLoadGame: Actor: {:08X} has actor base {:08X}.", actor->formID, actor->GetActorBase()->formID);
									}
									else {
										logger::debug("PostLoadGame: Actor: {:08X} has no actor base.", actor->formID);
									}
									updater.startUpdate(actor);
								}
								else if (iUpdateNPCVisualsOnLoad == 2) {
									if (actor->data.objectReference) {
										logger::debug("PostLoadGame: Actor: {:08X} NPC: {:08X} Name: {} has been marked for Disable/Enable.", actor->formID, actor->data.objectReference->formID, actor->data.objectReference->GetName());
									}
									else {
										logger::debug("PostLoadGame: Actor: {:08X} 3D has been marked for Disable/Enable.", actor->formID);
									}

									if (actor->GetActorBase()) {
										logger::debug("PostLoadGame: Actor: {:08X} has actor base {:08X}.", actor->formID, actor->GetActorBase()->formID);
									}
									else {
										logger::debug("PostLoadGame: Actor: {:08X} has no actor base.", actor->formID);
									}
									updater.startUpdateByEnable(actor);
								}
								else {
									logger::debug("PostLoadGame: Actor: {:08X} No Visual Update.", actor->formID, iUpdateNPCVisualsOnLoad);
								}
							}

							if (it->second.voiceType && actor->data.objectReference) {
								RE::TESNPC* npc = actor->data.objectReference->As<RE::TESNPC>();
								if (npc) {
									npc->voiceType = it->second.voiceType;
									logger::debug("PostLoadGame: {:08X} {} updated voiceType to {:08X}", npc->formID, npc->fullName.c_str(), it->second.voiceType->formID);
								}
							}

							//if (false && it->second.npcVisualStyleList) {
							//	updater.startUpdateByEnable(actor);
							//}

						}
					}
					//if (statRefreshEnabled) {
					//actor->InitValues();
					//actor->ApplyPerksFromBase();
					//}
				}

				//}
			}
		}

		const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
		double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
		logger::debug(">>>>>>>>>>>>>>>>>>>>>>>>>>> loaded save file in {} seconds", seconds);

		loadOnlyOnce = false;
		break;
	}

	case SKSE::MessagingInterface::kNewGame:
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		if (player) {
			auto npc = player->data.objectReference->As<RE::TESNPC>();
			if (npc) {
			auto it = actData.npcData.find(npc->formID);
				if (it != actData.npcData.end()) {
					//logger::debug("set stats template");
					if (it->second.health >= 0)
						npc->actorData.healthOffset = it->second.health;
					if (it->second.stamina >= 0)
						npc->actorData.staminaOffset = it->second.stamina;
					if (it->second.magicka >= 0)
						npc->actorData.magickaOffset = it->second.magicka;

				}
				else {
					//logger::debug("no set stats template");
				}
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
			player->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
			}
		}
		break;
	}

	case SKSE::MessagingInterface::kDataLoaded:
	{
		const auto start_time_gameLoaded{ std::chrono::system_clock::now() };
		SkyPatcher_VisualStyleApply = createKeyword();
		FormDeleteEventSink::Init();

		if (GetPrivateProfileInt(settingsSection, "iEnableMagicEffectPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			MGEF::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\magicEffect\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Magic Effect Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableAmmoPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			AMMO::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ammo\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Ammo Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableWeaponPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			WEAPON::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\weapon\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Weapon Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableArmorPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			ARMOR::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\armor\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Armor Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableRacePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			RACE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\race\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Race Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableNPCPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableSetLevelDirectlyByPCMult", 0, configFile)) {
				NPC::setbEnableSetLevelDirectlyByPCMult();
			}

			actData = NPC::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\npc");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> NPC Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableIngestiblePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			ALCH::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ingestible\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Ingestible Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableIngredientPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			INGR::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\ingredient\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Ingredient Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableSpellPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			SPELL::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\spell\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Spell Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableEnchantmentPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			ENCH::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\enchantment\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Enchantment Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableProjectilePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			PROJECTILE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\projectile\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Projectile Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableOutfitPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			OUTFIT::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\outfit\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Outfit Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableScrollPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			SCROLL::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\scroll\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Scroll Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableMiscPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			MISC::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\misc\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Misc Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableSoulGemPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			SOULGEM::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\soulGem\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Soul Gem Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableFactionPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			FACTION::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\faction\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Faction Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableConstructibleObjectPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			COBJ::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\constructibleObject\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Constructible Object Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableBookPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			BOOK::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\book\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Book Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableMovementTypePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			MOVETYPE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\movementType\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Movement Type Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableLeveledListPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			LEVELEDLIST::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\leveledList\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Leveled List Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableContainerPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			CONTAINER::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\container\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Container Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableFormListPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };
			FORMLIST::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\formList\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> FormList Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableReferencePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };

			refData = REFERENCE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\reference");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Reference Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableCellPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };

			CELL::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\cell\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Cell Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableLocationPatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };

			LOCATION::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\location\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Location Patcher finished in {} seconds", seconds);
		}

		if (GetPrivateProfileInt(settingsSection, "iEnableEncounterZonePatching", 0, configFile)) {
			const auto start_time{ std::chrono::system_clock::now() };

			ENCOUNTERZONE::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\encounterzone\\");
			const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
			double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
			logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Encounterzone Patcher finished in {} seconds", seconds);
		}

		//CELL::readConfig(".\\Data\\SKSE\\Plugins\\SkyPatcher\\cell\\");

		auto player = RE::PlayerCharacter::GetSingleton();
		auto npc = player->data.objectReference->As<RE::TESNPC>();
		player->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
		player->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
		player->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
		//logger::debug("playerskill NPC {} {}", npc->playerSkills.values[RE::TESNPC::Skills::kOneHanded], npc->playerSkills.values[RE::TESNPC::Skills::kTwoHanded]);
		//logger::debug("playerskill SKILL {} {}", player->skills->data->skills[RE::TESNPC::Skills::kOneHanded].level, player->skills->data->skills[RE::TESNPC::Skills::kTwoHanded].level);
		//
		//// One-Handed
		//player->skills->data->skills[RE::TESNPC::Skills::kOneHanded].level = (npc->playerSkills.values[RE::TESNPC::Skills::kOneHanded] + npc->playerSkills.offsets[RE::TESNPC::Skills::kOneHanded]);
		//player->SetBaseActorValue(RE::ActorValue::kOneHanded, (npc->playerSkills.values[RE::TESNPC::Skills::kOneHanded] + npc->playerSkills.offsets[RE::TESNPC::Skills::kOneHanded]));

		//// Two-Handed
		//player->skills->data->skills[RE::TESNPC::Skills::kTwoHanded].level = (npc->playerSkills.values[RE::TESNPC::Skills::kTwoHanded] + npc->playerSkills.offsets[RE::TESNPC::Skills::kTwoHanded]);
		//player->SetBaseActorValue(RE::ActorValue::kTwoHanded, (npc->playerSkills.values[RE::TESNPC::Skills::kTwoHanded] + npc->playerSkills.offsets[RE::TESNPC::Skills::kTwoHanded]));

		//// Marksman
		//player->skills->data->skills[RE::TESNPC::Skills::kMarksman].level = (npc->playerSkills.values[RE::TESNPC::Skills::kMarksman] + npc->playerSkills.offsets[RE::TESNPC::Skills::kMarksman]);
		//player->SetBaseActorValue(RE::ActorValue::kArchery, (npc->playerSkills.values[RE::TESNPC::Skills::kMarksman] + npc->playerSkills.offsets[RE::TESNPC::Skills::kMarksman]));

		//// Block
		//player->skills->data->skills[RE::TESNPC::Skills::kBlock].level = (npc->playerSkills.values[RE::TESNPC::Skills::kBlock] + npc->playerSkills.offsets[RE::TESNPC::Skills::kBlock]);
		//player->SetBaseActorValue(RE::ActorValue::kBlock, (npc->playerSkills.values[RE::TESNPC::Skills::kBlock] + npc->playerSkills.offsets[RE::TESNPC::Skills::kBlock]));

		//// Smithing
		//player->skills->data->skills[RE::TESNPC::Skills::kSmithing].level = (npc->playerSkills.values[RE::TESNPC::Skills::kSmithing] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSmithing]);
		//player->SetBaseActorValue(RE::ActorValue::kSmithing, (npc->playerSkills.values[RE::TESNPC::Skills::kSmithing] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSmithing]));

		//// Heavy Armor
		//player->skills->data->skills[RE::TESNPC::Skills::kHeavyArmor].level = (npc->playerSkills.values[RE::TESNPC::Skills::kHeavyArmor] + npc->playerSkills.offsets[RE::TESNPC::Skills::kHeavyArmor]);
		//player->SetBaseActorValue(RE::ActorValue::kHeavyArmor, (npc->playerSkills.values[RE::TESNPC::Skills::kHeavyArmor] + npc->playerSkills.offsets[RE::TESNPC::Skills::kHeavyArmor]));

		//// Light Armor
		//player->skills->data->skills[RE::TESNPC::Skills::kLightArmor].level = (npc->playerSkills.values[RE::TESNPC::Skills::kLightArmor] + npc->playerSkills.offsets[RE::TESNPC::Skills::kLightArmor]);
		//player->SetBaseActorValue(RE::ActorValue::kLightArmor, (npc->playerSkills.values[RE::TESNPC::Skills::kLightArmor] + npc->playerSkills.offsets[RE::TESNPC::Skills::kLightArmor]));

		//// Pickpocket
		//player->skills->data->skills[RE::TESNPC::Skills::kPickpocket].level = (npc->playerSkills.values[RE::TESNPC::Skills::kPickpocket] + npc->playerSkills.offsets[RE::TESNPC::Skills::kPickpocket]);
		//player->SetBaseActorValue(RE::ActorValue::kPickpocket, (npc->playerSkills.values[RE::TESNPC::Skills::kPickpocket] + npc->playerSkills.offsets[RE::TESNPC::Skills::kPickpocket]));

		//// Lockpicking
		//player->skills->data->skills[RE::TESNPC::Skills::kLockpicking].level = (npc->playerSkills.values[RE::TESNPC::Skills::kLockpicking] + npc->playerSkills.offsets[RE::TESNPC::Skills::kLockpicking]);
		//player->SetBaseActorValue(RE::ActorValue::kLockpicking, (npc->playerSkills.values[RE::TESNPC::Skills::kLockpicking] + npc->playerSkills.offsets[RE::TESNPC::Skills::kLockpicking]));

		//// Sneak
		//player->skills->data->skills[RE::TESNPC::Skills::kSneak].level = (npc->playerSkills.values[RE::TESNPC::Skills::kSneak] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSneak]);
		//player->SetBaseActorValue(RE::ActorValue::kSneak, (npc->playerSkills.values[RE::TESNPC::Skills::kSneak] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSneak]));

		//// Alchemy
		//player->skills->data->skills[RE::TESNPC::Skills::kAlchemy].level = (npc->playerSkills.values[RE::TESNPC::Skills::kAlchemy] + npc->playerSkills.offsets[RE::TESNPC::Skills::kAlchemy]);
		//player->SetBaseActorValue(RE::ActorValue::kAlchemy, (npc->playerSkills.values[RE::TESNPC::Skills::kAlchemy] + npc->playerSkills.offsets[RE::TESNPC::Skills::kAlchemy]));

		//// Speechcraft
		//player->skills->data->skills[RE::TESNPC::Skills::kSpeechcraft].level = (npc->playerSkills.values[RE::TESNPC::Skills::kSpeechcraft] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSpeechcraft]);
		//player->SetBaseActorValue(RE::ActorValue::kSpeech, (npc->playerSkills.values[RE::TESNPC::Skills::kSpeechcraft] + npc->playerSkills.offsets[RE::TESNPC::Skills::kSpeechcraft]));

		//// Alteration
		//player->skills->data->skills[RE::TESNPC::Skills::kAlteration].level = (npc->playerSkills.values[RE::TESNPC::Skills::kAlteration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kAlteration]);
		//player->SetBaseActorValue(RE::ActorValue::kAlteration, (npc->playerSkills.values[RE::TESNPC::Skills::kAlteration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kAlteration]));

		//// Conjuration
		//player->skills->data->skills[RE::TESNPC::Skills::kConjuration].level = (npc->playerSkills.values[RE::TESNPC::Skills::kConjuration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kConjuration]);
		//player->SetBaseActorValue(RE::ActorValue::kConjuration, (npc->playerSkills.values[RE::TESNPC::Skills::kConjuration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kConjuration]));

		//// Destruction
		//player->skills->data->skills[RE::TESNPC::Skills::kDestruction].level = (npc->playerSkills.values[RE::TESNPC::Skills::kDestruction] + npc->playerSkills.offsets[RE::TESNPC::Skills::kDestruction]);
		//player->SetBaseActorValue(RE::ActorValue::kDestruction, (npc->playerSkills.values[RE::TESNPC::Skills::kDestruction] + npc->playerSkills.offsets[RE::TESNPC::Skills::kDestruction]));

		//// Illusion
		//player->skills->data->skills[RE::TESNPC::Skills::kIllusion].level = (npc->playerSkills.values[RE::TESNPC::Skills::kIllusion] + npc->playerSkills.offsets[RE::TESNPC::Skills::kIllusion]);
		//player->SetBaseActorValue(RE::ActorValue::kIllusion, (npc->playerSkills.values[RE::TESNPC::Skills::kIllusion] + npc->playerSkills.offsets[RE::TESNPC::Skills::kIllusion]));

		//// Restoration
		//player->skills->data->skills[RE::TESNPC::Skills::kRestoration].level = (npc->playerSkills.values[RE::TESNPC::Skills::kRestoration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kRestoration]);
		//player->SetBaseActorValue(RE::ActorValue::kRestoration, (npc->playerSkills.values[RE::TESNPC::Skills::kRestoration] + npc->playerSkills.offsets[RE::TESNPC::Skills::kRestoration]));

		//// Enchanting
		//player->skills->data->skills[RE::TESNPC::Skills::kEnchanting].level = (npc->playerSkills.values[RE::TESNPC::Skills::kEnchanting] + npc->playerSkills.offsets[RE::TESNPC::Skills::kEnchanting]);
		//player->SetBaseActorValue(RE::ActorValue::kEnchanting, (npc->playerSkills.values[RE::TESNPC::Skills::kEnchanting] + npc->playerSkills.offsets[RE::TESNPC::Skills::kEnchanting]));

		//logger::debug("playerskill NPC {} {}", npc->playerSkills.values[RE::TESNPC::Skills::kOneHanded], npc->playerSkills.values[RE::TESNPC::Skills::kTwoHanded]);
		//logger::debug("playerskill SKILL {} {}", player->skills->data->skills[RE::TESNPC::Skills::kOneHanded].level, player->skills->data->skills[RE::TESNPC::Skills::kTwoHanded].level);
		//

		//RE::BGSMovementType aa;
		//aa.movementTypeData.defaultData.speeds[RE::Movement::SPEED_DIRECTIONS::SPEED_DIRECTION::kLeft][RE::Movement::MaxSpeeds::kWalk];

		const auto processLists = RE::ProcessLists::GetSingleton();
		for (auto actorHandles : processLists->allProcesses) {
			for (const auto actorHandle : *actorHandles) {
				const auto& actorPtr = actorHandle;
				const auto  actorHand = actorPtr.get();
				auto        actor = actorHand.get();

				if (actor && actor->data.objectReference) {
					auto it = actData.npcData.find(actor->data.objectReference->formID);
					if (it != actData.npcData.end()) {
						//logger::debug(FMT_STRING("Actor from ProcessList {:08X} "), actor->formID);
						if (actor->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
							//refr->extraList.GetByType(RE::ExtraDataType::kLevCreaModifier);
							auto data = (RE::ExtraLeveledCreature*)actor->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
							if (data) {
								//logger::debug(FMT_STRING("Actor {:08X}  orginalBase {:08X} templateBAse {:08X}"), actor->formID, data->originalBase->formID, data->templateBase->formID);
								if (data->templateBase) {
									data->originalBase->actorData.healthOffset = data->templateBase->actorData.healthOffset;
									data->originalBase->actorData.staminaOffset = data->templateBase->actorData.staminaOffset;
									data->originalBase->actorData.magickaOffset = data->templateBase->actorData.magickaOffset;
									actor->data.objectReference->As<RE::TESNPC>()->actorData = data->templateBase->actorData;
								}
							}
						}
						auto npc = actor->data.objectReference->As<RE::TESNPC>();
						//if (actor->formID == 0x000014) {
						//	playerHealth = npc->actorData.healthOffset;
						//	playerStamina = npc->actorData.staminaOffset;
						//	playerMagicka = npc->actorData.magickaOffset;
						//}
						if (npc) {
							actor->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
							actor->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
							actor->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
							actor->InitValues();
							actor->ApplyPerksFromBase();
						}
						//auto data = (RE::ExtraLeveledCreature*)actor->extraList.GetByType(RE::ExtraDataType::k);
						//if (data) {
						//
						//}
						//logger::debug(FMT_STRING("Actor {:08X} updated with stats: health {} mag {} stam {}"), actor->formID, npc->actorData.healthOffset+npc->race->data.startingHealth,npc->actorData.magickaOffset + npc->race->data.startingMagicka,npc->actorData.staminaOffset + npc->race->data.startingStamina);
						//actor->InitValues();
					}
				}
			}
		}

		if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableUnlevelNPCs", 0, configFile)) {
			const auto                          dataHandler = RE::TESDataHandler::GetSingleton();
			RE::BSTArray<RE::BGSEncounterZone*> EncounterZoneArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();
			RE::BSTArray<RE::TESLevCharacter*>  LevCharArray = dataHandler->GetFormArray<RE::TESLevCharacter>();

			std::for_each(std::execution::par, EncounterZoneArray.begin(), EncounterZoneArray.end(), [&](auto& encounterZone) {
				encounterZone->data.maxLevel = 127;
				logger::debug(FMT_STRING("Encounterzone {:08X} unleveled."), encounterZone->formID);
				});

			std::for_each(std::execution::par, LevCharArray.begin(), LevCharArray.end(), [&](auto& levChar) {
				levChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
				logger::debug(FMT_STRING("Changed TESLevCharacterList {:08X}"), levChar->formID);
				for (auto& entry : levChar->entries) {
					if (entry.form) {
						entry.level = 1;
						logger::debug(FMT_STRING("-> NPC {:08X} unleveled in LevCharList to level 1 "), entry.form->formID);
					}
				}
				});

			//if (form && form->formType == RE::FormType::LeveledNPC) {
			//	RE::TESLevCharacter* tempLLChar = (RE::TESLevCharacter*)form->As<RE::TESLevCharacter>();
			//	tempLLChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
			//	for (auto& entry : tempLLChar->entries) {
			//		if (entry.form) {
			//			entry.level = 1;
			//			logger::debug(FMT_STRING("NPC {:08X} unleveled to level 1 "), entry.form->formID);
			//		}
			//	}
			//}
		}

		if (statRefreshExcludeEnabled) {
			//logger::debug("statRefreshExcludeEnabled 2");
			extractToArr2D(excludeNPCs, "iUpdateNPCExcludeList\\s*=([^:]+)", excludedNPCsArr);
			for (auto arr : excludedNPCsArr) {
				if (arr.size() > 0 && IsPluginInstalled(arr[0].c_str())) {
					logger::debug("Excluded mod found: {}", arr[0]);
					for (int i = 1; i < arr.size(); i++) {
						RE::TESForm* npc = GetFormFromIdentifier(arr[i].c_str());
						if (npc && npc->formType == RE::FormType::NPC)
							excludedNPCsFormIDs.push_back(npc->formID);
					}
				}
			}
		}

		if (excludedNPCsFormIDs.size() <= 0) {
			statRefreshExcludeEnabled = false;
			logger::debug("No mods found to exclude. Shutting down exclusion function.");
		}

		//    for (const auto& pair : actData.npcData) {
		//	logger::debug("Key: {:08X} {} {} {} ", pair.first, pair.second.health, pair.second.stamina, pair.second.magicka );
		//}

		//auto allForms = RE::TESForm::GetAllForms();
		//std::for_each(std::execution::par, allForms.first->begin(), allForms.first->end(), [&](auto& pair) {
		//	auto& key = pair.first;
		//	auto& form = pair.second;
		//	if (GetPrivateProfileInt(settingsSectionFeatures, "iEnableUnlevelNPCs", 0, configFile)) {
		//		if (form && form->formType == RE::FormType::LeveledNPC) {
		//			RE::TESLevCharacter* tempLLChar = (RE::TESLevCharacter*)form->As<RE::TESLevCharacter>();
		//			tempLLChar->llFlags = RE::TESLeveledList::Flag::kCalculateForEachItemInCount;
		//			for (auto& entry : tempLLChar->entries) {
		//				if (entry.form) {
		//					entry.level = 1;
		//					logger::debug(FMT_STRING("NPC {:08X} unleveled to level 1 "), entry.form->formID);
		//				}
		//			}
		//		}
		//		if (form && form->formType == RE::FormType::EncounterZone) {
		//			RE::BGSEncounterZone* tempEncounterZone = (RE::BGSEncounterZone*)form->As<RE::BGSEncounterZone>();
		//			//tempEncounterZone->data.minLevel = 1;
		//			tempEncounterZone->data.maxLevel = 127;
		//			logger::debug(FMT_STRING("Encounterzone {:08X} unleveled."), tempEncounterZone->formID);
		//		}
		//	}
		//	if (form && form->formType == RE::FormType::ActorCharacter) {
		//		//logger::debug(FMT_STRING("IS Actor  {} {:08X}"), form->formType.underlying(), form->formID);
		//		auto refr = (RE::Actor*)form;
		//		if (refr && refr->data.objectReference) {
		//
		//			//refr->ShouldPerformRevert();
		//			//refr->extraList.RemoveByType(RE::ExtraDataType::kLevCreaModifier);
		//			//if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
		//			//	refr->data.objectReference = nullptr;
		//			//	logger::debug(FMT_STRING("Removed kLevCreaModifier"));
		//			//}
		//			//refr->InitValues();
		//			//auto aaa =  refr->GetTemplateActorBase();
		//			//logger::debug(FMT_STRING("IS NPC {:08X}"), npc->formID);
		//			//logger::debug(FMT_STRING("Stats Health {} {} {} Race {:08X}"), npc->actorData.healthOffset, npc->actorData.staminaOffset, npc->actorData.magickaOffset, npc->race->formID);
		//			//	auto player = refr->data.objectReference->As<RE::TESNPC>();
		//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, player->actorData.healthOffset + player->race->data.startingHealth);
		//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, player->actorData.magickaOffset + player->race->data.startingMagicka);
		//			//	refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, player->actorData.staminaOffset + player->race->data.startingStamina);
		//			//	//refr->InitValues();
		//			//	//refr->ApplyPerksFromBase();
		//			//}
		//			//auto cache = refr->As<RE::Actor>()->currentProcess->cachedValues->actorValueCache;
		//			//for (RE::CachedValueData* it = cache.begin(); it != cache.end(); ++it)
		//			//{
		//			//	// Access the extra data using 'it'
		//			//	logger::debug(FMT_STRING("v1 {} v2 {} v3 {}"), it->pad1, it->pad2, it->value);
		//			//	// Do something with 'extraData'
		//			//}
		//			//logger::debug(FMT_STRING("Map sizes {} {} {} "), b.healthMap.size(), b.staminaMap.size(), b.magickaMap.size());
		//			//if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
		//			//	auto healthIt = b.healthMap.find(npc->race->formID);
		//			//	if (healthIt != b.healthMap.end()) {
		//			//		npc->actorData.healthOffset = healthIt->second;
		//			//		//logger::debug(FMT_STRING("Setting Health."));
		//			//	}
		//			//	auto stamIt = b.staminaMap.find(npc->race->formID);
		//			//	if (stamIt != b.staminaMap.end()) {
		//			//		npc->actorData.staminaOffset = stamIt->second;
		//			//	}
		//			//	auto magIt = b.magickaMap.find(npc->race->formID);
		//			//	if (magIt != b.magickaMap.end()) {
		//			//		npc->actorData.magickaOffset = magIt->second;
		//			//	}
		//			//	//logger::debug(FMT_STRING("Stats Health {} {} {} Race {:08X}"), npc->actorData.healthOffset, npc->actorData.staminaOffset, npc->actorData.magickaOffset, npc->race->formID);
		//			//	StatWeights swActor;
		//			//	swActor.healthWeight = npc->npcClass->data.attributeWeights.health;
		//			//	swActor.staminaWeight = npc->npcClass->data.attributeWeights.stamina;
		//			//	swActor.magickaWeight = npc->npcClass->data.attributeWeights.magicka;
		//			//
		//			//	auto autoHealthIt = b.autoHealthMap.find(npc->race->formID);
		//			//	if (autoHealthIt != b.autoHealthMap.end()) {
		//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
		//			//		npc->actorData.healthOffset = tempResult.health;
		//			//		//logger::debug(FMT_STRING("Setting Health."));
		//			//	}
		//			//	auto autoStamIt = b.autoStaminaMap.find(npc->race->formID);
		//			//	if (autoStamIt != b.autoStaminaMap.end()) {
		//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
		//			//		npc->actorData.staminaOffset = tempResult.stamina;
		//			//	}
		//			//	auto autoMagIt = b.autoMagickaMap.find(npc->race->formID);
		//			//	if (autoMagIt != b.autoMagickaMap.end()) {
		//			//		auto tempResult = calculateAttributePoints(npc->actorData.level, swActor);
		//			//		npc->actorData.magickaOffset = tempResult.magicka;
		//			//	}
		//			//}
		//			if (refr->extraList.HasType(RE::ExtraDataType::kLevCreaModifier)) {
		//				//refr->extraList.GetByType(RE::ExtraDataType::kLevCreaModifier);
		//				auto data = (RE::ExtraLeveledCreature*)refr->extraList.GetByType(RE::ExtraDataType::kLeveledCreature);
		//				if (data) {
		//					//logger::debug(FMT_STRING("Actor {:08X}  orginalBase {:08X} templateBAse {:08X}"), refr->formID, data->originalBase->formID, data->templateBase->formID);
		//					if (data->templateBase) {
		//						refr->data.objectReference->As<RE::TESNPC>()->actorData = data->templateBase->actorData;
		//						//refr->data.objectReference->As<RE::TESNPC>()->perks = data->templateBase->perks;
		//						//refr->data.objectReference->As<RE::TESNPC>()->perkCount = data->templateBase->perkCount;
		//					}
		//				}
		//			}
		//
		//			auto npc = refr->data.objectReference->As<RE::TESNPC>();
		//			if (refr->formID == 0x000014) {
		//				playerHealth = npc->actorData.healthOffset;
		//				playerStamina = npc->actorData.staminaOffset;
		//				playerMagicka = npc->actorData.magickaOffset;
		//			}
		//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kHealth, npc->actorData.healthOffset + npc->race->data.startingHealth);
		//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kMagicka, npc->actorData.magickaOffset + npc->race->data.startingMagicka);
		//			refr->As<RE::Actor>()->SetBaseActorValue(RE::ActorValue::kStamina, npc->actorData.staminaOffset + npc->race->data.startingStamina);
		//			logger::debug(FMT_STRING("Actor OLD {:08X}  "), refr->formID);
		//			//logger::debug(FMT_STRING("Actor got patched {:08X}  "), refr->formID);
		//			//refr->InitValues();
		//			//refr->ApplyPerksFromBase();
		//		}
		//		//logger::debug(FMT_STRING("----------------------"));
		//	}
		//});

		const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_gameLoaded) };
		double     seconds = static_cast<double>(elapsed.count()) / 1000000.0;
		logger::info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Game load finished in {} seconds", seconds);

		break;
	}
	default:
		break;
	}


}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("Skypatcher");
	v.AuthorName("Zzyxzz");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
	}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "SkyPatcher";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver
#	ifndef SKYRIMVR
		< SKSE::RUNTIME_1_5_39
#	else
					> SKSE::RUNTIME_VR_1_4_15_1
#	endif
		) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	if (GetPrivateProfileInt(settingsSectionLog, "iEnablelog", 0, configFile)) {
		log->set_level(spdlog::level::debug);
		log->flush_on(spdlog::level::debug);
	}
	else {
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
	}

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e][%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());
	SKSE::Init(a_skse);
	RegisterSerialization();
	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);
	//FormDeleteEventSink::Init();
	CONTAINER::iAllowLeveledListsAddedToContainers = GetPrivateProfileInt(settingsSectionFeatures, "iAllowLeveledListsAddedToContainers", 0, configFile);
	logger::info("Enabled Features *******");
	if (CONTAINER::iAllowLeveledListsAddedToContainers == 0) {
		logger::info("iAllowLeveledListsAddedToContainers false");
	}
	else if (CONTAINER::iAllowLeveledListsAddedToContainers == 1) {
		logger::info("iAllowLeveledListsAddedToContainers true");
	}
	
	npcPatchingEnabled = GetPrivateProfileInt(settingsSectionFeatures, "iUpdateNPC", 0, configFile);
	statRefreshEnabled = GetPrivateProfileInt(settingsSectionFeatures, "iRefreshNPCStats", 0, configFile);
	RefPatchingEnabled = GetPrivateProfileInt(settingsSectionFeatures, "iUpdateRefs", 0, configFile);
	statRefreshExcludeEnabled = GetPrivateProfileInt(settingsSectionFeatures, "iUpdateNPCExclude", 0, configFile);

	iUpdateNPCVisualsOnLoad = GetPrivateProfileInt(settingsSectionFeatures, "iUpdateNPCVisualsOnLoad", 1, configFile);
	
	logger::info("iUpdateRefs {}", RefPatchingEnabled);
	logger::info("iUpdateNPC {}", npcPatchingEnabled);
	logger::info("iRefreshNPCStats {}", statRefreshEnabled);
	logger::info("iUpdateNPCExclude {}", statRefreshExcludeEnabled);
	logger::info("iUpdateNPCVisualsOnLoad {}", iUpdateNPCVisualsOnLoad);
	logger::info("************************");
	LoadIniFilesRecursive(".\\Data\\SKSE\\Plugins\\SkyPatcher\\");
	logger::info("************************");
	//prepare Race Attack Data information
	if (GetPrivateProfileInt(settingsSection, "iEnableRaceHookPatching", 0, configFile)) {
		RACE::readConfigHook(".\\Data\\SKSE\\Plugins\\SkyPatcher\\raceHook\\");
		RACE::LoadRaceDataHook::InstallHook();
	}

	//	//prepare Race Attack Data information
	//if (GetPrivateProfileInt(settingsSection, "iEnableNPCHookPatching", 0, configFile)) {
	//	RACE::readConfigHook(".\\Data\\SKSE\\Plugins\\SkyPatcher\\raceHook\\");
	//	RACE::LoadRaceDataHook::InstallHook();
	//}

	//if (GetPrivateProfileInt(settingsSection, "iEnableRaceHookPatching", 0, configFile)) {
	//RACE::readConfigHook(".\\Data\\SKSE\\Plugins\\SkyPatcher\\raceHook\\");
	//CONTAINER::readConfigHook(".\\Data\\SKSE\\Plugins\\SkyPatcher\\container\\");
	//CONTAINER::LoadRaceDataHook::InstallHook();
	//}

	if (npcPatchingEnabled)
		InstallActorHook();

	if (RefPatchingEnabled)
		InstallRefHook();

	//logger::debug("statRefreshExcludeEnabled {}", statRefreshExcludeEnabled);
	if (statRefreshEnabled && statRefreshExcludeEnabled) {
		std::ifstream configFile(".\\Data\\SKSE\\Plugins\\SkyPatcher.ini");

		std::string line;
		while (std::getline(configFile, line)) {
			// Check if the line starts with the desired prefix
			if (line.compare(0, sizeof("iUpdateNPCExcludeList") - 1, "iUpdateNPCExcludeList") == 0) {
				//logger::debug("line found");
				excludeNPCs = line;
				break;
			}
		}

		configFile.close();
	}
	return true;
}
