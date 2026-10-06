#include "enginelists.h"

#include <cstring>
#include <unordered_set>

namespace EngineLists
{
	namespace
	{
		struct EntryIndexList
		{
			void*               allocator;
			const std::uint8_t* data;
			std::uint32_t       capacity;
			std::uint32_t       pad14;
			std::uint32_t       size;
			std::uint32_t       pad1C;
		};
		static_assert(offsetof(EntryIndexList, data) == 0x08);
		static_assert(offsetof(EntryIndexList, size) == 0x18);

		using AddLeveledObject_t = RE::LEVELED_OBJECT* (*)(RE::TESLeveledList*, std::uint16_t, std::uint16_t, RE::TESForm*, RE::ContainerItemExtra*);
		using RemoveLeveledEntries_t = void (*)(RE::TESLeveledList*, const EntryIndexList*);
		using AddContainerObject_t = RE::ContainerObject* (*)(RE::TESContainer*, RE::TESBoundObject*, std::int32_t, RE::ContainerItemExtra*);
		using RemoveContainerObject_t = void (*)(RE::TESContainer*, RE::ContainerObject*);
		using AddAlternateTexture_t = void (*)(RE::TESModelTextureSwap*, const char*, std::uint32_t, RE::BGSTextureSet*);
		using ClearAlternateTextures_t = void (*)(RE::TESModelTextureSwap*);

		constexpr std::uint8_t kAddLeveledObjectPrologue[] = { 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x66, 0x44, 0x89, 0x44, 0x24, 0x18, 0x53, 0x55, 0x56, 0x57 };
		constexpr std::uint8_t kRemoveLeveledEntriesPrologue[] = { 0x48, 0x8B, 0xC4, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x40 };
		constexpr std::uint8_t kAddContainerObjectPrologue[] = { 0x48, 0x8B, 0xC4, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x40 };
		constexpr std::uint8_t kRemoveContainerObjectPrologue[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20 };
		constexpr std::uint8_t kAddAlternateTexturePrologue[] = { 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x50, 0x10, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57 };
		constexpr std::uint8_t kClearAlternateTexturesPrologue[] = { 0x40, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0xC7, 0x44, 0x24, 0x20, 0xFE, 0xFF, 0xFF, 0xFF, 0x48 };

		constexpr std::size_t kScriptAddedCountOffset = 0x13;

		constexpr std::uint32_t kMaxLeveledEntries = 255;

		struct Functions
		{
			AddLeveledObject_t      addLeveledObject{ nullptr };
			RemoveLeveledEntries_t  removeLeveledEntries{ nullptr };
			AddContainerObject_t    addContainerObject{ nullptr };
			RemoveContainerObject_t removeContainerObject{ nullptr };
			bool                    available{ false };
		};

		struct TextureFunctions
		{
			AddAlternateTexture_t    add{ nullptr };
			ClearAlternateTextures_t clear{ nullptr };
			bool                     available{ false };
		};

		template <std::size_t N>
		std::uintptr_t Resolve(std::uint64_t a_id, const std::uint8_t (&a_prologue)[N], bool& a_ok)
		{
			const auto address = REL::ID(a_id).address();
			if (!address || std::memcmp(reinterpret_cast<const void*>(address), a_prologue, N) != 0) {
				a_ok = false;
				return 0;
			}
			return address;
		}

		const Functions& Engine()
		{
			static const Functions functions = [] {
				Functions f{};
				bool      ok = true;
				const auto addLL = Resolve(OFFSET(14577, 14749), kAddLeveledObjectPrologue, ok);
				const auto removeLL = Resolve(OFFSET(14581, 14753), kRemoveLeveledEntriesPrologue, ok);
				const auto addCont = Resolve(OFFSET(14377, 14517), kAddContainerObjectPrologue, ok);
				const auto removeCont = Resolve(OFFSET(14380, 14522), kRemoveContainerObjectPrologue, ok);
				if (ok) {
					f.addLeveledObject = reinterpret_cast<AddLeveledObject_t>(addLL);
					f.removeLeveledEntries = reinterpret_cast<RemoveLeveledEntries_t>(removeLL);
					f.addContainerObject = reinterpret_cast<AddContainerObject_t>(addCont);
					f.removeContainerObject = reinterpret_cast<RemoveContainerObject_t>(removeCont);
					f.available = true;
					logger::info("Leveled list and container editing: active");
				} else {
					logger::warn("Leveled list and container editing: not active - this game version is not supported");
				}
				return f;
			}();
			return functions;
		}

		const TextureFunctions& TextureEngine()
		{
			static const TextureFunctions functions = [] {
				TextureFunctions f{};
				bool             ok = true;
				const auto       add = Resolve(OFFSET(14657, 14834), kAddAlternateTexturePrologue, ok);
				const auto       clear = Resolve(OFFSET(14656, 14833), kClearAlternateTexturesPrologue, ok);
				if (ok) {
					f.add = reinterpret_cast<AddAlternateTexture_t>(add);
					f.clear = reinterpret_cast<ClearAlternateTextures_t>(clear);
					f.available = true;
					logger::info("Alternate texture editing: active");
				} else {
					logger::warn("Alternate texture editing: not active - this game version is not supported");
				}
				return f;
			}();
			return functions;
		}

		struct AlternateTextureCopy
		{
			RE::BGSTextureSet* textureSet;
			std::uint32_t      index3D;
			RE::BSFixedString  name3D;
		};

		std::vector<AlternateTextureCopy> SnapshotAlternateTextures(const RE::TESModelTextureSwap* a_model)
		{
			std::vector<AlternateTextureCopy> entries;
			for (std::uint32_t i = 0; a_model && a_model->alternateTextures && i < a_model->numAlternateTextures; ++i) {
				const auto& entry = a_model->alternateTextures[i];
				entries.push_back({ entry.textureSet, entry.index3D, entry.name3D });
			}
			return entries;
		}

		std::uint8_t ScriptAddedCount(const RE::TESLeveledList* a_list)
		{
			return reinterpret_cast<const std::uint8_t*>(a_list)[kScriptAddedCountOffset];
		}

		RE::TESLeveledList* AsLeveledList(RE::TESForm* a_form)
		{
			if (!a_form) {
				return nullptr;
			}
			switch (a_form->GetFormType()) {
			case RE::FormType::LeveledItem:
				return static_cast<RE::TESLevItem*>(a_form);
			case RE::FormType::LeveledNPC:
				return static_cast<RE::TESLevCharacter*>(a_form);
			case RE::FormType::LeveledSpell:
				return static_cast<RE::TESLevSpell*>(a_form);
			default:
				return nullptr;
			}
		}

		bool ContainsForm(RE::TESLeveledList* a_list, RE::TESForm* a_target)
		{
			std::unordered_set<RE::TESLeveledList*> visited;
			std::vector<RE::TESLeveledList*>        pending{ a_list };
			while (!pending.empty()) {
				auto* current = pending.back();
				pending.pop_back();
				if (!current || !visited.insert(current).second) {
					continue;
				}
				const auto* entries = current->entries.data();
				for (std::uint32_t i = 0; entries && i < current->numEntries; ++i) {
					auto* form = entries[i].form;
					if (form == a_target) {
						return true;
					}
					if (auto* nested = AsLeveledList(form)) {
						pending.push_back(nested);
					}
				}
			}
			return false;
		}
	}

	bool IsAvailable()
	{
		return Engine().available;
	}

	bool CanHoldForm(RE::TESForm* owner, RE::TESLeveledList* list, RE::TESForm* form, std::string& reason)
	{
		if (!list || !form) {
			reason = "form not found";
			return false;
		}
		if (!list->GetCanContainFormsOfType(form->GetFormType())) {
			reason = "this form type is not allowed in this leveled list";
			return false;
		}
		if (form == owner) {
			reason = "a leveled list cannot contain itself";
			return false;
		}
		if (auto* nested = AsLeveledList(form); nested && owner && ContainsForm(nested, owner)) {
			reason = "the added leveled list already contains this list (circular reference)";
			return false;
		}
		return true;
	}

	bool AddLeveledObject(RE::TESForm* owner, RE::TESLeveledList* list, RE::TESForm* form, std::uint16_t level, std::uint16_t count)
	{
		if (!IsAvailable() || !list) {
			return false;
		}
		const auto ownerID = owner ? owner->GetFormID() : 0;
		std::string reason;
		if (!CanHoldForm(owner, list, form, reason)) {
			logger::warn(FMT_STRING("leveled list {:08X}: cannot add {:08X}: {}"), ownerID, form ? form->GetFormID() : 0, reason);
			return false;
		}
		if (list->numEntries >= kMaxLeveledEntries) {
			logger::warn(FMT_STRING("leveled list {:08X}: cannot add {:08X}: the list already has {} entries"), ownerID, form->GetFormID(), kMaxLeveledEntries);
			return false;
		}
		return Engine().addLeveledObject(list, level, count, form, nullptr) != nullptr;
	}

	std::size_t RemoveLeveledObjects(RE::TESForm* owner, RE::TESLeveledList* list, const std::function<bool(const RE::LEVELED_OBJECT&)>& pred)
	{
		if (!IsAvailable() || !list || list->numEntries == 0) {
			return 0;
		}
		if (ScriptAddedCount(list) != 0) {
			logger::warn(FMT_STRING("leveled list {:08X}: entries not removed, the list contains entries added by scripts"), owner ? owner->GetFormID() : 0);
			return 0;
		}

		std::vector<std::uint8_t> indices;
		const auto*               entries = list->entries.data();
		for (std::uint32_t i = 0; entries && i < list->numEntries; ++i) {
			if (pred(entries[i])) {
				indices.push_back(static_cast<std::uint8_t>(i));
			}
		}
		if (indices.empty()) {
			return 0;
		}

		const auto     size = static_cast<std::uint32_t>(indices.size());
		EntryIndexList view{ nullptr, indices.data(), size, 0, size, 0 };
		Engine().removeLeveledEntries(list, &view);
		return indices.size();
	}

	void ClearLeveledList(RE::TESForm* owner, RE::TESLeveledList* list)
	{
		RemoveLeveledObjects(owner, list, [](const RE::LEVELED_OBJECT&) { return true; });
	}

	void ForEachLeveledObject(RE::TESLeveledList* list, const std::function<void(RE::LEVELED_OBJECT&)>& fn)
	{
		if (!list) {
			return;
		}
		auto* entries = list->entries.data();
		for (std::uint32_t i = 0; entries && i < list->numEntries; ++i) {
			fn(entries[i]);
		}
	}

	bool AddContainerObject(RE::TESContainer& container, RE::TESBoundObject* object, std::int32_t count)
	{
		if (!IsAvailable() || !object || count == 0) {
			return false;
		}
		return Engine().addContainerObject(&container, object, count, nullptr) != nullptr;
	}

	std::size_t RemoveContainerObjects(RE::TESContainer& container, const std::function<bool(const RE::ContainerObject&)>& pred)
	{
		if (!IsAvailable() || !container.containerObjects) {
			return 0;
		}
		std::vector<RE::ContainerObject*>          matches;
		std::unordered_set<RE::ContainerObject*> seen;
		for (std::uint32_t i = 0; i < container.numContainerObjects; ++i) {
			auto* entry = container.containerObjects[i];
			if (entry && seen.insert(entry).second && pred(*entry)) {
				matches.push_back(entry);
			}
		}
		for (auto* entry : matches) {
			Engine().removeContainerObject(&container, entry);
		}
		return matches.size();
	}

	bool RemoveContainerObjectWithCount(RE::TESContainer& container, RE::TESBoundObject* object, std::int32_t count)
	{
		bool removed = false;
		RemoveContainerObjects(container, [&](const RE::ContainerObject& entry) {
			if (!removed && entry.obj == object && entry.count == count) {
				removed = true;
				return true;
			}
			return false;
		});
		return removed;
	}

	std::size_t RemoveContainerObjectInstances(RE::TESContainer& container, RE::TESBoundObject* object)
	{
		return RemoveContainerObjects(container, [object](const RE::ContainerObject& entry) {
			return entry.obj == object;
		});
	}

	std::size_t RemoveContainerObjectsByKeyword(RE::TESContainer& container, RE::BGSKeyword* keyword)
	{
		return RemoveContainerObjects(container, [keyword](const RE::ContainerObject& entry) {
			const auto* keywordForm = entry.obj ? entry.obj->As<RE::BGSKeywordForm>() : nullptr;
			return keywordForm && keywordForm->HasKeyword(keyword);
		});
	}

	void ClearContainer(RE::TESContainer& container)
	{
		RemoveContainerObjects(container, [](const RE::ContainerObject&) { return true; });
	}

	bool AlternateTexturesAvailable()
	{
		return TextureEngine().available;
	}

	bool AddAlternateTexture(RE::TESModelTextureSwap* model, RE::BGSTextureSet* textureSet, const char* name3D, std::uint32_t index3D)
	{
		if (!AlternateTexturesAvailable() || !model || !textureSet) {
			return false;
		}
		TextureEngine().add(model, name3D ? name3D : "", index3D, textureSet);
		return true;
	}

	std::size_t RemoveAlternateTextures(RE::TESModelTextureSwap* model, const std::function<bool(const RE::TESModelTextureSwap::AlternateTexture&)>& pred)
	{
		if (!AlternateTexturesAvailable() || !model || !model->alternateTextures || model->numAlternateTextures == 0) {
			return 0;
		}
		std::vector<AlternateTextureCopy> kept;
		std::size_t                       removed = 0;
		for (std::uint32_t i = 0; i < model->numAlternateTextures; ++i) {
			const auto& entry = model->alternateTextures[i];
			if (pred(entry)) {
				++removed;
			} else {
				kept.push_back({ entry.textureSet, entry.index3D, entry.name3D });
			}
		}
		if (removed == 0) {
			return 0;
		}
		TextureEngine().clear(model);
		for (const auto& entry : kept) {
			AddAlternateTexture(model, entry.textureSet, entry.name3D.c_str(), entry.index3D);
		}
		return removed;
	}

	void ClearAlternateTextures(RE::TESModelTextureSwap* model)
	{
		if (AlternateTexturesAvailable() && model) {
			TextureEngine().clear(model);
		}
	}

	void CopyAlternateTextures(RE::TESModelTextureSwap* target, const RE::TESModelTextureSwap* source)
	{
		if (!AlternateTexturesAvailable() || !target || target == source) {
			return;
		}
		const auto entries = SnapshotAlternateTextures(source);
		TextureEngine().clear(target);
		for (const auto& entry : entries) {
			AddAlternateTexture(target, entry.textureSet, entry.name3D.c_str(), entry.index3D);
		}
	}
}
