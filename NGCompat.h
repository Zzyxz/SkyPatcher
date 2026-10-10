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
