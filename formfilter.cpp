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
}
