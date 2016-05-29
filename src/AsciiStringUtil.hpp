#pragma once

#include "StringRef.hpp"

namespace AsciiStringUtil
{
	/**
	 * Find the first occurrence of an unprintable character in an ASCII string.
	 * Return the position of the found character, or string.len if not found.
	 */
	unsigned int FindUnprintable(StringRef string);

	/**
	 * Find the first occurrence of a printable character in an ASCII string.
	 * Return the position of the found character, or string.len if not found.
	 */
	unsigned int FindPrintable(StringRef string);

	/**
	 * Find a maximum of maxPositions space characters in a string. Set those
	 * positions in the posOut array. Return the number of spaces found.
	 */
	unsigned int FindSpacesInString(StringRef string,
									unsigned int* posOut,
									unsigned int maxPositions);

	int ParseInt(StringRef string);
}
