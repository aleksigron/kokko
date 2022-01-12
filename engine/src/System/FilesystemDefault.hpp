#pragma once

#include "System/Filesystem.hpp"

class FilesystemDefault : public Filesystem
{
public:
	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) override;
	virtual bool ReadText(const char* path, kokko::String& output) override;
	virtual bool Write(const char* path, ArrayView<const uint8_t> content, bool append) override;
};
