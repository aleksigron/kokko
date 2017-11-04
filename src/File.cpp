#include "File.hpp"

#include <cstdio>
#include <cstring>

Buffer<unsigned char> File::ReadBinary(StringRef path)
{
	Buffer<char> pathNullTerminated;
	pathNullTerminated.Allocate(path.len + 1);

	char* pathCstr = pathNullTerminated.Data();
	std::memcpy(pathCstr, path.str, path.len);
	pathCstr[path.len] = '\0';

	return ReadBinary(pathCstr);
}

Buffer<unsigned char> File::ReadBinary(const char* path)
{
	Buffer<unsigned char> fileContents;

	FILE* fileHandle = fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		fseek(fileHandle, 0L, SEEK_END);
		unsigned long fileLength = ftell(fileHandle);
		rewind(fileHandle);

		// Get the file contents
		fileContents.Allocate(fileLength);
		fread(fileContents.Data(), 1, fileLength, fileHandle);
		fclose(fileHandle);
	}

	return fileContents;
}

Buffer<char> File::ReadText(StringRef path)
{
	Buffer<char> pathNullTerminated;
	pathNullTerminated.Allocate(path.len + 1);
	
	char* pathCstr = pathNullTerminated.Data();
	std::memcpy(pathCstr, path.str, path.len);
	pathCstr[path.len] = '\0';

	return ReadText(pathCstr);
}

Buffer<char> File::ReadText(const char* path)
{
	Buffer<char> fileContents;

	FILE* fileHandle = fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		fseek(fileHandle, 0L, SEEK_END);
		unsigned long fileLength = ftell(fileHandle);
		rewind(fileHandle);

		// Get the file contents
		fileContents.Allocate(fileLength + 1);
		fread(fileContents.Data(), 1, fileLength, fileHandle);
		fclose(fileHandle);

		// Null-terminate so it can be used in places expecting a c-string
		fileContents[fileLength] = '\0';
	}

	return fileContents;
}
