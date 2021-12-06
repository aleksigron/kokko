#include "System/FilesystemDefault.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

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
		size_t read = std::fread(output.GetData(), 1, fileLength, fileHandle);
		output.Resize(read);

		std::fclose(fileHandle);

		return true;
	}

	return false;
}

bool FilesystemDefault::ReadText(const char* path, String& output)
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
		output.Resize(fileLength);

		size_t read = std::fread(output.GetData(), 1, fileLength, fileHandle);
		output.Resize(read);

		std::fclose(fileHandle);

		return true;
	}

	return false;
}

bool FilesystemDefault::WriteText(const char* path, ArrayView<const char> content, bool append)
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
