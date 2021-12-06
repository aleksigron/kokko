#pragma once

#include <cstddef>

#include "Core/Optional.hpp"

namespace kokko
{
	template <typename Type>
	void IntegerToHexadecimal(Type value, char* writeToBuffer)
	{
		static const char digits[] = "0123456789abcdef";
		constexpr size_t numChars = sizeof(Type) * 2;

		for (size_t i = 0, j = (numChars - 1) * 4; i < numChars; ++i, j -= 4)
		{
			writeToBuffer[i] = digits[(value >> j) & 0x0f];
		}

		writeToBuffer[numChars] = '\0';
	}

	template <typename Type>
	Optional<Type> HexadecimalToInteger(const char* hexStr)
	{
		constexpr size_t numChars = sizeof(Type) * 2;
		auto convert = [](unsigned char c) -> unsigned char {
			return (c & 0xF) + 9 * (c >> 6);
		};

		Type result = 0;

		for (size_t i = 0, j = (numChars - 1) * 4; i < numChars; ++i, j -= 4)
		{
			char c = hexStr[i];
			if (c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'f')
				return Optional<Type>();

			result |= convert(static_cast<unsigned char>(c)) << j;
		}

		return result;
	}
}
