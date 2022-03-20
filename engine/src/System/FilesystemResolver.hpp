#pragma once

namespace kokko
{
class String;

class FilesystemResolver
{
public:
	// Try to resolve a virtual path to real path
	virtual bool ResolvePath(const char* path, String& resultOut) = 0;
};

} // namespace kokko
