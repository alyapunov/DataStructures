/*
 * Copyright (c) 2020, Aleksandr Lyapunov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <malloc.h>

#include <cstddef>
#include <utility>

#include <MemMesurer.hpp>
#include <Reporter.hpp>
#include <Types.hpp>
#include <Timer.hpp>

namespace {
template<size_t I, class SEQUENCE>
struct getter;

template<size_t I, size_t... SEQUENCE>
struct getter<I, std::index_sequence<SEQUENCE...>> {
	static constexpr size_t arr[] = {SEQUENCE...};
	static constexpr size_t value = arr[I];
};

template<class SEQUENCE>
struct sizer;

template<size_t... SEQUENCE>
struct sizer<std::index_sequence<SEQUENCE...>> {
	static constexpr size_t value = sizeof ... (SEQUENCE);
};

}

inline double mem_used()
{
	return mallinfo().uordblks / 1024. / 1024.;
}

template <class SIZES, class TYPES,
	template <typename TYPE> class STRUCTS,
	template <size_t SIZE, typename  TYPE, typename STRUCT> class TESTS>
struct AllTests {
	using sizes_t = SIZES;
	using types_t = TYPES;
	template <typename TYPE>
	using structs_t = STRUCTS<TYPE>;
	template <size_t SIZE, typename  TYPE, typename STRUCT>
	using tests_t = TESTS<SIZE, TYPE, STRUCT>;

	static constexpr size_t size0 = getter<0, sizes_t>::value;
	using type0 = std::tuple_element_t<0, types_t>;
	using struct0 = std::tuple_element_t<0, structs_t<type0>>;
	static constexpr size_t sizes_size = sizer<sizes_t>::value;
	static constexpr size_t types_size = std::tuple_size_v<types_t>;
	static constexpr size_t structs_size = std::tuple_size_v<structs_t<type0>>;
	static constexpr size_t tests_size = std::tuple_size_v<tests_t<size0, type0, struct0>>;

	static constexpr size_t size()
	{
		return sizes_size * types_size * structs_size * tests_size;
	}

	template <size_t I>
	struct OneTest {
		static constexpr size_t MORE0 = I;
		static constexpr size_t TEST_I = MORE0 % tests_size;
		static constexpr size_t MORE1 = MORE0 / tests_size;

		static constexpr size_t STRUCT_I = MORE1 % structs_size;
		static constexpr size_t MORE2 = MORE1 / structs_size;

		static constexpr size_t TYPE_I = MORE2 % types_size;
		static constexpr size_t MORE3 = MORE2 / types_size;

		static_assert(MORE3 < sizes_size, "wrong index caclulation");
		static constexpr size_t SIZE_I = MORE3;

		static constexpr size_t size = getter<SIZE_I, sizes_t>::value;
		using type_t = std::tuple_element_t<TYPE_I, types_t>;
		using struct_t = std::tuple_element_t<STRUCT_I, structs_t<type_t>>;
		using test_t = std::tuple_element_t<TEST_I, tests_t<size, type_t, struct_t>>;
	};

	template <class ONE_TEST>
	static size_t run_one(Reporter& reporter)
	{
		constexpr size_t size = ONE_TEST::size;
		using type_t = typename ONE_TEST::type_t;
		using struct_t = typename ONE_TEST::struct_t;
		using test_t = typename ONE_TEST::test_t;

		MemMesurer mem_mesurer;
		double bestMrps = 0;
		size_t side_effect = 0;
		{
			test_t test;
			size_t rounds = 256 * 1024 / size;
			rounds = rounds ? rounds : 1;
			for (size_t i = 0; i < rounds; i++) {
				test.prepare();
				Timer tm;
				mem_mesurer.probe();
				tm.start();
				auto res = test.test(mem_mesurer);
				tm.stop();
				mem_mesurer.probe();
				test.cleanup();
				bestMrps = std::max(bestMrps,
						    tm.Mrps(res.op_count));
				side_effect = res.side_effect;
			}
		}
		double MB_used = mem_mesurer.maxUsage() / 1024 / 1024;
		double bytes_per_record = mem_mesurer.maxUsage() / size;
		double MB_leak = mem_mesurer.leak() / 1024 / 1024;

		reporter.report(size, TypeTraits<type_t>::name, struct_t::family,
				struct_t::name, test_t::name,
				bestMrps, MB_used, bytes_per_record, MB_leak, side_effect);

		return 1;
	}

	template <class ONE_TEST>
	static size_t run_one_check(Reporter& reporter)
	{
		constexpr size_t size = ONE_TEST::size;
		using type_t = typename ONE_TEST::type_t;
		using struct_t = typename ONE_TEST::struct_t;
		using test_t = typename ONE_TEST::test_t;

		if constexpr (size == 0 ||
			std::is_same_v<type_t, nullptr_t> ||
			std::is_same_v<struct_t, nullptr_t> ||
			std::is_same_v<test_t, nullptr_t>) {
			return 0;
		} else {
			return run_one<ONE_TEST>(reporter);
		}
	}

	template <size_t... IDX>
	static size_t run_all_impl(Reporter& reporter, std::index_sequence<IDX...>)
	{
		return ((run_one_check<OneTest<IDX>>(reporter))+...);
	}

	static size_t run_all()
	{
		Reporter reporter;
		return run_all_impl(reporter, std::make_index_sequence<size()>{});
	}
};
