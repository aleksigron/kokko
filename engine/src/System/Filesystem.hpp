#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/String.hpp"

namespace kokko
{
class Allocator;
class FilesystemResolver;

class Filesystem
{
public:
	// Creates a filesystem instance with no path resolver
	// It will use the passed in path as is in read and write functions
	Filesystem();

	// 
	Filesystem(Allocator* allocator, kokko::FilesystemResolver* resolver);

	// Allocator is also needed for temporary 
	void SetResolver(Allocator* allocator, kokko::FilesystemResolver* resolver);

	bool ReadBinary(const char* path, Array<uint8_t>& output);
	bool ReadText(const char* path, kokko::String& output);
	bool Write(const char* path, ArrayView<const uint8_t> content, bool append);

private:
	bool ReadBinaryInternal(const char* path, Array<uint8_t>& output);
	bool ReadTextInternal(const char* path, kokko::String& output);
	bool WriteInternal(const char* path, ArrayView<const uint8_t> content, bool append);

private:
	kokko::FilesystemResolver* resolver;
	kokko::String pathStore;
};

} // namespace kokko
