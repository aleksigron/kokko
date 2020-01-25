#include "Rendering/Shader.hpp"

#include "System/IncludeOpenGL.hpp"

#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#include "System/File.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Engine/Engine.hpp"
#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"

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
	uniformMatMVP(-1),
	uniformMatMV(-1),
	uniformMatVP(-1),
	uniformMatM(-1),
	uniformMatV(-1),
	uniformMatP(-1),
	transparencyType(TransparencyType::Opaque),
	materialUniformCount(0)
{

}

bool Shader::LoadFromConfiguration(BufferRef<char> configuration)
{
	using MemberItr = rapidjson::Value::ConstMemberIterator;
	using ValueItr = rapidjson::Value::ConstValueIterator;

	const char* uniformNames[Shader::MaxMaterialUniforms];
	ShaderUniformType uniformTypes[Shader::MaxMaterialUniforms];
	unsigned int uniformCount = 0;

	char* data = configuration.data;
	unsigned long size = configuration.count;

	rapidjson::Document config;
	config.Parse(data, size);

	MemberItr renderTypeItr = config.FindMember("transparencyType");

	if (renderTypeItr != config.MemberEnd() && renderTypeItr->value.IsString())
	{
		StringRef renderTypeStr;
		renderTypeStr.str = renderTypeItr->value.GetString();
		renderTypeStr.len = renderTypeItr->value.GetStringLength();

		uint32_t renderTypeHash = Hash::FNV1a_32(renderTypeStr.str, renderTypeStr.len);

		switch (renderTypeHash)
		{
			case "opaque"_hash:
				this->transparencyType = TransparencyType::Opaque;
				break;

			case "alphaTest"_hash:
				this->transparencyType = TransparencyType::AlphaTest;
				break;

			case "skybox"_hash:
				this->transparencyType = TransparencyType::Skybox;
				break;

			case "transparentMix"_hash:
				this->transparencyType = TransparencyType::TransparentMix;
				break;

			case "transparentAdd"_hash:
				this->transparencyType = TransparencyType::TransparentAdd;
				break;

			case "transparentSub"_hash:
				this->transparencyType = TransparencyType::TransparentSub;
				break;
		}
	}

	MemberItr uniformListItr = config.FindMember("materialUniforms");

	if (uniformListItr != config.MemberEnd() && uniformListItr->value.IsArray())
	{
		ValueItr uItr = uniformListItr->value.Begin();
		ValueItr uEnd = uniformListItr->value.End();
		for (; uItr != uEnd; ++uItr)
		{
			MemberItr nameItr = uItr->FindMember("name");
			MemberItr typeItr = uItr->FindMember("type");

			if (nameItr != uItr->MemberEnd() && nameItr->value.IsString() &&
				typeItr != uItr->MemberEnd() && typeItr->value.IsString())
			{
				uniformNames[uniformCount] = nameItr->value.GetString();

				const char* typeStr = typeItr->value.GetString();
				unsigned int typeStrLen = typeItr->value.GetStringLength();

				uint32_t typeHash = Hash::FNV1a_32(typeStr, typeStrLen);

				switch (typeHash)
				{
					case "tex2d"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Tex2D;
						++uniformCount;
						break;

					case "texCube"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::TexCube;
						++uniformCount;
						break;

					case "mat4x4"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Mat4x4;
						++uniformCount;
						break;

					case "vec4"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Vec4;
						++uniformCount;
						break;

					case "vec3"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Vec3;
						++uniformCount;
						break;

					case "vec2"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Vec2;
						++uniformCount;
						break;

					case "float"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Float;
						++uniformCount;
						break;

					case "int"_hash:
						uniformTypes[uniformCount] = ShaderUniformType::Int;
						++uniformCount;
						break;

					default:
						break;
				}
			}
		}
	}

	MemberItr vsItr = config.FindMember("vertexShaderFile");
	MemberItr fsItr = config.FindMember("fragmentShaderFile");

	if (vsItr == config.MemberEnd() || !vsItr->value.IsString() ||
		fsItr == config.MemberEnd() || !fsItr->value.IsString())
		return false;

	StringRef vsPath(vsItr->value.GetString(), vsItr->value.GetStringLength());
	StringRef fsPath(fsItr->value.GetString(), fsItr->value.GetStringLength());

	Buffer<char> vertexSource = File::ReadText(vsPath);
	Buffer<char> fragmentSource = File::ReadText(fsPath);

	if (this->CompileAndLink(vertexSource.GetRef(), fragmentSource.GetRef()))
	{
		this->AddMaterialUniforms(uniformCount, uniformTypes, uniformNames);

		return true;
	}

	return false;
}

bool Shader::CompileAndLink(BufferRef<char> vertSource, BufferRef<char> fragSource)
{
	GLuint vertexShader = 0;

	if (this->Compile(ShaderType::Vertex, vertSource, vertexShader) == false)
	{
		assert(false);
		return false;
	}
	
	GLuint fragmentShader = 0;

	if (this->Compile(ShaderType::Fragment, fragSource, fragmentShader) == false)
	{
		// Release already compiled vertex shader
		glDeleteShader(vertexShader);

		assert(false);
		return false;
	}
	
	// At this point we know that both shader compilations were successful
	
	// Link the program
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	
	// Release shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	// Check link status
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
	
	if (linkStatus == GL_TRUE)
	{
		driverId = programId;
		uniformMatMVP = glGetUniformLocation(programId, "_MVP");
		uniformMatMV = glGetUniformLocation(programId, "_MV");
		uniformMatVP = glGetUniformLocation(programId, "_VP");
		uniformMatM = glGetUniformLocation(programId, "_M");
		uniformMatV = glGetUniformLocation(programId, "_V");
		uniformMatP = glGetUniformLocation(programId, "_P");
		
		return true;
	}
	else
	{
		driverId = 0;

		// Get info log length
		GLint infoLogLength = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if (infoLogLength > 0)
		{
			String infoLog;
			infoLog.Resize(infoLogLength);

			// Get info log
			glGetProgramInfoLog(programId, infoLogLength, nullptr, infoLog.Begin());

			DebugLog* log = Engine::GetInstance()->GetDebug()->GetLog();
			log->Log(infoLog);
		}

		assert(false);
		return false;
	}
}

bool Shader::Compile(ShaderType type, BufferRef<char> source, GLuint& shaderIdOut)
{
	GLenum shaderType = 0;
	if (type == ShaderType::Vertex)
		shaderType = GL_VERTEX_SHADER;
	else if (type == ShaderType::Fragment)
		shaderType = GL_FRAGMENT_SHADER;
	else
		return false;

	if (source.IsValid())
	{
		const char* data = source.data;
		int length = static_cast<int>(source.count);

		GLuint shaderId = glCreateShader(shaderType);

		// Copy shader source to OpenGL
		glShaderSource(shaderId, 1, &data, &length);

		glCompileShader(shaderId);

		// Check compile status
		GLint compileStatus = GL_FALSE;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_TRUE)
		{
			shaderIdOut = shaderId;
			return true;
		}
		else
		{
			// Get info log length
			GLint infoLogLength = 0;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

			if (infoLogLength > 0)
			{
				String infoLog;
				infoLog.Resize(infoLogLength);

				// Print out info log
				glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog.Begin());

				DebugLog* log = Engine::GetInstance()->GetDebug()->GetLog();
				log->Log(infoLog);
			}
		}
	}

	return false;
}

void Shader::AddMaterialUniforms(unsigned int count,
								 const ShaderUniformType* types,
								 const char** names)
{
	this->materialUniformCount = count;

	for (unsigned uIndex = 0; uIndex < count; ++uIndex)
	{
		ShaderUniform& uniform = this->materialUniforms[uIndex];

		// Get the uniform location from OpenGL
		uniform.location = glGetUniformLocation(driverId, names[uIndex]);

		// Compute uniform name hash
		unsigned len = std::strlen(names[uIndex]);
		uniform.nameHash = Hash::FNV1a_32(names[uIndex], len);

		uniform.type = types[uIndex];

		// Make sure the uniform was found
		assert(uniform.location >= 0);
	}
}
