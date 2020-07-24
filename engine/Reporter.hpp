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

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <iostream>

class Reporter {
public:
	Reporter() = default;
	inline ~Reporter();
	inline void report(size_t size, const char *type, const char *family,
			   const char *struct_name, const char *test_name,
			   double Mrps, double MB_use, double bytes_per_record,
			   double MB_leak, size_t check);
	void done();

private:

	static inline const char *fmt(const char *str, size_t size);
	template <class T>
	static inline const char *fmt(const T& num, size_t size);

	void delimiter_line();
	void intro();
	void epilog();

	template <size_t col, class T, class... ARGS>
	void line(T&& t, ARGS&&... args);

	struct Columm {
		const char *name;
		size_t width;
	};

	static constexpr std::array<Columm, 10> Columns
		{{
			 {"Size", 10},
			 {"Type", 14},
			 {"Family", 8},
			 {"Struct", 22},
			 {"Test", 18},
			 {"Mrps", 13},
			 {"MB use", 13},
			 {"Bytes/elem", 13},
			 {"MB leak", 13},
			 {"Check", 10}
		 }};

	bool need_epilog = false;

};

const char *Reporter::fmt(const char *str, size_t size)
{
	static char buf[256];
	size_t len = strlen(str);
	assert(len <= size);
	size_t ls = 0, rs = 0;
	if (size > len) {
		ls = (size - len + 1) / 2;
		rs = (size - len) / 2;
	}
	char *p = buf;
	while (ls--)
		*p++ = ' ';
	while (*str)
		*p++ = *str++;
	while (rs--)
		*p++ = ' ';
	*p = 0;
	return buf;
}

template <class T>
const char *Reporter::fmt(const T& num, size_t size)
{
	std::string tmp = std::to_string(num);
	return fmt(tmp.c_str(), size);
}

void Reporter::delimiter_line()
{
	std::cout << '+';
	for (const Columm& c: Columns) {
		for (size_t i = 0; i < c.width; i++)
			std::cout << '-';
		std::cout << '+';
	}
	std::cout << std::endl;
}

void Reporter::intro()
{
	delimiter_line();

	std::cout << '|';
	for (const Columm& c: Columns) {
		std::cout << fmt(c.name, c.width) << '|';
	}
	std::cout << std::endl;

	delimiter_line();
}

void Reporter::epilog()
{
	delimiter_line();
}

void Reporter::done()
{
	if (need_epilog) {
		need_epilog = false;
		epilog();
	}
}

Reporter::~Reporter()
{
	done();
}

template <size_t col, class T, class... ARGS>
void Reporter::line(T&& t, ARGS&&... more)
{
	if constexpr (col == 0)
		std::cout << '|';

	std::cout << fmt(t, Columns[col].width);
	std::cout << '|';
	if constexpr (col + 1 < Columns.size())
		line<col + 1>(std::forward<ARGS>(more)...);

	if constexpr (col == 0)
		std::cout << std::endl;
}

void Reporter::report(size_t size, const char *type, const char *family,
		      const char *struct_name, const char *test_name,
		      double Mrps, double MB_use, double bytes_per_record,
		      double MB_leak, size_t check)
{
	if (!need_epilog)
		intro();
	need_epilog = true;

	line<0>(size, type, family, struct_name, test_name, Mrps, MB_use, bytes_per_record, MB_leak, check);
}
