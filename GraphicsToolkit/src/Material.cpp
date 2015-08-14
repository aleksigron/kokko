#include "Material.h"

#include "ShaderProgram.h"

static const unsigned int UniformDataSize[] = {
	4, // Texture2D
	64, // Mat4x4
	12, // Vec3
	8, // Vec2
	4 // Float
};

void Material::SetShader(const ShaderProgram& shader)
{
	if (this->uniformData != nullptr) // Release old uniform data
	{
		delete[] this->uniformData;
		this->uniformData = nullptr;
	}

	this->shader = shader.id;

	// Copy uniform information
	this->uniformCount = shader.materialUniformCount;
	this->usedUniformData = 0;

	for (unsigned i = 0; i < uniformCount; ++i)
	{
		uniforms[i].location = shader.materialUniforms[i].location;
		uniforms[i].type = shader.materialUniforms[i].type;
		uniforms[i].dataOffset = usedUniformData;

		// Increment the amount of data the uniforms have used
		int typeIndex = static_cast<int>(shader.materialUniforms[i].type);
		usedUniformData += UniformDataSize[typeIndex];
	}

	if (usedUniformData > 0)
	{
		this->uniformData = new unsigned char[usedUniformData];
	}
}