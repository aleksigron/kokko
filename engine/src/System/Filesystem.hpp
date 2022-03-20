#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/String.hpp"

class Allocator;

namespace kokko
{
class FilesystemResolver;

class Filesystem
{
public:
	explicit Filesystem(Allocator* allocator);
	Filesystem(Allocator* allocator, kokko::FilesystemResolver* resolver);

	void SetResolver(kokko::FilesystemResolver* resolver);

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
