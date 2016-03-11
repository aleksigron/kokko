#pragma once

#include "ShaderProgram.hpp"
#include "JsonReader.hpp"

#include <cstring>

class ShaderConfigReader
{
private:
	StringRef currentKey;

	StringRef vertexShader;
	StringRef fragmentShader;

	StringRef uniformNames[ShaderProgram::MaxMaterialUniforms];
	ShaderUniformType uniformTypes[ShaderProgram::MaxMaterialUniforms];
	unsigned int uniformCount = 0;

public:
	bool Read(JsonReader& reader, ShaderProgram& shader)
	{
		JsonReader::CallbackInfo cb;
		cb.userData = this;
		cb.readString = reinterpret_cast<JsonReader::ReadStringFn>(rs);
		cb.closeObject = reinterpret_cast<JsonReader::OpenCloseFn>(co);
		cb.openArray = reinterpret_cast<JsonReader::OpenCloseFn>(oa);
		cb.closeArray = reinterpret_cast<JsonReader::OpenCloseFn>(ca);

		reader.Parse(cb);

		if (vertexShader.IsValid() && fragmentShader.IsValid())
		{
			char* vs = new char[vertexShader.len + 1];
			for (unsigned i = 0; i < vertexShader.len; ++i)
				vs[i] = vertexShader.str[i];
			vs[vertexShader.len] = '\0';

			char* fs = new char[fragmentShader.len + 1];
			for (unsigned i = 0; i < fragmentShader.len; ++i)
				fs[i] = fragmentShader.str[i];
			fs[fragmentShader.len] = '\0';

			if (shader.Load(vs, fs))
			{
				shader.AddMaterialUniforms(uniformCount, uniformTypes, uniformNames);

				return true;
			}
		}
		
		return false;
	}

	static void co(const StringRef& k, ShaderConfigReader* self)
	{
		if (self->currentKey.ValueEquals("materialUniforms"))
			self->uniformCount++;
	}

	static void oa(const StringRef& k, ShaderConfigReader* self)
	{
		if (k.IsValid())
			self->currentKey = k;
	}

	static void ca(const StringRef& k, ShaderConfigReader* self)
	{
		if (k.ReferenceEquals(self->currentKey))
			self->currentKey.Invalidate();
	}

	static void rs(const StringRef& k, const StringRef& v, ShaderConfigReader* self)
	{
		if (self->currentKey.IsValid() == false)
		{
			if (k.ValueEquals("vertexShaderFile"))
				self->vertexShader = v;
			else if (k.ValueEquals("fragmentShaderFile"))
				self->fragmentShader = v;
		}
		else if (self->currentKey.ValueEquals("materialUniforms"))
		{
			if (k.ValueEquals("name"))
			{
				self->uniformNames[self->uniformCount] = v;
			}
			else if (k.ValueEquals("type"))
			{
				for (unsigned i = 0; i < ShaderUniform::TypeCount; ++i)
				{
					// Check what type of uniform this is
					if (v.ValueEquals(ShaderUniform::TypeNames[i]))
					{
						self->uniformTypes[self->uniformCount] = static_cast<ShaderUniformType>(i);
						break;
					}
				}
			}
		}
	}
};
