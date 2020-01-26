#include "System/File.hpp"

#include <cstdio>
#include <cstring>

bool File::ReadBinary(const char* path, Buffer<unsigned char>& output)
{
	FILE* fileHandle = fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		unsigned long fileLength = ftell(fileHandle);
		std::rewind(fileHandle);

		// Get the file contents
		output.Allocate(fileLength);
		std::fread(output.Data(), 1, fileLength, fileHandle);
		std::fclose(fileHandle);

		return true;
	}

	return false;
}

bool File::ReadText(const char* path, Buffer<char>& output)
{
	FILE* fileHandle = std::fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		unsigned long fileLength = std::ftell(fileHandle);
		std::rewind(fileHandle);

		// Get the file contents
		output.Allocate(fileLength + 1);

		std::fread(output.Data(), 1, fileLength, fileHandle);
		std::fclose(fileHandle);

		// Null-terminate so it can be used as a c-string
		output[fileLength] = '\0';

		return true;
	}

	return false;
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
