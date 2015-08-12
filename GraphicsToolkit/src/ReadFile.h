#pragma once

#include <cstdio>

#include "Buffer.h"

namespace File
{
	Buffer<unsigned char> Read(const char* path)
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
			fread(fileContents.Data(), sizeof(char), fileLength, fileHandle);
			fclose(fileHandle);
		}

		return fileContents;
	}
}