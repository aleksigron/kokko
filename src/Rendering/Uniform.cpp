#include "Rendering/Uniform.hpp"

#include "Rendering/UniformBufferData.hpp"

const UniformTypeInfo UniformTypeInfo::Types[] = {

	UniformTypeInfo{ 4, 4, 0, "", true }, // Texture2D
	UniformTypeInfo{ 4, 4, 0, "", true }, // TextureCube
	UniformTypeInfo{ 64, 16, 6, "mat4x4", false },
	UniformTypeInfo{ 16, 16, 4, "vec4", false },
	UniformTypeInfo{ 16, 16, 4, "vec3", false },
	UniformTypeInfo{ 8, 8, 4, "vec2", false },
	UniformTypeInfo{ 4, 4, 5, "float", false },
	UniformTypeInfo{ 4, 4, 3, "int", false }
};

void BufferUniform::UpdateToUniformBuffer(unsigned char* dataBuffer, unsigned char* uniformBuffer)
{
	switch (type)
	{
	case UniformDataType::Tex2D:
	case UniformDataType::TexCube:
		break;

	case UniformDataType::Mat4x4:
	{
		const Mat4x4f& val = GetValue<Mat4x4f>(dataBuffer);
		UniformBuffer::SetScalarMat4x4f(uniformBuffer, bufferObjectOffset, val);
		break;
	}
	case UniformDataType::Vec4:
	{
		Vec4f* val = reinterpret_cast<Vec4f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec4f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Vec3:
	{
		Vec3f* val = reinterpret_cast<Vec3f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec3f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Vec2:
	{
		Vec2f* val = reinterpret_cast<Vec2f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec2f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Float:
	{
		float* val = reinterpret_cast<float*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarFloat(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Int:
	{
		int* val = reinterpret_cast<int*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarInt(uniformBuffer, bufferObjectOffset, *val);
		break;
	}

	default:
		break;
	}
}