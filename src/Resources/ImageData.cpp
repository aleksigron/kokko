#include "Resources/ImageData.hpp"

ImageData::ImageData() :
	imageData(nullptr),
	imageDataSize(0),
	imageSize(0, 0),
	pixelFormat(0),
	componentDataType(0),
	compressedSize(0),
	compressed(false)
{
}
