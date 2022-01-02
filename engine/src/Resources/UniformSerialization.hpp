#pragma once

#include "rapidjson/document.h"

namespace kokko
{

struct BufferUniform;
struct TextureUniform;

class UniformData;

void SerializeUniformToJson(
	const UniformData& uniformData,
	const BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator);

}
