#pragma once

#include <cstddef>

// Safer alternative to strncpy when not directly copying to an array
// Prefer StringCopySafe when possible.
// Always terminates the string in the destination buffer.
void StringCopyN(char* destination, const char* source, size_t destBufferCount);

// Safe string copy
// Detects destination buffer size by taking a reference to an array
// Always terminates the string in the destination buffer
// Based on https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
template <size_t CharCount>
void StringCopySafe(char(&destination)[CharCount], const char* source)
{
	StringCopyN(destination, source, CharCount);
}

bool StringIsEmpty(const char* str);
void SetEmptyString(char* str);
