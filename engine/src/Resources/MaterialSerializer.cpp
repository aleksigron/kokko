#include "Resources/MaterialSerializer.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "Rendering/Uniform.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/UniformSerialization.hpp"

namespace {

const rapidjson::Value* FindVariableValue(const rapidjson::Value& variablesArray, const ConstStringView& name)
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
				ConstStringView(nameItr->value.GetString(), nameItr->value.GetStringLength()).ValueEquals(name))
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

bool MaterialSerializer::DeserializeMaterial(MaterialId id, ConstStringView config)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	rapidjson::Document doc;
	doc.Parse(config.str, config.len);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return false;

	ShaderId shaderId = ShaderId::Null;

	auto uidResult = Uid::FromString(ArrayView(shaderItr->value.GetString(), shaderItr->value.GetStringLength()));
	if (uidResult.HasValue())
	{
		Uid shaderUid = uidResult.GetValue();
		shaderId = shaderManager->FindShaderByUid(shaderUid);
	}

	// This initializes material uniforms from the shader's data
	materialManager->SetMaterialShader(id, shaderId);

	if (shaderId == ShaderId::Null)
		return false;

	MaterialManager::MaterialData& material = materialManager->data.material[id.i];
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	MemberItr variablesItr = doc.FindMember("variables");
	bool variablesArrayIsValid = variablesItr != doc.MemberEnd() && variablesItr->value.IsArray();

	Array<unsigned char> uniformScratchBuffer(allocator);

	for (auto& uniform : material.uniformData.GetBufferUniforms())
	{
		const rapidjson::Value* varValue = nullptr;
		if (variablesArrayIsValid)
			varValue = FindVariableValue(variablesItr->value, uniform.name);
		else
			varValue = nullptr;

		DeserializeUniformFromJson(material.uniformData, uniform, varValue, uniformScratchBuffer);
	}

	// TEXTURE UNIFORMS

	for (auto& uniform : material.uniformData.GetTextureUniforms())
	{
		// TODO: Find a more robust solution to find default values for textures
		bool isNormalTexture = uniform.name.StartsWith(ConstStringView("normal"));

		TextureId textureId = TextureId::Null;

		const rapidjson::Value* varValue = nullptr;

		if (variablesArrayIsValid &&
			(varValue = FindVariableValue(variablesItr->value, uniform.name)) != nullptr &&
			varValue->IsString())
		{
			auto uidParseResult = Uid::FromString(ArrayView(varValue->GetString(), varValue->GetStringLength()));

			if (uidParseResult.HasValue())
				textureId = textureManager->FindTextureByUid(uidParseResult.GetValue(), isNormalTexture);
		}

		if (textureId == TextureId::Null)
		{
			if (isNormalTexture)
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

	char shaderUid[Uid::StringLength];
	shader.uid.WriteTo(ArrayView(shaderUid));
	rapidjson::Value shaderUidValue(shaderUid, Uid::StringLength);

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
	doc.AddMember("shader", shaderUidValue, alloc);
	doc.AddMember("variables", variables, alloc);

	rapidjson::StringBuffer jsonStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(jsonStringBuffer);
	doc.Accept(writer);
	
	// TODO: Figure out how to avoid copying the value
	out.Assign(ConstStringView(jsonStringBuffer.GetString(), jsonStringBuffer.GetLength()));
}

}
