#pragma once

class Allocator;

struct StringRef;

#include "Core/String.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemDefault.hpp"

class FilesystemRelative : public Filesystem
{
public:
	FilesystemRelative(Allocator* allocator);
	virtual ~FilesystemRelative() = default;

	void SetBasePath(StringRef path);

	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) override;
	virtual bool ReadText(const char* path, kokko::String& output) override;
	virtual bool Write(const char* path, ArrayView<const uint8_t> content, bool append) override;

private:
	kokko::String pathStore;
	size_t basePathLength;
	FilesystemDefault defaultFs;
};
