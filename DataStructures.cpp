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

#include <malloc.h>

#include <algorithm>
#include <cstddef>
#include <utility>

#include <Runner.hpp>
#include <Types.hpp>
#include <Structs.hpp>
#include <Tests.hpp>

using sizes = std::index_sequence<
	1024,
	32 * 1024,
	1 * 1024 * 1024,
	16 * 1024 * 1024,
	0>;

using types = std::tuple<
	uint64_t,
	char_ptr,
	nullptr_t
>;

template <typename TYPE>
using structs = std::tuple<
	StdSetStruct<TYPE>,
	StdUnorderedSetStruct<TYPE>,
	nullptr_t
>;

template <size_t SIZE, typename TYPE, class STRUCT>
using tests = std::tuple<
	Insert<SIZE, TYPE, STRUCT>,
	Delete<SIZE, TYPE, STRUCT>,
	SearchHit<SIZE, TYPE, STRUCT>,
	SearchMiss<SIZE, TYPE, STRUCT>,
	SearchMixed<SIZE, TYPE, STRUCT>,
	RandWorkload<SIZE, TYPE, STRUCT>,
	nullptr_t
>;

int main()
{
	int rc = mallopt(M_ARENA_MAX, 1);
	if (rc != 1)
		std::cout << "Failed to set mallopt. Memory measurement could be inaccurate." << std::endl;

	size_t total_run = AllTests<sizes, types, structs, tests>::run_all();

	std::cout << "Total tests was run: " << total_run << std::endl;
}
