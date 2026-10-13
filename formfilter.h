#ifndef FORMFILTER_H
#define FORMFILTER_H

#include "PCH.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace formfilter
{
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

	const std::vector<std::string>& noStrings();

	bool hasAllPlugins(const std::vector<std::string>& a_names);
	bool hasAnyPlugin(const std::vector<std::string>& a_names);

	bool matchesModName(const CommonFilterView& a_v, const RE::TESForm* a_form);

	bool matchesKeywordsAnd(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool matchesKeywordsOr(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool hasExcludedKeyword(const CommonFilterView& a_v, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);

	bool matchesEditorIdAnd(const CommonFilterView& a_v, const RE::TESForm* a_form);
	bool matchesEditorIdOr(const CommonFilterView& a_v, const RE::TESForm* a_form);
	bool hasExcludedEditorId(const CommonFilterView& a_v, const RE::TESForm* a_form);

	bool isObjectExcluded(const CommonFilterView& a_v, const RE::TESForm* a_form, RE::FormType a_type);

	struct ResolvedCommonFilters
	{
		const std::vector<std::string>* modNames = nullptr;

		std::vector<RE::BGSKeyword*> keywords;
		std::vector<RE::BGSKeyword*> keywordsOr;
		std::vector<RE::BGSKeyword*> keywordsExcluded;

		std::vector<std::string> editorIdContains;
		std::vector<std::string> editorIdContainsOr;
		std::vector<std::string> editorIdContainsExcluded;

		std::vector<RE::FormID> objectExcluded;

		bool allPluginsInstalled = true;
		bool anyPluginInstalled = true;
	};

	ResolvedCommonFilters resolve(const CommonFilterView& a_v, RE::FormType a_objectExcludedType);

	std::vector<RE::TESForm*> resolveForms(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<RE::FormID> resolveFormIDs(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<RE::FormID> resolveFormIDsKeepingHoles(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<std::string> toLowerAll(const std::vector<std::string>& a_in);

	bool matchesModName(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);

	bool matchesKeywordsAnd(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool matchesKeywordsOr(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);
	bool hasExcludedKeyword(const ResolvedCommonFilters& a_r, const RE::BGSKeywordForm* a_kf, const RE::BGSKeywordForm* a_alt);

	bool matchesEditorIdAnd(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);
	bool matchesEditorIdOr(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);
	bool hasExcludedEditorId(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);

	bool isObjectExcluded(const ResolvedCommonFilters& a_r, const RE::TESForm* a_form);

	inline constexpr std::uint64_t sigForm(RE::FormID a_id)
	{
		return static_cast<std::uint64_t>(a_id);
	}
	inline constexpr std::uint64_t sigTagged(std::uint32_t a_tag, std::uint32_t a_value)
	{
		return (static_cast<std::uint64_t>(a_tag) << 32) | static_cast<std::uint64_t>(a_value);
	}

	void appendMaskBits(std::vector<std::uint64_t>& a_out, std::uint32_t a_tag, std::uint32_t a_mask);

	struct IndexStats
	{
		std::uint64_t rulesKeyed = 0;
		std::uint64_t rulesScanned = 0;
		std::uint64_t rulesStale = 0;
		std::uint64_t candidateVisits = 0;
		std::uint64_t scanVisits = 0;
		std::uint64_t indexBuilds = 0;
	};
	IndexStats& stats();

	class FormSignalIndex
	{
	public:
		void add(std::uint64_t a_key, std::uint32_t a_formIdx);

		void gather(const std::vector<std::uint64_t>& a_keys, std::vector<std::uint32_t>& a_out) const;

		std::size_t signals() const { return m_buckets.size(); }

	private:
		std::unordered_map<std::uint64_t, std::vector<std::uint32_t>> m_buckets;
	};

	void addKeywordSignals(FormSignalIndex& a_index, const RE::BGSKeywordForm* a_kf, std::uint32_t a_formIdx);

	enum class Op : std::uint8_t
	{
		Keyword,
		EditorId,
		NameContains,
		ModelContains,
		Race,
		NpcClass,
		Outfit,
		CombatStyle,
		VoiceType,
		Faction,
		ObjectId,
		NpcFlag,
		NpcTemplateFlag,
		NpcTemplate,
		NpcInheritsTemplateFlag,
		NpcSkillRange,
		WeapAnimType,
		WeapSkill,
		WeapModel1st,
		WeapBound,
		ArmorType,
		BipedSlot,
		ArmorAddon,
	};

	enum class Mode : std::uint8_t
	{
		And,
		Or,
		Exclude,
	};

	struct Predicate
	{
		std::uint8_t  op;
		std::uint8_t  mode;
		std::uint8_t  skipIfPlayer;
		std::uint8_t  pad = 0;
		std::uint32_t operands;
		std::uint16_t count;
	};

	struct Disjunct
	{
		std::uint16_t first;
		std::uint16_t count;
		std::uint32_t cost;
	};

	struct MatchProgram
	{
		std::vector<Predicate>     preds;
		std::vector<Disjunct>      positive;
		std::vector<Predicate>     assigns;
		std::vector<Disjunct>      positive2;
		std::vector<Predicate>     negative;
		std::vector<std::uint64_t> pool;
		std::vector<std::string>   strings;

		const std::vector<std::string>* modNames = nullptr;

		bool catchAll = false;
		bool skipRule = false;
		bool unkeyable = false;

		std::uint32_t mark() const { return static_cast<std::uint32_t>(pool.size()); }
	};

	struct MatchTarget
	{
		const RE::TESForm*        form = nullptr;
		const RE::BGSKeywordForm* kf = nullptr;
		const RE::BGSKeywordForm* alt = nullptr;
		const RE::TESNPC*         npc = nullptr;
		const RE::TESObjectWEAP*  weapon = nullptr;
		const RE::TESObjectARMO*  armor = nullptr;
		bool                      isPlayer = false;
	};

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

	Predicate makePredicate(MatchProgram& a_prog, Op a_op, Mode a_mode,
		const std::vector<std::uint64_t>& a_values, bool a_skipIfPlayer = false);
	void pushDisjunct(MatchProgram& a_prog, std::vector<Disjunct>& a_stage, std::size_t a_firstPred);

	std::vector<std::uint64_t> idOperands(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<std::uint64_t> ptrOperands(const std::vector<std::string>& a_ids, RE::FormType a_type);
	std::vector<std::uint64_t> needleOperands(MatchProgram& a_prog, const std::vector<std::string>& a_raw,
		bool a_normalizePath = false);
	void addPred(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer = false);
	void addNegative(MatchProgram& a_prog, Op a_op, Mode a_mode, const std::vector<std::uint64_t>& a_vals,
		bool a_skipIfPlayer = false);
	void sortCheapestFirst(std::vector<Disjunct>& a_stage);

	inline constexpr std::uint32_t kTagWeapAnimType = 1;
	inline constexpr std::uint32_t kTagWeapSkill = 2;
	inline constexpr std::uint32_t kTagArmorType = 3;
	inline constexpr std::uint32_t kTagBipedSlot = 4;

	bool programSignals(const MatchProgram& a_prog, std::vector<std::uint64_t>& a_keys);

	struct ActionRow
	{
		std::string_view name;
		std::uint32_t    value;
		std::uint8_t     mode;
		std::string_view label;
	};

	const ActionRow* findAction(const ActionRow* a_rows, std::size_t a_count, const std::string& a_raw);
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
