#ifndef SP_PARSETABLE_H
#define SP_PARSETABLE_H

// =====================================================================
// PHASE 2 -- the key table: a config line is DATA, so parsing it should be
// a table lookup, not a wall of hand-written extractors.
//
// Every create_patch_instruction* in this codebase is a straight-line list of
// 40-98 extract*() calls, one per supported key, and each of those runs a
// std::regex_search over the WHOLE line. Cost is therefore O(keys DEFINED),
// not O(keys PRESENT): a five-key line pays for all 98 searches, and ~76 % of
// parse time is spent scanning for keys that are not on the line at all.
//
// This header replaces that with one pass over the line plus a hash/trie
// lookup per '=' actually found. Each form type declares a `constexpr`
// sp::KeySpec table -- key text, field kind, and the offset of the member the
// value lands in -- and `sp::KeyTable::parse` interprets it.
//
//   struct KeySpec { std::string_view key; FieldKind kind; uint16_t offset; };
//
// EQUIVALENCE. The rewrite is exact, not approximate, and the 28 golden
// snapshots are the gate. The three properties that make it exact:
//
//  1. Every extractor's regex has the shape `KEY\s*=([^:]+)`. `[^:]` cannot
//     cross a ':', and KEY contains no ':', so a match lies wholly inside one
//     ':'-delimited token, and the captured value is exactly "the rest of the
//     token after that '='".
//  2. `KEY\s*=` matches at a given '=' iff the token text before that '=',
//     with trailing whitespace removed, ENDS WITH KEY. That is a suffix test,
//     so one reversed trie over all the keys finds every key that fires at a
//     given '=' in a single walk. This reproduces the substring behaviour of
//     the originals exactly -- and that behaviour is load-bearing:
//     `spellsToAdd\s*=` (icase) really does fire on `levSpellsToAdd=...`, and
//     `ObjectsToRemove` / `objectsToRemove` really do fill two different
//     members from the same token.
//  3. `regex_search` takes the LEFTMOST match, so scanning tokens left to
//     right and taking the first hit per key is the same answer. (For two
//     '=' positions e1 < e2 the corresponding match starts are ordered the
//     same way: the whitespace run before e2 cannot reach back past e1,
//     because e1 holds a '=', which is not whitespace.)
//
// Case sensitivity is a property of the extractor, so it is derived from the
// kind rather than spelled out per row: extractValueString is the one
// extractor that does NOT pass std::regex::icase, so Value / ValueTilde are
// case-sensitive and every other kind is not.
//
// NOTHING IS RESOLVED HERE. Parsing must stay free of CommonLib singletons --
// golden_dump runs with no game behind it -- and patch_instruction's layout
// must not change, because the golden snapshots serialize it member by
// member. This header only decides WHICH member a value lands in.
// =====================================================================

#include "PCH.h"
#include "utility.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace sp
{
	// How a value is turned into fields. Each kind corresponds 1:1 to one of
	// the seven extractors in utility.cpp, or to one of NPC's four bespoke
	// inline blocks.
	enum class FieldKind : std::uint8_t
	{
		List,          // extractData          -> vector<string>, comma list
		ListAssign,    // extractDataStrings   -> vector<string>, comma list
		Value,         // extractValueString   -> string, "none" when absent
		ValueTilde,    // extractValueString with the `=\s*~([^~]+)~` pattern
		Arr2D,         // extractToArr2D       -> vector<vector<string>>
		MultiInt,      // extractMultiDataInt  -> (vector<string>, vector<int>, vector<int>)
		MultiFloat,    // extractMultiDataFloat-> (vector<string>, vector<float>, vector<float>)
		LevelRange,    // npc  levelRange=A~B  -> (string, string)
		FactionRanks,  // npc  factionsToAdd   -> (vector<string>, vector<float>, vector<float>)
		ObjectsToAdd,  // npc  objectsToAdd    -> (vector<string>, vector<string>)
		AttackData3D,  // npc  attackDataTo{Add,Change} -> vector<vector<vector<string>>>
	};

	inline constexpr std::uint16_t kNoField = 0xFFFFu;

	struct KeySpec
	{
		std::string_view key;                 // "keywordsToAdd"
		FieldKind        kind;                // how the value is parsed
		std::uint16_t    off1;                // offsetof() of the member it lands in
		std::uint16_t    off2 = kNoField;     // second member, for the multi-field kinds
		std::uint16_t    off3 = kNoField;     // third member
	};

	// extractValueString is the ONE extractor that omits std::regex::icase.
	inline constexpr bool keyIsCaseSensitive(FieldKind a_kind)
	{
		return a_kind == FieldKind::Value || a_kind == FieldKind::ValueTilde;
	}

	// A kind that writes "none" into its member when the key is absent.
	inline constexpr bool defaultsToNone(FieldKind a_kind)
	{
		return a_kind == FieldKind::Value || a_kind == FieldKind::ValueTilde ||
			a_kind == FieldKind::LevelRange;
	}

	namespace detail
	{
		inline bool isRegexSpace(char c)
		{
			// std::regex's \s, spelled out so the '=' scan cannot drift from it.
			return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
		}

		inline char lowerAscii(char c)
		{
			return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		// utility.cpp's trim(): removes SPACES only, and returns the input
		// untouched when it is all spaces. Used by extractToArr2D and by NPC's
		// attackData blocks. Deliberately not the same as trimWs below.
		inline std::string trimSpaces(std::string_view s)
		{
			const std::size_t first = s.find_first_not_of(' ');
			if (first == std::string_view::npos) {
				return std::string(s);
			}
			const std::size_t last = s.find_last_not_of(' ');
			return std::string(s.substr(first, last - first + 1));
		}

		// The inline erase(find_if_not(::isspace)) pattern used by
		// extractData / extractDataStrings / extractValueString / the
		// extractMultiData* family: removes ALL whitespace at both ends.
		inline std::string trimWs(std::string_view s)
		{
			std::size_t b = 0;
			std::size_t e = s.size();
			while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) {
				++b;
			}
			while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
				--e;
			}
			return std::string(s.substr(b, e - b));
		}

		template <class T>
		T* at(void* a_base, std::uint16_t a_off)
		{
			return reinterpret_cast<T*>(static_cast<char*>(a_base) + a_off);
		}

		// `[^,]+` over the captured value, trimmed, "none" dropped. Shared by
		// extractData and extractDataStrings, which differ only in whether they
		// append to or assign the destination -- and the destination is always
		// empty when they run, so that difference is not observable.
		inline void pushCommaList(std::string_view a_value, std::vector<std::string>& a_out)
		{
			std::size_t i = 0;
			while (i < a_value.size()) {
				if (a_value[i] == ',') {
					++i;
					continue;
				}
				const std::size_t start = i;
				while (i < a_value.size() && a_value[i] != ',') {
					++i;
				}
				std::string piece = trimWs(a_value.substr(start, i - start));
				if (piece != "none") {
					a_out.push_back(std::move(piece));
				}
			}
		}

		// extractToArr2D, verbatim, on an already-captured value.
		inline void applyArr2D(std::string_view a_value, std::vector<std::vector<std::string>>& a_out)
		{
			const std::string valueLine(a_value);
			std::vector<std::string> arr;
			std::size_t startPos = valueLine.find("=") + 1;  // npos + 1 == 0, exactly as upstream
			std::size_t pos = 0;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				arr.push_back(trimSpaces(std::string_view(valueLine).substr(startPos, pos - startPos)));
				startPos = pos + 1;
			}
			arr.push_back(trimSpaces(std::string_view(valueLine).substr(startPos)));

			std::vector<std::vector<std::string>> arr2D(arr.size());
			for (std::size_t i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				std::size_t              start = 0;
				while (start <= arr[i].size()) {
					const std::size_t innerPos = arr[i].find('~', start);
					const std::size_t end = (innerPos == std::string::npos) ? arr[i].size() : innerPos;
					splitArr.push_back(trimSpaces(std::string_view(arr[i]).substr(start, end - start)));
					if (innerPos == std::string::npos) {
						break;
					}
					start = innerPos + 1;
				}
				arr2D[i] = std::move(splitArr);
			}
			a_out = std::move(arr2D);
		}

		// The `(\w+)\s*=\s*([\d.]+)(?:\s*~\s*([\d.]+))?` sub-scan shared by
		// extractMultiDataInt / extractMultiDataFloat. Kept as a regex on
		// purpose: it now runs only when the key is actually on the line, which
		// is the whole point of the table.
		inline const std::regex& multiDataRegex()
		{
			static const std::regex re("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);
			return re;
		}

		// NPC's factionsToAdd sub-scan.
		inline const std::regex& factionRankRegex()
		{
			static const std::regex re("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?",
				std::regex::icase);
			return re;
		}

		// NPC's objectsToAdd sub-scan.
		inline const std::regex& objectsToAddRegex()
		{
			static const std::regex re("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+)", std::regex::icase);
			return re;
		}

		// NPC's attackDataToAdd / attackDataToChange block, verbatim.
		inline void applyAttackData3D(std::string_view a_value,
			std::vector<std::vector<std::vector<std::string>>>& a_out)
		{
			const std::string valueLine(a_value);
			std::vector<std::vector<std::vector<std::string>>> arr3D;

			std::vector<std::string> sections;
			std::size_t startPos = 0;
			std::size_t pos = 0;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				sections.push_back(valueLine.substr(startPos, pos - startPos));
				startPos = pos + 1;
			}
			sections.push_back(valueLine.substr(startPos));

			for (const auto& section : sections) {
				std::vector<std::string> arr;
				startPos = 0;
				pos = 0;
				std::string token;
				while ((pos = section.find("~", startPos)) != std::string::npos) {
					token = section.substr(startPos, pos - startPos);
					token = trimSpaces(token);
					arr.push_back(token);
					startPos = pos + 1;
				}
				token = section.substr(startPos);
				token = trimSpaces(token);
				arr.push_back(token);

				std::vector<std::vector<std::string>> arr2D(arr.size());
				for (std::size_t i = 0; i < arr.size(); i++) {
					std::vector<std::string> splitArr;
					std::size_t              innerPos = 0;
					std::string              innerToken;
					while ((innerPos = arr[i].find("=")) != std::string::npos) {
						innerToken = arr[i].substr(0, innerPos);
						innerToken = trimSpaces(innerToken);
						splitArr.push_back(innerToken);
						arr[i].erase(0, innerPos + 1);
					}
					innerToken = arr[i];
					innerToken = trimSpaces(innerToken);
					splitArr.push_back(innerToken);
					arr2D[i] = splitArr;
				}
				arr3D.push_back(arr2D);
			}
			a_out = std::move(arr3D);
		}
	}

	// The interpreter. One instance per form type, built once from that type's
	// constexpr KeySpec table.
	class KeyTable
	{
	public:
		KeyTable(const KeySpec* a_specs, std::size_t a_count) :
			m_specs(a_specs), m_count(a_count)
		{
			// parse() carries the per-line "already matched" flags on the
			// stack; no form type comes close to this, but fail loudly rather
			// than silently if one ever does.
			if (a_count > 256) {
				std::abort();
			}
			m_nodes.emplace_back();
			for (std::size_t i = 0; i < a_count; ++i) {
				insert(static_cast<std::uint16_t>(i));
			}
		}

		void parse(const std::string& a_line, void* a_dest) const
		{
			// One bit per spec; nothing here allocates for a table under 256
			// keys, which every form type is.
			bool seen[256] = {};
			const std::size_t n = a_line.size();

			std::size_t tokStart = 0;
			while (tokStart <= n) {
				std::size_t tokEnd = a_line.find(':', tokStart);
				if (tokEnd == std::string::npos) {
					tokEnd = n;
				}
				for (std::size_t e = tokStart; e < tokEnd; ++e) {
					if (a_line[e] != '=') {
						continue;
					}
					// `\s*` before the '='.
					std::size_t k = e;
					while (k > tokStart && detail::isRegexSpace(a_line[k - 1])) {
						--k;
					}
					walk(a_line, tokStart, k, e, tokEnd, seen, a_dest);
				}
				if (tokEnd == n) {
					break;
				}
				tokStart = tokEnd + 1;
			}

			// Every key the line did NOT carry still has to land its default:
			// extractValueString writes the literal "none" when it finds
			// nothing, and so does NPC's levelRange block.
			for (std::size_t i = 0; i < m_count; ++i) {
				if (seen[i] || !defaultsToNone(m_specs[i].kind)) {
					continue;
				}
				*detail::at<std::string>(a_dest, m_specs[i].off1) = "none";
				if (m_specs[i].off2 != kNoField) {
					*detail::at<std::string>(a_dest, m_specs[i].off2) = "none";
				}
			}
		}

	private:
		struct Node
		{
			// Small, sorted-by-insertion child list. The reversed-key trie is
			// at most as deep as the longest key, and branching is tiny below
			// the first character, so a linear scan beats a hash here.
			std::vector<std::pair<char, std::int32_t>> kids;
			std::vector<std::uint16_t>                 specs;  // key length == depth
		};

		void insert(std::uint16_t a_spec)
		{
			const std::string_view key = m_specs[a_spec].key;
			std::int32_t           node = 0;
			for (std::size_t d = 1; d <= key.size(); ++d) {
				const char c = detail::lowerAscii(key[key.size() - d]);
				std::int32_t next = -1;
				for (const auto& kid : m_nodes[node].kids) {
					if (kid.first == c) {
						next = kid.second;
						break;
					}
				}
				if (next < 0) {
					next = static_cast<std::int32_t>(m_nodes.size());
					m_nodes.emplace_back();
					m_nodes[node].kids.emplace_back(c, next);
				}
				node = next;
			}
			m_nodes[node].specs.push_back(a_spec);
			if (key.size() > m_maxKeyLen) {
				m_maxKeyLen = key.size();
			}
		}

		// Walk the reversed trie backwards from a_keyEnd (the position just
		// past the last non-whitespace character before the '='), reporting
		// every key that is a suffix of a_line[a_tokStart, a_keyEnd).
		void walk(const std::string& a_line, std::size_t a_tokStart, std::size_t a_keyEnd,
			std::size_t a_eq, std::size_t a_tokEnd, bool* a_seen, void* a_dest) const
		{
			std::int32_t      node = 0;
			const std::size_t limit = std::min(a_keyEnd - a_tokStart, m_maxKeyLen);
			for (std::size_t d = 1; d <= limit; ++d) {
				const char   c = detail::lowerAscii(a_line[a_keyEnd - d]);
				std::int32_t next = -1;
				for (const auto& kid : m_nodes[node].kids) {
					if (kid.first == c) {
						next = kid.second;
						break;
					}
				}
				if (next < 0) {
					return;
				}
				node = next;
				for (const std::uint16_t s : m_nodes[node].specs) {
					if (a_seen[s]) {
						continue;
					}
					const KeySpec& spec = m_specs[s];
					if (keyIsCaseSensitive(spec.kind) &&
						std::memcmp(a_line.data() + (a_keyEnd - d), spec.key.data(), d) != 0) {
						continue;
					}
					if (apply(spec, a_line, a_eq, a_tokEnd, a_dest)) {
						a_seen[s] = true;
					}
				}
			}
		}

		// Returns false when the value part of the pattern did not match, in
		// which case regex_search would have carried on looking -- so the key
		// must stay unseen.
		static bool apply(const KeySpec& a_spec, const std::string& a_line, std::size_t a_eq,
			std::size_t a_tokEnd, void* a_dest)
		{
			using detail::at;

			if (a_spec.kind == FieldKind::ValueTilde) {
				// `=\s*~([^~]+)~`, which is NOT bounded by ':'.
				std::size_t i = a_eq + 1;
				while (i < a_line.size() && detail::isRegexSpace(a_line[i])) {
					++i;
				}
				if (i >= a_line.size() || a_line[i] != '~') {
					return false;
				}
				const std::size_t close = a_line.find('~', i + 1);
				if (close == std::string::npos || close == i + 1) {
					return false;
				}
				*at<std::string>(a_dest, a_spec.off1) =
					detail::trimWs(std::string_view(a_line).substr(i + 1, close - i - 1));
				return true;
			}

			if (a_spec.kind == FieldKind::LevelRange) {
				// `levelRange\s*=\s*(\d+)\s*~\s*(\d+)`
				std::size_t i = a_eq + 1;
				const std::size_t n = a_line.size();
				while (i < n && detail::isRegexSpace(a_line[i])) {
					++i;
				}
				const std::size_t d1 = i;
				while (i < n && a_line[i] >= '0' && a_line[i] <= '9') {
					++i;
				}
				if (i == d1) {
					return false;
				}
				const std::size_t d1End = i;
				while (i < n && detail::isRegexSpace(a_line[i])) {
					++i;
				}
				if (i >= n || a_line[i] != '~') {
					return false;
				}
				++i;
				while (i < n && detail::isRegexSpace(a_line[i])) {
					++i;
				}
				const std::size_t d2 = i;
				while (i < n && a_line[i] >= '0' && a_line[i] <= '9') {
					++i;
				}
				if (i == d2) {
					return false;
				}
				*at<std::string>(a_dest, a_spec.off1) = a_line.substr(d1, d1End - d1);
				*at<std::string>(a_dest, a_spec.off2) = a_line.substr(d2, i - d2);
				return true;
			}

			// Everything else captures `([^:]+)`: the rest of the token.
			if (a_eq + 1 >= a_tokEnd) {
				return false;  // empty capture -- `+` needs at least one char
			}
			const std::string_view value =
				std::string_view(a_line).substr(a_eq + 1, a_tokEnd - a_eq - 1);

			switch (a_spec.kind) {
			case FieldKind::List:
			case FieldKind::ListAssign:
				detail::pushCommaList(value, *at<std::vector<std::string>>(a_dest, a_spec.off1));
				return true;

			case FieldKind::Value:
				*at<std::string>(a_dest, a_spec.off1) = detail::trimWs(value);
				return true;

			case FieldKind::Arr2D:
				detail::applyArr2D(value, *at<std::vector<std::vector<std::string>>>(a_dest, a_spec.off1));
				return true;

			case FieldKind::AttackData3D:
				detail::applyAttackData3D(value,
					*at<std::vector<std::vector<std::vector<std::string>>>>(a_dest, a_spec.off1));
				return true;

			case FieldKind::MultiInt:
			case FieldKind::MultiFloat: {
				const std::string valueStr(value);
				auto&             names = *at<std::vector<std::string>>(a_dest, a_spec.off1);
				std::sregex_iterator it(valueStr.begin(), valueStr.end(), detail::multiDataRegex());
				const std::sregex_iterator end;
				while (it != end) {
					const std::string name = detail::trimWs((*it)[1].str());
					if (name == "none") {
						break;
					}
					names.push_back(name);
					const std::string lo = (*it)[2].str();
					const std::string hi = (*it)[3].str() != "" ? (*it)[3].str() : lo;
					if (a_spec.kind == FieldKind::MultiInt) {
						at<std::vector<int>>(a_dest, a_spec.off2)->push_back(std::stoi(lo));
						at<std::vector<int>>(a_dest, a_spec.off3)->push_back(std::stoi(hi));
					} else {
						at<std::vector<float>>(a_dest, a_spec.off2)->push_back(std::stof(lo));
						at<std::vector<float>>(a_dest, a_spec.off3)->push_back(std::stof(hi));
					}
					++it;
				}
				return true;
			}

			case FieldKind::FactionRanks: {
				const std::string    valueStr(value);
				std::vector<std::string> names;
				std::vector<float>       lo;
				std::vector<float>       hi;
				std::sregex_iterator it(valueStr.begin(), valueStr.end(), detail::factionRankRegex());
				const std::sregex_iterator end;
				while (it != end) {
					const std::string name = detail::trimWs((*it)[1].str());
					if (name == "none") {
						break;
					}
					names.push_back(name);
					lo.push_back(std::stof((*it)[2].str()));
					hi.push_back((*it)[3].str() != "" ? std::stof((*it)[3].str()) : std::stof((*it)[2].str()));
					++it;
				}
				*at<std::vector<std::string>>(a_dest, a_spec.off1) = std::move(names);
				*at<std::vector<float>>(a_dest, a_spec.off2) = std::move(lo);
				*at<std::vector<float>>(a_dest, a_spec.off3) = std::move(hi);
				return true;
			}

			case FieldKind::ObjectsToAdd: {
				const std::string    valueStr(value);
				auto&                objs = *at<std::vector<std::string>>(a_dest, a_spec.off1);
				auto&                vals = *at<std::vector<std::string>>(a_dest, a_spec.off2);
				std::sregex_iterator it(valueStr.begin(), valueStr.end(), detail::objectsToAddRegex());
				const std::sregex_iterator end;
				for (; it != end; ++it) {
					objs.push_back((*it)[1].str());
					vals.push_back((*it)[2].str());
				}
				return true;
			}

			default:
				return true;
			}
		}

		std::vector<Node> m_nodes;
		const KeySpec*    m_specs;
		std::size_t       m_count;
		std::size_t       m_maxKeyLen = 0;
	};
}

#endif
