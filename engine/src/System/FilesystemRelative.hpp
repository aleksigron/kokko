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
	virtual bool ReadText(const char* path, String& output) override;
	virtual bool WriteText(const char* path, ArrayView<const char> content, bool append) override;

private:
	String pathStore;
	size_t basePathLength;
	FilesystemDefault defaultFs;
};
