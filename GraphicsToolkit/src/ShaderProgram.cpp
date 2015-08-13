#include "ShaderProgram.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include <cstdio>

#include <cassert>

#include "File.h"
#include "JsonReader.h"
#include "ShaderConfigReader.h"

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
				// Print out info log
				char* infoLog = new char[infoLogLength + 1];
				glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog);
				printf("%s\n", infoLog);
				
				delete[] infoLog;
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
		JsonReader reader;
		reader.SetContent(reinterpret_cast<const char*>(configBuffer.Data()),
						  static_cast<unsigned int>(configBuffer.Count()));

		ShaderConfigReader configReader;
		configReader.Read(reader, *this);

		return true;
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
		this->oglId = programId;
		this->mvpUniformLocation = glGetUniformLocation(programId, "MVP");
		
		return true;
	}
	else
	{
		this->oglId = 0;

		// Get info log length
		GLint infoLogLength = 0;
		glGetProgramiv(this->oglId, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if (infoLogLength > 0)
		{
			// Print out info log
			char* infoLog = new char[infoLogLength + 1];
			glGetProgramInfoLog(this->oglId, infoLogLength, NULL, infoLog);
			printf("%s\n", infoLog);
			
			delete[] infoLog;
		}

		assert(false);
		return false;
	}
}