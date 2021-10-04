#pragma once

#include "System/Filesystem.hpp"

class FilesystemDefault : public Filesystem
{
public:
	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) override;
	virtual bool ReadText(const char* path, String& output) override;
	virtual bool Write(const char* path, ArrayView<char> content, bool append) override;
};
