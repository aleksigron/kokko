#include "Rendering/Shader.hpp"

const unsigned int ShaderUniform::TypeSizes[] = {
	4, // Texture2D
	4, // TextureCube
	64, // Mat4x4
	16, // Vec4
	12, // Vec3
	8, // Vec2
	4, // Float
	4 // Int
};

Shader::Shader() :
	nameHash(0),
	driverId(0),
	transparencyType(TransparencyType::Opaque),
	materialUniformCount(0)
{
}
