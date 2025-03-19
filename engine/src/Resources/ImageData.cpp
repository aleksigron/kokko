#include "Resources/ImageData.hpp"

namespace kokko
{

ImageData::ImageData() :
	imageData(nullptr),
	imageDataSize(0),
	imageSize(0, 0),
	pixelFormat(RenderTextureBaseFormat::RGBA),
	componentDataType(RenderTextureDataType::UnsignedByte),
	compressedSize(0),
	compressed(false)
{
}

} // namespace kokko
