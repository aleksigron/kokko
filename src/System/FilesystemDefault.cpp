#include "System/FilesystemDefault.hpp"

#include <cstdio>
#include <cstring>

#include "Core/Core.hpp"

bool FilesystemDefault::ReadBinary(const char* path, Array<unsigned char>& output)
{
	KOKKO_PROFILE_FUNCTION();

	FILE* fileHandle = fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		long fileLength = ftell(fileHandle);
		std::rewind(fileHandle);

		// Get the file contents
		output.Resize(fileLength);
		std::fread(output.GetData(), 1, fileLength, fileHandle);
		std::fclose(fileHandle);

		return true;
	}

	return false;
}

bool FilesystemDefault::ReadText(const char* path, Array<char>& output)
{
	KOKKO_PROFILE_FUNCTION();

	FILE* fileHandle = std::fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		long fileLength = std::ftell(fileHandle);
		std::rewind(fileHandle);

		// Get the file contents
		output.Resize(fileLength + 1);

		std::fread(output.GetData(), 1, fileLength, fileHandle);
		std::fclose(fileHandle);

		// Null-terminate so it can be used as a c-string
		output[fileLength] = '\0';

		return true;
	}

	return false;
}

bool FilesystemDefault::ReadText(const char* path, Allocator* allocator, char*& strOut, size_t& lenOut)
{
	KOKKO_PROFILE_FUNCTION();

	FILE* fileHandle = std::fopen(path, "rb");

	if (fileHandle != nullptr)
	{
		// Find the size of the file
		std::fseek(fileHandle, 0L, SEEK_END);
		long fileLength = std::ftell(fileHandle);
		std::rewind(fileHandle);

		void* buffer = allocator->Allocate(static_cast<size_t>(fileLength) + 1);

		std::fread(buffer, 1, fileLength, fileHandle);
		std::fclose(fileHandle);

		// Null-terminate so it can be used as a c-string
		static_cast<char*>(buffer)[fileLength] = '\0';

		strOut = static_cast<char*>(buffer);
		lenOut = fileLength;

		return true;
	}

	return false;
}

bool FilesystemDefault::Write(const char* path, ArrayView<char> content, bool append)
{
	KOKKO_PROFILE_FUNCTION();

	FILE* fileHandle = fopen(path, append ? "ab" : "wb");

	if (fileHandle != nullptr)
	{
		size_t written = fwrite(content.GetData(), 1, content.GetCount(), fileHandle);
		fclose(fileHandle);

		return written == content.GetCount();
	}

	return false;
}
