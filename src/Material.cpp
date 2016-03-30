#include "Material.hpp"

#include "rapidjson/document.h"

#include "ResourceManager.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "StringRef.hpp"
#include "File.hpp"
#include "Hash.hpp"

#include "ValueSerialization.hpp"

void Material::SetShader(const Shader* shader)
{
	if (this->uniformData != nullptr) // Release old uniform data
	{
		delete[] this->uniformData;
		this->uniformData = nullptr;
	}

	this->shaderId = shader->nameHash;

	// Copy uniform information
	this->uniformCount = shader->materialUniformCount;
	this->usedUniformData = 0;

	for (unsigned i = 0; i < uniformCount; ++i)
	{
		const ShaderUniform* mu = shader->materialUniforms + i;

		uniforms[i].location = mu->location;
		uniforms[i].nameHash = mu->nameHash;
		uniforms[i].type = mu->type;
		uniforms[i].dataOffset = usedUniformData;

		// Increment the amount of data the uniforms have used
		int typeIndex = static_cast<int>(mu->type);
		usedUniformData += ShaderUniform::TypeSizes[typeIndex];
	}

	if (usedUniformData > 0)
	{
		this->uniformData = new unsigned char[usedUniformData];

		// TODO: Set default values for uniforms
	}
}

bool Material::LoadFromConfiguration(Buffer<char>& configuration, ResourceManager* res)
{
	using namespace rapidjson;

	Document config;
	config.ParseInsitu(configuration.Data());

	assert(config.HasMember("shader"));

	const char* shaderName = config["shader"].GetString();
	Shader* shader = res->GetShader(shaderName);

	if (shader != nullptr)
	{
		// This initializes material uniforms from the shader's data
		this->SetShader(shader);

		Value::ConstMemberIterator variablesItr = config.FindMember("variables");

		if (variablesItr != config.MemberEnd())
		{
			const Value& vars = variablesItr->value;

			for (unsigned int i = 0, count = vars.Size(); i < count; ++i)
			{
				const Value& var = vars[i];

				assert(var.IsObject());
				assert(var.HasMember("name"));
				assert(var.HasMember("value"));

				const Value& varName = var["name"];
				const char* varNameStr = varName.GetString();
				unsigned int varNameStrLen = varName.GetStringLength();

				int varIndex = -1;
				uint32_t varNameHash = Hash::FNV1a_32(varNameStr, varNameStrLen);

				// Find the index at which there's a variable with the same name
				for (unsigned int j = 0; j < uniformCount; ++j)
				{
					if (shader->materialUniforms[j].nameHash == varNameHash)
					{
						varIndex = j;
						break;
					}
				}

				// The variable was found
				if (varIndex >= 0)
				{
					// Now let's try to read the value

					ShaderUniformType type = uniforms[varIndex].type;

					const Value& varVal = var["value"];

					using namespace ValueSerialization;

					switch (type)
					{
						case ShaderUniformType::Vec4:
							this->SetUniformValueByIndex(varIndex, Deserialize_Vec4f(varVal));
							break;

						case ShaderUniformType::Vec2:
							this->SetUniformValueByIndex(varIndex, Deserialize_Vec2f(varVal));
							break;

						case ShaderUniformType::Vec3:
							this->SetUniformValueByIndex(varIndex, Deserialize_Vec3f(varVal));
							break;

						case ShaderUniformType::Float:
							this->SetUniformValueByIndex(varIndex, Deserialize_Float(varVal));
							break;

						case ShaderUniformType::Int:
							this->SetUniformValueByIndex(varIndex, Deserialize_Int(varVal));
							break;

						case ShaderUniformType::Mat4x4:
							this->SetUniformValueByIndex(varIndex, Deserialize_Mat4x4f(varVal));
							break;

						case ShaderUniformType::Tex2D:
						{
							const char* texturePath = varVal.GetString();
							Texture* texture = res->GetTexture(texturePath);
							if (texture != nullptr)
								this->SetUniformValueByIndex(varIndex, texture->nameHash);
						}
							break;
					}
				}
			}
		}

		return true;
	}

	return false;
}







