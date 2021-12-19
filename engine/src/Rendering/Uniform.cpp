#include "Rendering/Uniform.hpp"

#include <cstring>

namespace kokko
{

UniformTypeInfo::UniformTypeInfo(unsigned int size, unsigned int alignment,
	const char* typeName, bool isArray, bool isTexture) :
	size(size),
	alignment(alignment),
	typeNameLength(0),
	typeName(typeName),
	isArray(isArray),
	isTexture(isTexture)
{
	typeNameLength = static_cast<unsigned int>(std::strlen(typeName));
}

// Must match order with UniformDataType
const UniformTypeInfo UniformTypeInfo::Types[] = {
	UniformTypeInfo(4, 4, "", false, true), // Texture2D
	UniformTypeInfo(4, 4, "", false, true), // TextureCube
	UniformTypeInfo(64, 16, "mat4x4", false, false),
	UniformTypeInfo(64, 16, "mat4x4", true, false),
	UniformTypeInfo(48, 16, "mat3x3", false, false),
	UniformTypeInfo(48, 16, "mat3x3", true, false),
	UniformTypeInfo(16, 16, "vec4", false, false),
	UniformTypeInfo(16, 16, "vec4", true, false),
	UniformTypeInfo(12, 16, "vec3", false, false),
	UniformTypeInfo(12, 16, "vec3", true, false),
	UniformTypeInfo(8, 8, "vec2", false, false),
	UniformTypeInfo(8, 16, "vec2", true, false),
	UniformTypeInfo(4, 4, "float", false, false),
	UniformTypeInfo(4, 16, "float", true, false),
	UniformTypeInfo(4, 4, "int", false, false),
	UniformTypeInfo(4, 16, "int", true, false)
};

}
