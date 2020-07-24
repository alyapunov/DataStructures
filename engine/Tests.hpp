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

#include <algorithm>
#include <cassert>

#include <MemMeasurer.hpp>
#include <Types.hpp>

struct TestResult {
	size_t op_count;
	size_t side_effect;
};

template <typename TYPE>
struct TestBase {
	TestBase() = default;

	template <size_t COUNT, size_t MAX>
	void genData()
	{
		srand(0);
		static_assert(COUNT * sizeof(TYPE) <= MAX_DATA_SIZE, "increase buffer");
		m_DataSize = COUNT;
		for (size_t i = 0; i < m_DataSize; i++)
			m_Data[i] = TypeTraits<TYPE>::gen(MAX);
		std::random_shuffle(m_Data, m_Data + m_DataSize);
	}

	~TestBase()
	{
		TypeTraits<TYPE>::free_gen();
	}

	TYPE*& m_Data = TypeTraits<TYPE>::data();
	size_t m_DataSize = 0;
};

template <size_t SIZE, typename TYPE, class STRUCT>
struct Insert : TestBase<TYPE> {
	Insert()
	{
		this->template genData<SIZE, SIZE * 10>();
	}
	~Insert()
	{
		assert(m_Set.size() == 0);
	}

	void prepare()
	{
		assert(m_Set.size() == 0);
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		size_t inserted = 0;
		for (size_t i = 0; i < SIZE; i++) {
			inserted += m_Set.insert(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		assert(inserted == m_Set.size());
		return TestResult{SIZE, inserted};
	}

	void cleanup()
	{
		m_Set.clear();
	}

	STRUCT m_Set;
	static constexpr const char *name = "insert";
};

template <size_t SIZE, typename TYPE, class STRUCT>
struct Delete : TestBase<TYPE> {
	Delete()
	{
		this->template genData<SIZE, SIZE * 10>();
	}

	~Delete()
	{
		assert(m_Set.size() == 0);
	}

	void prepare()
	{
		assert(m_Set.size() == 0);
		for (size_t i = 0; i < SIZE; i++)
			m_Set.insert(this->m_Data[i]);
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		size_t deteted = 0;
		size_t was_size = m_Set.size();
		for (size_t i = 0; i < SIZE; i++) {
			deteted += m_Set.remove(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		assert(deteted == was_size); (void)was_size;
		return TestResult{SIZE, deteted};
	}

	void cleanup()
	{
		assert(m_Set.size() == 0);
	}

	STRUCT m_Set;
	static constexpr const char *name = "delete";
};

template <size_t SIZE, typename TYPE, class STRUCT>
struct SearchHit : TestBase<TYPE> {
	SearchHit()
	{
		this->template genData<SIZE, SIZE * 10>();
		for (size_t i = 0; i < SIZE; i++)
			m_Set.insert(this->m_Data[i]);
	}

	~SearchHit()
	{
		m_Set.clear();
	}

	void prepare()
	{
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		size_t res = 0;
		for (size_t i = 0; i < SIZE; i++) {
			res += m_Set.has(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		assert(res == SIZE);
		return TestResult{SIZE, res};
	}

	void cleanup()
	{
	}

	STRUCT m_Set;
	static constexpr const char *name = "search hit";
};

template <size_t SIZE, typename TYPE, class STRUCT>
struct SearchMiss : TestBase<TYPE> {
	SearchMiss()
	{
		this->template genData<SIZE * 2, SIZE * 10>();
		for (size_t i = 0; i < SIZE; i++)
			m_Set.insert(this->m_Data[i]);
	}

	~SearchMiss()
	{
		m_Set.clear();
	}

	void prepare()
	{
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		size_t res = 0;
		for (size_t i = SIZE; i < SIZE * 2; i++) {
			res += m_Set.has(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		return TestResult{SIZE, res};
	}

	void cleanup()
	{
	}

	STRUCT m_Set;
	static constexpr const char *name = "search miss";
};

template <size_t SIZE, typename TYPE, class STRUCT>
struct SearchMixed : TestBase<TYPE> {
	SearchMixed()
	{
		this->template genData<SIZE * 2, SIZE * 10>();
		for (size_t i = 0; i < this->m_DataSize; i += 2)
			m_Set.insert(this->m_Data[i]);
	}

	~SearchMixed()
	{
		m_Set.clear();
	}

	void prepare()
	{
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		size_t res = 0;
		for (size_t i = 0; i < this->m_DataSize; i++) {
			res += m_Set.has(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		return TestResult{this->m_DataSize, res};
	}

	void cleanup()
	{
	}

	STRUCT m_Set;
	static constexpr const char *name = "search mixed";
};


template <size_t SIZE, typename TYPE, class STRUCT>
struct RandWorkload : TestBase<TYPE> {
	RandWorkload()
	{
		this->template genData<SIZE * 4, SIZE * 2>();
	}

	~RandWorkload()
	{
		assert(m_Set.size() == 0);
	}

	void prepare()
	{
		for (size_t i = 0; i < SIZE; i++)
			m_Set.insert(this->m_Data[i]);
	}

	TestResult test(MemMeasurer& mem_measurer) __attribute_noinline__
	{
		for (size_t i = SIZE; i < this->m_DataSize; i++) {
			if (m_Set.has(this->m_Data[i]))
				m_Set.remove(this->m_Data[i]);
			else
				m_Set.insert(this->m_Data[i]);
			if (i % 1024 == 0)
				mem_measurer.probe();
		}
		return TestResult{this->m_DataSize - SIZE, m_Set.size()};
	}

	void cleanup()
	{
		m_Set.clear();
	}

	STRUCT m_Set;
	static constexpr const char *name = "rand workload";
};
