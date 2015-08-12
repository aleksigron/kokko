#pragma once

#include "ShaderProgram.h"
#include "JsonReader.h"

#include <cstring>

class ShaderConfigReader
{
private:
	StringRef currentKey;
	StringRef vertexShader;
	StringRef fragmentShader;

public:
	void Read(JsonReader& reader, ShaderProgram& shader)
	{
		JsonReader::CallbackInfo cb;
		cb.userData = this;
		cb.readString = reinterpret_cast<JsonReader::ReadStringFn>(rs);

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

			shader.Load(vs, fs);
		}
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
	}
};