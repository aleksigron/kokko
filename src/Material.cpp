#include "Material.hpp"

#include "rapidjson/document.h"

#include "ResourceManager.hpp"
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

				int variableIndex = -1;
				uint32_t varNameHash = Hash::FNV1a_32(varNameStr, varNameStrLen);

				// Find the index at which there's a variable with the same name
				for (unsigned int j = 0; j < uniformCount; ++j)
				{
					if (shader->materialUniforms[j].nameHash == varNameHash)
					{
						variableIndex = j;
						break;
					}
				}

				// The variable was found
				if (variableIndex >= 0)
				{
					// Now let's try to read the value

					ShaderMaterialUniform& u = uniforms[variableIndex];

					const Value& varVal = var["value"];

					switch (u.type)
					{
					case ShaderUniformType::Vec3:
						Vec3f v3 = ValueSerialization::Deserialize_Vec3f(varVal);
						this->SetUniformValueByIndex(variableIndex, v3);
						break;
					}
				}
			}
		}

		return true;
	}

	return false;
}







