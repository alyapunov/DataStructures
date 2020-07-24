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
#include <set>
#include <unordered_set>

template <typename TYPE>
struct StdSetStruct {
	bool insert(const TYPE& t)
	{
		return m_Core.insert(t).second;
	}
	bool remove(const TYPE& t)
	{
		return m_Core.erase(t) != 0;
	}
	bool has(const TYPE& t) const
	{
		return m_Core.find(t) != m_Core.end();
	}
	void clear()
	{
		m_Core.clear();
	}
	size_t size() const
	{
		return m_Core.size();
	}
	static constexpr const char *family = "tree";
	static constexpr const char *name = "std::set";
	static constexpr bool use = true;

	std::set<TYPE> m_Core;
};

template <typename TYPE>
struct StdUnorderedSetStruct {
	bool insert(const TYPE& t)
	{
		return m_Core.insert(t).second;
	}
	bool remove(const TYPE& t)
	{
		return m_Core.erase(t) != 0;
	}
	bool has(const TYPE& t) const
	{
		return m_Core.find(t) != m_Core.end();
	}
	void clear()
	{
		m_Core.clear();
	}
	size_t size() const
	{
		return m_Core.size();
	}
	static constexpr const char *family = "hash";
	static constexpr const char *name = "std::unordered_set";
	static constexpr bool use = true;

	std::unordered_set<TYPE> m_Core;
};
