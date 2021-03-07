#include "Resources/ShaderLoader.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "rapidjson/document.h"

#include "Core/BufferRef.hpp"
#include "Core/Hash.hpp"
#include "Core/HashMap.hpp"
#include "Core/Sort.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Debug/LogHelper.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/ShaderManager.hpp"

#include "System/File.hpp"

struct FileString
{
	char* string;
	size_t length;
};

static bool LoadIncludes(
	const rapidjson::Value& value,
	HashMap<uint32_t, FileString>& includeFiles,
	Allocator* allocator)
{
	if (value.IsArray() == false)
	{
		Log::Error("LoadIncludes: value is not an array");
		return false;
	}

	using ValueItr = rapidjson::Value::ConstValueIterator;
	for (ValueItr itr = value.Begin(), end = value.End(); itr != end; ++itr)
	{
		if (itr->IsString())
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFiles.Lookup(hash);

			// File with this hash hasn't been read before, read it now
			if (file == nullptr)
			{
				char* stringBuffer;
				size_t stringLength;

				if (File::ReadText(itr->GetString(), allocator, stringBuffer, stringLength))
				{
					FileString str;
					str.string = stringBuffer;
					str.length = stringLength;

					file = includeFiles.Insert(hash);
					file->second = str;
				}
				else
					Log::Error("Shader include couldn't be read from file");
			}
		}
		else
			Log::Error("Shader include isn't a string");
	}

	return true;
}

static bool ProcessSource(
	const char* mainPath,
	StringRef versionStr,
	StringRef uniformBlock,
	const rapidjson::Value* includePaths,
	HashMap<uint32_t, FileString>& includeFiles,
	Allocator* allocator,
	Buffer<char>& output)
{
	// Count include files length
	std::size_t totalLength = versionStr.len + uniformBlock.len;

	if (includePaths != nullptr)
	{
		if (includePaths->IsArray() == false)
		{
			Log::Error("ProcessSource: includePaths->value is not an array");
			return false;
		}

		for (auto itr = includePaths->Begin(), end = includePaths->End(); itr != end; ++itr)
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFiles.Lookup(hash);

			if (file == nullptr)
			{
				Log::Error("ProcessSource: include file source not found from includeFiles map");
				return false;
			}

			totalLength += file->second.length;
		}
	}

	Buffer<char> mainFile(allocator);

	if (File::ReadText(mainPath, mainFile) == false)
	{
		Log::Error("ProcessSource: failed to read main shader file");
		return false;
	}

	totalLength += mainFile.Count();

	output.Allocate(totalLength);

	// Concatenate all files together

	char* dest = output.Data();
	std::memcpy(dest, versionStr.str, versionStr.len);
	dest += versionStr.len;

	if (uniformBlock.str != nullptr)
	{
		std::memcpy(dest, uniformBlock.str, uniformBlock.len);
		dest += uniformBlock.len;
	}

	if (includePaths != nullptr)
	{
		for (auto itr = includePaths->Begin(), end = includePaths->End(); itr != end; ++itr)
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFiles.Lookup(hash);

			std::memcpy(dest, file->second.string, file->second.length);
			dest += file->second.length;
		}
	}

	std::strcpy(dest, mainFile.Data());

	return true;
}

struct AddUniforms_UniformData
{
	StringRef name;
	unsigned int arraySize;
	UniformDataType type;
};

static bool BufferUniformSortPredicate(const BufferUniform& a, const BufferUniform& b)
{
	const UniformTypeInfo& aType = UniformTypeInfo::Types[static_cast<unsigned int>(a.type)];
	const UniformTypeInfo& bType = UniformTypeInfo::Types[static_cast<unsigned int>(b.type)];
	return aType.size < bType.size;
}

static void AddUniforms(
	ShaderData& shaderOut,
	BufferRef<const AddUniforms_UniformData> uniforms,
	Allocator* allocator)
{
	static_assert(UniformBlockBinding::Material < 10, "Material uniform block binding point must be less than 10");
	const char* blockStartFormat = "layout(std140, binding = %d) uniform MaterialBlock {\n";
	const size_t blockStartPlaceholdersLen = 2;
	const size_t blockStartLen = std::strlen(blockStartFormat) - blockStartPlaceholdersLen + 1;
	const char* blockRowFormat = "%s %s;\n";
	const size_t blockRowPlaceholdersLen = 4;
	const size_t blockRowFixedMaxLen = std::strlen(blockRowFormat) - blockRowPlaceholdersLen;
	const char* blockEnd = "};\n";
	const size_t blockEndLen = std::strlen(blockEnd);

	// Calculate how much memory we need to store:
	// - Uniform names
	// - Uniform block definition
	// - Buffer uniform definitions
	// - Texture uniform definitions

	size_t nameBytes = 0;
	size_t uniformBlockBytes = blockStartLen + blockEndLen + 1;

	size_t textureUniformCount = 0;
	size_t bufferUniformCount = 0;

	for (unsigned uIndex = 0; uIndex < uniforms.count; ++uIndex)
	{
		const AddUniforms_UniformData& uniform = uniforms[uIndex];
		UniformTypeInfo type = UniformTypeInfo::FromType(uniform.type);

		if (type.isTexture)
		{
			nameBytes += uniform.name.len + 1;
			++textureUniformCount;
		}
		else
		{
			nameBytes += uniform.name.len + 1;
			uniformBlockBytes += uniform.name.len + type.typeNameLength + blockRowFixedMaxLen;
			++bufferUniformCount;
		}
	}

	size_t bufferUniformBytes = sizeof(BufferUniform) * bufferUniformCount;
	size_t textureUniformBytes = sizeof(TextureUniform) * textureUniformCount;
	size_t shaderDataBytes = nameBytes + uniformBlockBytes + bufferUniformBytes + textureUniformBytes;

	// Allocate memory

	shaderOut.buffer = allocator->Allocate(shaderDataBytes);

	char* namePtr = static_cast<char*>(shaderOut.buffer);
	char* uniformBlockPtr = namePtr + nameBytes;
	BufferUniform* bufferUniformPtr = reinterpret_cast<BufferUniform*>(uniformBlockPtr + uniformBlockBytes);
	TextureUniform* textureUniformPtr = reinterpret_cast<TextureUniform*>(bufferUniformPtr + bufferUniformCount);

	shaderOut.uniforms.bufferUniforms = bufferUniformPtr;
	shaderOut.uniforms.textureUniforms = textureUniformPtr;

	// Copy uniform definitions to allocated memory
	
	unsigned int textureUniformsCopied = 0;
	unsigned int bufferUniformsCopied = 0;

	for (unsigned uIndex = 0; uIndex < uniforms.count; ++uIndex)
	{
		ShaderUniform* baseUniform = nullptr;

		UniformDataType dataType = uniforms[uIndex].type;

		if (UniformTypeInfo::FromType(dataType).isTexture)
		{
			TextureUniform& uniform = shaderOut.uniforms.textureUniforms[textureUniformsCopied];

			// Since shader is not compiled at this point, we can't know the uniform location
			uniform.uniformLocation = -1;
			uniform.textureName = 0;

			switch (dataType)
			{
			case UniformDataType::Tex2D:
				uniform.textureTarget = RenderTextureTarget::Texture2d;
				break;
			case UniformDataType::TexCube:
				uniform.textureTarget = RenderTextureTarget::TextureCubeMap;
				break;
			default:
				uniform.textureTarget = RenderTextureTarget::Texture2d;
				break;
			}

			++textureUniformsCopied;

			baseUniform = static_cast<ShaderUniform*>(&uniform);
		}
		else
		{
			BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[bufferUniformsCopied];
			uniform.dataOffset = 0;
			uniform.bufferObjectOffset = 0;
			uniform.arraySize = uniforms[uIndex].arraySize;

			++bufferUniformsCopied;

			baseUniform = static_cast<ShaderUniform*>(&uniform);
		}

		// Copy uniform name
		const StringRef& uniformName = uniforms[uIndex].name;
		std::strncpy(namePtr, uniformName.str, uniformName.len + 1);
		baseUniform->name = StringRef(namePtr, uniformName.len);
		namePtr += uniformName.len + 1;

		// Compute uniform name hash
		baseUniform->nameHash = Hash::FNV1a_32(baseUniform->name.str, baseUniform->name.len);
		baseUniform->type = dataType;
	}

	shaderOut.uniforms.bufferUniformCount = bufferUniformCount;
	shaderOut.uniforms.textureUniformCount = textureUniformCount;

	// Order buffer uniforms based on size
	InsertionSortPred(shaderOut.uniforms.bufferUniforms, bufferUniformCount, BufferUniformSortPredicate);

	// Calculate CPU and GPU buffer offsets for buffer uniforms

	unsigned int dataBufferOffset = 0;
	unsigned int bufferObjectOffset = 0;

	for (unsigned int i = 0, count = bufferUniformCount; i < count; ++i)
	{
		BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
		const UniformTypeInfo& type = UniformTypeInfo::FromType(uniform.type);

		unsigned int alignmentModulo = bufferObjectOffset % type.alignment;
		if (alignmentModulo > 0)
			bufferObjectOffset += type.alignment - alignmentModulo;

		uniform.dataOffset = dataBufferOffset;
		uniform.bufferObjectOffset = bufferObjectOffset;

		if (type.isArray)
		{
			unsigned int typeSize = type.size > type.alignment ? type.size : type.alignment;

			dataBufferOffset += sizeof(unsigned int) + type.size * uniform.arraySize;
			bufferObjectOffset += typeSize;
		}
		else
		{
			dataBufferOffset += type.size;
			bufferObjectOffset += type.size;
		}
	}

	shaderOut.uniforms.uniformDataSize = dataBufferOffset;
	shaderOut.uniforms.uniformBufferSize = bufferObjectOffset;

	// Generate uniform block definition

	if (shaderOut.uniforms.bufferUniformCount == 0)
	{
		shaderOut.uniformBlockDefinition = StringRef();
	}
	else
	{
		shaderOut.uniformBlockDefinition.str = uniformBlockPtr;
		
		int written = std::sprintf(uniformBlockPtr, blockStartFormat, UniformBlockBinding::Material);
		uniformBlockPtr += written;

		for (unsigned int i = 0, count = shaderOut.uniforms.bufferUniformCount; i < count; ++i)
		{
			const BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
			const UniformTypeInfo& typeInfo = UniformTypeInfo::FromType(uniform.type);

			int written = std::sprintf(uniformBlockPtr, blockRowFormat, typeInfo.typeName, uniform.name.str);
			uniformBlockPtr += written;
		}

		std::strncpy(uniformBlockPtr, blockEnd, blockEndLen + 1);
		uniformBlockPtr += blockEndLen;

		shaderOut.uniformBlockDefinition.len = uniformBlockPtr - shaderOut.uniformBlockDefinition.str;
	}
}

static void UpdateTextureUniformLocations(
	ShaderData& shaderInOut,
	RenderDevice* renderDevice)
{
	for (unsigned idx = 0, count = shaderInOut.uniforms.textureUniformCount; idx < count; ++idx)
	{
		TextureUniform& u = shaderInOut.uniforms.textureUniforms[idx];
		u.uniformLocation = renderDevice->GetUniformLocation(shaderInOut.driverId, u.name.str);
	}
}

static bool Compile(
	ShaderData& shaderOut,
	Allocator* allocator,
	RenderDevice* renderDevice,
	RenderShaderStage stage,
	BufferRef<char> source,
	unsigned int& shaderIdOut)
{
	if (source.IsValid())
	{
		const char* data = source.data;
		int length = static_cast<int>(source.count);

		unsigned int shaderId = renderDevice->CreateShaderStage(stage);

		renderDevice->SetShaderStageSource(shaderId, data, length);
		renderDevice->CompileShaderStage(shaderId);

		// Check compile status
		if (renderDevice->GetShaderStageCompileStatus(shaderId))
		{
			shaderIdOut = shaderId;
			return true;
		}
		else
		{
			// Get info log length
			int infoLogLength = renderDevice->GetShaderStageInfoLogLength(shaderId);

			if (infoLogLength > 0)
			{
				String infoLog(allocator);
				infoLog.Resize(infoLogLength);

				// Print out info log
				renderDevice->GetShaderStageInfoLog(shaderId, infoLogLength, infoLog.Begin());

				Log::Error(infoLog.GetCStr(), infoLog.GetLength());
			}
		}
	}

	return false;
}

static bool CompileAndLink(
	ShaderData& shaderOut,
	BufferRef<char> vertSource,
	BufferRef<char> fragSource,
	Allocator* allocator,
	RenderDevice* renderDevice)
{
	unsigned int vertexShader = 0;

	if (Compile(shaderOut, allocator, renderDevice, RenderShaderStage::VertexShader, vertSource, vertexShader) == false)
	{
		Log::Error("Compilation of vertex shader failed");
		return false;
	}

	unsigned int fragmentShader = 0;

	if (Compile(shaderOut, allocator, renderDevice, RenderShaderStage::FragmentShader, fragSource, fragmentShader) == false)
	{
		// Release already compiled vertex shader
		renderDevice->DestroyShaderStage(vertexShader);

		Log::Error("Compilation of fragment shader failed");
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
	if (renderDevice->GetShaderProgramLinkStatus(programId))
	{
		shaderOut.driverId = programId;

		return true;
	}
	else
	{
		shaderOut.driverId = 0;

		int infoLogLength = renderDevice->GetShaderProgramInfoLogLength(programId);

		if (infoLogLength > 0)
		{
			String infoLog(allocator);
			infoLog.Resize(infoLogLength);

			// Get info log
			renderDevice->GetShaderProgramInfoLog(programId, infoLogLength, infoLog.Begin());

			Log::Error(infoLog.GetCStr(), infoLog.GetLength());
		}

		Log::Error("Linking of shader program failed");
		return false;
	}
}

bool ShaderLoader::LoadFromConfiguration(
	ShaderData& shaderOut,
	BufferRef<char> configuration,
	Allocator* allocator,
	RenderDevice* renderDevice)
{
	using MemberItr = rapidjson::Value::ConstMemberIterator;
	using ValueItr = rapidjson::Value::ConstValueIterator;

	char* data = configuration.data;
	unsigned long size = configuration.count;

	rapidjson::Document config;
	config.Parse(data, size);

	// Set default value
	shaderOut.transparencyType = TransparencyType::Opaque;

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
			shaderOut.transparencyType = TransparencyType::Opaque;
			break;

		case "alphaTest"_hash:
			shaderOut.transparencyType = TransparencyType::AlphaTest;
			break;

		case "skybox"_hash:
			shaderOut.transparencyType = TransparencyType::Skybox;
			break;

		case "transparentMix"_hash:
			shaderOut.transparencyType = TransparencyType::TransparentMix;
			break;

		case "transparentAdd"_hash:
			shaderOut.transparencyType = TransparencyType::TransparentAdd;
			break;

		case "transparentSub"_hash:
			shaderOut.transparencyType = TransparencyType::TransparentSub;
			break;
		}
	}

	static const size_t MaxUniformCount = 32;
	AddUniforms_UniformData uniforms[MaxUniformCount];
	unsigned int uniformCount = 0;

	MemberItr uniformListItr = config.FindMember("properties");

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
				AddUniforms_UniformData& uniform = uniforms[uniformCount];

				uniform.name = StringRef(nameItr->value.GetString(), nameItr->value.GetStringLength());

				uint32_t typeHash = Hash::FNV1a_32(typeItr->value.GetString(), typeItr->value.GetStringLength());

				switch (typeHash)
				{
				case "tex2d"_hash: uniform.type = UniformDataType::Tex2D; break;
				case "texCube"_hash: uniform.type = UniformDataType::TexCube; break;
				case "mat4x4"_hash: uniform.type = UniformDataType::Mat4x4; break;
				case "mat4x4Array"_hash: uniform.type = UniformDataType::Mat4x4Array; break;
				case "vec4"_hash: uniform.type = UniformDataType::Vec4; break;
				case "vec4Array"_hash: uniform.type = UniformDataType::Vec4Array; break;
				case "vec3"_hash: uniform.type = UniformDataType::Vec3; break;
				case "vec3Array"_hash: uniform.type = UniformDataType::Vec3Array; break;
				case "vec2"_hash: uniform.type = UniformDataType::Vec2; break;
				case "vec2Array"_hash: uniform.type = UniformDataType::Vec2Array; break;
				case "float"_hash: uniform.type = UniformDataType::Float; break;
				case "floatArray"_hash: uniform.type = UniformDataType::FloatArray; break;
				case "int"_hash: uniform.type = UniformDataType::Int; break;
				case "intArray"_hash: uniform.type = UniformDataType::IntArray; break;

				default:
					break;
				}

				uniform.arraySize = 0;

				if (UniformTypeInfo::FromType(uniform.type).isArray)
				{
					MemberItr sizeItr = uItr->FindMember("size");
					if (sizeItr != uItr->MemberEnd() && sizeItr->value.IsInt())
						uniform.arraySize = sizeItr->value.GetInt();
					else
						Log::Error("Failed to parse shader array uniform because JSON didn't contain size");
				}

				uniformCount += 1;
			}
		}
	}

	BufferRef<const AddUniforms_UniformData> uniformBufferRef(uniforms, uniformCount);
	AddUniforms(shaderOut, uniformBufferRef, allocator);

	MemberItr vsItr = config.FindMember("vs");
	MemberItr fsItr = config.FindMember("fs");

	if (vsItr == config.MemberEnd() || !vsItr->value.IsObject() ||
		fsItr == config.MemberEnd() || !fsItr->value.IsObject())
		return false;

	MemberItr vsMainItr = vsItr->value.FindMember("main");
	MemberItr fsMainItr = fsItr->value.FindMember("main");

	if (vsMainItr == vsItr->value.MemberEnd() || !vsMainItr->value.IsString() ||
		fsMainItr == fsItr->value.MemberEnd() || !fsMainItr->value.IsString())
		return false;

	// Load all include files, they can be shared between shader stages

	HashMap<uint32_t, FileString> includeFiles(allocator);

	bool includeLoadSuccess = true;

	MemberItr vsIncItr = vsItr->value.FindMember("includes");
	if (vsIncItr != vsItr->value.MemberEnd())
	{
		if (LoadIncludes(vsIncItr->value, includeFiles, allocator) == false)
			includeLoadSuccess = false;
	}

	MemberItr fsIncItr = fsItr->value.FindMember("includes");
	if (fsIncItr != fsItr->value.MemberEnd())
	{
		if (LoadIncludes(fsIncItr->value, includeFiles, allocator) == false)
			includeLoadSuccess = false;
	}

	// Process included files into complete source

	bool processSuccess = true;

	Buffer<char> vertSrc(allocator);
	Buffer<char> fragSrc(allocator);

	if (includeLoadSuccess)
	{
		StringRef versionStr("#version 450\n");
		StringRef uniformBlock = shaderOut.uniformBlockDefinition;

		const char* vsPath = vsMainItr->value.GetString();
		const rapidjson::Value* vsInc = vsIncItr != vsItr->value.MemberEnd() ? &vsIncItr->value : nullptr;
		if (ProcessSource(vsPath, versionStr, uniformBlock, vsInc, includeFiles, allocator, vertSrc) == false)
			processSuccess = false;

		const char* fsPath = fsMainItr->value.GetString();
		const rapidjson::Value* fsInc = fsIncItr != fsItr->value.MemberEnd() ? &fsIncItr->value : nullptr;
		if (ProcessSource(fsPath, versionStr, uniformBlock, fsInc, includeFiles, allocator, fragSrc) == false)
			processSuccess = false;
	}

	bool success = false;

	if (processSuccess &&
		CompileAndLink(shaderOut, vertSrc.GetRef(), fragSrc.GetRef(), allocator, renderDevice))
	{
		UpdateTextureUniformLocations(shaderOut, renderDevice);

		success = true;
	}

	// Release loaded include files
	for (auto itr = includeFiles.Begin(), end = includeFiles.End(); itr != end; ++itr)
	{
		allocator->Deallocate(itr->second.string);
	}

	return success;
}
