#pragma once

namespace VoiceSafety
{
	void Build();

	[[nodiscard]] bool CanSwap(RE::Actor* a_actor, const RE::BGSVoiceType* a_old, const RE::BGSVoiceType* a_new);
}
