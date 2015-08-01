#include "ImageData.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

void ImageData::LoadTestData()
{
	size.x = 64;
	size.y = 64;

	int pixels = size.x * size.y;
	dataSize = pixels * 4;
	hasAlpha = true;
	data = new unsigned char[dataSize];

	for (int i = 0; i < pixels; ++i)
	{
		*(data + i*4 + 0) = (i * 3 + 91) % 251;
		*(data + i*4 + 1) = (i * 7 + 59) % 239;
		*(data + i*4 + 2) = (i * 5 + 13) % 251;
		*(data + i*4 + 3) = 255;
	}
}

void ImageData::LoadTga(const char* filePath)
{
	static const uint8_t uncompressed[12] =
	{ 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	static const uint8_t compressed[12] =
	{ 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	static const size_t tgaHeaderSize = 18;

	FILE* fileHandle = fopen(filePath, "rb");
	if (fileHandle != nullptr)
	{
		fseek(fileHandle, 0L, SEEK_END);
		size_t fileSize = ftell(fileHandle);
		rewind(fileHandle);

		if (fileSize > tgaHeaderSize)
		{
			uint8_t header[tgaHeaderSize];

			if (fread(header, 2, tgaHeaderSize / 2, fileHandle) > 0)
			{
				uint8_t bitsPerPixel = header[16];
				size.x = header[13] * 0xFF + header[12];
				size.y = header[15] * 0xFF + header[14];

				dataSize = (bitsPerPixel / 8) * size.x * size.x;
				data = new uint8_t[dataSize];

				// Header indicates that this file is not compressed
				if (memcmp(uncompressed, &header, sizeof(uncompressed)) == 0)
				{
					size_t elemSize = 1;
					if (dataSize % 4 == 0)
					{
						elemSize = 4;
						hasAlpha = true;
					}
					else if (dataSize % 3 == 0)
					{
						elemSize = 3;
						hasAlpha = false;
					}

					fread(data, elemSize, dataSize / elemSize, fileHandle);
				}
				// Header indicates that this file is compressed
				else if (memcmp(compressed, &header, sizeof(compressed)) == 0)
				{
					uint32_t pixel = 0;
					size_t byteIndex = 0;
					int pixelIndex = 0;
					uint8_t chunk = 0;
					size_t bytesPerPixel = (bitsPerPixel / 8);

					// No alpha channel
					if (bitsPerPixel == 24)
					{
						hasAlpha = false;

						do
						{
							fread(&chunk, 1, sizeof(chunk), fileHandle);

							if (chunk < 128)
							{
								++chunk;

								for (int i = 0; i < chunk; ++i, ++pixelIndex)
								{
									fread(&pixel, 1, bytesPerPixel, fileHandle);

									data[byteIndex++] = pixel << 16 & 0xFF;
									data[byteIndex++] = pixel << 8 & 0xFF;
									data[byteIndex++] = pixel & 0xFF;
								}
							}
							else
							{
								chunk -= 127;
								fread(&pixel, 1, bytesPerPixel, fileHandle);

								for (int i = 0; i < chunk; ++i, ++pixelIndex)
								{
									data[byteIndex++] = pixel << 16 & 0xFF;
									data[byteIndex++] = pixel << 8 & 0xFF;
									data[byteIndex++] = pixel & 0xFF;
								}
							}
						}
						while (pixelIndex < size.x * size.y);
					}
					// Has alpha channel
					else if (bitsPerPixel == 32)
					{
						hasAlpha = true;

						do
						{
							fread(&chunk, 1, sizeof(chunk), fileHandle);

							if (chunk < 128)
							{
								++chunk;

								for (int i = 0; i < chunk; ++i, ++pixelIndex)
								{
									fread(&pixel, 1, bytesPerPixel, fileHandle);

									data[byteIndex++] = pixel << 24 & 0xFF;
									data[byteIndex++] = pixel << 16 & 0xFF;
									data[byteIndex++] = pixel << 8 & 0xFF;
									data[byteIndex++] = pixel & 0xFF;
								}
							}
							else
							{
								chunk -= 127;
								fread(&pixel, 1, bytesPerPixel, fileHandle);

								for (int i = 0; i < chunk; ++i, ++pixelIndex)
								{
									data[byteIndex++] = pixel << 24 & 0xFF;
									data[byteIndex++] = pixel << 16 & 0xFF;
									data[byteIndex++] = pixel << 8 & 0xFF;
									data[byteIndex++] = pixel & 0xFF;
								}
							}
						}
						while (pixelIndex < size.x * size.y);
					}
				}

				delete[] data;
			}
		}

		fclose(fileHandle);
	}
}