#include "armor.h"
#include "formfilter.h"
#include "parsetable.h"
#include "utility.h"

namespace ARMOR
{
	// =====================================================================
	// PHASE 1 -- ARMOR's own filter vocabulary, resolved once per rule.
	// Same rationale, and the same parse-time prohibition, as WEAPON's copy;
	// see formfilter.h.
	// =====================================================================
	namespace
	{
		bool lookupArmorType(const std::string& a_lower, RE::BIPED_MODEL::ArmorType& a_out)
		{
			if (a_lower == "light") {
				a_out = RE::BIPED_MODEL::ArmorType::kLightArmor;
				return true;
			}
			if (a_lower == "heavy") {
				a_out = RE::BIPED_MODEL::ArmorType::kHeavyArmor;
				return true;
			}
			if (a_lower == "clothing") {
				a_out = RE::BIPED_MODEL::ArmorType::kClothing;
				return true;
			}
			return false;
		}

		std::vector<std::uint64_t> armorTypeOperands(const std::vector<std::string>& a_in)
		{
			std::vector<std::uint64_t> out;
			out.reserve(a_in.size());
			for (const auto& s : a_in) {
				RE::BIPED_MODEL::ArmorType t{};
				if (lookupArmorType(toLowerCase(s), t)) {
					out.push_back(static_cast<std::uint32_t>(t));
				}
			}
			return out;
		}

		std::vector<std::uint64_t> slotOperands(const std::vector<std::string>& a_in)
		{
			std::vector<std::uint64_t> out;
			out.reserve(a_in.size());
			for (const auto& s : a_in) {
				// std::stoi's throw behaviour is preserved, only hoisted: it now
				// throws before the form loop rather than on the first form the
				// loop reaches. Both terminate the process; nothing catches it.
				out.push_back(static_cast<std::uint32_t>(getBipedObjectSlot(std::stoi(s))));
			}
			return out;
		}

		// ==================================================================
		// PHASE 2 -- ARMOR's rule compiler.
		//
		// The ~280 lines of filter chain that used to be inlined in
		// process_patch_instructions_armor are compiled once per rule into a
		// formfilter::MatchProgram. See formfilter.h.
		// ==================================================================
		formfilter::MatchProgram compileRule(const patch_instruction_armor& line)
		{
			using formfilter::Mode;
			using formfilter::Op;

			formfilter::MatchProgram p;
			p.modNames = &line.modNames;
			p.skipRule = (!line.hasPlugin.empty() && !formfilter::hasAllPlugins(line.hasPlugin)) ||
				(!line.hasPluginOr.empty() && !formfilter::hasAnyPlugin(line.hasPluginOr));

			std::size_t first = p.preds.size();
			if (!line.filterByEditorIdContains.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::And,
					formfilter::needleOperands(p, line.filterByEditorIdContains));
			}
			if (!line.filterByEditorIdContainsOr.empty()) {
				formfilter::addPred(p, Op::EditorId, Mode::Or,
					formfilter::needleOperands(p, line.filterByEditorIdContainsOr));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			first = p.preds.size();
			if (!line.filterByNameContains.empty()) {
				formfilter::addPred(p, Op::NameContains, Mode::And,
					formfilter::needleOperands(p, line.filterByNameContains));
			}
			if (!line.filterByNameContainsOr.empty()) {
				formfilter::addPred(p, Op::NameContains, Mode::Or,
					formfilter::needleOperands(p, line.filterByNameContainsOr));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			if (!line.filterByArmorType.empty()) {
				first = p.preds.size();
				formfilter::addPred(p, Op::ArmorType, Mode::Or, armorTypeOperands(line.filterByArmorType));
				formfilter::pushDisjunct(p, p.positive, first);
			}

			first = p.preds.size();
			if (!line.bipedSlot.empty()) {
				formfilter::addPred(p, Op::BipedSlot, Mode::And, slotOperands(line.bipedSlot));
			}
			if (!line.bipedSlotOr.empty()) {
				formfilter::addPred(p, Op::BipedSlot, Mode::Or, slotOperands(line.bipedSlotOr));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			first = p.preds.size();
			if (!line.keywords.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::And,
					formfilter::ptrOperands(line.keywords, RE::FormType::Keyword));
			}
			if (!line.keywordsOr.empty()) {
				formfilter::addPred(p, Op::Keyword, Mode::Or,
					formfilter::ptrOperands(line.keywordsOr, RE::FormType::Keyword));
			}
			formfilter::pushDisjunct(p, p.positive, first);

			first = p.preds.size();
			if (!line.filterByArmorAddons.empty()) {
				// The one AND list here that must keep a slot for an identifier
				// that did not resolve: its upstream loop resets its accumulator
				// at the TOP of every iteration, so a trailing non-resolving
				// entry drags the whole test to false. 0 is the hole marker, and
				// Mode::And on Op::ArmorAddon is the one arm that honours it.
				std::vector<std::uint64_t> holes;
				for (const RE::FormID id : formfilter::resolveFormIDsKeepingHoles(
						 line.filterByArmorAddons, RE::FormType::Armature)) {
					holes.push_back(id);
				}
				formfilter::addPred(p, Op::ArmorAddon, Mode::And, holes);
			}
			if (!line.filterByArmorAddonsOr.empty()) {
				formfilter::addPred(p, Op::ArmorAddon, Mode::Or,
					formfilter::idOperands(line.filterByArmorAddonsOr, RE::FormType::Armature));
			}
			formfilter::pushDisjunct(p, p.positive, first);
			formfilter::sortCheapestFirst(p.positive);

			p.catchAll = line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() &&
				line.bipedSlot.empty() && line.bipedSlotOr.empty() && line.filterByArmorType.empty() &&
				line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() &&
				line.filterByNameContains.empty() && line.filterByNameContainsOr.empty() &&
				line.filterByArmorAddons.empty() && line.filterByArmorAddonsOr.empty();

			// --- negative stage, in source order ---------------------------
			if (!line.keywordsExcluded.empty()) {
				formfilter::addNegative(p, Op::Keyword, Mode::Exclude,
					formfilter::ptrOperands(line.keywordsExcluded, RE::FormType::Keyword));
			}
			if (!line.bipedSlotExcluded.empty()) {
				formfilter::addNegative(p, Op::BipedSlot, Mode::Exclude, slotOperands(line.bipedSlotExcluded));
			}
			if (!line.filterByEditorIdContainsExcluded.empty()) {
				formfilter::addNegative(p, Op::EditorId, Mode::Exclude,
					formfilter::needleOperands(p, line.filterByEditorIdContainsExcluded));
			}
			if (!line.filterByNameContainsExcluded.empty()) {
				formfilter::addNegative(p, Op::NameContains, Mode::Exclude,
					formfilter::needleOperands(p, line.filterByNameContainsExcluded));
			}
			if (!line.filterByArmorAddonsExcluded.empty()) {
				formfilter::addNegative(p, Op::ArmorAddon, Mode::Exclude,
					formfilter::idOperands(line.filterByArmorAddonsExcluded, RE::FormType::Armature));
			}
			if (!line.objectExcluded.empty()) {
				formfilter::addNegative(p, Op::ObjectId, Mode::Exclude,
					formfilter::idOperands(line.objectExcluded, RE::FormType::Armor));
			}
			if (!line.filterByArmorTypeExcluded.empty()) {
				formfilter::addNegative(p, Op::ArmorType, Mode::Exclude,
					armorTypeOperands(line.filterByArmorTypeExcluded));
			}
			if (!line.restrictKeywords.empty()) {
				formfilter::addNegative(p, Op::Keyword, Mode::Or,
					formfilter::ptrOperands(line.restrictKeywords, RE::FormType::Keyword));
			}
			if (!line.restrictSlots.empty()) {
				formfilter::addNegative(p, Op::BipedSlot, Mode::Or, slotOperands(line.restrictSlots));
			}

			return p;
		}

		// patch() writes indexed attributes, so a rule applied earlier can
		// change whether a later rule matches. Every write site in
		// ARMOR::patch to the keyword array, bipedModelData.armorType,
		// bipedModelData.bipedObjectSlots or armorAddons is guarded by one of:
		//   keywordsToAdd / keywordsToRemove, mirrorArmor, templateArmor,
		//   bipedSlotsToAdd (setBipedSlot), bipedSlotsToRemove
		//   (removeBipedSlot), armorType, clearArmorAddons,
		//   armorAddonsToAdd (armorAddonToAdd), armorAddonsToRemove
		//   (armorAddonToRemove).
		// From the first such rule on, the index is stale.
		bool armorRuleInvalidatesIndex(const patch_instruction_armor& line)
		{
			// mirrorArmor / templateArmor / armorType / clearArmorAddons come
			// from extractValueString, which writes the literal "none" when the
			// key is ABSENT -- so they are never empty, and the test has to be
			// the same `!empty() && != "none"` guard their patch() sites use.
			const auto live = [](const std::string& v) { return !v.empty() && v != "none"; };
			return !line.keywordsToAdd.empty() || !line.keywordsToRemove.empty() ||
				live(line.mirrorArmor) || live(line.templateArmor) ||
				!line.setBipedSlot.empty() || !line.removeBipedSlot.empty() ||
				live(line.armorType) || live(line.clearArmorAddons) ||
				!line.armorAddonToAdd.empty() || !line.armorAddonToRemove.empty();
		}

		void buildArmorIndex(const RE::BSTArray<RE::TESObjectARMO*>& a_array, formfilter::FormSignalIndex& a_index)
		{
			const std::uint32_t n = static_cast<std::uint32_t>(a_array.size());
			std::vector<std::uint64_t> bits;
			for (std::uint32_t i = 0; i < n; ++i) {
				RE::TESObjectARMO* armor = a_array[i];
				if (!armor) {
					continue;
				}
				formfilter::addKeywordSignals(a_index, static_cast<const RE::BGSKeywordForm*>(armor), i);

				bits.clear();
				formfilter::appendMaskBits(bits, formfilter::kTagArmorType,
					static_cast<std::uint32_t>(armor->bipedModelData.armorType.underlying()));
				formfilter::appendMaskBits(bits, formfilter::kTagBipedSlot,
					static_cast<std::uint32_t>(armor->bipedModelData.bipedObjectSlots.underlying()));
				for (const std::uint64_t key : bits) {
					a_index.add(key, i);
				}

				for (const auto& addon : armor->armorAddons) {
					if (addon) {
						a_index.add(formfilter::sigForm(addon->formID), i);
					}
				}
			}
		}
	}

	// ==================================================================
	// PHASE 2 -- ARMOR's key table. One row per upstream extract*() call, in
	// the same order. See parsetable.h.
	// ==================================================================
	namespace
	{
#define ARMO_F(member) static_cast<std::uint16_t>(offsetof(patch_instruction_armor, member))

		constexpr sp::KeySpec kArmorKeys[] = {
			{ "filterByArmors", sp::FieldKind::List, ARMO_F(object) },
			{ "filterByArmorsExcluded", sp::FieldKind::List, ARMO_F(objectExcluded) },
			{ "filterByKeywords", sp::FieldKind::List, ARMO_F(keywords) },
			{ "filterByKeywordsOr", sp::FieldKind::List, ARMO_F(keywordsOr) },
			{ "filterByKeywordsExcluded", sp::FieldKind::List, ARMO_F(keywordsExcluded) },
			{ "mirrorArmor", sp::FieldKind::Value, ARMO_F(mirrorArmor) },
			{ "damageResist", sp::FieldKind::Value, ARMO_F(damageResist) },
			{ "damageResistMatch", sp::FieldKind::Value, ARMO_F(damageResistMatch) },
			{ "damageResistMultiply", sp::FieldKind::Value, ARMO_F(damageResistMult) },
			{ "health", sp::FieldKind::Value, ARMO_F(health) },
			{ "objectEffect", sp::FieldKind::Value, ARMO_F(objectEffect) },
			{ "keywordsToAdd", sp::FieldKind::List, ARMO_F(keywordsToAdd) },
			{ "keywordsToRemove", sp::FieldKind::List, ARMO_F(keywordsToRemove) },
			{ "filterByBipedSlots", sp::FieldKind::ListAssign, ARMO_F(bipedSlot) },
			{ "filterByBipedSlotsOr", sp::FieldKind::ListAssign, ARMO_F(bipedSlotOr) },
			{ "filterByBipedSlotsExcluded", sp::FieldKind::ListAssign, ARMO_F(bipedSlotExcluded) },
			{ "bipedSlotsToAdd", sp::FieldKind::ListAssign, ARMO_F(setBipedSlot) },
			{ "bipedSlotsToRemove", sp::FieldKind::ListAssign, ARMO_F(removeBipedSlot) },
			{ "fullName", sp::FieldKind::ValueTilde, ARMO_F(fullName) },
			{ "modelFemale", sp::FieldKind::Value, ARMO_F(modelFemale) },
			{ "modelMale", sp::FieldKind::Value, ARMO_F(modelMale) },
			{ "value", sp::FieldKind::Value, ARMO_F(value) },
			{ "valueMult", sp::FieldKind::Value, ARMO_F(valueMult) },
			{ "weight", sp::FieldKind::Value, ARMO_F(weight) },
			{ "weightMult", sp::FieldKind::Value, ARMO_F(weightMult) },
			{ "armorType", sp::FieldKind::Value, ARMO_F(armorType) },
			{ "pickUpSound", sp::FieldKind::Value, ARMO_F(pickUpSound) },
			{ "putDownSound", sp::FieldKind::Value, ARMO_F(putDownSound) },
			{ "filterByModNames", sp::FieldKind::ListAssign, ARMO_F(modNames) },
			{ "filterByArmorTypes", sp::FieldKind::ListAssign, ARMO_F(filterByArmorType) },
			{ "filterByArmorTypesExcluded", sp::FieldKind::ListAssign, ARMO_F(filterByArmorTypeExcluded) },
			{ "altBlockMaterialType", sp::FieldKind::Value, ARMO_F(altBlockMaterialType) },
			{ "blockBashImpactDataSet", sp::FieldKind::Value, ARMO_F(blockBashImpactDataSet) },
			{ "equipSlot", sp::FieldKind::Value, ARMO_F(equipSlot) },
			{ "enchantAmount", sp::FieldKind::Value, ARMO_F(enchantAmount) },
			{ "filterByNameContains", sp::FieldKind::ListAssign, ARMO_F(filterByNameContains) },
			{ "filterByNameContainsOr", sp::FieldKind::ListAssign, ARMO_F(filterByNameContainsOr) },
			{ "filterByNameContainsExcluded", sp::FieldKind::ListAssign, ARMO_F(filterByNameContainsExcluded) },
			{ "filterByEditorIdContains", sp::FieldKind::ListAssign, ARMO_F(filterByEditorIdContains) },
			{ "filterByEditorIdContainsOr", sp::FieldKind::ListAssign, ARMO_F(filterByEditorIdContainsOr) },
			{ "filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign, ARMO_F(filterByEditorIdContainsExcluded) },
			{ "reCalcArmorRating", sp::FieldKind::Arr2D, ARMO_F(reCalcArmorRating) },
			{ "reCalcArmorRatingv2", sp::FieldKind::Arr2D, ARMO_F(reCalcArmorRatingv2) },
			{ "filterByArmorAddons", sp::FieldKind::ListAssign, ARMO_F(filterByArmorAddons) },
			{ "filterByArmorAddonsOr", sp::FieldKind::ListAssign, ARMO_F(filterByArmorAddonsOr) },
			{ "filterByArmorAddonsExcluded", sp::FieldKind::ListAssign, ARMO_F(filterByArmorAddonsExcluded) },
			{ "armorAddonsToAdd", sp::FieldKind::List, ARMO_F(armorAddonToAdd) },
			{ "armorAddonsToRemove", sp::FieldKind::List, ARMO_F(armorAddonToRemove) },
			{ "clearArmorAddons", sp::FieldKind::Value, ARMO_F(clearArmorAddons) },
			{ "templateArmor", sp::FieldKind::Value, ARMO_F(templateArmor) },
			{ "minX", sp::FieldKind::Value, ARMO_F(minX) },
			{ "minY", sp::FieldKind::Value, ARMO_F(minY) },
			{ "minZ", sp::FieldKind::Value, ARMO_F(minZ) },
			{ "maxX", sp::FieldKind::Value, ARMO_F(maxX) },
			{ "maxY", sp::FieldKind::Value, ARMO_F(maxY) },
			{ "maxZ", sp::FieldKind::Value, ARMO_F(maxZ) },
			{ "dwMatch", sp::FieldKind::Value, ARMO_F(dwMatch) },
			{ "modelMatch", sp::FieldKind::Value, ARMO_F(modelMatch) },
			{ "hasPlugins", sp::FieldKind::ListAssign, ARMO_F(hasPlugin) },
			{ "hasPluginsOr", sp::FieldKind::ListAssign, ARMO_F(hasPluginOr) },
			{ "armaRaceToAdd", sp::FieldKind::Arr2D, ARMO_F(armaRaceToAdd) },
			{ "setFlags", sp::FieldKind::ListAssign, ARMO_F(setFlags) },
			{ "removeFlags", sp::FieldKind::ListAssign, ARMO_F(removeFlags) },
			{ "restrictToKeywords", sp::FieldKind::List, ARMO_F(restrictKeywords) },
			{ "restrictToBipedSlots", sp::FieldKind::List, ARMO_F(restrictSlots) },
		};

#undef ARMO_F

		const sp::KeyTable& armorKeyTable()
		{
			static const sp::KeyTable table(kArmorKeys, std::size(kArmorKeys));
			return table;
		}
	}

	struct patch_instruction_armor create_patch_instruction_armor(const std::string& line)
	{
		patch_instruction_armor l;
<<<<<<< HEAD
		armorKeyTable().parse(line, &l);
=======

		extractData(line, "filterByArmors\\s*=([^:]+)", l.object);

		extractData(line, "filterByArmorsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractData(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractData(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractData(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractValueString(line, "mirrorArmor\\s*=([^:]+)", l.mirrorArmor);
		extractValueString(line, "damageResist\\s*=([^:]+)", l.damageResist);
		extractValueString(line, "damageResistMatch\\s*=([^:]+)", l.damageResistMatch);

		extractValueString(line, "damageResistMultiply\\s*=([^:]+)", l.damageResistMult);

		extractValueString(line, "health\\s*=([^:]+)", l.health);

		extractValueString(line, "objectEffect\\s*=([^:]+)", l.objectEffect);

		extractData(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractData(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractDataStrings(line, "filterByBipedSlots\\s*=([^:]+)", l.bipedSlot);

		extractDataStrings(line, "filterByBipedSlotsOr\\s*=([^:]+)", l.bipedSlotOr);

		extractDataStrings(line, "filterByBipedSlotsExcluded\\s*=([^:]+)", l.bipedSlotExcluded);

		extractDataStrings(line, "bipedSlotsToAdd\\s*=([^:]+)", l.setBipedSlot);

		extractDataStrings(line, "bipedSlotsToRemove\\s*=([^:]+)", l.removeBipedSlot);

		extractValueString(line, "fullName\\s*=\\s*~([^~]+)~", l.fullName);

		extractValueString(line, "modelFemale\\s*=([^:]+)", l.modelFemale);
		extractValueString(line, "modelMale\\s*=([^:]+)", l.modelMale);
		extractValueString(line, "value\\s*=([^:]+)", l.value);
		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);
		extractValueString(line, "armorType\\s*=([^:]+)", l.armorType);
		

		extractValueString(line, "pickUpSound\\s*=([^:]+)", l.pickUpSound);
		extractValueString(line, "putDownSound\\s*=([^:]+)", l.putDownSound);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);
		extractDataStrings(line, "filterByArmorTypes\\s*=([^:]+)", l.filterByArmorType);
		extractDataStrings(line, "filterByArmorTypesExcluded\\s*=([^:]+)", l.filterByArmorTypeExcluded);
		extractValueString(line, "altBlockMaterialType\\s*=([^:]+)", l.altBlockMaterialType);
		extractValueString(line, "blockBashImpactDataSet\\s*=([^:]+)", l.blockBashImpactDataSet);
		extractValueString(line, "equipSlot\\s*=([^:]+)", l.equipSlot);
		extractValueString(line, "enchantAmount\\s*=([^:]+)", l.enchantAmount);

		extractDataStrings(line, "filterByNameContains\\s*=([^:]+)", l.filterByNameContains);
		extractDataStrings(line, "filterByNameContainsOr\\s*=([^:]+)", l.filterByNameContainsOr);
		extractDataStrings(line, "filterByNameContainsExcluded\\s*=([^:]+)", l.filterByNameContainsExcluded);
		extractDataStrings(line, "filterByEditorIdContains\\s*=([^:]+)", l.filterByEditorIdContains);
		extractDataStrings(line, "filterByEditorIdContainsOr\\s*=([^:]+)", l.filterByEditorIdContainsOr);
		extractDataStrings(line, "filterByEditorIdContainsExcluded\\s*=([^:]+)", l.filterByEditorIdContainsExcluded);
		extractToArr2D(line, "reCalcArmorRating\\s*=([^:]+)", l.reCalcArmorRating);
		extractToArr2D(line, "reCalcArmorRatingv2\\s*=([^:]+)", l.reCalcArmorRatingv2);
		
		
		extractDataStrings(line, "filterByArmorAddons\\s*=([^:]+)", l.filterByArmorAddons);
		extractDataStrings(line, "filterByArmorAddonsOr\\s*=([^:]+)", l.filterByArmorAddonsOr);
		extractDataStrings(line, "filterByArmorAddonsExcluded\\s*=([^:]+)", l.filterByArmorAddonsExcluded);
		
		extractData(line, "armorAddonsToAdd\\s*=([^:]+)", l.armorAddonToAdd);
		extractData(line, "armorAddonsToRemove\\s*=([^:]+)", l.armorAddonToRemove);
		extractValueString(line, "clearArmorAddons\\s*=([^:]+)", l.clearArmorAddons);
		
		extractValueString(line, "templateArmor\\s*=([^:]+)", l.templateArmor);
		
		extractValueString(line, "minX\\s*=([^:]+)", l.minX);
		extractValueString(line, "minY\\s*=([^:]+)", l.minY);
		extractValueString(line, "minZ\\s*=([^:]+)", l.minZ);
		extractValueString(line, "maxX\\s*=([^:]+)", l.maxX);
		extractValueString(line, "maxY\\s*=([^:]+)", l.maxY);
		extractValueString(line, "maxZ\\s*=([^:]+)", l.maxZ);

		extractValueString(line, "valueMatch\\s*=([^:]+)", l.valueMatch);
		extractValueString(line, "dwMatch\\s*=([^:]+)", l.dwMatch);
		extractValueString(line, "modelMatch\\s*=([^:]+)", l.modelMatch);

		extractDataStrings(line, "hasPlugins\\s*=([^:]+)", l.hasPlugin);
		extractDataStrings(line, "hasPluginsOr\\s*=([^:]+)", l.hasPluginOr);

		extractToArr2D(line, "armaRaceToAdd\\s*=([^:]+)", l.armaRaceToAdd);

		extractDataStrings(line, "setFlags\\s*=([^:]+)", l.setFlags);
		extractDataStrings(line, "removeFlags\\s*=([^:]+)", l.removeFlags);

		extractData(line, "restrictToKeywords\\s*=([^:]+)", l.restrictKeywords);
		extractData(line, "restrictToBipedSlots\\s*=([^:]+)", l.restrictSlots);

>>>>>>> origin/new-main
		return l;
	}


	void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto                       dataHandler = RE::TESDataHandler::GetSingleton();
<<<<<<< HEAD
		auto& ArmorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();

		// PHASE 3: built on first use.
		formfilter::FormSignalIndex index;
		bool                        indexBuilt = false;
		bool                        indexStale = false;
		std::vector<std::uint64_t>  keys;
		std::vector<std::uint32_t>  candidates;

=======
		const auto& ArmorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
>>>>>>> origin/new-main
		for (const auto& line : tokens) {
			indexStale = indexStale || armorRuleInvalidatesIndex(line);

			if (!line.object.empty()) {
				for (const auto& npcstring : line.object) {
					RE::TESForm*       currentform = nullptr;
					RE::TESObjectARMO* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::FormType::Armor) {
						//logger::debug("Form {:08X} ", currentform->formID);
						curobj = (RE::TESObjectARMO*)currentform;
						patch(line, curobj);
					}
				}
			}

			if (!line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.bipedSlot.empty() && line.bipedSlotOr.empty() && line.filterByArmorType.empty() && line.filterByEditorIdContains.empty() && line.filterByEditorIdContainsOr.empty() && line.filterByNameContains.empty() && line.filterByNameContainsOr.empty() && line.filterByArmorAddons.empty() && line.filterByArmorAddonsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			// PHASE 1 + 2: the rule is compiled ONCE into a flat predicate
			// program; the form loop below is a walk over it.
			const formfilter::MatchProgram program = compileRule(line);
			if (program.skipRule) {
				continue;
			}

			// PHASE 3: restrict the scan to the armors carrying at least one of
			// the signals this rule names, read off the program's opcodes.
			const bool keyed = !indexStale && formfilter::programSignals(program, keys);
			if (keyed) {
				if (!indexBuilt) {
					buildArmorIndex(ArmorArray, index);
					indexBuilt = true;
					++formfilter::stats().indexBuilds;
				}
				index.gather(keys, candidates);
			}
			const std::uint32_t scanCount =
				static_cast<std::uint32_t>(keyed ? candidates.size() : ArmorArray.size());
			// Diagnostics only; see formfilter::IndexStats.
			if (keyed) {
				++formfilter::stats().rulesKeyed;
				formfilter::stats().candidateVisits += scanCount;
			} else {
				if (indexStale) {
					++formfilter::stats().rulesStale;
				}
				++formfilter::stats().rulesScanned;
				formfilter::stats().scanVisits += scanCount;
			}

			// Reused across the whole form loop: the lowered fullName and the
			// lowered EditorID now allocate once, not once per pair.
			formfilter::MatchScratch scratch;
			formfilter::MatchTarget  target;
			for (std::uint32_t scanIdx = 0; scanIdx < scanCount; ++scanIdx) {
				const auto& curobj = ArmorArray[keyed ? candidates[scanIdx] : scanIdx];
				if (!curobj || curobj->IsDeleted()) {
					continue;
				}

				if (!line.modNames.empty() && !formfilter::matchesModName(program, curobj)) {
					continue;
				}

				target.form = curobj;
				target.kf = static_cast<const RE::BGSKeywordForm*>(curobj);
				target.armor = curobj;
				if (formfilter::runProgram(program, target, scratch)) {
					patch(line, curobj);
				}
			}
		}
	}

	void* readConfig(const std::string& folder)
	{
		char        skipChar = ';';
		std::string extension = ".ini";

		DIR*                   dir;
		struct dirent*         ent;
		std::list<std::string> directories{ folder };
		std::string            currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string  fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						} else {
							std::string fileName = ent->d_name;
							size_t      pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();

								if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr)) {
									if (!IsPluginInstalled(modname)) {
										logger::info("************************************************************");
										logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), modname, fullPath);
										logger::info("************************************************************");
										continue;
									}
								}
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								std::string                        line;
								std::ifstream                      infile;
								std::list<patch_instruction_armor> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									if (line.empty()) {
										continue;
									}

									tokens.push_back(create_patch_instruction_armor(line));
								}
								infile.close();
								process_patch_instructions_armor(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
	}

<<<<<<< HEAD
	// =====================================================================
	// PHASE 4 -- ARMOR's action tables. See formfilter.h "PHASE 4".
	//
	// Three ladders, each re-lowercasing the token once per rung. The
	// `armorType` one did it three times to reject a token; setFlags and
	// removeFlags were the same four rungs written twice.
	// =====================================================================
	namespace
	{
		using formfilter::ActionRow;

		// The log line spells these with capitals ("LightArmor"), the config
		// token does not; `label` carries the log spelling.
		constexpr ActionRow kArmorTypeRows[] = {
			{ "lightarmor", static_cast<std::uint32_t>(RE::BIPED_MODEL::ArmorType::kLightArmor), 0, "LightArmor" },
			{ "heavyarmor", static_cast<std::uint32_t>(RE::BIPED_MODEL::ArmorType::kHeavyArmor), 0, "HeavyArmor" },
			{ "clothing", static_cast<std::uint32_t>(RE::BIPED_MODEL::ArmorType::kClothing), 0, "Clothing" },
		};

		// RecordFlags is a raw bit mask OR'd into formFlags, not an enum set.
		constexpr ActionRow kArmorFlagRows[] = {
			{ "nonplayable", static_cast<std::uint32_t>(RE::TESObjectARMO::RecordFlags::kNonPlayable) },
			{ "deleted", static_cast<std::uint32_t>(RE::TESObjectARMO::RecordFlags::kDeleted) },
			{ "shield", static_cast<std::uint32_t>(RE::TESObjectARMO::RecordFlags::kShield) },
			{ "ignored", static_cast<std::uint32_t>(RE::TESObjectARMO::RecordFlags::kIgnored) },
		};
	}

	void* patch(const ARMOR::patch_instruction_armor& line, RE::TESObjectARMO* curobj)
=======
	void* patch(const ARMOR::patch_instruction_armor& line, RE::TESObjectARMO* curobj)
>>>>>>> origin/new-main
	{

		if (!line.mirrorArmor.empty() && line.mirrorArmor != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.mirrorArmor;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					if(templateArmor){
						curobj->boundData = templateArmor->boundData;
						curobj->bipedModelData = templateArmor->bipedModelData;
						curobj->armorAddons = templateArmor->armorAddons;
						curobj->constraintTemplate = templateArmor->constraintTemplate;
						curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
						curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
						curobj->ReplaceModel();
						curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
						curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
						curobj->race = templateArmor->race;
						curobj->pickupSound = templateArmor->pickupSound;
						curobj->putdownSound = templateArmor->putdownSound;
						curobj->equipSlot = templateArmor->equipSlot;
						curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
						curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
						logger::debug(FMT_STRING("armor formid: {:08X} {} mirrored armor from {:08X}"), curobj->formID, curobj->fullName, templateArmor->formID);
					}
					else {
						logger::debug(FMT_STRING("armor formid: {:08X} {} could not mirror from {}"), curobj->formID, curobj->fullName, string_form);
					}
				}
			}
			catch (const std::exception& e) {
			}
		}

		if (!line.reCalcArmorRating.empty()) {
			for (const auto& objectToAdd : line.reCalcArmorRating) {
				float startRating = std::stof(objectToAdd[0]);
				float multiplierBase = std::stof(objectToAdd[1]);
				float multiplierSoft = std::stof(objectToAdd[2]);
				float multiplierHard = std::stof(objectToAdd[3]);
				float softCap = std::stof(objectToAdd[4]);
				float hardCap = std::stof(objectToAdd[5]);
				float gapSpreader = std::stof(objectToAdd[6]);
				float multiplier = 1;
				float armorRating = 1;
				if (curobj->armorRating > 0 && (curobj->armorRating / 100) > startRating) {
					armorRating = curobj->armorRating / 100;
					armorRating = gapSpreader + armorRating;
					if (armorRating < softCap) {
						multiplier = multiplierBase;
					} else if (armorRating >= softCap && armorRating < hardCap) {
						multiplier = multiplierSoft;
					} else if (armorRating >= hardCap) {
						multiplier = multiplierHard;
					}
					uint32_t newArmor = (armorRating + (-(armorRating - startRating)) + pow(1 + armorRating - startRating, multiplier)) * 100;
					logger::debug(FMT_STRING("armor formid: {:08X} {} recalc armorRating Before {} After {} Multiplier used {}"), curobj->formID, curobj->fullName, armorRating, (newArmor / 100), multiplier);
					curobj->armorRating = newArmor;
				}
			}
		}

		if (!line.reCalcArmorRatingv2.empty()) {
			for (const auto& p : line.reCalcArmorRatingv2) {
				// v2 Parameter-Layout (7 Werte, damit dein Parser unverändert bleiben kann):
				// [0] startRating
				// [1] maxGain
				// [2] k
				// [3] minGain (optional, meist 0)
				// [4] outSoftCap (optional, 0 = aus)
				// [5] outHardCap (optional, 0 = aus)
				// [6] gapSpreader

				if (p.size() < 7) {
					continue;
				}

				float startRating = std::stof(p[0]);
				float maxGain = std::stof(p[1]);
				float k = std::stof(p[2]);
				float minGain = std::stof(p[3]);
				float outSoftCap = std::stof(p[4]);
				float outHardCap = std::stof(p[5]);
				float gapSpreader = std::stof(p[6]);

				float base = curobj->armorRating / 100.0f;
				if (base <= 0.0f || base <= startRating) {
					continue;
				}

				float ar = base + gapSpreader;

				float x = ar - startRating;
				if (x < 0.0f) {
					x = 0.0f;
				}

				// Sättigung: gain -> nähert sich maxGain an
				// gain = minGain + (maxGain - minGain) * (1 - exp(-k * x))
				float gain = minGain + (maxGain - minGain) * (1.0f - std::exp(-k * x));
				float out = startRating + gain;

				// Optionaler Output Softcap, falls du zusätzlich noch härter bremsen willst
				if (outSoftCap > 0.0f && outHardCap > outSoftCap && out > outSoftCap) {
					float over = out - outSoftCap;
					float span = outHardCap - outSoftCap;

					// nochmal Sättigung auf den Output
					out = outSoftCap + span * (1.0f - std::exp(-over / 0.8f));
				}

				uint32_t newArmor = static_cast<uint32_t>(out * 100.0f);

				logger::debug(
					FMT_STRING("armor formid: {:08X} {} recalc v2 Before {} After {} (x={}, gain={})"),
					curobj->formID, curobj->fullName, base, (newArmor / 100.0f), x, gain);

				curobj->armorRating = newArmor;
			}
		}


		if (!line.damageResist.empty() && line.damageResist != "none") {
			try {
				curobj->armorRating = stoi(line.damageResist) * 100;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating / 100);
			} catch (const std::exception& e) {
			}
		}

		if (!line.damageResistMult.empty() && line.damageResistMult != "none") {
			try {
				curobj->armorRating = (uint32_t)(curobj->armorRating * stof(line.damageResistMult));
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed(multiplied) armor rating {}"), curobj->formID, curobj->fullName, curobj->armorRating / 100);
			} catch (const std::exception& e) {
			}
		}

		if (!line.damageResistMatch.empty() && line.damageResistMatch != "none") {
			try {
				RE::TESForm*    currentform = nullptr;
				RE::TESObjectARMO* object = nullptr;
				currentform = GetFormFromIdentifier(line.damageResistMatch);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					object = (RE::TESObjectARMO*)currentform;
					if (object) {
						curobj->armorRating = object->armorRating;
						logger::debug(FMT_STRING("armor formid: {:08X} {} match armor rating of armor {:08X} {}"), curobj->formID, curobj->fullName, object->formID, object->fullName);
					}
				} else {
					logger::debug(FMT_STRING("armor formid: {:08X} {} object is not an armor {}"), curobj->formID, curobj->fullName, line.damageResistMatch);
				}
				
			} catch (const std::exception& e) {
			}
		}

		if (!line.objectEffect.empty() && line.objectEffect != "none") {
			RE::TESForm*         currentform = nullptr;
			RE::EnchantmentItem* obj = nullptr;
			std::string          string_form = line.objectEffect;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::FormType::Enchantment) {
				obj = (RE::EnchantmentItem*)currentform;
				curobj->formEnchanting = obj;
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed object effect to {:08X}"), curobj->formID, curobj->fullName, obj->formID);
			} else if (toLowerCase(line.objectEffect) == "null") {
				curobj->formEnchanting = nullptr;
				logger::debug(FMT_STRING("armor formid: {:08X} {} removed object effect"), curobj->formID, curobj->fullName);
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					if(curobj->AddKeyword((RE::BGSKeyword*)currentform))
						logger::debug(FMT_STRING("armor formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Keyword) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("armor formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.setBipedSlot.empty()) {
			for (const auto& slotString : line.setBipedSlot) {
				int slot = std::stoi(slotString);
				curobj->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
				logger::debug(FMT_STRING("armor added bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
				for (const auto& arma : curobj->armorAddons) {
					arma->bipedModelData.bipedObjectSlots.set(getBipedObjectSlot(slot));
					logger::debug(FMT_STRING("Added bipedSlot to {:08X} Slot {}"), arma->formID, slotString);
				}
			}
		}

		if (!line.removeBipedSlot.empty()) {
			for (const auto& slotString : line.removeBipedSlot) {
				int slot = std::stoi(slotString);
				curobj->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
				logger::debug(FMT_STRING("armor removed bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
				for (const auto& arma : curobj->armorAddons) {
					arma->bipedModelData.bipedObjectSlots.reset(getBipedObjectSlot(slot));
					logger::debug(FMT_STRING("Removed bipedSlot from {:08X} Slot {}"), arma->formID, slotString);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = std::stoi(line.value);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.weightMult.empty() && line.weightMult != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMult);
				logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = static_cast<int32_t>(std::ceil(curobj->value * std::stof(line.valueMult)));
				logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
			}
		}

		if (!line.armorType.empty() && line.armorType != "none") {
			if (const auto* row = formfilter::findAction(kArmorTypeRows, line.armorType)) {
				curobj->bipedModelData.armorType = static_cast<RE::BIPED_MODEL::ArmorType>(row->value);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed armorType to {}"), curobj->formID, curobj->fullName, row->label);
			}
		}

		if (!line.pickUpSound.empty() && line.pickUpSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.pickUpSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->pickupSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed pickUpSound {:08X}"), curobj->formID, curobj->fullName, curobj->pickupSound->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.putDownSound.empty() && line.putDownSound != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.putDownSound;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::SoundRecord) {
					curobj->putdownSound = (RE::BGSSoundDescriptorForm*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed putDownSound {:08X}"), curobj->formID, curobj->fullName, curobj->putdownSound->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.modelFemale.empty() && line.modelFemale != "none") {
			try {
				curobj->worldModels[RE::SEXES::kFemale].SetModel(line.modelFemale.c_str());
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed female model path {}"), curobj->formID, curobj->fullName, curobj->worldModels[RE::SEXES::kFemale].model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (!line.modelMale.empty() && line.modelMale != "none") {
			try {
				curobj->worldModels[RE::SEXES::kMale].SetModel(line.modelMale.c_str());
				curobj->ReplaceModel();
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed male model path {}"), curobj->formID, curobj->fullName, curobj->worldModels[RE::SEXES::kMale].model.c_str());
			} catch (const std::exception& e) {
			}
		}

		if (!line.altBlockMaterialType.empty() && line.altBlockMaterialType != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.altBlockMaterialType;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::MaterialType) {
					curobj->altBlockMaterialType = (RE::BGSMaterialType*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed altBlockMaterialType {:08X}"), curobj->formID, curobj->fullName, curobj->altBlockMaterialType->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.blockBashImpactDataSet.empty() && line.blockBashImpactDataSet != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.blockBashImpactDataSet;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::ImpactDataSet) {
					curobj->blockBashImpactDataSet = (RE::BGSImpactDataSet*)currentform;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed blockBashImpactDataSet {:08X}"), curobj->formID, curobj->fullName, curobj->blockBashImpactDataSet->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.equipSlot.empty() && line.equipSlot != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.equipSlot;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::EquipSlot) {
					curobj->SetEquipSlot((RE::BGSEquipSlot*)currentform);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed equipSlot {:08X}"), curobj->formID, curobj->fullName, curobj->equipSlot->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.enchantAmount.empty() && line.enchantAmount != "none") {
			try {
				curobj->amountofEnchantment = std::stoi(line.enchantAmount);
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed enchantAmount {}"), curobj->formID, curobj->fullName, curobj->amountofEnchantment);
			} catch (const std::exception& e) {
			}
		}



		if (!line.clearArmorAddons.empty() && line.clearArmorAddons != "none") {
			curobj->armorAddons.clear();
			logger::debug(FMT_STRING("armor formid: {:08X} {} cleared all armor addons"), curobj->formID, curobj->fullName);
		}

		if (!line.armorAddonToRemove.empty()) {
			for (const auto& objectToRemove : line.armorAddonToRemove) {
				std::string        removeFormStr = objectToRemove;
				RE::TESObjectARMA* removeForm = (RE::TESObjectARMA*)GetFormFromIdentifier(removeFormStr);
				if (removeForm) {
					auto it = curobj->armorAddons.begin();
					while (it != curobj->armorAddons.end()) {
						//logger::debug(FMT_STRING("spell {:08X} {}, effect {:08X} cost {} {}"), curobj->formID, curobj->fullName, (*it)->baseEffect->formID, (*it)->cost, curobj->CalculateMagickaCost(RE::PlayerCharacter::GetSingleton()));

						if ((*it) && (*it)->formID == removeForm->formID) {
							it = curobj->armorAddons.erase(it);
							logger::debug(FMT_STRING("armor {:08X} {}, {:08X} armor addon removed, armor addons left {}"), curobj->formID, curobj->fullName, removeForm->formID, curobj->armorAddons.size());
						} else {
							++it;
						}
					}
				}
			}
		}

		if (!line.armorAddonToAdd.empty()) {
			for (size_t i = 0; i < line.armorAddonToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.armorAddonToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armature) {
					curobj->armorAddons.push_back((RE::TESObjectARMA*)currentform);
					logger::debug(FMT_STRING("armor formid: {:08X} added armor addon {:08X} "), curobj->formID, (currentform)->formID);
				}
			}
		}

		if (!line.templateArmor.empty() && line.templateArmor != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.templateArmor;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->templateArmor = templateArmor;
					curobj->boundData = templateArmor->boundData;
					curobj->bipedModelData = templateArmor->bipedModelData;
					curobj->armorAddons = templateArmor->armorAddons;
					curobj->constraintTemplate = templateArmor->constraintTemplate;
					curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
					curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
					curobj->ReplaceModel();
					curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
					curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
					curobj->race = templateArmor->race;
					curobj->pickupSound = templateArmor->pickupSound;
					curobj->putdownSound = templateArmor->putdownSound;
					curobj->equipSlot = templateArmor->equipSlot;
					curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
					curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
					curobj->keywords = templateArmor->keywords;
					curobj->numKeywords = templateArmor->numKeywords;
					curobj->armorRating = templateArmor->armorRating;
					curobj->weight = templateArmor->weight;
					curobj->descriptionText = templateArmor->descriptionText;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed templateArmor {:08X}"), curobj->formID, curobj->fullName, curobj->templateArmor->formID);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.dwMatch.empty() && line.dwMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.dwMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->armorRating = templateArmor->armorRating;
					curobj->weight = templateArmor->weight;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed damage resist and weight to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.valueMatch.empty() && line.valueMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.valueMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->value = templateArmor->value;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed value to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::exception& e) {
			}
		}

		if (!line.modelMatch.empty() && line.modelMatch != "none") {
			try {
				RE::TESForm* currentform = nullptr;
				std::string  string_form = line.modelMatch;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::FormType::Armor) {
					auto templateArmor = (RE::TESObjectARMO*)currentform;
					curobj->boundData = templateArmor->boundData;
					//curobj->bipedModelData = templateArmor->bipedModelData;
					curobj->bipedModelData.bipedObjectSlots = templateArmor->bipedModelData.bipedObjectSlots;
					curobj->armorAddons = templateArmor->armorAddons;
					//curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
					//curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
					if (templateArmor->worldModels[RE::SEXES::kMale].GetModel()) {
						//curobj->worldModels[RE::SEXES::kMale] = templateArmor->worldModels[RE::SEXES::kMale];
						curobj->worldModels[RE::SEXES::kMale].SetModel(templateArmor->worldModels[RE::SEXES::kMale].GetModel());
					} else {
						curobj->worldModels[RE::SEXES::kMale].ClearDataComponent();
					}
					if (templateArmor->worldModels[RE::SEXES::kFemale].GetModel()) {
						//curobj->worldModels[RE::SEXES::kFemale] = templateArmor->worldModels[RE::SEXES::kFemale];
						curobj->worldModels[RE::SEXES::kFemale].SetModel(templateArmor->worldModels[RE::SEXES::kFemale].GetModel());
					} else {
						curobj->worldModels[RE::SEXES::kFemale].ClearDataComponent();
					}
					//curobj->worldModels[RE::SEXES::kTotal] = templateArmor->worldModels[RE::SEXES::kTotal];
					curobj->ReplaceModel();
					curobj->inventoryIcons[RE::SEXES::kFemale] = templateArmor->inventoryIcons[RE::SEXES::kFemale];
					curobj->inventoryIcons[RE::SEXES::kMale] = templateArmor->inventoryIcons[RE::SEXES::kMale];
					curobj->race = templateArmor->race;
					curobj->pickupSound = templateArmor->pickupSound;
					curobj->putdownSound = templateArmor->putdownSound;
					curobj->equipSlot = templateArmor->equipSlot;
					curobj->blockBashImpactDataSet = templateArmor->blockBashImpactDataSet;
					curobj->altBlockMaterialType = templateArmor->altBlockMaterialType;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed model to {:08X} {}"), curobj->formID, curobj->fullName, templateArmor->formID, templateArmor->fullName);
				}
			} catch (const std::exception& e) {
			}
		}

		 if (!line.minX.empty() && line.minX != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minX);
				curobj->boundData.boundMin.x = std::stoi(line.minX);
			} catch (const std::exception& e) {
			}
		}

		if (!line.minY.empty() && line.minY != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minY);
				curobj->boundData.boundMin.y = std::stoi(line.minY);
			} catch (const std::exception& e) {
			}
		}

		if (!line.minZ.empty() && line.minZ != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed minZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.minZ);
				curobj->boundData.boundMin.z = std::stoi(line.minZ);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxX.empty() && line.maxX != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxX to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxX);
				curobj->boundData.boundMax.x = std::stoi(line.maxX);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxY.empty() && line.maxY != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxY to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxY);
				curobj->boundData.boundMax.y = std::stoi(line.maxY);
			} catch (const std::exception& e) {
			}
		}

		if (!line.maxZ.empty() && line.maxZ != "none") {
			try {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed maxZ to {}"), curobj->formID, clib_util::editorID::get_editorID(curobj), line.maxZ);
				curobj->boundData.boundMax.z = std::stoi(line.maxZ);
			} catch (const std::exception& e) {
			}
		}

		if (!line.armaRaceToAdd.empty()) {
			for (const auto& objectToAdd : line.armaRaceToAdd) {
				if (objectToAdd.empty()) {
					logger::warn("armor formid: {:08X}: ignoring empty armaRaceToAdd entry", curobj->formID);
					continue;
				}
				std::string  armaToAddTo = objectToAdd[0];  // Armature identifier
				RE::TESForm* currentform = GetFormFromIdentifier(armaToAddTo);

				if (currentform && currentform->formType == RE::FormType::Armature) {
					// Find the correct armorAddon by matching formID
					auto it = std::find_if(curobj->armorAddons.begin(), curobj->armorAddons.end(),
						[&](const RE::TESObjectARMA* addon) {
							return addon && addon->formID == currentform->formID;
						});

					if (it != curobj->armorAddons.end()) {
						// Dereference the iterator to access the found armorAddon
						RE::TESObjectARMA* armorAddon = *it;

						// Loop through all races starting from index 1
						for (size_t i = 1; i < objectToAdd.size(); ++i) {
							std::string  raceToAddStr = objectToAdd[i];
							RE::TESForm* raceToAdd = GetFormFromIdentifier(raceToAddStr);

							if (raceToAdd && raceToAdd->As<RE::TESRace>()) {
								// Check if the race already exists in the armorAddon->races
								bool raceExists = std::any_of(armorAddon->additionalRaces.begin(), armorAddon->additionalRaces.end(),
									[&](RE::TESForm* existingRace) {
								return existingRace && existingRace->formID == raceToAdd->formID;
									});

								if (!raceExists) {
									// Add the race if it doesn't already exist
									armorAddon->additionalRaces.push_back(raceToAdd->As<RE::TESRace>());

									logger::debug(FMT_STRING("armor formid: {:08X} {} Added race with formID: {:08X} to armorAddon with formID: {:08X}"),
										curobj->formID, curobj->fullName, raceToAdd->formID, armorAddon->formID);
								} else {
									logger::debug(FMT_STRING("armor formid: {:08X} {} Race with formID: {:08X} already exists in armorAddon with formID: {:08X}"),
										curobj->formID, curobj->fullName, raceToAdd->formID, armorAddon->formID);
								}
							}
						}
					} else {
						logger::warn(FMT_STRING("armor formid: {:08X} {} No matching armorAddon found for formID: {:08X}"), curobj->formID, curobj->fullName, currentform->formID);
					}
				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} Armature form not found for identifier: {}"), curobj->formID, curobj->fullName, armaToAddTo);
				}
			}
		}

		if (!line.setFlags.empty()) {
			bool changedFlags = false;

			for (const auto& setFlagRaw : line.setFlags) {
				if (const auto* row = formfilter::findAction(kArmorFlagRows, setFlagRaw)) {
					curobj->formFlags |= row->value;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag enabled {}"), curobj->formID, curobj->fullName, row->name);
					changedFlags = true;

				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} unknown flag '{}' in setFlags"), curobj->formID, curobj->fullName, setFlagRaw);
				}
			}

			// Wenn mindestens ein Flag gesetzt wurde, Änderung an den Flags markieren,
			// damit es ins Save übernommen wird.
			if (changedFlags) {
				curobj->AddChange(RE::TESForm::ChangeFlags::kFlags);
			}
		}


		if (!line.removeFlags.empty()) {
			bool changedFlags = false;

			for (const auto& removeFlagRaw : line.removeFlags) {
				if (const auto* row = formfilter::findAction(kArmorFlagRows, removeFlagRaw)) {
					curobj->formFlags &= ~row->value;
					logger::debug(FMT_STRING("armor formid: {:08X} {} flag disabled {}"), curobj->formID, curobj->fullName, row->name);
					changedFlags = true;

				} else {
					logger::warn(FMT_STRING("armor formid: {:08X} {} unknown flag '{}' in removeFlags"),
						curobj->formID, curobj->fullName, removeFlagRaw);
				}
			}

			if (changedFlags) {
				curobj->AddChange(RE::TESForm::ChangeFlags::kFlags);
			}
		}



		

	}

}
