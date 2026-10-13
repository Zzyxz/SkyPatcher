#include "voicesafety.h"

#include <unordered_map>
#include <unordered_set>

namespace VoiceSafety
{
	namespace
	{
		using FunctionID = RE::FUNCTION_DATA::FunctionID;
		using OpCode = RE::CONDITION_ITEM_DATA::OpCode;

		struct QuestRole
		{
			std::vector<const RE::BGSListForm*> voiceLists;
			std::unordered_set<RE::FormID>      factions;
			std::unordered_set<RE::FormID>      npcs;
			std::unordered_set<RE::FormID>      classes;
		};

		std::unordered_set<RE::FormID> addressed;
		std::vector<QuestRole>         roles;
		bool                           built = false;

		bool OnSpeaker(const RE::CONDITION_ITEM_DATA& a_data)
		{
			return a_data.object == RE::CONDITIONITEMOBJECT::kSelf && !a_data.flags.swapTarget;
		}

		bool Compare(OpCode a_op, float a_lhs, float a_rhs)
		{
			switch (a_op) {
			case OpCode::kEqualTo:
				return a_lhs == a_rhs;
			case OpCode::kNotEqualTo:
				return a_lhs != a_rhs;
			case OpCode::kGreaterThan:
				return a_lhs > a_rhs;
			case OpCode::kGreaterThanOrEqualTo:
				return a_lhs >= a_rhs;
			case OpCode::kLessThan:
				return a_lhs < a_rhs;
			case OpCode::kLessThanOrEqualTo:
				return a_lhs <= a_rhs;
			default:
				return false;
			}
		}

		bool SelectsMatch(const RE::CONDITION_ITEM_DATA& a_data)
		{
			if (a_data.flags.global) {
				return true;
			}
			const float value = a_data.comparisonValue.f;
			const auto  op = a_data.flags.opCode;
			return Compare(op, 1.0f, value) && !Compare(op, 0.0f, value);
		}

		RE::FormID ParamFormID(const RE::CONDITION_ITEM_DATA& a_data, RE::FormType a_type)
		{
			const auto* form = static_cast<const RE::TESForm*>(a_data.functionData.params[0]);
			return form && form->GetFormType() == a_type ? form->GetFormID() : 0;
		}

		void ReadSelectors(const RE::TESCondition& a_conditions, QuestRole& a_role, bool a_collectAddressed)
		{
			for (auto* item = a_conditions.head; item; item = item->next) {
				const auto& data = item->data;
				if (!OnSpeaker(data) || !SelectsMatch(data)) {
					continue;
				}
				switch (data.functionData.function.get()) {
				case FunctionID::kGetIsID:
					if (const auto id = ParamFormID(data, RE::FormType::NPC)) {
						a_role.npcs.insert(id);
						if (a_collectAddressed) {
							addressed.insert(id);
						}
					}
					break;
				case FunctionID::kGetInFaction:
					if (const auto id = ParamFormID(data, RE::FormType::Faction)) {
						a_role.factions.insert(id);
					}
					break;
				case FunctionID::kGetIsClass:
					if (const auto id = ParamFormID(data, RE::FormType::Class)) {
						a_role.classes.insert(id);
					}
					break;
				default:
					break;
				}
			}
		}

		std::size_t ReadLineSpeakers()
		{
			std::uintptr_t table = 0;
			if (REL::Module::IsVR()) {
				return 0;
			}
			if (const auto offset = REL::IDDatabase::get().try_id2offset(REL::Module::IsAE() ? 369960 : 502417)) {
				table = REL::Module::get().base() + *offset;
			}
			if (!table) {
				return 0;
			}
			const auto capacity = *reinterpret_cast<const std::uint32_t*>(table + 0x4);
			const auto entries = *reinterpret_cast<const std::uint8_t* const*>(table + 0x20);
			if (!entries || capacity == 0 || capacity > (1u << 24)) {
				return 0;
			}
			std::size_t found = 0;
			for (std::uint32_t i = 0; i < capacity; ++i) {
				const auto* entry = entries + std::size_t{ 24 } * i;
				const auto  next = *reinterpret_cast<const std::uintptr_t*>(entry + 16);
				const auto  value = *reinterpret_cast<const std::uintptr_t*>(entry + 8);
				if (!next || value < (std::uintptr_t{ 1 } << 32)) {
					continue;
				}
				const auto* speaker = reinterpret_cast<const RE::TESForm*>(value);
				if (speaker->GetFormType() == RE::FormType::NPC) {
					addressed.insert(speaker->GetFormID());
					++found;
				}
			}
			return found;
		}

		bool ListContains(const RE::BGSListForm* a_list, const RE::BGSVoiceType* a_voice)
		{
			if (!a_list || !a_voice) {
				return false;
			}
			for (const auto* form : a_list->forms) {
				if (form == a_voice) {
					return true;
				}
			}
			if (a_list->scriptAddedTempForms) {
				for (const auto id : *a_list->scriptAddedTempForms) {
					if (id == a_voice->GetFormID()) {
						return true;
					}
				}
			}
			return false;
		}

		void IdentityRecords(RE::Actor* a_actor, std::vector<const RE::TESNPC*>& a_out)
		{
			const auto addChain = [&a_out](const RE::TESForm* a_form) {
				const auto* npc = a_form ? a_form->As<RE::TESNPC>() : nullptr;
				for (int depth = 0; npc && depth < 16; ++depth) {
					a_out.push_back(npc);
					if (!npc->actorData.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kTraits)) {
						break;
					}
					npc = npc->baseTemplateForm ? npc->baseTemplateForm->As<RE::TESNPC>() : nullptr;
				}
			};
			addChain(a_actor->data.objectReference);
			if (const auto* leveled = a_actor->extraList.GetByType<RE::ExtraLeveledCreature>()) {
				addChain(leveled->originalBase);
				addChain(leveled->templateBase);
			}
		}
	}

	void Build()
	{
		addressed.clear();
		roles.clear();
		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			return;
		}

		std::unordered_map<const RE::TESQuest*, QuestRole> perQuest;
		for (auto* form : dataHandler->GetFormArray(RE::FormType::Dialogue)) {
			const auto* topic = form ? form->As<RE::TESTopic>() : nullptr;
			if (!topic || !topic->topicInfos) {
				continue;
			}
			auto& role = perQuest[topic->ownerQuest];
			for (std::uint32_t i = 0; i < topic->numTopicInfos; ++i) {
				if (const auto* info = topic->topicInfos[i]) {
					ReadSelectors(info->objConditions, role, true);
				}
			}
		}

		std::size_t uniqueAliases = 0;
		for (auto* form : dataHandler->GetFormArray(RE::FormType::Quest)) {
			auto* quest = form ? form->As<RE::TESQuest>() : nullptr;
			if (!quest) {
				continue;
			}
			auto& role = perQuest[quest];
			ReadSelectors(quest->objConditions, role, true);
			for (auto* item = quest->objConditions.head; item; item = item->next) {
				const auto& data = item->data;
				if (data.functionData.function.get() != FunctionID::kGetIsVoiceType || !OnSpeaker(data)) {
					continue;
				}
				const auto* param = static_cast<const RE::TESForm*>(data.functionData.params[0]);
				if (param && param->GetFormType() == RE::FormType::FormList) {
					role.voiceLists.push_back(static_cast<const RE::BGSListForm*>(param));
				}
			}
			for (auto* alias : quest->aliases) {
				auto* refAlias = alias ? skyrim_cast<RE::BGSRefAlias*>(alias) : nullptr;
				if (refAlias && refAlias->fillType == RE::BGSBaseAlias::FILL_TYPE::kUniqueActor &&
					refAlias->fillData.uniqueActor.uniqueActor) {
					addressed.insert(refAlias->fillData.uniqueActor.uniqueActor->GetFormID());
					++uniqueAliases;
				}
			}
		}

		for (auto& [quest, role] : perQuest) {
			if (!role.voiceLists.empty()) {
				roles.push_back(std::move(role));
			}
		}
		const auto speakers = ReadLineSpeakers();
		built = true;
		logger::info("Voice protection: active ({} NPCs with dialogue of their own, {} voice-gated dialogue groups)",
			addressed.size(), roles.size());
		logger::debug("Voice protection: {} unique quest aliases, {} line speakers", uniqueAliases, speakers);
	}

	bool CanSwap(RE::Actor* a_actor, const RE::BGSVoiceType* a_old, const RE::BGSVoiceType* a_new)
	{
		if (!built || !a_actor || a_old == a_new) {
			return true;
		}
		if (!a_old || !a_new) {
			return false;
		}

		std::vector<const RE::TESNPC*> identity;
		IdentityRecords(a_actor, identity);
		for (const auto* npc : identity) {
			if (addressed.contains(npc->GetFormID())) {
				return false;
			}
		}

		const auto* base = a_actor->GetActorBase();
		for (const auto& role : roles) {
			bool selected = false;
			if (base) {
				selected = base->npcClass && role.classes.contains(base->npcClass->GetFormID());
				for (const auto& rank : base->factions) {
					if (selected) {
						break;
					}
					selected = rank.faction && rank.rank > -1 && role.factions.contains(rank.faction->GetFormID());
				}
			}
			for (const auto* npc : identity) {
				if (selected) {
					break;
				}
				selected = role.npcs.contains(npc->GetFormID());
			}
			if (!selected) {
				continue;
			}
			for (const auto* list : role.voiceLists) {
				if (ListContains(list, a_old) != ListContains(list, a_new)) {
					return false;
				}
			}
		}
		return true;
	}
}
