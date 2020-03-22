#include "Rendering/Shader.hpp"

#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#include "Core/Hash.hpp"
#include "Core/HashMap.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Debug/LogHelper.hpp"

#include "Rendering/RenderDevice.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

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

bool LoadIncludes(
	rapidjson::Value::ConstMemberIterator member,
	HashMap<uint32_t, BufferRef<char>>& includeFiles,
	Allocator* allocator)
{
	if (member->value.IsArray() == false)
	{
		Log::Info("Error: LoadIncludes() member->value is not an array");
		return false;
	}

	using ValueItr = rapidjson::Value::ConstValueIterator;
	for (ValueItr itr = member->value.Begin(), end = member->value.End(); itr != end; ++itr)
	{
		if (itr->IsString())
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFiles.Lookup(hash);

			// File with this hash hasn't been read before, read it now
			if (file == nullptr)
			{
				BufferRef<char> content;
				if (File::ReadText(itr->GetString(), allocator, content))
				{
					file = includeFiles.Insert(hash);
					file->second = content;
				}
				else
					Log::Info("Error: shader include couldn't be read from file");
			}
		}
		else
			Log::Info("Error: shader include isn't a string");
	}

	return true;
}

bool ProcessSource(
	const char* mainPath,
	rapidjson::Value::ConstMemberIterator includePaths,
	HashMap<uint32_t, BufferRef<char>>& includeFiles,
	Allocator* allocator,
	Buffer<char>& output)
{
	static const char versionStr[] = "#version 440\n";
	static const std::size_t versionStrLen = sizeof(versionStr) - 1;

	if (includePaths->value.IsArray() == false)
	{
		Log::Info("Error: ProcessSource() includePaths->value is not an array");
		return false;
	}

	// Count include files length
	std::size_t totalLength = versionStrLen;

	const rapidjson::Value& pathArray = includePaths->value;
	for (auto itr = pathArray.Begin(), end = pathArray.End(); itr != end; ++itr)
	{
		uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
		auto* file = includeFiles.Lookup(hash);

		if (file == nullptr)
		{
			Log::Info("Error: ProcessSource() include file source not found from includeFiles map");
			return false;
		}

		totalLength += file->second.count - 1;
	}

	Buffer<char> mainFile(allocator);

	if (File::ReadText(mainPath, mainFile) == false)
	{
		Log::Info("Error: ProcessSource() failed to read main shader file");
		return false;
	}

	totalLength += mainFile.Count();

	output.Allocate(totalLength);

	// Concatenate all files together

	char* dest = output.Data();
	std::strcpy(dest, versionStr);
	dest += versionStrLen;

	for (auto itr = pathArray.Begin(), end = pathArray.End(); itr != end; ++itr)
	{
		uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
		auto* file = includeFiles.Lookup(hash);

		std::strcpy(dest, file->second.data);
		dest += file->second.count - 1;
	}

	std::strcpy(dest, mainFile.Data());

	return true;
}

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

bool Shader::LoadFromConfiguration(
	BufferRef<char> configuration,
	Allocator* allocator,
	RenderDevice* renderDevice)
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

	// Set default value
	this->transparencyType = TransparencyType::Opaque;

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

	// Load all include files, they can be shared between shader stages

	HashMap<uint32_t, BufferRef<char>> includeFiles(allocator);

	bool includeLoadSuccess = true;

	MemberItr vsIncItr = config.FindMember("vsIncludes");
	if (vsIncItr != config.MemberEnd())
	{
		if (LoadIncludes(vsIncItr, includeFiles, allocator) == false)
			includeLoadSuccess = false;
	}

	MemberItr fsIncItr = config.FindMember("fsIncludes");
	if (fsIncItr != config.MemberEnd())
	{
		if (LoadIncludes(fsIncItr, includeFiles, allocator) == false)
			includeLoadSuccess = false;
	}

	// Process included files into complete source

	bool processSuccess = true;

	Buffer<char> vertexSource(allocator);
	Buffer<char> fragmentSource(allocator);

	if (includeLoadSuccess)
	{
		const char* vsPath = vsItr->value.GetString();
		const char* fsPath = fsItr->value.GetString();

		if (vsIncItr != config.MemberEnd())
		{
			if (ProcessSource(vsPath, vsIncItr, includeFiles, allocator, vertexSource) == false)
				processSuccess = false;
		}
		else if (File::ReadText(vsPath, vertexSource) == false)
			processSuccess = false;

		if (fsIncItr != config.MemberEnd())
		{
			if (ProcessSource(fsPath, fsIncItr, includeFiles, allocator, fragmentSource) == false)
				processSuccess = false;
		}
		else if (File::ReadText(fsPath, fragmentSource) == false)
			processSuccess = false;
	}

	bool success = false;

	if (processSuccess &&
		this->CompileAndLink(vertexSource.GetRef(), fragmentSource.GetRef(), allocator, renderDevice))
	{
		this->AddMaterialUniforms(renderDevice, uniformCount, uniformTypes, uniformNames);

		success = true;
	}

	// Release loaded include files
	for (auto itr = includeFiles.Begin(), end = includeFiles.End(); itr != end; ++itr)
	{
		allocator->Deallocate(itr->second.data);
	}

	return success;
}

bool Shader::CompileAndLink(
	BufferRef<char> vertSource,
	BufferRef<char> fragSource,
	Allocator* allocator,
	RenderDevice* renderDevice)
{
	unsigned int vertexShader = 0;

	if (this->Compile(allocator, renderDevice, ShaderType::Vertex, vertSource, vertexShader) == false)
	{
		assert(false);
		return false;
	}
	
	unsigned int fragmentShader = 0;

	if (this->Compile(allocator, renderDevice, ShaderType::Fragment, fragSource, fragmentShader) == false)
	{
		// Release already compiled vertex shader
		renderDevice->DestroyShaderStage(vertexShader);

		assert(false);
		return false;
	}
	
	// At this point we know that both shader compilations were successful
	
	// Link the program
	unsigned int programId = renderDevice->CreateShaderProgram();
	renderDevice->AttachShaderStageToProgram(programId, vertexShader);
	renderDevice->AttachShaderStageToProgram(programId, fragmentShader);
	renderDevice->LinkShaderProgram(programId);
	
	// Release shaders
	renderDevice->DestroyShaderStage(vertexShader);
	renderDevice->DestroyShaderStage(fragmentShader);
	
	// Check link status
	int linkStatus = renderDevice->GetShaderProgramParameterInt(programId, GL_LINK_STATUS);
	
	if (linkStatus == GL_TRUE)
	{
		this->driverId = programId;

		uniformMatMVP = renderDevice->GetUniformLocation(programId, "_MVP");
		uniformMatMV = renderDevice->GetUniformLocation(programId, "_MV");
		uniformMatVP = renderDevice->GetUniformLocation(programId, "_VP");
		uniformMatM = renderDevice->GetUniformLocation(programId, "_M");
		uniformMatV = renderDevice->GetUniformLocation(programId, "_V");
		uniformMatP = renderDevice->GetUniformLocation(programId, "_P");
		
		return true;
	}
	else
	{
		this->driverId = 0;

		int infoLogLength = renderDevice->GetShaderProgramParameterInt(programId, GL_INFO_LOG_LENGTH);
		
		if (infoLogLength > 0)
		{
			String infoLog(allocator);
			infoLog.Resize(infoLogLength);

			// Get info log
			renderDevice->GetShaderProgramInfoLog(programId, infoLogLength, infoLog.Begin());

			Log::Info(infoLog.GetCStr(), infoLog.GetLength());
		}

		assert(false);
		return false;
	}
}

bool Shader::Compile(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderType type,
	BufferRef<char> source,
	unsigned int& shaderIdOut)
{
	unsigned int shaderType = 0;
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

		unsigned int shaderId = renderDevice->CreateShaderStage(shaderType);

		renderDevice->SetShaderStageSource(shaderId, data, length);
		renderDevice->CompileShaderStage(shaderId);

		// Check compile status
		int compileStatus = renderDevice->GetShaderStageParameterInt(shaderId, GL_COMPILE_STATUS);

		if (compileStatus == GL_TRUE)
		{
			shaderIdOut = shaderId;
			return true;
		}
		else
		{
			// Get info log length
			int infoLogLength = renderDevice->GetShaderStageParameterInt(shaderId, GL_INFO_LOG_LENGTH);

			if (infoLogLength > 0)
			{
				String infoLog(allocator);
				infoLog.Resize(infoLogLength);

				// Print out info log
				renderDevice->GetShaderStageInfoLog(shaderId, infoLogLength, infoLog.Begin());

				Log::Info(infoLog.GetCStr(), infoLog.GetLength());
			}
		}
	}

	return false;
}

void Shader::AddMaterialUniforms(
	RenderDevice* renderDevice,
	unsigned int count,
	const ShaderUniformType* types,
	const char** names)
{
	this->materialUniformCount = count;

	for (unsigned uIndex = 0; uIndex < count; ++uIndex)
	{
		ShaderUniform& uniform = this->materialUniforms[uIndex];

		// Get the uniform location in shader program
		uniform.location = renderDevice->GetUniformLocation(driverId, names[uIndex]);

		// Compute uniform name hash
		unsigned len = std::strlen(names[uIndex]);
		uniform.nameHash = Hash::FNV1a_32(names[uIndex], len);

		uniform.type = types[uIndex];

		// Make sure the uniform was found
		assert(uniform.location >= 0);
	}
}
