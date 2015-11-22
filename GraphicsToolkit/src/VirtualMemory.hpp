#pragma once

#include <cstddef>

namespace VirtualMemory
{
	void* ReserveAddressSpace(std::size_t reserveSize);
	bool FreeAddressSpace(void* address, std::size_t reserveSize);

	bool CommitReservedSpace(void* address, std::size_t commitSize);
	bool DecommitReservedSpace(void* address, std::size_t commitSize);
}
