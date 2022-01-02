#include "Resources/MaterialSerializer.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/Uniform.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/ValueSerialization.hpp"
#include "Resources/UniformSerialization.hpp"

namespace {

const rapidjson::Value* FindVariableValue(const rapidjson::Value& variablesArray, const StringRef& name)
{
	if (variablesArray.IsArray())
	{
		rapidjson::Value::ConstValueIterator varItr = variablesArray.Begin();
		rapidjson::Value::ConstValueIterator varEnd = variablesArray.End();

		for (; varItr < varEnd; ++varItr)
		{
			rapidjson::Value::ConstMemberIterator nameItr = varItr->FindMember("name");

			if (nameItr != varItr->MemberEnd() &&
				nameItr->value.IsString() &&
				StringRef(nameItr->value.GetString(), nameItr->value.GetStringLength()).ValueEquals(name))
			{
				rapidjson::Value::ConstMemberIterator valueItr = varItr->FindMember("value");
				if (valueItr != varItr->MemberEnd())
					return &valueItr->value;
				else
					return nullptr;
			}
		}
	}

	return nullptr;
}

unsigned int PrepareUniformArray(const rapidjson::Value* jsonValue, unsigned int uniformArraySize)
{
	unsigned int valueCount = 0;

	if (jsonValue == nullptr)
		KK_LOG_WARN("Failed to read uniform, because JSON value does not exist.");
	else if (jsonValue->IsArray() == false)
		KK_LOG_WARN("Failed to read uniform, because JSON value was not an array.");
	else
	{
		valueCount = jsonValue->Size();

		if (valueCount > uniformArraySize)
		{
			KK_LOG_WARN("JSON array was longer than shader array");
			valueCount = uniformArraySize;
		}
	}

	return valueCount;
}

void SetBufferUniformValue(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	Array<unsigned char>& cacheBuffer)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;

	switch (uniform.type)
	{
	case kokko::UniformDataType::Mat4x4:
	{
		Mat4x4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat4x4f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Mat4x4Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Mat4x4f));
		Mat4x4f* buffer = reinterpret_cast<Mat4x4f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat4x4f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Mat3x3:
	{
		Mat3x3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat3x3f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Mat3x3Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Mat3x3f));
		Mat3x3f* buffer = reinterpret_cast<Mat3x3f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat3x3f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec4:
	{
		Vec4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec4f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec4Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec4f));
		Vec4f* buffer = reinterpret_cast<Vec4f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec4f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec3:
	{
		Vec3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec3f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec3Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec3f));
		Vec3f* buffer = reinterpret_cast<Vec3f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec3f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec2:
	{
		Vec2f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec2f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec2Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec2f));
		Vec2f* buffer = reinterpret_cast<Vec2f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec2f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Float:
	{
		float val = 0.0f;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Float(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::FloatArray:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(float));
		float* buffer = reinterpret_cast<float*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Float(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Int:
	{
		int val = 0;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Int(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::IntArray:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(int));
		int* buffer = reinterpret_cast<int*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Int(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	default:
		break;
	}
}

} // Anonymous namespace

namespace kokko
{

MaterialSerializer::MaterialSerializer(
	Allocator* allocator,
	MaterialManager* materialManager,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	materialManager(materialManager),
	shaderManager(shaderManager),
	textureManager(textureManager)
{
}

MaterialSerializer::~MaterialSerializer()
{
}

bool MaterialSerializer::DeserializeMaterial(MaterialId id, StringRef config)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	rapidjson::Document doc;
	doc.Parse(config.str, config.len);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return false;

	StringRef path(shaderItr->value.GetString(), shaderItr->value.GetStringLength());
	ShaderId shaderId = shaderManager->GetIdByPath(path);

	if (shaderId == ShaderId::Null)
		return false;

	// This initializes material uniforms from the shader's data
	materialManager->SetMaterialShader(id, shaderId);

	MaterialManager::MaterialData& material = materialManager->data.material[id.i];
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	MemberItr variablesItr = doc.FindMember("variables");
	const rapidjson::Value* varValue = nullptr;
	bool variablesArrayIsValid = variablesItr != doc.MemberEnd() && variablesItr->value.IsArray();

	Array<unsigned char> uniformScratchBuffer(allocator);

	for (auto& uniform : material.uniformData.GetBufferUniforms())
	{
		if (variablesArrayIsValid)
			varValue = FindVariableValue(variablesItr->value, uniform.name);
		else
			varValue = nullptr;

		SetBufferUniformValue(material.uniformData, uniform, varValue, uniformScratchBuffer);
	}

	// TEXTURE UNIFORMS

	for (auto& uniform : material.uniformData.GetTextureUniforms())
	{
		TextureId textureId = TextureId::Null;

		if (variablesArrayIsValid &&
			(varValue = FindVariableValue(variablesItr->value, uniform.name)) != nullptr &&
			varValue->IsString())
		{
			auto uidParseResult = Uid::FromString(ArrayView(varValue->GetString(), varValue->GetStringLength()));

			if (uidParseResult.HasValue())
				textureId = textureManager->FindTextureByUid(uidParseResult.GetValue());
		}

		if (textureId == TextureId::Null)
		{
			// TODO: Find a more robust solution to find default values for textures
			if (uniform.name.StartsWith(StringRef("normal")))
				textureId = textureManager->GetId_EmptyNormal();
			else
				textureId = textureManager->GetId_White2D();
		}

		uniform.textureId = textureId;

		const TextureData& texture = textureManager->GetTextureData(textureId);
		assert(texture.textureObjectId != 0);
		uniform.textureObject = texture.textureObjectId;
	}

	materialManager->UpdateUniformsToGPU(id);

	return true;
}

void MaterialSerializer::SerializeToString(MaterialId id, String& out)
{
	KOKKO_PROFILE_FUNCTION();

	char uidStrBuffer[Uid::StringLength];

	rapidjson::Document doc;
	auto& alloc = doc.GetAllocator();
	
	ShaderId shaderId = materialManager->GetMaterialShader(id);
	const UniformData& uniforms = materialManager->GetMaterialUniforms(id);
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	rapidjson::Value shaderPath(shader.path.str, shader.path.len);

	rapidjson::Value variables(rapidjson::kArrayType);

	for (const auto& bufferUniform : uniforms.GetBufferUniforms())
	{
		rapidjson::Value uniformValue(rapidjson::kObjectType);

		rapidjson::Value name(bufferUniform.name.str, bufferUniform.name.len);
		uniformValue.AddMember("name", name, alloc);

		rapidjson::Value value;
		SerializeUniformToJson(uniforms, bufferUniform, value, alloc);
		uniformValue.AddMember("value", value, alloc);

		variables.PushBack(uniformValue, alloc);
	}

	for (const auto& textureUniform : uniforms.GetTextureUniforms())
	{
		rapidjson::Value uniformValue(rapidjson::kObjectType);

		rapidjson::Value name(textureUniform.name.str, textureUniform.name.len);
		uniformValue.AddMember("name", name, alloc);

		const TextureData& texture = textureManager->GetTextureData(textureUniform.textureId);
		texture.uid.WriteTo(uidStrBuffer);
		rapidjson::Value value(uidStrBuffer, sizeof(uidStrBuffer), alloc);
		uniformValue.AddMember("value", value, alloc);

		variables.PushBack(uniformValue, alloc);
	}

	doc.SetObject();
	doc.AddMember("shader", shaderPath, alloc);
	doc.AddMember("variables", variables, alloc);

	rapidjson::StringBuffer jsonStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(jsonStringBuffer);
	doc.Accept(writer);
	
	// TODO: Figure out how to avoid copying the value
	out.Assign(StringRef(jsonStringBuffer.GetString(), jsonStringBuffer.GetLength()));
}

}
