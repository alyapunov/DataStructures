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

#include <MemMeasurer.hpp>
#include <Reporter.hpp>
#include <Types.hpp>
#include <Timer.hpp>

namespace {

template<size_t I, class SEQUENCE>
struct sequence_element;

template<size_t I, size_t... SEQUENCE>
struct sequence_element<I, std::index_sequence<SEQUENCE...>> {
	static constexpr size_t arr[] = {SEQUENCE...};
	static constexpr size_t value = arr[I];
};

template<size_t I, class SEQUENCE>
constexpr size_t sequence_element_v = sequence_element<I, SEQUENCE>::value;

template<class SEQUENCE>
struct sequence_size;

template<size_t... SEQUENCE>
struct sequence_size<std::index_sequence<SEQUENCE...>> {
	static constexpr size_t value = sizeof ... (SEQUENCE);
};

template<class SEQUENCE>
constexpr size_t sequence_size_v = sequence_size<SEQUENCE>::value;

} // anonymous namespace

template <class SIZES, class TYPES, template <typename TYPE> class STRUCTS,
	template <size_t SIZE, typename TYPE, typename STRUCT> class TESTS>
struct AllTests {

	static constexpr size_t size0 = sequence_element_v<0, SIZES>;
	using type0 = std::tuple_element_t<0, TYPES>;
	using struct0 = std::tuple_element_t<0, STRUCTS<type0>>;
	static constexpr size_t sizes_size = sequence_size_v<SIZES>;
	static constexpr size_t types_size = std::tuple_size_v<TYPES>;
	static constexpr size_t structs_size = std::tuple_size_v<STRUCTS<type0>>;
	static constexpr size_t tests_size = std::tuple_size_v<TESTS<size0, type0, struct0>>;

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

		static constexpr size_t size = sequence_element_v<SIZE_I, SIZES>;
		using type_t = std::tuple_element_t<TYPE_I, TYPES>;
		using struct_t = std::tuple_element_t<STRUCT_I, STRUCTS<type_t>>;
		using test_t = std::tuple_element_t<TEST_I, TESTS<size, type_t, struct_t>>;
	};

	template <class ONE_TEST>
	static size_t run_one(Reporter& reporter)
	{
		constexpr size_t size = ONE_TEST::size;
		using type_t = typename ONE_TEST::type_t;
		using struct_t = typename ONE_TEST::struct_t;
		using test_t = typename ONE_TEST::test_t;

		MemMeasurer mem_measurer;
		double bestMrps = 0;
		size_t side_effect = 0;
		{
			test_t test;
			size_t rounds = 256 * 1024 / size;
			rounds = rounds ? rounds : 1;
			for (size_t i = 0; i < rounds; i++) {
				test.prepare();
				Timer tm;
				mem_measurer.probe();
				tm.start();
				auto res = test.test(mem_measurer);
				tm.stop();
				mem_measurer.probe();
				test.cleanup();
				bestMrps = std::max(bestMrps,
						    tm.Mrps(res.op_count));
				side_effect = res.side_effect;
			}
		}
		double MB_used = mem_measurer.maxUsage() / 1024 / 1024;
		double bytes_per_record = mem_measurer.maxUsage() / size;
		double MB_leak = mem_measurer.leak() / 1024 / 1024;

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
