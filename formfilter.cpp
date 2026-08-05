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
}
