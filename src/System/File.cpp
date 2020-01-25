#include "System/File.hpp"

#include <cstdio>
#include <cstring>

#include "Core/String.hpp"

Buffer<unsigned char> File::ReadBinary(StringRef path)
{
	return ReadBinary(String(path).GetCStr());
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
	return ReadText(String(path).GetCStr());
}

Buffer<char> File::ReadText(const char* path)
{
	Buffer<char> fileContents;

	FILE* fileHandle = std::fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		unsigned long fileLength = std::ftell(fileHandle);
		std::rewind(fileHandle);

		// Get the file contents
		fileContents.Allocate(fileLength + 1);

		if (fileContents.IsValid())
		{
			std::fread(fileContents.Data(), 1, fileLength, fileHandle);
			std::fclose(fileHandle);

			// Null-terminate so it can be used as a c-string
			fileContents[fileLength] = '\0';
		}
	}

	return fileContents;
}

bool File::Write(StringRef path, BufferRef<char> content, bool append)
{
	return Write(String(path).GetCStr(), content, append);
}

bool File::Write(const char* path, BufferRef<char> content, bool append)
{
	FILE* fileHandle = fopen(path, append ? "ab" : "wb");

	if (fileHandle != nullptr)
	{
		size_t written = fwrite(content.data, sizeof(char), content.count, fileHandle);
		fclose(fileHandle);

		return written == content.count;
	}

	return false;
}
