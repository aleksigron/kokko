#include "ShaderProgram.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#include "File.hpp"
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

void ShaderProgram::SetAllocator(StackAllocator* allocator)
{
	this->allocator = allocator;
}

bool ShaderProgram::CompileShader(ShaderType type, const char* filePath, GLuint& shaderIdOut)
{
	GLenum shaderType = 0;
	if (type == ShaderType::Vertex)
		shaderType = GL_VERTEX_SHADER;
	else if (type == ShaderType::Fragment)
		shaderType = GL_FRAGMENT_SHADER;
	else
		return false;

	Buffer<unsigned char> fileContents = File::Read(filePath);

	if (fileContents.IsValid())
	{
		const char* data = reinterpret_cast<const char*>(fileContents.Data());
		int length = static_cast<int>(fileContents.Count());

		GLuint shaderId = glCreateShader(shaderType);
		
		// Copy shader source to OpenGL
		glShaderSource(shaderId, 1, &data, &length);

		fileContents.Deallocate();

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

bool ShaderProgram::LoadFromConfiguration(const char* configurationPath)
{
	Buffer<unsigned char> configBuffer = File::Read(configurationPath);

	if (configBuffer.IsValid())
	{
		const char* vsFilePath = nullptr;
		const char* fsFilePath = nullptr;

		StringRef uniformNames[ShaderProgram::MaxMaterialUniforms];
		ShaderUniformType uniformTypes[ShaderProgram::MaxMaterialUniforms];
		unsigned int uniformCount = 0;

		char* data = reinterpret_cast<char*>(configBuffer.Data());
		unsigned long size = configBuffer.Count();

		using namespace rapidjson;

		Document config;
		config.Parse(data, size);

		Value::ConstMemberIterator itr = config.FindMember("vertexShaderFile");

		if (itr != config.MemberEnd())
		{
			const Value& v = itr->value;

			if (v.IsString())
			{
				vsFilePath = v.GetString();
			}
		}

		itr = config.FindMember("fragmentShaderFile");

		if (itr != config.MemberEnd())
		{
			const Value& v = itr->value;

			if (v.IsString())
			{
				fsFilePath = v.GetString();
			}
		}

		itr = config.FindMember("materialUniforms");

		if (itr != config.MemberEnd())
		{
			const Value& v = itr->value;

			if (v.IsArray())
			{
				for (unsigned muIndex = 0, muCount = v.Size(); muIndex < muCount; ++muIndex)
				{
					const Value& mu = v[muIndex];

					if (mu.IsObject())
					{
						Value::ConstMemberIterator nameItr = mu.FindMember("name");

						if (nameItr != mu.MemberEnd())
						{
							const Value& name = nameItr->value;

							if (name.IsString())
							{
								uniformNames[uniformCount].str = name.GetString();
								uniformNames[uniformCount].len = name.GetStringLength();
							}
						}

						Value::ConstMemberIterator typeItr = mu.FindMember("type");

						if (typeItr->value.IsString())
						{
							const Value& type = typeItr->value;

							if (type.IsString())
							{
								const char* typeStr = type.GetString();

								for (unsigned typeIndex = 0; typeIndex < ShaderUniform::TypeCount; ++typeIndex)
								{
									// Check what type of uniform this is
									if (std::strcmp(typeStr, ShaderUniform::TypeNames[typeIndex]) == 0)
									{
										uniformTypes[uniformCount] = static_cast<ShaderUniformType>(typeIndex);
										break;
									}
								}
							}
						}
					}

					++uniformCount;
				}
			}
		}

		if (vsFilePath != nullptr && fsFilePath != nullptr)
		{
			if (this->Load(vsFilePath, fsFilePath))
			{
				this->AddMaterialUniforms(uniformCount, uniformTypes, uniformNames);

				return true;
			}
		}
	}

	return false;
}

bool ShaderProgram::Load(const char* vertShaderFilePath, const char* fragShaderFilePath)
{
	GLuint vertexShader = 0;

	if (this->CompileShader(ShaderType::Vertex, vertShaderFilePath, vertexShader) == false)
	{
		assert(false);
		return false;
	}
	
	GLuint fragmentShader = 0;

	if (this->CompileShader(ShaderType::Fragment, fragShaderFilePath, fragmentShader) == false)
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
		uniform.type = types[uIndex];
		uniform.location = glGetUniformLocation(driverId, buffer);

		// The uniform could be found
		assert(uniform.location >= 0);
	}
}
