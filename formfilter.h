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

	// =====================================================================
	// PHASE 1 -- resolve once per rule.
	//
	// CommonFilterView above is a bundle of references to the RAW TEXT the
	// rule was parsed from, and every matcher taking it re-interprets that
	// text on every (form, rule) pair: "Skyrim.esm|0013BA9" is split,
	// hex-parsed and looked up again for every single form in the load order,
	// millions of times, to produce the same RE::FormID every time. Likewise
	// every editorId needle is re-lowercased per pair although it is fixed at
	// load.
	//
	// ResolvedCommonFilters is the same vocabulary with the text already
	// interpreted. It is built ONCE PER RULE, from a CommonFilterView, inside
	// process_patch_instructions -- deliberately NOT at parse time:
	// create_patch_instruction must stay free of CommonLib singletons
	// (golden_dump runs with no game behind it and would crash), and
	// patch_instruction's layout must not change, because the golden
	// snapshots serialize it member by member.
	//
	// Only npc.cpp / weapon.cpp / armor.cpp use this. The other 21 form types
	// keep the CommonFilterView overloads, deliberately: they have no
	// apply-layer benchmark, and an unmeasured change repeated across 21
	// files is exactly what this project has been avoiding.
	struct ResolvedCommonFilters
	{
		// Still text, and deliberately so. matchesModName compares against
		// RE::TESForm::GetFile(0)->fileName with std::string::operator==, i.e.
		// CASE-SENSITIVELY. Resolving these names through LookupModByName
		// (which is case-INsensitive) and comparing TESFile* would match
		// strictly more rules than upstream does. Left as a per-pair string
		// compare rather than silently widened.
		const std::vector<std::string>* modNames = nullptr;

		// Identifiers that resolved to a FormType::Keyword form. Identifiers
		// that did NOT resolve are dropped: in every upstream keyword loop a
		// non-resolving entry is a pure no-op (the AND loop's accumulator is
		// not reset at the top of an iteration), so dropping them cannot
		// change the outcome.
		std::vector<RE::BGSKeyword*> keywords;
		std::vector<RE::BGSKeyword*> keywordsOr;
		std::vector<RE::BGSKeyword*> keywordsExcluded;

		// The needles, lowercased once.
		std::vector<std::string> editorIdContains;
		std::vector<std::string> editorIdContainsOr;
		std::vector<std::string> editorIdContainsExcluded;

		// objectExcluded identifiers that resolved to the FormType the call
		// site demands, reduced to the only thing the test reads: the formID.
		std::vector<RE::FormID> objectExcluded;

		// hasPlugin / hasPluginOr consult no form at all, so they collapse to
		// two bools for the whole rule. Empty list => the upstream `!empty()`
		// guard never runs the test => true.
		bool allPluginsInstalled = true;
		bool anyPluginInstalled = true;
	};

	// Build the resolved set. Calls RE::TESDataHandler / RE::TESForm lookups,
	// so it must only ever run with a game (or the mock) behind it.
	ResolvedCommonFilters resolve(const CommonFilterView& a_v, RE::FormType a_objectExcludedType);

	// --- helpers the per-type files use for their own filter vocabulary ----
	// Resolve identifiers to the forms of a_type they name, dropping every
	// identifier that does not resolve to that type.
	std::vector<RE::TESForm*> resolveForms(const std::vector<std::string>& a_ids, RE::FormType a_type);
	// Same, reduced to formIDs.
	std::vector<RE::FormID> resolveFormIDs(const std::vector<std::string>& a_ids, RE::FormType a_type);
	// Same, but one output slot per input, with 0 marking "did not resolve".
	// Needed wherever an upstream AND-loop resets its accumulator at the top of
	// each iteration, which makes a non-resolving entry observable.
	std::vector<RE::FormID> resolveFormIDsKeepingHoles(const std::vector<std::string>& a_ids, RE::FormType a_type);
	// toLowerCase applied to every element, once.
	std::vector<std::string> toLowerAll(const std::vector<std::string>& a_in);

	// --- the resolved matchers --------------------------------------------
	bool matchesModName(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);

	bool matchesKeywordsAnd(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool matchesKeywordsOr(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool hasExcludedKeyword(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);

	bool matchesEditorIdAnd(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);
	bool matchesEditorIdOr(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);
	bool hasExcludedEditorId(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);

	bool isObjectExcluded(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);
}

#endif
