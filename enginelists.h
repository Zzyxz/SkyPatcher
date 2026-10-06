#ifndef ENGINE_LISTS_H
#define ENGINE_LISTS_H

#include "PCH.h"
#include <functional>

namespace EngineLists
{
	bool IsAvailable();

	bool CanHoldForm(RE::TESForm* owner, RE::TESLeveledList* list, RE::TESForm* form, std::string& reason);

	bool AddLeveledObject(RE::TESForm* owner, RE::TESLeveledList* list, RE::TESForm* form, std::uint16_t level, std::uint16_t count);

	std::size_t RemoveLeveledObjects(RE::TESForm* owner, RE::TESLeveledList* list, const std::function<bool(const RE::LEVELED_OBJECT&)>& pred);

	void ClearLeveledList(RE::TESForm* owner, RE::TESLeveledList* list);

	void ForEachLeveledObject(RE::TESLeveledList* list, const std::function<void(RE::LEVELED_OBJECT&)>& fn);

	bool AddContainerObject(RE::TESContainer& container, RE::TESBoundObject* object, std::int32_t count);

	std::size_t RemoveContainerObjects(RE::TESContainer& container, const std::function<bool(const RE::ContainerObject&)>& pred);

	bool RemoveContainerObjectWithCount(RE::TESContainer& container, RE::TESBoundObject* object, std::int32_t count);

	std::size_t RemoveContainerObjectInstances(RE::TESContainer& container, RE::TESBoundObject* object);

	std::size_t RemoveContainerObjectsByKeyword(RE::TESContainer& container, RE::BGSKeyword* keyword);

	void ClearContainer(RE::TESContainer& container);

	bool AlternateTexturesAvailable();

	bool AddAlternateTexture(RE::TESModelTextureSwap* model, RE::BGSTextureSet* textureSet, const char* name3D, std::uint32_t index3D);

	std::size_t RemoveAlternateTextures(RE::TESModelTextureSwap* model, const std::function<bool(const RE::TESModelTextureSwap::AlternateTexture&)>& pred);

	void ClearAlternateTextures(RE::TESModelTextureSwap* model);

	void CopyAlternateTextures(RE::TESModelTextureSwap* target, const RE::TESModelTextureSwap* source);
}

#endif
