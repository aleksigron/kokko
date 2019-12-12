#include "VirtualMemory.hpp"

#if defined(__APPLE__) || defined(__linux__)
#define USE_MMAP
#include <sys/mman.h>

#elif defined(_WIN32)
#define USE_VIRTUAL_ALLOC
#include "Windows.h"

#else
#error Unsupported platform
#endif

void* VirtualMemory::ReserveAddressSpace(std::size_t reserveSize)
{
	void* result = nullptr;

#ifdef USE_MMAP
	void* mem = mmap(nullptr, reserveSize, PROT_READ | PROT_WRITE,
					 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (mem != MAP_FAILED)
	{
		result = mem;
	}
#endif

#ifdef USE_VIRTUAL_ALLOC
	void* mem = VirtualAlloc(nullptr, reserveSize, MEM_RESERVE, PAGE_READWRITE);

	if (mem != nullptr)
	{
		result = mem;
	}
#endif

	return result;
}

bool VirtualMemory::FreeAddressSpace(void* address, std::size_t reserveSize)
{
	bool result = false;

#ifdef USE_MMAP
	result = munmap(address, reserveSize) == 0;
#endif

#ifdef USE_VIRTUAL_ALLOC
	result = VirtualFree(address, 0, MEM_RELEASE) == TRUE;
#endif

	return result;
}

bool VirtualMemory::CommitReservedSpace(void* address, std::size_t commitSize)
{
	bool result = false;

#ifdef USE_MMAP
	result = true;
#endif

#ifdef USE_VIRTUAL_ALLOC
	result = VirtualAlloc(address, commitSize, MEM_COMMIT, PAGE_READWRITE) != nullptr;
#endif

	return result;
}

bool VirtualMemory::DecommitReservedSpace(void* address, std::size_t commitSize)
{
	bool result = false;

#ifdef USE_MMAP
	result = true;
#endif

#ifdef USE_VIRTUAL_ALLOC
	result = VirtualFree(address, commitSize, MEM_DECOMMIT) == TRUE;
#endif

	return result;
}
