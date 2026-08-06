// PROTOTYPE (proto/common-filters). See formfilter.h for the rationale.
//
// Every loop below is a verbatim transcription of the upstream per-type body,
// including its quirks, because the only equivalence check available for the
// apply layer is the benchmark's match_rate. Two deliberate non-changes:
//
//   * `std::string string_form = keywordstring;` is kept even though
//     GetFormFromIdentifier takes a const&. It is a pure waste of one
//     std::string copy per (form, identifier), but removing it here would
//     mix a real optimization into a measurement whose whole purpose is to
//     isolate the cost of the indirection. Left for a follow-up.
//   * `curobj->GetFile(0)` is dereferenced without a null check, exactly as
//     all 24 copies do. GetFile(0) returns nullptr for a dynamic form; the
//     upstream code would crash there and so does this.

#include "formfilter.h"
#include "utility.h"

#include <algorithm>

namespace formfilter
{
	const std::vector<std::string>& noStrings()
	{
		static const std::vector<std::string> empty{};
		return empty;
	}

	bool hasAllPlugins(const std::vector<std::string>& a_names)
	{
		for (const auto& modName : a_names) {
			if (!IsPluginInstalled(modName.c_str())) {
				return false;
			}
		}
		return true;
	}

	bool hasAnyPlugin(const std::vector<std::string>& a_names)
	{
		for (const auto& modName : a_names) {
			if (IsPluginInstalled(modName.c_str())) {
				return true;
			}
		}
		return false;
	}

	bool matchesModName(const CommonFilterView& a_v, const RE::TESForm* a_form)
	{
		const char* const fileName = a_form->GetFile(0)->fileName;
		for (const auto& modName : a_v.modNames) {
			if (modName == fileName) {
				return true;
			}
		}
		return false;
	}

	namespace
	{
		// The single place the two-holder keyword test lives. a_alt is NPC's
		// race; nullptr for every other form type.
		inline bool holdsKeyword(const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt, RE::BGSKeyword* a_keyword)
		{
			return const_cast<RE::BGSKeywordForm*>(a_kf)->HasKeyword(a_keyword) ||
			       (a_alt && const_cast<RE::BGSKeywordForm*>(a_alt)->HasKeyword(a_keyword));
		}
	}

	bool matchesKeywordsAnd(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		bool keywordAnd = false;
		for (const auto& keywordstring : a_v.keywords) {
			RE::TESForm*    currentform = nullptr;
			RE::BGSKeyword* keyword = nullptr;

			std::string string_form = keywordstring;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				keyword = (RE::BGSKeyword*)currentform;

				if (holdsKeyword(a_kf, a_alt, keyword)) {
					keywordAnd = true;
				} else {
					keywordAnd = false;
					break;
				}
			}
		}
		return keywordAnd;
	}

	bool matchesKeywordsOr(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		for (const auto& keywordstring : a_v.keywordsOr) {
			RE::TESForm*    currentform = nullptr;
			RE::BGSKeyword* keyword = nullptr;

			std::string string_form = keywordstring;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				keyword = (RE::BGSKeyword*)currentform;

				if (holdsKeyword(a_kf, a_alt, keyword)) {
					return true;
				}
			}
		}
		return false;
	}

	bool hasExcludedKeyword(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		for (const auto& keywordstring : a_v.keywordsExcluded) {
			RE::TESForm*    currentform = nullptr;
			RE::BGSKeyword* keyword = nullptr;

			std::string string_form = keywordstring;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Keyword) {
				keyword = (RE::BGSKeyword*)currentform;

				if (holdsKeyword(a_kf, a_alt, keyword)) {
					return true;
				}
			}
		}
		return false;
	}

	bool matchesEditorIdAnd(const CommonFilterView& a_v, const RE::TESForm* a_form)
	{
		bool contains = false;
		for (const auto& editorString : a_v.editorIdContains) {
			if (toLowerCase(clib_util::editorID::get_editorID(a_form)).find(toLowerCase(editorString)) != std::string::npos) {
				contains = true;
			} else {
				contains = false;
				break;
			}
		}
		return contains;
	}

	bool matchesEditorIdOr(const CommonFilterView& a_v, const RE::TESForm* a_form)
	{
		for (const auto& editorString : a_v.editorIdContainsOr) {
			if (toLowerCase(clib_util::editorID::get_editorID(a_form)).find(toLowerCase(editorString)) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	bool hasExcludedEditorId(const CommonFilterView& a_v, const RE::TESForm* a_form)
	{
		for (const auto& editorString : a_v.editorIdContainsExcluded) {
			if (toLowerCase(clib_util::editorID::get_editorID(a_form)).find(toLowerCase(editorString)) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	bool isObjectExcluded(const CommonFilterView& a_v, const RE::TESForm* a_form, RE::FormType a_type)
	{
		for (const auto& npcstring : a_v.objectExcluded) {
			RE::TESForm* currentform = nullptr;

			std::string string_form = npcstring;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == a_type) {
				if (a_form->formID == currentform->formID) {
					return true;
				}
			}
		}
		return false;
	}

	// =====================================================================
	// PHASE 1 -- resolve once per rule. See formfilter.h for the rationale
	// and for why each equivalence below holds.
	// =====================================================================

	std::vector<RE::TESForm*> resolveForms(const std::vector<std::string>& a_ids, RE::FormType a_type)
	{
		std::vector<RE::TESForm*> out;
		out.reserve(a_ids.size());
		for (const auto& id : a_ids) {
			RE::TESForm* form = GetFormFromIdentifier(id);
			if (form && form->formType == a_type) {
				out.push_back(form);
			}
		}
		return out;
	}

	std::vector<RE::FormID> resolveFormIDs(const std::vector<std::string>& a_ids, RE::FormType a_type)
	{
		std::vector<RE::FormID> out;
		out.reserve(a_ids.size());
		for (const auto& id : a_ids) {
			RE::TESForm* form = GetFormFromIdentifier(id);
			if (form && form->formType == a_type) {
				out.push_back(form->formID);
			}
		}
		return out;
	}

	std::vector<RE::FormID> resolveFormIDsKeepingHoles(const std::vector<std::string>& a_ids, RE::FormType a_type)
	{
		std::vector<RE::FormID> out;
		out.reserve(a_ids.size());
		for (const auto& id : a_ids) {
			RE::TESForm* form = GetFormFromIdentifier(id);
			// 0 is not a valid form ID (LookupByID(0) is null), so it is a safe
			// "this identifier did not resolve to a_type" marker.
			out.push_back((form && form->formType == a_type) ? form->formID : 0);
		}
		return out;
	}

	std::vector<std::string> toLowerAll(const std::vector<std::string>& a_in)
	{
		std::vector<std::string> out;
		out.reserve(a_in.size());
		for (const auto& s : a_in) {
			out.push_back(toLowerCase(s));
		}
		return out;
	}

	namespace
	{
		std::vector<RE::BGSKeyword*> resolveKeywords(const std::vector<std::string>& a_ids)
		{
			std::vector<RE::BGSKeyword*> out;
			out.reserve(a_ids.size());
			for (const auto& id : a_ids) {
				RE::TESForm* form = GetFormFromIdentifier(id);
				if (form && form->formType == RE::FormType::Keyword) {
					out.push_back(static_cast<RE::BGSKeyword*>(form));
				}
			}
			return out;
		}
	}

	ResolvedCommonFilters resolve(const CommonFilterView& a_v, RE::FormType a_objectExcludedType)
	{
		ResolvedCommonFilters r;
		r.modNames = &a_v.modNames;

		r.keywords = resolveKeywords(a_v.keywords);
		r.keywordsOr = resolveKeywords(a_v.keywordsOr);
		r.keywordsExcluded = resolveKeywords(a_v.keywordsExcluded);

		r.editorIdContains = toLowerAll(a_v.editorIdContains);
		r.editorIdContainsOr = toLowerAll(a_v.editorIdContainsOr);
		r.editorIdContainsExcluded = toLowerAll(a_v.editorIdContainsExcluded);

		r.objectExcluded = resolveFormIDs(a_v.objectExcluded, a_objectExcludedType);

		// Mirrors the call sites' own `!list.empty() && test(list)` guard, so an
		// empty list yields true and the caller can drop the emptiness test.
		r.allPluginsInstalled = a_v.hasPlugin.empty() || hasAllPlugins(a_v.hasPlugin);
		r.anyPluginInstalled = a_v.hasPluginOr.empty() || hasAnyPlugin(a_v.hasPluginOr);

		return r;
	}

	bool matchesModName(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form)
	{
		const char* const fileName = a_form->GetFile(0)->fileName;
		for (const auto& modName : *a_r.modNames) {
			if (modName == fileName) {
				return true;
			}
		}
		return false;
	}

	bool matchesKeywordsAnd(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		bool keywordAnd = false;
		for (auto* keyword : a_r.keywords) {
			if (holdsKeyword(a_kf, a_alt, keyword)) {
				keywordAnd = true;
			} else {
				keywordAnd = false;
				break;
			}
		}
		return keywordAnd;
	}

	bool matchesKeywordsOr(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		for (auto* keyword : a_r.keywordsOr) {
			if (holdsKeyword(a_kf, a_alt, keyword)) {
				return true;
			}
		}
		return false;
	}

	bool hasExcludedKeyword(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt)
	{
		for (auto* keyword : a_r.keywordsExcluded) {
			if (holdsKeyword(a_kf, a_alt, keyword)) {
				return true;
			}
		}
		return false;
	}

	// The form's own EditorID is lowercased ONCE per call instead of once per
	// needle -- upstream recomputed toLowerCase(get_editorID(form)) inside the
	// needle loop. get_editorID is deterministic for a given form, so this is a
	// pure hoist. The empty-list early-out reproduces upstream's result exactly:
	// with no needles the loop never ran and the accumulator stayed false.
	bool matchesEditorIdAnd(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form)
	{
		if (a_r.editorIdContains.empty()) {
			return false;
		}
		const std::string editorId = toLowerCase(clib_util::editorID::get_editorID(a_form));
		bool              contains = false;
		for (const auto& needle : a_r.editorIdContains) {
			if (editorId.find(needle) != std::string::npos) {
				contains = true;
			} else {
				contains = false;
				break;
			}
		}
		return contains;
	}

	bool matchesEditorIdOr(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form)
	{
		if (a_r.editorIdContainsOr.empty()) {
			return false;
		}
		const std::string editorId = toLowerCase(clib_util::editorID::get_editorID(a_form));
		for (const auto& needle : a_r.editorIdContainsOr) {
			if (editorId.find(needle) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	bool hasExcludedEditorId(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form)
	{
		if (a_r.editorIdContainsExcluded.empty()) {
			return false;
		}
		const std::string editorId = toLowerCase(clib_util::editorID::get_editorID(a_form));
		for (const auto& needle : a_r.editorIdContainsExcluded) {
			if (editorId.find(needle) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	bool isObjectExcluded(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form)
	{
		for (const RE::FormID id : a_r.objectExcluded) {
			if (a_form->formID == id) {
				return true;
			}
		}
		return false;
	}

	// =====================================================================
	// PHASE 3 -- the rule index. See formfilter.h for the soundness argument.
	// =====================================================================

	IndexStats& stats()
	{
		static IndexStats s;
		return s;
	}

	void appendMaskBits(std::vector<std::uint64_t>& a_out, std::uint32_t a_tag, std::uint32_t a_mask)
	{
		while (a_mask) {
			const std::uint32_t bit = a_mask & (~a_mask + 1u);  // lowest set bit
			a_out.push_back(sigTagged(a_tag, bit));
			a_mask &= ~bit;
		}
	}

	void FormSignalIndex::add(std::uint64_t a_key, std::uint32_t a_formIdx)
	{
		auto& bucket = m_buckets[a_key];
		// Forms are indexed in ascending order, so a form that carries the same
		// signal twice (the same keyword listed twice, a race whose keyword the
		// NPC also carries) only ever collides with the last entry.
		if (bucket.empty() || bucket.back() != a_formIdx) {
			bucket.push_back(a_formIdx);
		}
	}

	void FormSignalIndex::gather(const std::vector<std::uint64_t>& a_keys, std::vector<std::uint32_t>& a_out) const
	{
		a_out.clear();
		if (a_keys.empty()) {
			return;
		}
		// The overwhelmingly common case: a rule names exactly one signal, so
		// the bucket IS the candidate list and is already ascending and unique.
		if (a_keys.size() == 1) {
			const auto it = m_buckets.find(a_keys.front());
			if (it != m_buckets.end()) {
				a_out = it->second;
			}
			return;
		}
		for (const std::uint64_t key : a_keys) {
			const auto it = m_buckets.find(key);
			if (it != m_buckets.end()) {
				a_out.insert(a_out.end(), it->second.begin(), it->second.end());
			}
		}
		// Sorting restores the form array's own order, which is what makes the
		// candidate walk visit forms in exactly the sequence the full scan did.
		std::sort(a_out.begin(), a_out.end());
		a_out.erase(std::unique(a_out.begin(), a_out.end()), a_out.end());
	}

	void addKeywordSignals(FormSignalIndex& a_index, const RE::BGSKeywordForm* a_kf, std::uint32_t a_formIdx)
	{
		if (!a_kf || !a_kf->keywords) {
			return;
		}
		const std::uint32_t n = a_kf->numKeywords;
		for (std::uint32_t i = 0; i < n; ++i) {
			if (a_kf->keywords[i]) {
				a_index.add(sigForm(a_kf->keywords[i]->formID), a_formIdx);
			}
		}
	}

	// =====================================================================
	// PHASE 2 -- the predicate interpreter. See formfilter.h.
	// =====================================================================
	namespace
	{
		// Static cost, in arbitrary units, of testing ONE operand of an opcode.
		// Only the ORDER matters. The two substring opcodes are two orders of
		// magnitude dearer than an integer compare because each one runs
		// get_editorID (a cross-DLL call), a toLowerCase allocation and a
		// std::string::find; everything else is a load and a compare, or a
		// short linear scan over an array the form already owns.
		std::uint32_t opCost(Op a_op)
		{
			switch (a_op) {
			case Op::EditorId:
			case Op::ModelContains:
				return 64;
			case Op::NameContains:
				return 40;
			case Op::ArmorAddon:
				return 12;
			case Op::Keyword:
				return 8;
			case Op::Faction:
				return 4;
			default:
				return 1;
			}
		}

		const std::string& lowerEditorId(const MatchTarget& a_t, MatchScratch& a_s)
		{
			if (!a_s.haveEditorId) {
				a_s.editorId = toLowerCase(clib_util::editorID::get_editorID(a_t.form));
				a_s.haveEditorId = true;
			}
			return a_s.editorId;
		}

		const std::string& lowerFullName(const MatchTarget& a_t, MatchScratch& a_s)
		{
			if (!a_s.haveFullName) {
				a_s.fullName = toLowerCase(a_t.armor->fullName.c_str());
				a_s.haveFullName = true;
			}
			return a_s.fullName;
		}

		const std::string& maleModelPath(const MatchTarget& a_t, MatchScratch& a_s)
		{
			if (!a_s.haveMaleModel) {
				a_s.maleModel = a_t.npc->race
					? normalizePath(trim(toLowerCase(a_t.npc->race->skeletonModels[RE::SEXES::kMale].model.c_str())))
					: std::string{};
				a_s.haveMaleModel = true;
			}
			return a_s.maleModel;
		}

		inline bool holdsKeywordPtr(const MatchTarget& a_t, RE::BGSKeyword* a_kw)
		{
			return holdsKeyword(a_t.kf, a_t.alt, a_kw);
		}

		// Does operand i of a_p hold for a_t? The single switch the whole
		// filter chain reduces to.
		bool operandHolds(const MatchProgram& a_prog, const Predicate& a_p, std::uint16_t a_i,
			const MatchTarget& a_t, MatchScratch& a_s)
		{
			const std::uint64_t* const ops = a_prog.pool.data() + a_p.operands;

			switch (static_cast<Op>(a_p.op)) {
			case Op::Keyword:
				return holdsKeywordPtr(a_t, reinterpret_cast<RE::BGSKeyword*>(static_cast<std::uintptr_t>(ops[a_i])));

			case Op::EditorId:
				return lowerEditorId(a_t, a_s).find(a_prog.strings[static_cast<std::size_t>(ops[a_i])]) != std::string::npos;

			case Op::NameContains:
				return lowerFullName(a_t, a_s).find(a_prog.strings[static_cast<std::size_t>(ops[a_i])]) != std::string::npos;

			case Op::ModelContains:
				// Upstream leaves `found` alone for a raceless NPC, i.e. the
				// test passes vacuously.
				return !a_t.npc->race ||
				       maleModelPath(a_t, a_s).find(a_prog.strings[static_cast<std::size_t>(ops[a_i])]) != std::string::npos;

			case Op::Race:
				return a_t.npc->race && a_t.npc->race->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::NpcClass:
				// NOTE: upstream's filterByClassExclude dereferences npcClass
				// with NO null check and would crash on a classless NPC. The
				// null check here can only change the outcome for a form that
				// would have crashed.
				return a_t.npc->npcClass && a_t.npc->npcClass->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::Outfit:
				return a_t.npc->defaultOutfit && a_t.npc->defaultOutfit->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::CombatStyle:
				return a_t.npc->combatStyle && a_t.npc->combatStyle->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::VoiceType:
				return a_t.npc->voiceType && a_t.npc->voiceType->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::Faction:
				return const_cast<RE::TESNPC*>(a_t.npc)->HasFaction(
					reinterpret_cast<RE::TESFaction*>(static_cast<std::uintptr_t>(ops[a_i])));

			case Op::ObjectId:
				return a_t.form->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::NpcFlag:
				return a_t.npc->actorData.actorBaseFlags.any(
					static_cast<RE::ACTOR_BASE_DATA::Flag>(static_cast<std::uint32_t>(ops[a_i])));

			case Op::NpcTemplateFlag:
				return a_t.npc->actorData.templateUseFlags.any(
					static_cast<RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG>(static_cast<std::uint32_t>(ops[a_i])));

			case Op::NpcSkillRange: {
				const std::uint64_t* const e = ops + std::size_t{ 3 } * a_i;
				const int value = a_t.npc->playerSkills.values[static_cast<std::size_t>(e[0])];
				return value >= static_cast<int>(e[1]) && value <= static_cast<int>(e[2]);
			}

			case Op::WeapAnimType:
				return a_t.weapon->weaponData.animationType ==
				       static_cast<RE::WEAPON_TYPE>(static_cast<std::uint32_t>(ops[a_i]));

			case Op::WeapSkill:
				return a_t.weapon->weaponData.skill ==
				       static_cast<RE::ActorValue>(static_cast<std::uint32_t>(ops[a_i]));

			case Op::WeapModel1st:
				return a_t.weapon->firstPersonModelObject &&
				       a_t.weapon->firstPersonModelObject->formID == static_cast<RE::FormID>(ops[a_i]);

			case Op::WeapBound:
				// restrictToFlags' upstream loop can only leave `found` true if
				// its FIRST entry is "boundweapon" and the flag is set; every
				// other entry writes found = false before the break can fire.
				return ops[0] != 0 &&
				       a_t.weapon->weaponData.flags2.any(RE::TESObjectWEAP::Data::Flag2::kBoundWeapon);

			case Op::ArmorType:
				return (static_cast<std::uint32_t>(a_t.armor->bipedModelData.armorType.underlying()) &
				           static_cast<std::uint32_t>(ops[a_i])) != 0;

			case Op::BipedSlot:
				return (static_cast<std::uint32_t>(a_t.armor->bipedModelData.bipedObjectSlots.underlying()) &
				           static_cast<std::uint32_t>(ops[a_i])) != 0;

			case Op::ArmorAddon: {
				const RE::FormID id = static_cast<RE::FormID>(ops[a_i]);
				for (const auto& addon : a_t.armor->armorAddons) {
					if (addon && addon->formID == id) {
						return true;
					}
				}
				return false;
			}
			}
			return false;
		}

		bool predicateHolds(const MatchProgram& a_prog, const Predicate& a_p,
			const MatchTarget& a_t, MatchScratch& a_s)
		{
			switch (static_cast<Mode>(a_p.mode)) {
			case Mode::And:
				// Empty is FALSE: every upstream AND accumulator starts false.
				if (a_p.count == 0) {
					return false;
				}
				if (static_cast<Op>(a_p.op) == Op::ArmorAddon) {
					// The one AND loop in this codebase that resets its
					// accumulator at the TOP of each iteration, which makes a
					// hole (an identifier that did not resolve to an Armature)
					// observable. Transcribed rather than reduced.
					bool foundInList = false;
					for (std::uint16_t i = 0; i < a_p.count; ++i) {
						foundInList = false;
						if (a_prog.pool[a_p.operands + i]) {
							foundInList = operandHolds(a_prog, a_p, i, a_t, a_s);
							if (!foundInList) {
								return false;
							}
						}
					}
					return foundInList;
				}
				for (std::uint16_t i = 0; i < a_p.count; ++i) {
					if (!operandHolds(a_prog, a_p, i, a_t, a_s)) {
						return false;
					}
				}
				return true;

			case Mode::Or:
				for (std::uint16_t i = 0; i < a_p.count; ++i) {
					if (operandHolds(a_prog, a_p, i, a_t, a_s)) {
						return true;
					}
				}
				return false;

			case Mode::Exclude:
				for (std::uint16_t i = 0; i < a_p.count; ++i) {
					if (operandHolds(a_prog, a_p, i, a_t, a_s)) {
						return false;
					}
				}
				return true;
			}
			return false;
		}

		bool disjunctHolds(const MatchProgram& a_prog, const Disjunct& a_d,
			const MatchTarget& a_t, MatchScratch& a_s)
		{
			for (std::uint16_t i = 0; i < a_d.count; ++i) {
				const Predicate& p = a_prog.preds[a_d.first + i];
				if (p.skipIfPlayer && a_t.isPlayer) {
					return false;
				}
				if (!predicateHolds(a_prog, p, a_t, a_s)) {
					return false;
				}
			}
			return a_d.count != 0;
		}
	}

	bool matchesModName(const MatchProgram& a_p, const RE::TESForm* a_form)
	{
		const char* const fileName = a_form->GetFile(0)->fileName;
		for (const auto& modName : *a_p.modNames) {
			if (modName == fileName) {
				return true;
			}
		}
		return false;
	}

	bool runProgram(const MatchProgram& a_prog, const MatchTarget& a_t, MatchScratch& a_scratch)
	{
		a_scratch.reset();

		bool found = false;
		for (const Disjunct& d : a_prog.positive) {
			if (disjunctHolds(a_prog, d, a_t, a_scratch)) {
				found = true;
				break;
			}
		}

		// NPC's filterByPCLevelMult / AutoCalc / Essential / Protected: each
		// ASSIGNS found, with no !found guard, so they run in source order and
		// after the first positive stage.
		for (const Predicate& p : a_prog.assigns) {
			found = predicateHolds(a_prog, p, a_t, a_scratch);
		}

		if (!found) {
			for (const Disjunct& d : a_prog.positive2) {
				if (disjunctHolds(a_prog, d, a_t, a_scratch)) {
					found = true;
					break;
				}
			}
		}

		if (!found && a_prog.catchAll && !a_t.isPlayer) {
			found = true;
		}

		for (const Predicate& p : a_prog.negative) {
			if (!found) {
				break;
			}
			if (p.skipIfPlayer && a_t.isPlayer) {
				continue;
			}
			found = predicateHolds(a_prog, p, a_t, a_scratch);
		}

		return found;
	}

	Predicate makePredicate(MatchProgram& a_prog, Op a_op, Mode a_mode,
		const std::vector<std::uint64_t>& a_values, bool a_skipIfPlayer)
	{
		Predicate p{};
		p.op = static_cast<std::uint8_t>(a_op);
		p.mode = static_cast<std::uint8_t>(a_mode);
		p.skipIfPlayer = a_skipIfPlayer ? 1u : 0u;
		p.operands = a_prog.mark();
		p.count = static_cast<std::uint16_t>(
			a_op == Op::NpcSkillRange ? a_values.size() / 3 : a_values.size());
		a_prog.pool.insert(a_prog.pool.end(), a_values.begin(), a_values.end());
		return p;
	}

	void pushDisjunct(MatchProgram& a_prog, std::vector<Disjunct>& a_stage, std::size_t a_firstPred)
	{
		const std::size_t n = a_prog.preds.size() - a_firstPred;
		if (n == 0) {
			return;
		}
		Disjunct d{};
		d.first = static_cast<std::uint16_t>(a_firstPred);
		d.count = static_cast<std::uint16_t>(n);
		d.cost = 0;
		for (std::size_t i = 0; i < n; ++i) {
			const Predicate& p = a_prog.preds[a_firstPred + i];
			d.cost += opCost(static_cast<Op>(p.op)) * (p.count ? p.count : 1u);
		}
		a_stage.push_back(d);
	}

	std::vector<std::uint64_t> idOperands(const std::vector<std::string>& a_ids, RE::FormType a_type)
	{
		std::vector<std::uint64_t> out;
		for (const RE::FormID id : resolveFormIDs(a_ids, a_type)) {
			out.push_back(id);
		}
		return out;
	}

	std::vector<std::uint64_t> ptrOperands(const std::vector<std::string>& a_ids, RE::FormType a_type)
	{
		std::vector<std::uint64_t> out;
		for (RE::TESForm* form : resolveForms(a_ids, a_type)) {
			out.push_back(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(form)));
		}
		return out;
	}

	std::vector<std::uint64_t> needleOperands(MatchProgram& a_prog, const std::vector<std::string>& a_raw,
		bool a_normalizePath)
	{
		std::vector<std::uint64_t> out;
		for (const auto& s : a_raw) {
			out.push_back(a_prog.strings.size());
			a_prog.strings.push_back(a_normalizePath ? normalizePath(trim(toLowerCase(s))) : toLowerCase(s));
		}
		return out;
	}

	void addPred(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer)
	{
		const Predicate pred = makePredicate(a_prog, a_op, a_mode, a_vals, a_skipIfPlayer);
		a_prog.preds.push_back(pred);
	}

	void addNegative(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer)
	{
		const Predicate pred = makePredicate(a_prog, a_op, a_mode, a_vals, a_skipIfPlayer);
		a_prog.negative.push_back(pred);
	}

	void sortCheapestFirst(std::vector<Disjunct>& a_stage)
	{
		std::stable_sort(a_stage.begin(), a_stage.end(),
			[](const Disjunct& a, const Disjunct& b) { return a.cost < b.cost; });
	}

	bool programSignals(const MatchProgram& a_prog, std::vector<std::uint64_t>& a_keys)
	{
		a_keys.clear();
		if (a_prog.catchAll || a_prog.unkeyable || !a_prog.assigns.empty()) {
			return false;
		}

		const auto emit = [&a_prog, &a_keys](const Predicate& p, std::uint16_t i) {
			const std::uint64_t v = a_prog.pool[p.operands + i];
			switch (static_cast<Op>(p.op)) {
			case Op::Keyword:
				a_keys.push_back(sigForm(reinterpret_cast<RE::BGSKeyword*>(static_cast<std::uintptr_t>(v))->formID));
				return true;
			case Op::Faction:
				a_keys.push_back(sigForm(reinterpret_cast<RE::TESFaction*>(static_cast<std::uintptr_t>(v))->formID));
				return true;
			case Op::Race:
			case Op::NpcClass:
			case Op::Outfit:
			case Op::CombatStyle:
			case Op::WeapModel1st:
			case Op::ArmorAddon:
				a_keys.push_back(sigForm(static_cast<RE::FormID>(v)));
				return true;
			case Op::WeapAnimType:
				a_keys.push_back(sigTagged(kTagWeapAnimType, static_cast<std::uint32_t>(v)));
				return true;
			case Op::WeapSkill:
				a_keys.push_back(sigTagged(kTagWeapSkill, static_cast<std::uint32_t>(v)));
				return true;
			case Op::ArmorType:
				appendMaskBits(a_keys, kTagArmorType, static_cast<std::uint32_t>(v));
				return true;
			case Op::BipedSlot:
				appendMaskBits(a_keys, kTagBipedSlot, static_cast<std::uint32_t>(v));
				return true;
			default:
				// Substring tests and everything else have no finite key set.
				return false;
			}
		};

		// BOTH positive stages: `found` is the OR over all of them, so every
		// one of them needs a necessary signal. (Missing `positive2` here made
		// a faction-filtered NPC rule visit only the candidates its OTHER
		// filters named -- caught by the harness's matched-pair census.)
		std::vector<const Disjunct*> stages;
		stages.reserve(a_prog.positive.size() + a_prog.positive2.size());
		for (const Disjunct& d : a_prog.positive) {
			stages.push_back(&d);
		}
		for (const Disjunct& d : a_prog.positive2) {
			stages.push_back(&d);
		}

		for (const Disjunct* dp : stages) {
			const Disjunct& d = *dp;
			// A conjunction needs ALL of its predicates, so ANY ONE of them
			// supplies a necessary signal. Prefer an And predicate (one key)
			// over an Or (one key per operand); that is exactly what Phase 3's
			// appendAndOrSignals did.
			const Predicate* chosen = nullptr;
			for (std::uint16_t i = 0; i < d.count; ++i) {
				const Predicate& p = a_prog.preds[d.first + i];
				if (static_cast<Mode>(p.mode) == Mode::And) {
					chosen = &p;
					break;
				}
				if (!chosen) {
					chosen = &p;
				}
			}
			if (!chosen) {
				continue;
			}
			if (static_cast<Mode>(chosen->mode) == Mode::And) {
				if (chosen->count == 0) {
					continue;  // can never hold, so the disjunct contributes nothing
				}
				if (static_cast<Op>(chosen->op) == Op::ArmorAddon) {
					// A 0 hole is skipped by the AND loop, so the first NON-ZERO
					// entry is the necessary signal. An all-holes list can never
					// pass, so contributing nothing is correct.
					for (std::uint16_t i = 0; i < chosen->count; ++i) {
						if (a_prog.pool[chosen->operands + i]) {
							emit(*chosen, i);
							break;
						}
					}
					continue;
				}
				if (!emit(*chosen, 0)) {
					return false;
				}
				continue;
			}
			for (std::uint16_t i = 0; i < chosen->count; ++i) {
				if (!emit(*chosen, i)) {
					return false;
				}
			}
		}
		return true;
	}

	// =====================================================================
	// PHASE 4 -- the action table.
	// =====================================================================
	const ActionRow* findAction(const ActionRow* a_rows, std::size_t a_count, const std::string& a_raw)
	{
		// ONE lowercase for the whole table, where the ladders did one per
		// rung. toLowerCase (utility.cpp) is the exact fold the ladders used:
		// std::tolower over unsigned char, byte by byte.
		return findActionLowered(a_rows, a_count, toLowerCase(a_raw));
	}

	const ActionRow* findActionLowered(const ActionRow* a_rows, std::size_t a_count, std::string_view a_lowered)
	{
		for (std::size_t i = 0; i < a_count; ++i) {
			if (a_lowered == a_rows[i].name) {
				return &a_rows[i];
			}
		}
		// Upstream's trailing `else`: the caller decides between "nothing" and
		// "log a warning". Never a silent default value.
		return nullptr;
	}
}
