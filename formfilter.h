#ifndef FORMFILTER_H
#define FORMFILTER_H

// PROTOTYPE (proto/common-filters) -- type-erased common filter matching.
//
// SkyPatcher has 24 form-type patchers, each of which re-implements the same
// filter vocabulary against its own `line_content` / `patch_instruction` struct.
// The shared filters need nothing beyond base-class functionality:
//
//   modNames                     -> RE::TESForm::GetFile(0)->fileName
//   keywords/Or/Excluded         -> RE::BGSKeywordForm::HasKeyword
//   filterByEditorIdContains*    -> clib_util::editorID::get_editorID(RE::TESForm*)
//   objectExcluded               -> RE::TESForm::formID (+ a FormType tag)
//   hasPlugin / hasPluginOr      -> nothing from the form at all
//
// So a single NON-TEMPLATE function taking RE::TESForm* / RE::BGSKeywordForm*
// serves every form type: one copy in the binary rather than 24. A template
// would emit exactly as much code as the 24 hand-written copies do.
//
// The per-type structs are NOT touched -- the golden snapshots serialize every
// member of patch_instruction and are the only proof behaviour is preserved.
// Instead the call site builds a CommonFilterView of *references* to the
// vectors it already owns. Constructing that view is 10 pointer stores and no
// allocation, and it is meant to be built ONCE PER RULE, outside the form loop.

#include "PCH.h"
#include <string>
#include <vector>

namespace formfilter
{
	// References only. No copies, no ownership. Must not outlive the rule it
	// was built from.
	struct CommonFilterView
	{
		const std::vector<std::string>& modNames;
		const std::vector<std::string>& hasPlugin;
		const std::vector<std::string>& hasPluginOr;
		const std::vector<std::string>& keywords;
		const std::vector<std::string>& keywordsOr;
		const std::vector<std::string>& keywordsExcluded;
		const std::vector<std::string>& editorIdContains;
		const std::vector<std::string>& editorIdContainsOr;
		const std::vector<std::string>& editorIdContainsExcluded;
		const std::vector<std::string>& objectExcluded;
	};

	// An empty vector to bind the members of CommonFilterView that a given
	// form type does not have. Never modified.
	const std::vector<std::string>& noStrings();

	// --- form-independent, hoistable out of the form loop entirely ----------
	// Every plugin named by hasPlugin is installed.
	bool hasAllPlugins(const std::vector<std::string>& a_names);
	// At least one plugin named by hasPluginOr is installed.
	bool hasAnyPlugin(const std::vector<std::string>& a_names);

	// --- per-form ----------------------------------------------------------
	// modNames: true if a_form's defining plugin is one of them. Caller is
	// responsible for the `!modNames.empty()` test, exactly as before.
	bool matchesModName(const CommonFilterView& a_v, const RE::TESForm* a_form);

	// keywords (AND). a_alt is an optional second keyword holder that also
	// satisfies the test -- NPC passes curobj->race here; every other form
	// type passes nullptr. Reproduces the upstream quirk that an identifier
	// which does not resolve to a Keyword leaves the result false.
	bool matchesKeywordsAnd(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	// keywordsOr (OR).
	bool matchesKeywordsOr(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	// keywordsExcluded: true if the form carries any excluded keyword.
	bool hasExcludedKeyword(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);

	// filterByEditorIdContains (AND) / ...Or (OR) / ...Excluded.
	bool matchesEditorIdAnd(const CommonFilterView& a_v, const RE::TESForm* a_form);
	bool matchesEditorIdOr(const CommonFilterView& a_v, const RE::TESForm* a_form);
	bool hasExcludedEditorId(const CommonFilterView& a_v, const RE::TESForm* a_form);

	// objectExcluded: true if a_form's formID is one of the excluded forms.
	// a_type is the FormType the identifier must resolve to -- the one place
	// the per-type code is genuinely type-specific, passed as a value.
	bool isObjectExcluded(const CommonFilterView& a_v, const RE::TESForm* a_form, RE::FormType a_type);
}

#endif
