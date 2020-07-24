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

#include <dlfcn.h>

#include <iostream>

#include <Timer.hpp>

static struct MemStatsType
{
	size_t count;
	size_t size;
} MemStats;

void *
malloc(size_t size)
{
	typedef void *(*parent_t)(size_t);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "malloc");
	size_t *res = (size_t *)parent(size + sizeof(size_t));
	if (res == nullptr)
		return nullptr;
	*res = size;
	MemStats.count++;
	MemStats.size += size;
	return res + 1;
}

void *
realloc(void *ptr, size_t size)
{
	typedef void *(*parent_t)(void*, size_t);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "realloc");

	if (ptr == NULL)
		return malloc(size);

	size_t *res = (size_t *)ptr - 1;
	res = (size_t *)parent(res, size + sizeof(size_t));
	if (res == nullptr)
		return nullptr;

	MemStats.size -= *res;
	*res = size;
	MemStats.size += *res;
	return res + 1;
}

void *
memalign(size_t alignment, size_t size)
{
	(void)alignment;
	(void)size;
	return nullptr;
}

void free(void *ptr)
{
	typedef void (*parent_t)(void*);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "free");

	if (ptr == nullptr)
		return;
	size_t *res = (size_t *)ptr - 1;
	MemStats.count--;
	MemStats.size -= *res;
	parent(res);
}

struct MemMesurer {
	MemMesurer()
	{
		m_Initial = m_Max = mem_used();
	}

	void probe()
	{
		double cur = mem_used();
		if (cur > m_Max)
			m_Max = cur;
	}

	double maxUsage()
	{
		return m_Max - m_Initial;
	}

	double leak()
	{
		return mem_used() - m_Initial;
	}

	static double mem_used()
	{
		return MemStats.size;
	}

	double m_Initial;
	double m_Max;

};
