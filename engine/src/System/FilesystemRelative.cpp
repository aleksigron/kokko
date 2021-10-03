#include "System/FilesystemRelative.hpp"

FilesystemRelative::FilesystemRelative(Allocator* allocator) :
	pathStore(allocator),
	basePathLength(0)
{
	pathStore.Reserve(512);
}

void FilesystemRelative::SetBasePath(StringRef path)
{
	pathStore.Assign(path);
	
	if (pathStore.GetLength() > 0)
	{
		char lastChar = pathStore[pathStore.GetLength() - 1];
		if (lastChar != '/' && lastChar != '\\')
			pathStore.Append('/');
	}

	basePathLength = pathStore.GetLength();
}

bool FilesystemRelative::ReadBinary(const char* path, Array<uint8_t>& output)
{
	pathStore.Append(path);
	bool result = defaultFs.ReadBinary(pathStore.GetCStr(), output);
	pathStore.Resize(basePathLength);

	return result;
}

bool FilesystemRelative::ReadText(const char* path, String& output)
{
	pathStore.Append(path);
	bool result = defaultFs.ReadText(pathStore.GetCStr(), output);
	pathStore.Resize(basePathLength);

	return result;
}

bool FilesystemRelative::ReadText(const char* path, Allocator* allocator, char*& strOut, size_t& lenOut)
{
	pathStore.Append(path);
	bool result = defaultFs.ReadText(pathStore.GetCStr(), allocator, strOut, lenOut);
	pathStore.Resize(basePathLength);

	return result;
}

bool FilesystemRelative::Write(const char* path, ArrayView<char> content, bool append)
{
	pathStore.Append(path);
	bool result = defaultFs.Write(pathStore.GetCStr(), content, append);
	pathStore.Resize(basePathLength);

	return result;
}
