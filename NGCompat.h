#pragma once

namespace ngcompat
{
	[[nodiscard]] inline bool HasFaction(const RE::TESNPC* a_npc, const RE::TESFaction* a_faction)
	{
		return a_npc && std::ranges::any_of(a_npc->factions, [&](const auto& a_rank) {
			return a_rank.faction == a_faction;
		});
	}

	[[nodiscard]] inline RE::TESWeightForm* AmmoWeight(RE::TESAmmo* a_ammo)
	{
		if (!a_ammo || REL::Module::IsVR()) {
			return nullptr;
		}
		return &REL::RelocateMember<RE::TESWeightForm>(a_ammo, 0xB0);
	}

	[[nodiscard]] inline RE::AMMO_DATA& AmmoData(RE::TESAmmo* a_ammo) noexcept
	{
		return a_ammo->GetRuntimeData().data;
	}

	[[nodiscard]] inline RE::BGSKeywordForm* KeywordForm(RE::TESForm* a_form) noexcept
	{
		if (!a_form) {
			return nullptr;
		}
		if (a_form->GetFormType() == RE::FormType::Ammo) {
			return static_cast<RE::TESAmmo*>(a_form)->AsKeywordForm();
		}
		return a_form->As<RE::BGSKeywordForm>();
	}

	[[nodiscard]] inline const RE::BGSKeywordForm* KeywordForm(const RE::TESForm* a_form) noexcept
	{
		return KeywordForm(const_cast<RE::TESForm*>(a_form));
	}

	[[nodiscard]] inline bool AmmoIsBolt(RE::TESAmmo* a_ammo) noexcept
	{
		return AmmoData(a_ammo).flags.none(RE::AMMO_DATA::Flag::kNonBolt);
	}

	[[nodiscard]] inline const RE::TESFileCollection* VRESLFileCollection() noexcept
	{
		static std::atomic<const RE::TESFileCollection*> collection{ nullptr };
		if (const auto* known = collection.load(std::memory_order_acquire); known || !REL::Module::IsVR()) {
			return known;
		}
		if (const auto module = GetModuleHandleW(L"skyrimvresl")) {
			using func_t = const RE::TESFileCollection* (*)();
			if (const auto get = reinterpret_cast<func_t>(GetProcAddress(module, "GetCompiledFileCollectionExtern"))) {
				const auto* found = get();
				collection.store(found, std::memory_order_release);
				return found;
			}
		}
		return nullptr;
	}

	[[nodiscard]] inline RE::TESForm* LookupFormVR(RE::TESDataHandler* a_dataHandler, RE::FormID a_rawFormID, std::string_view a_modName)
	{
		const auto* file = a_dataHandler->LookupModByName(a_modName);
		if (!file || file->compileIndex == 0xFF) {
			return nullptr;
		}
		RE::FormID formID = 0;
		if (VRESLFileCollection()) {
			formID = (static_cast<RE::FormID>(file->compileIndex) << 24) + (static_cast<RE::FormID>(file->smallFileCompileIndex) << 12) + a_rawFormID;
		} else {
			formID = (a_rawFormID & 0xFFFFFF) | (static_cast<RE::FormID>(file->compileIndex) << 24);
		}
		return RE::TESForm::LookupByID(formID);
	}

	[[nodiscard]] inline bool IsPluginLoaded(RE::TESDataHandler* a_dataHandler, std::string_view a_name)
	{
		if (const auto* collection = VRESLFileCollection()) {
			const auto matches = [&](const RE::TESFile* a_file) {
				return a_file && a_name.size() == std::strlen(a_file->fileName) &&
				       _strnicmp(a_file->fileName, a_name.data(), a_name.size()) == 0;
			};
			return std::ranges::any_of(collection->files, matches) || std::ranges::any_of(collection->smallFiles, matches);
		}
		return a_dataHandler->LookupLoadedModByName(a_name) || a_dataHandler->LookupLoadedLightModByName(a_name);
	}

	[[nodiscard]] inline RE::BGSAttackData* CreateAttackData()
	{
		auto* attackData = RE::malloc<RE::BGSAttackData>();
		if (!attackData) {
			return nullptr;
		}
		std::memset(reinterpret_cast<void*>(attackData), 0, sizeof(RE::BGSAttackData));
		using func_t = RE::BGSAttackData*(RE::BGSAttackData*);
		static REL::Relocation<func_t> ctor{ RELOCATION_ID(26718, 27397) };
		return ctor(attackData);
	}

	[[nodiscard]] inline bool HasOwnAttackData(const RE::TESNPC* a_npc) noexcept
	{
		if (!a_npc || !a_npc->attackDataMap) {
			return false;
		}
		if (a_npc->baseTemplateForm && a_npc->actorData.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData)) {
			return false;
		}
		return !a_npc->race || a_npc->attackDataMap != a_npc->race->attackDataMap;
	}

	inline void FollowRaceAttackData(RE::TESNPC* a_npc, const RE::TESRace* a_oldRace)
	{
		if (!a_npc || !a_npc->race || a_npc->race == a_oldRace) {
			return;
		}
		if (a_npc->baseTemplateForm && a_npc->actorData.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kAttackData)) {
			return;
		}
		if (!a_npc->attackDataMap || (a_oldRace && a_npc->attackDataMap == a_oldRace->attackDataMap)) {
			a_npc->attackDataMap = a_npc->race->attackDataMap;
		}
	}

	inline bool AddOrReplaceAttackData(RE::BGSAttackDataMap* a_map, RE::NiPointer<RE::BGSAttackData> a_data)
	{
		if (!a_map || !a_data || a_data->event.empty()) {
			return false;
		}
		const RE::BSFixedString key = a_data->event;
		auto [it, inserted] = a_map->attackDataMap.insert({ key, a_data });
		if (!inserted) {
			it->second = std::move(a_data);
		}
		return true;
	}

	inline void CopyHeadColors(RE::TESNPC* a_target, const RE::TESNPC* a_source, bool a_keepHairColor = false)
	{
		if (!a_target || !a_source) {
			return;
		}
		const auto* data = a_source->headRelatedData;
		if (!a_keepHairColor) {
			a_target->SetHairColor(data ? data->hairColor : nullptr);
		}
		a_target->SetFaceTexture(data ? data->faceDetails : nullptr);
	}

	template <class E>
	void SetFlag(E& a_flags, E a_flag) noexcept
	{
		a_flags = static_cast<E>(std::to_underlying(a_flags) | std::to_underlying(a_flag));
	}

	template <class E>
	void ClearFlag(E& a_flags, E a_flag) noexcept
	{
		a_flags = static_cast<E>(std::to_underlying(a_flags) & ~std::to_underlying(a_flag));
	}

	inline void SetBaseActorValue(RE::Actor* a_actor, RE::ActorValue a_av, float a_value)
	{
		if (a_actor) {
			a_actor->AsActorValueOwner()->SetBaseActorValue(a_av, a_value);
		}
	}
}
