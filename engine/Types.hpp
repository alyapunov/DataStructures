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
#include <cstdint>
#include <cstring>
#include <tuple>

#include <PMurHash.h>

enum {
	MH_STRN_HASH_SEED = 13U
};

const size_t MAX_DATA_SIZE = 512 * 1024 * 1024;
static inline char data_storage[MAX_DATA_SIZE];

inline uint32_t hash(const char *data, size_t size)
{
	uint32_t h = MH_STRN_HASH_SEED;
	uint32_t carry = 0;
	PMurHash32_Process(&h, &carry, data, size);
	return PMurHash32_Result(h, carry, size);
}

inline uint32_t hash(const char *cdata)
{
	return hash(cdata, strlen(cdata));
}

inline size_t true_rand()
{
	return rand();
	//return (size_t(rand()) << 10) ^ rand();
}

template <class TYPE>
struct TypeTraits;

template <>
struct TypeTraits<uint64_t> {
	using TYPE = uint64_t;
	static constexpr const char *name = "uint64_t";
	static uint32_t hash(TYPE t) { return t; }
	static int cmp(TYPE t1, TYPE t2) { return t1 < t2 ? -1 : t1 > t2; }
	static bool same(TYPE t1, TYPE t2) { return t1 == t2; }
	static bool equals(TYPE t1, TYPE t2) { return t1 == t2; }
	static TYPE gen(size_t max) { return true_rand() % max; }
	static void free_gen() { }
	static TYPE*& data()
	{
		static TYPE *ptr = (TYPE*)data_storage;
		return ptr;
	}
};

struct char_ptr {
	const char *core;
	bool operator<(const char_ptr& other) const
	{
		return strcmp(core, other.core) < 0;
	}
	bool operator==(const char_ptr& other) const
	{
		return strcmp(core, other.core) == 0;
	}
};

namespace std {
	template<>
	struct hash<char_ptr> {
		size_t operator()(char_ptr t) const { return ::hash(t.core); }
	};
}

template <>
struct TypeTraits<char_ptr> {
	using TYPE = char_ptr;
	static constexpr const char *name = "const char *";
	static uint32_t hash(TYPE t) { return ::hash(t.core); }
	static int cmp(TYPE t1, TYPE t2) { return strcmp(t1.core, t2.core); }
	static bool same(TYPE t1, TYPE t2) { return t1.core == t2.core; }
	static bool equals(TYPE t1, TYPE t2) { return cmp(t1, t2) == 0; }
	static constexpr size_t ALLOC_SIZE = 16;
	static char * StringBuf()
	{
		static char storage[ALLOC_SIZE / sizeof(char_ptr) * MAX_DATA_SIZE];
		return storage;
	}
	static size_t& stringBufPos() { static size_t pos; return pos; };
	static TYPE gen(size_t max)
	{
		static const char * letters =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";
		size_t r = true_rand() % max;
		size_t len = 10 + r % 4;
		r /= 4;
		assert(stringBufPos() < MAX_DATA_SIZE);
		char *p = StringBuf() + stringBufPos() * ALLOC_SIZE;
		stringBufPos()++;
		for (size_t i = 0; i < len; i++) {
			p[i] = letters[r % 64];
			r /= 64;
		}
		p[len] = 0;
		return TYPE{p};
	};
	static void free_gen()
	{
		stringBufPos() = 0;
	}
	static TYPE*& data()
	{
		static TYPE *ptr = (TYPE *)data_storage;
		return ptr;
	}
};
