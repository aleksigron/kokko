#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/StringRef.hpp"

namespace EncodingUtf8
{
	/**
	 * Encodes a codepoint and writes at most 4 bytes to the buffer utf8BytesOut.
	 * Returns how many bytes were encoded, 0 if codepoint is invalid.
	 */
	size_t EncodeCodepoint(uint32_t codepoint, char* utf8BytesOut);

	/**
	 * Decodes a codepoint to codepointOut.
	 * Returns how many bytes were decoded, 0 if input doesn't point to the first
	 * byte of a UTF-8 character.
	 */
	size_t DecodeCodepoint(const char* input, uint32_t& codepointOut);

	/**
	 * Counts how many unicode characters are in a UTF-8 encoded string.
	 */
	size_t CountCharacters(StringRef input);

	/**
	 * Returns the byte index for the first byte of the last character in a UTF-8
	 * encoded string. Returns input.len if no valid characters are present.
	 */
	size_t FindLastCharacter(StringRef input);
}
