#include "ShaderProgram.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include <cstddef>
#include <cstdio>


ShaderProgram::ShaderProgram()
{
	
}

ShaderProgram::~ShaderProgram()
{
	
}

bool ShaderProgram::CompileShader(ShaderType type, const char* filePath, GLuint& shaderIdOut)
{
	GLint shaderType = 0;
	if (type == ShaderType::Vertex)
		shaderType = GL_VERTEX_SHADER;
	else if (type == ShaderType::Fragment)
		shaderType = GL_FRAGMENT_SHADER;
	else
		return false;
	
	GLuint shaderId = 0;
	
	// Get the vertex shader content
	FILE* fileHandle = fopen(filePath, "rb");
	
	if (fileHandle != nullptr)
	{
		// Find the size of the file
		fseek(fileHandle, 0L, SEEK_END);
		std::size_t shaderLength = ftell(fileHandle);
		rewind(fileHandle);
		
		// Get the file contents
		char* shaderContent = new char[shaderLength + 1];
		fread(shaderContent, sizeof(char), shaderLength, fileHandle);
		fclose(fileHandle);
		
		// Null-terminate the string
		shaderContent[shaderLength] = '\0';
		
		shaderId = glCreateShader(shaderType);
		
		// Copy shader source
		glShaderSource(shaderId, 1, &shaderContent , NULL);
		delete[] shaderContent;
		shaderContent = nullptr;
		
		// Compile shader
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
				glGetShaderInfoLog(shaderId, infoLogLength, NULL, infoLog);
				printf("%s\n", infoLog);
				
				delete[] infoLog;
			}
		}
	}
	
	return false;
}

bool ShaderProgram::LoadShaders(const char* vertShaderFilePath, const char* fragShaderFilePath)
{
	GLuint vertexShader = 0;
	if (this->CompileShader(ShaderType::Vertex, vertShaderFilePath, vertexShader) == false)
	{
		return false;
	}
	
	GLuint fragmentShader = 0;
	if (this->CompileShader(ShaderType::Fragment, fragShaderFilePath, fragmentShader) == false)
	{
		// Release already compiled vertex shader
		glDeleteShader(vertexShader);
		
		return false;
	}
	
	// At this point we know that both shader compilations were successful
	
	// Link the program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	// Release shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	// Check link status
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	
	if (linkStatus == GL_TRUE)
	{
		this->shaderProgram = shaderProgram;
		
		return true;
	}
	else
	{
		// Get info log length
		GLint infoLogLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if (infoLogLength > 0)
		{
			// Print out info log
			char* infoLog = new char[infoLogLength + 1];
			glGetProgramInfoLog(shaderProgram, infoLogLength, NULL, infoLog);
			printf("%s\n", infoLog);
			
			delete[] infoLog;
		}
		
		return false;
	}
}