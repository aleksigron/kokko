#pragma once

namespace Memory
{
	long long KiloBytes(long long kiloBytes)
	{
		return kiloBytes * (1 << 10);
	}

	long long MegaBytes(long long megaBytes)
	{
		return megaBytes * (1 << 20);
	}

	long long GigaBytes(long long gigaBytes)
	{
		return gigaBytes * (1 << 30);
	}
}

constexpr unsigned long long operator ""_kB(unsigned long long kiloBytes)
{
	return kiloBytes * (1 << 10);
}

constexpr unsigned long long operator ""_MB(unsigned long long megaBytes)
{
	return megaBytes * (1 << 20);
}

constexpr unsigned long long operator ""_GB(unsigned long long gigaBytes)
{
	return gigaBytes * (1 << 30);
}
