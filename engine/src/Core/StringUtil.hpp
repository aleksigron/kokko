#pragma once

#include <cassert>
#include <cstddef>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"

namespace kokko
{
	template <typename Type>
	void IntegerToHexadecimal(Type value, ArrayView<char> out)
	{
		static const char digits[] = "0123456789abcdef";
		constexpr size_t numChars = sizeof(Type) * 2;

		assert(out.GetCount() >= numChars);

		for (size_t i = 0, j = (numChars - 1) * 4; i < numChars; ++i, j -= 4)
		{
			out[i] = digits[(value >> j) & 0x0f];
		}
	}

	template <typename Type>
	Optional<Type> HexadecimalToInteger(ArrayView<const char> string)
	{
		constexpr size_t numChars = sizeof(Type) * 2;

		assert(string.GetCount() >= numChars);

		auto convert = [](Type c) {
			return (c & 0xF) + 9 * (c >> 6);
		};

		Type result = 0;

		for (size_t i = 0, j = (numChars - 1) * 4; i < numChars; ++i, j -= 4)
		{
			char c = string[i];
			if (c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'f')
				return Optional<Type>();

			Type shifted = convert(static_cast<Type>(c)) << j;
			result |= shifted;
		}

		return result;
	}
}
