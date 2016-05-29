#include "AsciiStringUtil.hpp"

unsigned int AsciiStringUtil::FindPrintable(StringRef string)
{
	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end; ++itr)
	{
		char c = *itr;

		if (c < 127 && c > 31)
			return static_cast<unsigned int>(itr - string.str);
	}

	return string.len;
}

unsigned int AsciiStringUtil::FindUnprintable(StringRef string)
{
	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end; ++itr)
	{
		char c = *itr;

		if (c >= 127 || c <= 31)
			return static_cast<unsigned int>(itr - string.str);
	}

	return string.len;
}

unsigned int AsciiStringUtil::FindSpacesInString(StringRef string,
												 unsigned int* posOut,
												 unsigned int maxPositions)
{
	unsigned int foundCount = 0;

	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end && foundCount < maxPositions; ++itr)
	{
		if (*itr == ' ')
		{
			posOut[foundCount] = static_cast<unsigned int>(itr - string.str);
			++foundCount;
		}
	}

	return foundCount;
}

int AsciiStringUtil::ParseInt(StringRef string)
{
	int result = 0;
	int sign = 1;

	const char* itr = string.str;
	const char* end = string.str + string.len;

	if (*itr == '-')
	{
		sign = -1;
		++itr;
	}

	for (; itr != end; ++itr)
	{
		result = result * 10 + (*itr - '0');
	}

	return result * sign;
}
