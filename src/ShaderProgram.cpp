#include "ShaderProgram.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#include "File.hpp"
#include "Hash.hpp"
#include "StringRef.hpp"
#include "StackAllocator.hpp"

const char* const ShaderUniform::TypeNames[] =
{
	"tex2d",
	"mat4x4",
	"vec4",
	"vec3",
	"vec2",
	"float",
	"int"
};

const unsigned int ShaderUniform::TypeSizes[] = {
	4, // Texture2D
	64, // Mat4x4
	16, // Vec4
	12, // Vec3
	8, // Vec2
	4, // Float
	4 // Int
};

void ShaderProgram::SetAllocator(StackAllocator* allocator)
{
	this->allocator = allocator;
}

bool ShaderProgram::LoadFromConfiguration(Buffer<char>& configuration)
{
	const char* vsFilePath = nullptr;
	const char* fsFilePath = nullptr;

	StringRef uniformNames[ShaderProgram::MaxMaterialUniforms];
	ShaderUniformType uniformTypes[ShaderProgram::MaxMaterialUniforms];
	unsigned int uniformCount = 0;

	char* data = configuration.Data();
	unsigned long size = configuration.Count();

	using namespace rapidjson;

	Document config;
	config.Parse(data, size);

	assert(config.HasMember("vertexShaderFile"));
	assert(config.HasMember("fragmentShaderFile"));

	vsFilePath = config["vertexShaderFile"].GetString();
	fsFilePath = config["fragmentShaderFile"].GetString();

	Value::ConstMemberIterator uniformListItr = config.FindMember("materialUniforms");

	if (uniformListItr != config.MemberEnd())
	{
		const Value& list = uniformListItr->value;

		for (unsigned muIndex = 0, muCount = list.Size(); muIndex < muCount; ++muIndex)
		{
			const Value& mu = list[muIndex];

			assert(mu.HasMember("name"));
			assert(mu.HasMember("type"));

			const Value& name = mu["name"];
			uniformNames[uniformCount].str = name.GetString();
			uniformNames[uniformCount].len = name.GetStringLength();

			const char* typeStr = mu["type"].GetString();
			for (unsigned typeIndex = 0; typeIndex < ShaderUniform::TypeCount; ++typeIndex)
			{
				// Check what type of uniform this is
				if (std::strcmp(typeStr, ShaderUniform::TypeNames[typeIndex]) == 0)
				{
					uniformTypes[uniformCount] = static_cast<ShaderUniformType>(typeIndex);
					break;
				}
			}

			++uniformCount;
		}
	}

	if (vsFilePath != nullptr && fsFilePath != nullptr)
	{
		Buffer<char> vertexSource = File::ReadText(vsFilePath);
		Buffer<char> fragmentSource = File::ReadText(fsFilePath);

		if (this->CompileAndLink(vertexSource, fragmentSource))
		{
			this->AddMaterialUniforms(uniformCount, uniformTypes, uniformNames);

			return true;
		}
	}

	return false;
}

bool ShaderProgram::CompileAndLink(Buffer<char>& vertexSource, Buffer<char>& fragmentSource)
{
	GLuint vertexShader = 0;

	if (this->Compile(ShaderType::Vertex, vertexSource, vertexShader) == false)
	{
		assert(false);
		return false;
	}
	
	GLuint fragmentShader = 0;

	if (this->Compile(ShaderType::Fragment, fragmentSource, fragmentShader) == false)
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
		uniformMVP = glGetUniformLocation(programId, "_MVP");
		uniformMV = glGetUniformLocation(programId, "_MV");
		
		return true;
	}
	else
	{
		driverId = 0;

		// Get info log length
		GLint infoLogLength = 0;
		glGetProgramiv(driverId, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if (infoLogLength > 0)
		{
			StackAllocation infoLog = allocator->Allocate(infoLogLength + 1);
			char* infoLogBuffer = reinterpret_cast<char*>(infoLog.data);

			// Print out info log
			glGetProgramInfoLog(driverId, infoLogLength, NULL, infoLogBuffer);
			printf("%s\n", infoLogBuffer);
		}

		assert(false);
		return false;
	}
}

bool ShaderProgram::Compile(ShaderType type, Buffer<char>& source, GLuint& shaderIdOut)
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
		const char* data = source.Data();
		int length = static_cast<int>(source.Count());

		GLuint shaderId = glCreateShader(shaderType);

		// Copy shader source to OpenGL
		glShaderSource(shaderId, 1, &data, &length);

		source.Deallocate();

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
				StackAllocation infoLog = allocator->Allocate(infoLogLength + 1);
				char* infoLogBuffer = reinterpret_cast<char*>(infoLog.data);

				// Print out info log
				glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLogBuffer);
				printf("%s\n", infoLogBuffer);
			}
		}
	}

	return false;
}

void ShaderProgram::AddMaterialUniforms(unsigned int count,
										const ShaderUniformType* types,
										const StringRef* names)
{
	this->materialUniformCount = count;

	for (unsigned uIndex = 0; uIndex < count; ++uIndex)
	{
		const StringRef* name = names + uIndex;

		StackAllocation nameBuffer = this->allocator->Allocate(name->len + 1);
		char* buffer = reinterpret_cast<char*>(nameBuffer.data);

		// Copy string to a local buffer because it needs to be null terminated
		std::memcpy(buffer, name->str, name->len);
		buffer[name->len] = '\0'; // Null-terminate

		ShaderUniform& uniform = this->materialUniforms[uIndex];

		// Get the uniform location from OpenGL
		uniform.location = glGetUniformLocation(driverId, buffer);

		// Compute uniform name hash
		uniform.nameHash = Hash::FNV1a_32(name->str, name->len);

		uniform.type = types[uIndex];

		// Make sure the uniform was found
		assert(uniform.location >= 0);
	}
}
