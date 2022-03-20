#include "System/Filesystem.hpp"

#include <cstdio>

#include "Core/Core.hpp"

#include "System/FilesystemResolver.hpp"

namespace kokko
{

Filesystem::Filesystem(Allocator* allocator) :
	resolver(nullptr),
	pathStore(allocator)
{
}

Filesystem::Filesystem(Allocator* allocator, kokko::FilesystemResolver* resolver) :
	resolver(resolver),
	pathStore(allocator)
{
}

void Filesystem::SetResolver(kokko::FilesystemResolver* resolver)
{
	this->resolver = resolver;
}

bool Filesystem::ReadBinary(const char* path, Array<uint8_t>& output)
{
	if (resolver->ResolvePath(path, pathStore))
	{
		bool result = ReadBinaryInternal(pathStore.GetCStr(), output);
		pathStore.Clear();
		return result;
	}
	else
	{
		return ReadBinaryInternal(path, output);
	}
}

bool Filesystem::ReadText(const char* path, kokko::String& output)
{
	if (resolver->ResolvePath(path, pathStore))
	{
		bool result = ReadTextInternal(pathStore.GetCStr(), output);
		pathStore.Clear();
		return result;
	}
	else
	{
		return ReadTextInternal(path, output);
	}
}

bool Filesystem::Write(const char* path, ArrayView<const uint8_t> content, bool append)
{
	if (resolver->ResolvePath(path, pathStore))
	{
		bool result = WriteInternal(pathStore.GetCStr(), content, append);
		pathStore.Clear();
		return result;
	}
	else
	{
		return WriteInternal(path, content, append);
	}
}

bool Filesystem::ReadBinaryInternal(const char* path, Array<uint8_t>& output)
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

bool Filesystem::ReadTextInternal(const char* path, kokko::String& output)
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

bool Filesystem::WriteInternal(const char* path, ArrayView<const uint8_t> content, bool append)
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

} // namespace kokko
