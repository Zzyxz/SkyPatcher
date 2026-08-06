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
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
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

	// =====================================================================
	// PHASE 3 -- the rule index.
	//
	// process_patch_instructions is O(rules x forms): every rule walks the
	// whole form array. At the ~20,000 rules a heavy modlist ships that is
	// ~10^9 (form, rule) pairs, and Phase 1 only made each pair cheaper.
	//
	// Almost every scanning rule can only ever match a form that carries one
	// specific thing -- a keyword, a race, a class, a combat style, an outfit,
	// a faction, an armor addon, a biped slot, a weapon animation type. Those
	// are all small integers (a FormID, or an enum value). So the forms are
	// indexed ONCE by every such "signal" they carry, and a rule is then
	// evaluated only against the forms that carry at least one of the signals
	// its filters name.
	//
	// The nesting is deliberately NOT inverted. Keeping the rule loop on the
	// outside means the order in which rules are applied -- to each form and
	// globally, including the `object` loop that patches explicitly named
	// forms before the scan -- is byte-for-byte the upstream order. The only
	// thing that changes is that pairs which provably cannot match are never
	// visited. Inverting to form-outer would have moved every direct `object`
	// patch relative to every scan patch, which is observable whenever a form
	// is named by one rule and scan-matched by another.
	//
	// SOUNDNESS. For each rule the caller derives a key set K such that
	// `matches(rule, form)` implies `form carries some k in K`. K is the union
	// over the rule's positive (found = true) disjuncts of a NECESSARY signal
	// for that disjunct -- for an AND-list, any one of its members; for an
	// OR-list, all of them. A rule that carries a positive disjunct which is
	// not expressible this way (an EditorID/name substring test, a filter that
	// sets `found` with no `!found` guard, or the "no filters at all" catch-all
	// that matches everything) is NOT keyed and still scans every form. A rule
	// whose every positive disjunct resolved to nothing yields an EMPTY K,
	// which is correct: it can never match, so it visits no forms at all.
	// =====================================================================

	// A signal key is a 64-bit value: the low 32 bits are the value, the high
	// 32 bits a tag that keeps the value spaces apart. Tag 0 is reserved for
	// RE::FormID, which is globally unique across form types, so keywords,
	// races, classes, outfits, factions and armor addons all share it.
	inline constexpr std::uint64_t sigForm(RE::FormID a_id)
	{
		return static_cast<std::uint64_t>(a_id);
	}
	inline constexpr std::uint64_t sigTagged(std::uint32_t a_tag, std::uint32_t a_value)
	{
		return (static_cast<std::uint64_t>(a_tag) << 32) | static_cast<std::uint64_t>(a_value);
	}

	// Appends one key per SET BIT of a_mask. Used for the tests written as
	// `form.mask & filter.mask`, which is true iff the two share a bit -- so
	// the bits, not the mask, are the indexable unit. A zero mask contributes
	// nothing, which is right: `x & 0` is never true.
	void appendMaskBits(std::vector<std::uint64_t>& a_out, std::uint32_t a_tag, std::uint32_t a_mask);

	// Where each rule landed, and how many (form, rule) pairs each bucket cost.
	// Six counters incremented once per rule (and twice per rule for the visit
	// totals), i.e. nothing measurable; they exist so the bucket distribution
	// of a real config is a measurement rather than an assumption.
	struct IndexStats
	{
		std::uint64_t rulesKeyed = 0;      // restricted to a candidate list
		std::uint64_t rulesScanned = 0;    // a filter with no finite key set
		std::uint64_t rulesStale = 0;      // keyable, but an earlier rule's patch() invalidated the index
		std::uint64_t candidateVisits = 0; // forms visited by keyed rules
		std::uint64_t scanVisits = 0;      // forms visited by scanning rules
		std::uint64_t indexBuilds = 0;
	};
	IndexStats& stats();

	// form signal -> ascending indices into the form array.
	class FormSignalIndex
	{
	public:
		// a_formIdx must be non-decreasing across calls, which is what lets the
		// duplicate check be a single comparison against the bucket's back().
		void add(std::uint64_t a_key, std::uint32_t a_formIdx);

		// Ascending, unique form indices carrying at least one of a_keys.
		// a_out is cleared first. An empty a_keys yields an empty a_out.
		void gather(const std::vector<std::uint64_t>& a_keys, std::vector<std::uint32_t>& a_out) const;

		std::size_t signals() const { return m_buckets.size(); }

	private:
		std::unordered_map<std::uint64_t, std::vector<std::uint32_t>> m_buckets;
	};

	// Every keyword a form (or, for NPC, its race) holds. Reproduces the base
	// RE::BGSKeywordForm::HasKeyword, which is a linear scan over this array --
	// none of TESNPC, TESRace, TESObjectWEAP or TESObjectARMO overrides slot 04.
	void addKeywordSignals(FormSignalIndex& a_index, const RE::BGSKeywordForm* a_kf, std::uint32_t a_formIdx);

	// =====================================================================
	// PHASE 2 -- the predicate opcode table.
	//
	// NPC::findObject is ~590 lines of hand-written `if` blocks; WEAPON's and
	// ARMOR's equivalents are another ~430 inlined into their
	// process_patch_instructions. All three are the same program written out
	// three times, and every one of them re-tests `!line.someList.empty()` on
	// every (form, rule) pair to decide whether a filter is even present --
	// a question fixed the moment the rule was parsed.
	//
	// A rule is therefore COMPILED ONCE, into a MatchProgram: a flat list of
	// Predicates over ONE flat operand pool. Matching is a switch over the
	// opcodes. A rule that carries three filters holds three predicates and a
	// handful of pool words, instead of ~30 separately-allocated std::vectors
	// most of which are empty.
	//
	// STRUCTURE OF THE PROGRAM, and why it is safe to reorder.
	//
	//   found = OR over `positive` disjuncts          (each disjunct is an AND
	//                                                  chain of predicates)
	//   then  `assigns` are applied in order          (NPC only -- the four
	//                                                  filters that write
	//                                                  `found` with no !found
	//                                                  guard, so they can turn
	//                                                  a non-match INTO a match
	//                                                  and a match into a
	//                                                  non-match)
	//   then  if (!found) found = OR over `positive2` (NPC's EditorID and
	//                                                  faction pairs, which sit
	//                                                  after the assigns)
	//   then  if (!found) found = catchAll
	//   then  found &&= every `negative` predicate    (source order)
	//
	// Every disjunct in `positive` / `positive2` only ever WRITES true, and
	// every one that upstream guards with `!found` is skipped once found is
	// true, so their relative order cannot change the answer. That is what
	// makes CHEAPEST-FIRST ordering free: the disjuncts are stable-sorted by
	// a static per-opcode cost, so an integer compare gets the chance to
	// reject before an EditorID substring search runs. `assigns` and
	// `negative` are NOT reordered -- the former are order-sensitive by
	// construction, and the latter carry upstream's logging.
	// =====================================================================

	enum class Op : std::uint8_t
	{
		Keyword,        // operand: RE::BGSKeyword*      -- form, or its alt holder, carries it
		EditorId,       // operand: string index         -- lowered EditorID contains the needle
		NameContains,   // operand: string index         -- lowered fullName contains the needle
		ModelContains,  // operand: string index         -- NPC male skeleton path contains it
		Race,           // operand: RE::FormID
		NpcClass,       // operand: RE::FormID
		Outfit,         // operand: RE::FormID
		CombatStyle,    // operand: RE::FormID
		VoiceType,      // operand: RE::FormID
		Faction,        // operand: RE::TESFaction*
		ObjectId,       // operand: RE::FormID           -- compared to the form's own formID
		NpcFlag,        // operand: ACTOR_BASE_DATA::Flag bit
		NpcTemplateFlag,// operand: TEMPLATE_USE_FLAG bit
		NpcSkillRange,  // 3 operands per entry: skill index, lo, hi
		WeapAnimType,   // operand: RE::WEAPON_TYPE
		WeapSkill,      // operand: RE::ActorValue
		WeapModel1st,   // operand: RE::FormID of firstPersonModelObject
		WeapBound,      // 1 operand: 1 iff restrictToFlags[0] == "boundweapon"
		ArmorType,      // operand: BIPED_MODEL::ArmorType bit
		BipedSlot,      // operand: BipedObjectSlot bit
		ArmorAddon,     // operand: RE::FormID; Mode::And keeps 0 holes (see armor.cpp)
	};

	// And     -- every operand must hold. NOTE: an EMPTY operand list is
	//            FALSE, not true, because every upstream AND loop starts its
	//            accumulator at false and never runs.
	// Or      -- at least one operand holds.
	// Exclude -- no operand holds.
	enum class Mode : std::uint8_t
	{
		And,
		Or,
		Exclude,
	};

	struct Predicate
	{
		std::uint8_t  op;            // Op
		std::uint8_t  mode;          // Mode
		std::uint8_t  skipIfPlayer;  // NPC: several upstream blocks carry `formID != 0x7`
		std::uint8_t  pad = 0;
		std::uint32_t operands;      // offset into MatchProgram::pool
		std::uint16_t count;         // number of operands (entries, not words)
	};

	// A conjunction of predicates. Upstream's AND/OR filter PAIRS
	// (`(!and.empty() || !or.empty()) && andHolds && orHolds`) are exactly a
	// two-predicate disjunct.
	struct Disjunct
	{
		std::uint16_t first;
		std::uint16_t count;
		std::uint32_t cost;  // static, from the opcode table; sorts cheapest-first
	};

	struct MatchProgram
	{
		std::vector<Predicate>     preds;
		std::vector<Disjunct>      positive;
		std::vector<Predicate>     assigns;
		std::vector<Disjunct>      positive2;
		std::vector<Predicate>     negative;
		std::vector<std::uint64_t> pool;     // ONE flat operand pool for the whole rule
		std::vector<std::string>   strings;  // substring needles, indexed from the pool

		// The rule's modNames filter, still raw text -- see ResolvedCommonFilters.
		const std::vector<std::string>* modNames = nullptr;

		bool catchAll = false;   // no identifying filter at all: matches everything
		bool skipRule = false;   // hasPlugins/hasPluginsOr failed: matches nothing
		bool unkeyable = false;  // carries a filter with no finite index key set

		// Appends one operand and returns the pool offset of the first of a run.
		std::uint32_t mark() const { return static_cast<std::uint32_t>(pool.size()); }
	};

	// What a form looks like to the engine. Built per (form, rule) pair; a
	// handful of stores. Only the pointer for the program's own form type is
	// ever read, so no type tag is needed.
	struct MatchTarget
	{
		const RE::TESForm*        form = nullptr;
		const RE::BGSKeywordForm* kf = nullptr;
		const RE::BGSKeywordForm* alt = nullptr;  // NPC's race; nullptr elsewhere
		const RE::TESNPC*         npc = nullptr;
		const RE::TESObjectWEAP*  weapon = nullptr;
		const RE::TESObjectARMO*  armor = nullptr;
		bool                      isPlayer = false;
	};

	// Per-pair memo for the three expensive derived strings. Owned by the
	// caller and reused across the whole form loop so it allocates once.
	struct MatchScratch
	{
		std::string editorId;
		std::string fullName;
		std::string maleModel;
		bool        haveEditorId = false;
		bool        haveFullName = false;
		bool        haveMaleModel = false;

		void reset()
		{
			haveEditorId = false;
			haveFullName = false;
			haveMaleModel = false;
		}
	};

	bool runProgram(const MatchProgram& a_prog, const MatchTarget& a_t, MatchScratch& a_scratch);
	bool matchesModName(const MatchProgram& a_p, const RE::TESForm* a_form);

	// --- program construction ---------------------------------------------
	// Appends `a_values` to the pool and returns the predicate describing them.
	Predicate makePredicate(MatchProgram& a_prog, Op a_op, Mode a_mode,
		const std::vector<std::uint64_t>& a_values, bool a_skipIfPlayer = false);
	// Closes a disjunct over the predicates appended since a_firstPred.
	void pushDisjunct(MatchProgram& a_prog, std::vector<Disjunct>& a_stage, std::size_t a_firstPred);

	// The three operand encodings every rule compiler needs: identifiers to
	// formIDs, identifiers to form pointers, and substring needles into the
	// program's string pool.
	std::vector<std::uint64_t> idOperands(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<std::uint64_t> ptrOperands(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<std::uint64_t> needleOperands(MatchProgram& a_prog, const std::vector<std::string>& a_raw,
		bool a_normalizePath = false);
	// Append a predicate to the open disjunct / to the negative stage.
	void addPred(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer = false);
	void addNegative(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer = false);
	// Stable-sorts a positive stage cheapest-first.
	void sortCheapestFirst(std::vector<Disjunct>& a_stage);

	// --- Phase 3 signal derivation, now read off the opcodes ----------------
	// Distinct tags for the enum-valued signal spaces. Tag 0 stays RE::FormID.
	inline constexpr std::uint32_t kTagWeapAnimType = 1;
	inline constexpr std::uint32_t kTagWeapSkill = 2;
	inline constexpr std::uint32_t kTagArmorType = 3;
	inline constexpr std::uint32_t kTagBipedSlot = 4;

	// For each positive disjunct, emit a NECESSARY signal. Returns false if any
	// disjunct has no finite key set (a substring test, or the catch-all), in
	// which case the rule must scan every form. See formfilter.h's Phase 3
	// section for the soundness argument -- this is the same derivation, moved
	// from three hand-written per-type predicates onto the opcode table.
	bool programSignals(const MatchProgram& a_prog, std::vector<std::uint64_t>& a_keys);

	// =====================================================================
	// PHASE 4 -- the ACTION TABLE (the apply side).
	//
	// `patch()` is what the filter chains used to be: the same decision
	// written out over and over. Its dominant shape is a LADDER over a
	// lowercased config token --
	//
	//     for (const auto& f : line.setFlags) {
	//         if      (toLowerCase(f) == "female")    curobj->...set(kFemale);
	//         else if (toLowerCase(f) == "essential") curobj->...set(kEssential);
	//         ...                                     // 20 more rungs
	//     }
	//
	// -- and that ladder is a TABLE: a fixed map from a config token to a
	// small integer (a flag bit, an enum value, an array index, a slot
	// selector). Upstream re-derives it per PATCHED FORM, and in the worst
	// shape (WEAPON's `setSkillType`, 18 rungs) re-lowercases the token once
	// PER RUNG, so a token that lands on the last rung allocates 18 strings.
	// Two of NPC's ladders go further and rebuild a whole std::map per
	// patched form.
	//
	// An ActionRow table replaces all of that with one lowercase and one
	// linear scan over a `constexpr` array of string_views -- no allocation
	// after the single lowercase, no static-init, no map.
	//
	// EXACTNESS. Every ladder this replaces is a chain of `==` against
	// lowercase literals, so the table is an exact transcription only if
	//   * the rows keep upstream's SPELLING, including rows upstream spelled
	//     with a capital and therefore made unreachable (WEAPON's
	//     `"explodeOnly"`); a transcription may not quietly fix those;
	//   * the row ORDER is upstream's rung order, so a table that does
	//     contain duplicates resolves them the way the ladder did; and
	//   * a miss falls through to whatever upstream's trailing `else` did --
	//     nothing, or a warning. findAction returns nullptr and the call site
	//     keeps that branch.
	//
	// The lowercasing is `utility.h`'s toLowerCase, called once, so tokens
	// are folded exactly as the ladders folded them.
	// `label` exists because several ladders log a PHRASE rather than the
	// token ("changed hit behavior to no dismember or explode", and one
	// upstream typo, "changed sound level to very load"). Folding those into
	// one log statement would rewrite the log; carrying the phrase in the row
	// keeps every emitted line byte-for-byte what the ladder emitted. Rows
	// whose ladder logged the token itself leave it empty.
	struct ActionRow
	{
		std::string_view name;   // the config token, LOWERCASE unless upstream was not
		std::uint32_t    value;  // flag bit / enum value / array index / slot selector
		std::uint8_t     mode;   // per-table discriminator; 0 where a table has one mode
		std::string_view label;  // the ladder's log wording; empty where it logged `name`
	};

	// First row whose name equals a_raw folded to lower case, or nullptr.
	const ActionRow* findAction(const ActionRow* a_rows, std::size_t a_count, const std::string& a_raw);
	// Same, for a call site that ALREADY holds the folded token -- several
	// blocks need it for their log line whether the lookup hits or not, and
	// folding it twice would be slower than the ladder it replaced.
	const ActionRow* findActionLowered(const ActionRow* a_rows, std::size_t a_count, std::string_view a_lowered);

	template <std::size_t N>
	inline const ActionRow* findAction(const ActionRow (&a_rows)[N], const std::string& a_raw)
	{
		return findAction(a_rows, N, a_raw);
	}
	template <std::size_t N>
	inline const ActionRow* findActionLowered(const ActionRow (&a_rows)[N], std::string_view a_lowered)
	{
		return findActionLowered(a_rows, N, a_lowered);
	}
}

#endif
