#include "haircolor.h"

#include "npc.h"

extern NPC::ActorData actData;

namespace HairColor
{
	namespace
	{
		thread_local bool inFullBuild = false;

		const RE::BGSColorForm* FindHairColor(RE::Actor* a_actor)
		{
			const RE::BGSColorForm* found = nullptr;
			const auto check = [&found](const RE::TESForm* a_form) {
				const auto* npc = a_form ? a_form->As<RE::TESNPC>() : nullptr;
				for (int depth = 0; npc && !found && depth < 16; ++depth) {
					if (const auto it = actData.npcData.find(npc->GetFormID()); it != actData.npcData.end() && it->second.haircolor) {
						found = it->second.haircolor;
						break;
					}
					if (!npc->actorData.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits)) {
						break;
					}
					npc = npc->baseTemplateForm ? npc->baseTemplateForm->As<RE::TESNPC>() : nullptr;
				}
			};
			check(a_actor->data.objectReference);
			if (!found) {
				if (const auto* leveled = a_actor->extraList.GetByType<RE::ExtraLeveledCreature>()) {
					check(leveled->originalBase);
					if (!found) {
						check(leveled->templateBase);
					}
				}
			}
			return found;
		}

		void CollectOwnColorParts(const RE::BGSHeadPart* a_part, std::vector<const char*>& a_out, int a_depth = 0)
		{
			if (!a_part || a_depth > 8) {
				return;
			}
			if (a_part->color) {
				a_out.push_back(a_part->formEditorID.c_str());
			}
			for (const auto* extra : a_part->extraParts) {
				CollectOwnColorParts(extra, a_out, a_depth + 1);
			}
		}

		void Retint(RE::Actor* a_actor)
		{
			if (!a_actor) {
				return;
			}
			const auto* colorForm = FindHairColor(a_actor);
			if (!colorForm) {
				return;
			}
			const RE::NiColor tint{
				static_cast<float>(colorForm->color.red) / 127.5f,
				static_cast<float>(colorForm->color.green) / 127.5f,
				static_cast<float>(colorForm->color.blue) / 127.5f
			};

			std::vector<const char*> ownColorParts;
			if (const auto* base = a_actor->GetActorBase()) {
				const auto* npc = base->faceNPC ? base->faceNPC : base;
				for (std::uint32_t i = 0; npc->headParts && i < npc->numHeadParts; ++i) {
					CollectOwnColorParts(npc->headParts[i], ownColorParts);
				}
			}

			const auto tintTree = [&](RE::NiAVObject* a_root) {
				if (!a_root) {
					return;
				}
				RE::BSVisit::TraverseScenegraphGeometries(a_root, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
					auto* effect = a_geometry->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get();
					auto* lighting = effect ? netimmerse_cast<RE::BSLightingShaderProperty*>(effect) : nullptr;
					auto* material = lighting ? lighting->material : nullptr;
					if (!material || material->GetFeature() != RE::BSShaderMaterial::Feature::kHairTint) {
						return RE::BSVisit::BSVisitControl::kContinue;
					}
					const char* name = a_geometry->name.c_str();
					for (const auto* own : ownColorParts) {
						if (name && own && std::strcmp(name, own) == 0) {
							return RE::BSVisit::BSVisitControl::kContinue;
						}
					}
					if ((material->unk30 & 0x7FFFFFFF) == 0) {
						lighting->SetMaterial(material, true);
						material = lighting->material;
					}
					if (material && material->GetFeature() == RE::BSShaderMaterial::Feature::kHairTint) {
						static_cast<RE::BSLightingShaderMaterialHairTint*>(material)->tintColor = tint;
					}
					return RE::BSVisit::BSVisitControl::kContinue;
				});
			};

			tintTree(a_actor->Get3D(false));
			if (a_actor->IsPlayerRef()) {
				tintTree(a_actor->Get3D(true));
			}
		}

		struct FullBuild
		{
			static void thunk(RE::TESNPC* a_npc, RE::Actor* a_actor)
			{
				const bool outer = std::exchange(inFullBuild, true);
				func(a_npc, a_actor);
				inFullBuild = outer;
				Retint(a_actor);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct HeadRebuild
		{
			static std::uintptr_t thunk(RE::TESNPC* a_npc, RE::Actor* a_actor, std::uintptr_t a_arg3)
			{
				const auto result = func(a_npc, a_actor, a_arg3);
				if (!inFullBuild) {
					Retint(a_actor);
				}
				return result;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct WornRebuild
		{
			static std::uintptr_t thunk(RE::TESNPC* a_npc, RE::Actor* a_actor, void* a_biped, void* a_worn)
			{
				const auto result = func(a_npc, a_actor, a_biped, a_worn);
				if (!inFullBuild) {
					Retint(a_actor);
				}
				return result;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		template <class Hook>
		bool InstallAt(const REL::CallsiteID& a_owner, const REL::AutoCallsite& a_call, const char* a_what)
		{
			const auto site = REL::try_resolve_callsite(a_owner, a_call);
			if (!site) {
				return false;
			}
			Hook::func = SKSE::GetTrampoline().write_call<5>(*site.address, Hook::thunk);
			return true;
		}
	}

	void Install()
	{
		const REL::Version se{ 1, 5, 97, 0 };
		const REL::Version vr{ 1, 4, 15, 0 };
		const REL::Version ae16{ 1, 6, 1170, 0 };
		const REL::Version ae17{ 1, 7, 104, 0 };

		const bool full = InstallAt<FullBuild>(
			REL::RelocationID(19316, 19743),
			REL::AUTO_CALLSITE(REL::CallsiteID{ REL::RelocationID(24236, 24740) }).or_offset(0x30, se, ae16, ae17, vr),
			"full build");

		const auto headOffset = REL::Module::IsAE() ? 0x169 : 0x1E0;
		const bool head = InstallAt<HeadRebuild>(
			REL::RelocationID(24228, 24732),
			REL::AutoCallsite(REL::CallsiteID{ REL::RelocationID(24220, 24724) }, REL::AutoCallsiteBranch::kCall, REL::AutoCallsite::LAST)
				.or_offset(headOffset, se, ae16, ae17, vr),
			"head rebuild");

		const auto wornOffset = REL::Module::IsAE() ? 0x250 : 0x242;
		const bool worn = InstallAt<WornRebuild>(
			REL::CallsiteID{ REL::RelocationID(36636, 37644), 0x5FBBA0 },
			REL::AUTO_CALLSITE(REL::CallsiteID{ REL::RelocationID(24237, 24741) }).or_offset(wornOffset, se, ae16, ae17, vr),
			"worn items");

		if (full) {
			logger::info("Hair colors: active{}", head && worn ? "" : " (some rebuilds keep the original color)");
		} else {
			logger::warn("Hair colors: not active - another mod probably changes the same place");
		}
	}
}
