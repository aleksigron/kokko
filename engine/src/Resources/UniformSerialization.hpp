#pragma once

#include "rapidjson/document.h"

#include "Core/Array.hpp"

namespace kokko
{

struct BufferUniform;

class UniformData;

void SerializeUniformToJson(
	const UniformData& uniformData,
	const BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator);

void DeserializeUniformFromJson(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	Array<unsigned char>& cacheBuffer);

}
