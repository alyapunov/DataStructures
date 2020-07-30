#include <MemMeasurer.hpp>

#include <dlfcn.h>

struct MemStats
{
	size_t count;
	size_t size;
};

MemStats MemStatsInstance;

extern "C" {

void *
malloc(size_t size)
{
	typedef void *(*parent_t)(size_t);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "malloc");
	size_t *res = (size_t *)parent(size + sizeof(size_t));
	if (res == nullptr)
		return nullptr;
	*res = size;
	MemStatsInstance.count++;
	MemStatsInstance.size += size;
	return res + 1;
}

void *
calloc(size_t num, size_t elem_size)
{
	size_t size = num * elem_size;
	typedef void *(*parent_t)(size_t);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "calloc");
	size_t *res = (size_t *)parent(size + sizeof(size_t));
	if (res == nullptr)
		return nullptr;
	*res = size;
	MemStatsInstance.count++;
	MemStatsInstance.size += size;
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

	MemStatsInstance.size -= *res;
	*res = size;
	MemStatsInstance.size += *res;
	return res + 1;
}

void free(void *ptr)
{
	typedef void (*parent_t)(void*);
	static parent_t parent = (parent_t)dlsym(RTLD_NEXT, "free");

	if (ptr == nullptr)
		return;
	size_t *res = (size_t *)ptr - 1;
	MemStatsInstance.count--;
	MemStatsInstance.size -= *res;
	parent(res);
}

// Not implemented:
void *
memalign(size_t alignment, size_t size)
{
	(void)alignment;
	(void)size;
	return nullptr;
}

int
posix_memalign(void **memptr, size_t alignment, size_t size)
{
	(void)memptr;
	(void)alignment;
	(void)size;
	return -1;
}

void *
aligned_alloc(size_t alignment, size_t size)
{
	(void)alignment;
	(void)size;
	return nullptr;
}

void *
valloc(size_t size)
{
	(void)size;
	return nullptr;
}

void *
pvalloc(size_t size)
{
	(void)size;
	return nullptr;
}


double MemMeasurer::memUsed()
{
	return MemStatsInstance.size;
}

double MemMeasurer::countUsed()
{
	return MemStatsInstance.count;
}

} // extern "C" {
