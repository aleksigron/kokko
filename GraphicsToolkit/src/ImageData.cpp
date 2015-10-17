#include "ImageData.hpp"

#include <cstdint>
#include <cstring>

#include "File.hpp"

bool ImageData::LoadGlraw(const char *filePath)
{
	this->dataBuffer = File::Read(filePath);

	if (this->dataBuffer.IsValid())
	{
		unsigned char* d = this->dataBuffer.Data();

		// File is probably a glraw file
		if (*reinterpret_cast<uint16_t*>(d) == 0xC6F5)
		{
			const uint64_t preHeaderSize = sizeof(uint16_t) + sizeof(uint64_t);
			const uint64_t dataOffset = *reinterpret_cast<uint64_t*>(d + sizeof(uint16_t));
			const uint64_t headerSize = preHeaderSize + dataOffset;
			unsigned char* const imageData = d + dataOffset;

			unsigned char* it = d + preHeaderSize;

			// One loop iteration reads one property
			while (it < imageData)
			{
				unsigned char* inIt = it;
				uint8_t type = *inIt;
				inIt += 1;

				if (type == 1) // 32-bit integer
				{
					char* propertyKey = reinterpret_cast<char*>(inIt);
					unsigned long propertyKeyLen = strlen(propertyKey);

					// Move iterator to data starting point
					inIt += propertyKeyLen + 1;

					int32_t* propertyValue = reinterpret_cast<int32_t*>(inIt);

					if (strcmp(propertyKey, "height") == 0)
						imageSize.y = *propertyValue;
					else if (strcmp(propertyKey, "width") == 0)
						imageSize.x = *propertyValue;
					else if (strcmp(propertyKey, "compressedFormat") == 0)
					{
						pixelFormat = *propertyValue;
						compressed = true;
					}
					else if (strcmp(propertyKey, "size") == 0)
					{
						compressedSize = *propertyValue;
					}
					else if (strcmp(propertyKey, "format") == 0)
					{
						pixelFormat = *propertyValue;
						compressed = false;
					}
					else if (strcmp(propertyKey, "type") == 0)
						componentDataType = *propertyValue;

					inIt += sizeof(int32_t);
				}

				it = inIt;
			}

			this->imageData = imageData;
			this->imageDataSize = dataBuffer.Count() - headerSize;

			return true;
		}
	}

	return false;
}
