#pragma once

#include "StringRef.hpp"

namespace EncodingUtf8
{
	/**
	 * Encodes a codepoint and writes at most 4 bytes to the buffer utf8BytesOut.
	 * Returns how many bytes were encoded, 0 if codepoint is invalid.
	 */
	unsigned int EncodeCodepoint(unsigned int codepoint, char* utf8BytesOut);

	/**
	 * Decodes a codepoint to codepointOut.
	 * Returns how many bytes were decoded, 0 if input doesn't point to the first
	 * byte of a UTF8 character.
	 */
	unsigned int DecodeCodepoint(const char* input, unsigned int& codepointOut);

	/**
	 * Counts how many unicode characters are in a UTF-8 encoded byte array.
	 */
	unsigned int CountCharacters(StringRef input);
}