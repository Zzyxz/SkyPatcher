#pragma once

#define WIN32_LEAN_AND_MEAN

#include <ranges>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <MergeMapperPluginAPI.h>

#include <ankerl/unordered_dense.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#include <CLibUtil/distribution.hpp>
#include <CLibUtil/numeric.hpp>
#include <CLibUtil/rng.hpp>
#include <CLibUtil/string.hpp>
#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/singleton.hpp>

#include <ClibUtil/editorID.hpp>

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace string = clib_util::string;
namespace ini = clib_util::ini;
namespace dist = clib_util::distribution;

using namespace std::literals;
using namespace clib_util::singleton;
using SeedRNG = clib_util::RNG;

namespace stl
{
	using namespace SKSE::stl;
	using namespace clib_util;

	template <class F, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[0] };
		T::func = vtbl.write_vfunc(T::size, T::thunk);
	}

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

template <class CharT>
	using basic_zstring = std::basic_string_view<CharT>;

	using zstring = basic_zstring<char>;
	using zwstring = basic_zstring<wchar_t>;

	// owning pointer
	template <
		class T,
		class = std::enable_if_t<
			std::is_pointer_v<T>>>
	using owner = T;

	// non-owning pointer
	template <
		class T,
		class = std::enable_if_t<
			std::is_pointer_v<T>>>
	using observer = T;

	// non-null pointer
	template <
		class T,
		class = std::enable_if_t<
			std::is_pointer_v<T>>>
	using not_null = T;

	namespace nttp
	{
		template <class CharT, std::size_t N>
		struct string
		{
			using char_type = CharT;
			using pointer = char_type*;
			using const_pointer = const char_type*;
			using reference = char_type&;
			using const_reference = const char_type&;
			using size_type = std::size_t;

			static constexpr auto npos = static_cast<std::size_t>(-1);

			consteval string(const_pointer a_string) noexcept
			{
				for (size_type i = 0; i < N; ++i) {
					c[i] = a_string[i];
				}
			}

			[[nodiscard]] consteval const_reference operator[](size_type a_pos) const noexcept
			{
				assert(a_pos < N);
				return c[a_pos];
			}

			[[nodiscard]] consteval const_reference back() const noexcept { return (*this)[size() - 1]; }
			[[nodiscard]] consteval const_pointer   data() const noexcept { return c; }
			[[nodiscard]] consteval bool            empty() const noexcept { return this->size() == 0; }
			[[nodiscard]] consteval const_reference front() const noexcept { return (*this)[0]; }
			[[nodiscard]] consteval size_type       length() const noexcept { return N; }
			[[nodiscard]] consteval size_type       size() const noexcept { return length(); }

			template <std::size_t POS = 0, std::size_t COUNT = npos>
			[[nodiscard]] consteval auto substr() const noexcept
			{
				return string < CharT, COUNT != npos ? COUNT : N - POS > (this->data() + POS);
			}

			char_type c[N] = {};
		};

		template <class CharT, std::size_t N>
		string(const CharT (&)[N]) -> string<CharT, N - 1>;
	}

	template <class EF>                                        //
		requires(std::invocable<std::remove_reference_t<EF>>)  //
	class scope_exit
	{
	public:
		// 1)
		template <class Fn>
		explicit scope_exit(Fn&& a_fn)  //
			noexcept(std::is_nothrow_constructible_v<EF, Fn> ||
					 std::is_nothrow_constructible_v<EF, Fn&>)  //
			requires(!std::is_same_v<std::remove_cvref_t<Fn>, scope_exit> &&
					 std::is_constructible_v<EF, Fn>)
		{
			static_assert(std::invocable<Fn>);

			if constexpr (!std::is_lvalue_reference_v<Fn> &&
						  std::is_nothrow_constructible_v<EF, Fn>) {
				_fn.emplace(std::forward<Fn>(a_fn));
			} else {
				_fn.emplace(a_fn);
			}
		}

		// 2)
		scope_exit(scope_exit&& a_rhs)  //
			noexcept(std::is_nothrow_move_constructible_v<EF> ||
					 std::is_nothrow_copy_constructible_v<EF>)  //
			requires(std::is_nothrow_move_constructible_v<EF> ||
					 std::is_copy_constructible_v<EF>)
		{
			static_assert(!(std::is_nothrow_move_constructible_v<EF> && !std::is_move_constructible_v<EF>));
			static_assert(!(!std::is_nothrow_move_constructible_v<EF> && !std::is_copy_constructible_v<EF>));

			if (a_rhs.active()) {
				if constexpr (std::is_nothrow_move_constructible_v<EF>) {
					_fn.emplace(std::forward<EF>(*a_rhs._fn));
				} else {
					_fn.emplace(a_rhs._fn);
				}
				a_rhs.release();
			}
		}

		// 3)
		scope_exit(const scope_exit&) = delete;

		~scope_exit() noexcept
		{
			if (_fn.has_value()) {
				(*_fn)();
			}
		}

		void release() noexcept { _fn.reset(); }

	private:
		[[nodiscard]] bool active() const noexcept { return _fn.has_value(); }

		std::optional<std::remove_reference_t<EF>> _fn;
	};

	template <class EF>
	scope_exit(EF) -> scope_exit<EF>;

	template <
		class Enum,
		class Underlying = std::underlying_type_t<Enum>>
	class enumeration
	{
	public:
		using enum_type = Enum;
		using underlying_type = Underlying;

		static_assert(std::is_enum_v<enum_type>, "enum_type must be an enum");
		static_assert(std::is_integral_v<underlying_type>, "underlying_type must be an integral");

		constexpr enumeration() noexcept = default;

		constexpr enumeration(const enumeration&) noexcept = default;

		constexpr enumeration(enumeration&&) noexcept = default;

		template <class U2>  // NOLINTNEXTLINE(google-explicit-constructor)
		constexpr enumeration(enumeration<Enum, U2> a_rhs) noexcept :
			_impl(static_cast<underlying_type>(a_rhs.get()))
		{}

		template <class... Args>
		constexpr enumeration(Args... a_values) noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
			:
			_impl((static_cast<underlying_type>(a_values) | ...))
		{}

		~enumeration() noexcept = default;

		constexpr enumeration& operator=(const enumeration&) noexcept = default;
		constexpr enumeration& operator=(enumeration&&) noexcept = default;

		template <class U2>
		constexpr enumeration& operator=(enumeration<Enum, U2> a_rhs) noexcept
		{
			_impl = static_cast<underlying_type>(a_rhs.get());
		}

		constexpr enumeration& operator=(enum_type a_value) noexcept
		{
			_impl = static_cast<underlying_type>(a_value);
			return *this;
		}

		[[nodiscard]] explicit constexpr operator bool() const noexcept { return _impl != static_cast<underlying_type>(0); }

		[[nodiscard]] constexpr enum_type       operator*() const noexcept { return get(); }
		[[nodiscard]] constexpr enum_type       get() const noexcept { return static_cast<enum_type>(_impl); }
		[[nodiscard]] constexpr underlying_type underlying() const noexcept { return _impl; }

		template <class... Args>
		constexpr enumeration& set(Args... a_args) noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
		{
			_impl |= (static_cast<underlying_type>(a_args) | ...);
			return *this;
		}

		template <class... Args>
		constexpr enumeration& reset(Args... a_args) noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
		{
			_impl &= ~(static_cast<underlying_type>(a_args) | ...);
			return *this;
		}

		template <class... Args>
		[[nodiscard]] constexpr bool any(Args... a_args) const noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
		{
			return (_impl & (static_cast<underlying_type>(a_args) | ...)) != static_cast<underlying_type>(0);
		}

		template <class... Args>
		[[nodiscard]] constexpr bool all(Args... a_args) const noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
		{
			return (_impl & (static_cast<underlying_type>(a_args) | ...)) == (static_cast<underlying_type>(a_args) | ...);
		}

		template <class... Args>
		[[nodiscard]] constexpr bool none(Args... a_args) const noexcept  //
			requires(std::same_as<Args, enum_type> && ...)
		{
			return (_impl & (static_cast<underlying_type>(a_args) | ...)) == static_cast<underlying_type>(0);
		}

	private:
		underlying_type _impl{ 0 };
	};

	template <class... Args>
	enumeration(Args...) -> enumeration<
		std::common_type_t<Args...>,
		std::underlying_type_t<
			std::common_type_t<Args...>>>;

}

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#	define OFFSET_3(se, ae, vr) ae
#elif SKYRIMVR
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) vr
#else
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) se
#endif

#include "Version.h"
#include "Defs.h"
