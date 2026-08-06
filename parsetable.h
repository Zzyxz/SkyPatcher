#ifndef SP_PARSETABLE_H
#define SP_PARSETABLE_H

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
	enum class FieldKind : std::uint8_t
	{
		List,
		ListAssign,
		Value,
		ValueTilde,
		Arr2D,
		MultiInt,
		MultiFloat,
		LevelRange,
		FactionRanks,
		ObjectsToAdd,
		AttackData3D,
	};

	inline constexpr std::uint16_t kNoField = 0xFFFFu;

	struct KeySpec
	{
		std::string_view key;
		FieldKind        kind;
		std::uint16_t    off1;
		std::uint16_t    off2 = kNoField;
		std::uint16_t    off3 = kNoField;
	};

	inline constexpr bool keyIsCaseSensitive(FieldKind a_kind)
	{
		return a_kind == FieldKind::Value || a_kind == FieldKind::ValueTilde;
	}

	inline constexpr bool defaultsToNone(FieldKind a_kind)
	{
		return a_kind == FieldKind::Value || a_kind == FieldKind::ValueTilde ||
			a_kind == FieldKind::LevelRange;
	}

	namespace detail
	{
		inline bool isRegexSpace(char c)
		{
			return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
		}

		inline char lowerAscii(char c)
		{
			return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		inline std::string trimSpaces(std::string_view s)
		{
			const std::size_t first = s.find_first_not_of(' ');
			if (first == std::string_view::npos) {
				return std::string(s);
			}
			const std::size_t last = s.find_last_not_of(' ');
			return std::string(s.substr(first, last - first + 1));
		}

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

		inline void applyArr2D(std::string_view a_value, std::vector<std::vector<std::string>>& a_out)
		{
			const std::string valueLine(a_value);
			std::vector<std::string> arr;
			std::size_t startPos = valueLine.find("=") + 1;
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

		inline const std::regex& multiDataRegex()
		{
			static const std::regex re("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);
			return re;
		}

		inline const std::regex& factionRankRegex()
		{
			static const std::regex re("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?",
				std::regex::icase);
			return re;
		}

		inline const std::regex& objectsToAddRegex()
		{
			static const std::regex re("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+)", std::regex::icase);
			return re;
		}

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

	class KeyTable
	{
	public:
		KeyTable(const KeySpec* a_specs, std::size_t a_count) :
			m_specs(a_specs), m_count(a_count)
		{
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
			std::vector<std::pair<char, std::int32_t>> kids;
			std::vector<std::uint16_t>                 specs;
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

		static bool apply(const KeySpec& a_spec, const std::string& a_line, std::size_t a_eq,
			std::size_t a_tokEnd, void* a_dest)
		{
			using detail::at;

			if (a_spec.kind == FieldKind::ValueTilde) {
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

			if (a_eq + 1 >= a_tokEnd) {
				return false;
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
