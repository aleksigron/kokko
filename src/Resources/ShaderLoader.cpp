#include "Resources/ShaderLoader.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rapidjson/document.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/HashMap.hpp"
#include "Core/Range.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/ShaderManager.hpp"

#include "System/File.hpp"

static const size_t MaxStageCount = 2;
static const char* const LineBreakChars = "\r\n";
static const char* const WhitespaceChars = " \t\r\n";

namespace ShaderLoader
{
	struct StageSource
	{
		RenderShaderStage stage;
		StringRef source;
	};

	struct FileString
	{
		char* string;
		size_t length;
	};

	struct ProcessStore
	{
		HashMap<uint32_t, FileString>& includeFileCache;
		SortedArray<uint32_t>& filesIncludedInStage;
		String& pathString;
	};

	struct AddUniforms_UniformData
	{
		StringRef name;
		unsigned int arraySize;
		UniformDataType type;
	};
}


static bool LoadIncludes(
	const rapidjson::Value& value,
	HashMap<uint32_t, ShaderLoader::FileString>& includeFileCache,
	Allocator* allocator)
{
	KOKKO_PROFILE_FUNCTION();

	if (value.IsArray() == false)
	{
		KK_LOG_ERROR("LoadIncludes: value is not an array");
		return false;
	}

	using ValueItr = rapidjson::Value::ConstValueIterator;
	for (ValueItr itr = value.Begin(), end = value.End(); itr != end; ++itr)
	{
		if (itr->IsString())
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFileCache.Lookup(hash);

			// File with this hash hasn't been read before, read it now
			if (file == nullptr)
			{
				char* stringBuffer;
				size_t stringLength;

				if (File::ReadText(itr->GetString(), allocator, stringBuffer, stringLength))
				{
					ShaderLoader::FileString str;
					str.string = stringBuffer;
					str.length = stringLength;

					file = includeFileCache.Insert(hash);
					file->second = str;
				}
				else
					KK_LOG_ERROR("Shader include couldn't be read from file");
			}
		}
		else
			KK_LOG_ERROR("Shader include isn't a string");
	}

	return true;
}

static bool ProcessSource(
	const char* mainPath,
	StringRef versionStr,
	StringRef uniformBlock,
	const rapidjson::Value* includePaths,
	HashMap<uint32_t, ShaderLoader::FileString>& includeFileCache,
	Allocator* allocator,
	Array<char>& output)
{
	KOKKO_PROFILE_FUNCTION();

	// Count include files length
	std::size_t totalLength = versionStr.len + uniformBlock.len;

	if (includePaths != nullptr)
	{
		if (includePaths->IsArray() == false)
		{
			KK_LOG_ERROR("ProcessSource: includePaths->value is not an array");
			return false;
		}

		for (auto itr = includePaths->Begin(), end = includePaths->End(); itr != end; ++itr)
		{
			uint32_t hash = Hash::FNV1a_32(itr->GetString(), itr->GetStringLength());
			auto* file = includeFileCache.Lookup(hash);

			if (file == nullptr)
			{
				KK_LOG_ERROR("ProcessSource: include file source not found from includeFileCache map");
				return false;
			}

			totalLength += file->second.length;
		}
	}

	Array<char> mainFile(allocator);

	if (File::ReadText(mainPath, mainFile) == false)
	{
		KK_LOG_ERROR("ProcessSource: failed to read main shader file");
		return false;
	}

	totalLength += mainFile.GetCount();

	output.Resize(totalLength);

	// Concatenate all files together

	char* dest = output.GetData();
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
			auto* file = includeFileCache.Lookup(hash);

			std::memcpy(dest, file->second.string, file->second.length);
			dest += file->second.length;
		}
	}

	std::strcpy(dest, mainFile.GetData());

	return true;
}

static bool BufferUniformSortPredicate(const BufferUniform& a, const BufferUniform& b)
{
	const UniformTypeInfo& aType = UniformTypeInfo::Types[static_cast<unsigned int>(a.type)];
	const UniformTypeInfo& bType = UniformTypeInfo::Types[static_cast<unsigned int>(b.type)];
	return aType.size > bType.size;
}

static void AddUniforms(
	ShaderData& shaderOut,
	ArrayView<const ShaderLoader::AddUniforms_UniformData> uniforms,
	Allocator* allocator)
{
	KOKKO_PROFILE_FUNCTION();

	static_assert(UniformBlockBinding::Material < 10, "Material uniform block binding point must be less than 10");
	const char* blockStartFormat = "layout(std140, binding = %u) uniform MaterialBlock {\n";
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

	for (size_t uIndex = 0, uCount = uniforms.GetCount(); uIndex < uCount; ++uIndex)
	{
		const ShaderLoader::AddUniforms_UniformData& uniform = uniforms[uIndex];
		UniformTypeInfo type = UniformTypeInfo::FromType(uniform.type);

		if (type.isTexture)
		{
			nameBytes += uniform.name.len + 1;
			++textureUniformCount;
		}
		else
		{
			nameBytes += uniform.name.len + 1;
			// TODO: Calculate array definition length
			uniformBlockBytes += uniform.name.len + type.typeNameLength + blockRowFixedMaxLen;
			++bufferUniformCount;
		}
	}

	const size_t shaderDataAlignment = 8;
	nameBytes = (nameBytes + shaderDataAlignment - 1) / shaderDataAlignment * shaderDataAlignment;
	uniformBlockBytes = (uniformBlockBytes + shaderDataAlignment - 1) / shaderDataAlignment * shaderDataAlignment;

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

	for (size_t uIndex = 0, uCount = uniforms.GetCount(); uIndex < uCount; ++uIndex)
	{
		ShaderUniform* baseUniform = nullptr;

		UniformDataType dataType = uniforms[uIndex].type;

		if (UniformTypeInfo::FromType(dataType).isTexture)
		{
			TextureUniform& uniform = shaderOut.uniforms.textureUniforms[textureUniformsCopied];

			// Since shader is not compiled at this point, we can't know the uniform location
			uniform.uniformLocation = -1;
			uniform.textureObject = 0;

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

		std::memcpy(namePtr, uniformName.str, uniformName.len);
		namePtr[uniformName.len] = '\0';

		baseUniform->name = StringRef(namePtr, uniformName.len);
		namePtr += uniformName.len + 1;

		// Compute uniform name hash
		baseUniform->nameHash = Hash::FNV1a_32(baseUniform->name.str, baseUniform->name.len);
		baseUniform->type = dataType;
	}

	shaderOut.uniforms.bufferUniformCount = bufferUniformCount;
	shaderOut.uniforms.textureUniformCount = textureUniformCount;

	// Order buffer uniforms based on size
	BufferUniform* begin = shaderOut.uniforms.bufferUniforms;
	BufferUniform* end = begin + bufferUniformCount;
	std::sort(begin, end, BufferUniformSortPredicate);

	// Calculate CPU and GPU buffer offsets for buffer uniforms

	unsigned int dataBufferOffset = 0;
	unsigned int bufferObjectOffset = 0;

	for (unsigned int i = 0, count = bufferUniformCount; i < count; ++i)
	{
		BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
		const UniformTypeInfo& type = UniformTypeInfo::FromType(uniform.type);
		
		// Round up the offset to match type aligment
		bufferObjectOffset = (bufferObjectOffset + type.alignment - 1) / type.alignment * type.alignment;

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

	const unsigned int bufferSizeUnit = 16;

	shaderOut.uniforms.uniformDataSize = dataBufferOffset;
	shaderOut.uniforms.uniformBufferSize = (bufferObjectOffset + bufferSizeUnit - 1) / bufferSizeUnit * bufferSizeUnit;

	// Generate uniform block definition

	if (shaderOut.uniforms.bufferUniformCount == 0)
	{
		shaderOut.uniformBlockDefinition = StringRef();
	}
	else
	{
		shaderOut.uniformBlockDefinition.str = uniformBlockPtr;
		
		int written = std::sprintf(uniformBlockPtr, blockStartFormat, UniformBlockBinding::Material);
		assert(written > 0);
		uniformBlockPtr += written;

		for (unsigned int i = 0, count = shaderOut.uniforms.bufferUniformCount; i < count; ++i)
		{
			const BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
			const UniformTypeInfo& typeInfo = UniformTypeInfo::FromType(uniform.type);

			int written = std::sprintf(uniformBlockPtr, blockRowFormat, typeInfo.typeName, uniform.name.str);
			assert(written > 0);
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
	KOKKO_PROFILE_FUNCTION();

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
	StringRef source,
	unsigned int& shaderIdOut)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int shaderId = renderDevice->CreateShaderStage(stage);

	renderDevice->SetShaderStageSource(shaderId, source.str, source.len);
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

			KK_LOG_ERROR("Shader stage compilation failed:\n{}\nSource:\n{}", infoLog.GetCStr(), source.str);
		}
	}

	return false;
}

static bool CompileAndLink(
	ShaderData& shaderOut,
	ArrayView<const ShaderLoader::StageSource> stages,
	Allocator* allocator,
	RenderDevice* renderDevice,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int stageObjects[MaxStageCount];

	for (size_t i = 0, count = stages.GetCount(); i < stages.GetCount(); ++i)
	{
		const ShaderLoader::StageSource& stage = stages[i];
		if (Compile(shaderOut, allocator, renderDevice, stage.stage, stage.source, stageObjects[i]) == false)
		{
			for (size_t j = 0; j < i; ++j)
				renderDevice->DestroyShaderStage(stageObjects[j]);

			return false;
		}
	}

	// At this point we know that shader compilations were successful

	unsigned int programId;
	bool linkSucceeded;

	{
		KOKKO_PROFILE_SCOPE("Link program");

		// Link the program
		programId = renderDevice->CreateShaderProgram();

		for (size_t i = 0, count = stages.GetCount(); i < stages.GetCount(); ++i)
			renderDevice->AttachShaderStageToProgram(programId, stageObjects[i]);

		renderDevice->LinkShaderProgram(programId);

		// Release shaders
		for (size_t i = 0, count = stages.GetCount(); i < stages.GetCount(); ++i)
			renderDevice->DestroyShaderStage(stageObjects[i]);

		linkSucceeded = renderDevice->GetShaderProgramLinkStatus(programId);
	}

	// Check link status
	if (linkSucceeded)
	{
		shaderOut.driverId = programId;

		renderDevice->SetObjectLabel(RenderObjectType::Program, programId, debugName);

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

			KK_LOG_ERROR("Shader program link failed\n{}\n", infoLog.GetCStr());
		}
		else
			KK_LOG_ERROR("Shader program link failed");

		return false;
	}
}

bool ShaderLoader::LoadFromConfiguration(
	ShaderData& shaderOut,
	ArrayView<char> configuration,
	Allocator* allocator,
	RenderDevice* renderDevice,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	using MemberItr = rapidjson::Value::ConstMemberIterator;
	using ValueItr = rapidjson::Value::ConstValueIterator;

	char* data = configuration.GetData();
	unsigned long size = configuration.GetCount();

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
						KK_LOG_ERROR("Failed to parse shader array uniform because JSON didn't contain size");
				}

				uniformCount += 1;
			}
		}
	}

	ArrayView<const AddUniforms_UniformData> uniformBufferRef(uniforms, uniformCount);
	AddUniforms(shaderOut, uniformBufferRef, allocator);

	size_t stageCount = 0;
	struct StageInfo
	{
		MemberItr stageItr;
		MemberItr mainItr;
		MemberItr includeItr;
		RenderShaderStage stage;
	}
	stages[MaxStageCount];

	MemberItr vsItr = config.FindMember("vs");
	MemberItr fsItr = config.FindMember("fs");
	MemberItr csItr = config.FindMember("cs");

	if (vsItr != config.MemberEnd() && vsItr->value.IsObject() ||
		fsItr != config.MemberEnd() && fsItr->value.IsObject())
	{
		stages[0].stageItr = vsItr;
		stages[0].stage = RenderShaderStage::VertexShader;

		stages[1].stageItr = fsItr;
		stages[1].stage = RenderShaderStage::FragmentShader;

		stageCount = 2;
	}
	else if (csItr != config.MemberEnd() && csItr->value.IsObject())
	{
		stages[0].stageItr = csItr;
		stages[0].stage = RenderShaderStage::ComputeShader;

		stageCount = 1;
	}
	else
		return false;

	for (size_t i = 0; i < stageCount; ++i)
	{
		stages[i].mainItr = stages[i].stageItr->value.FindMember("main");

		if (stages[i].mainItr == stages[i].stageItr->value.MemberEnd() || !stages[i].mainItr->value.IsString())
			return false;
	}

	// Load all include files, they can be shared between shader stages

	HashMap<uint32_t, FileString> includeFileCache(allocator);

	bool includeLoadSuccess = true;

	for (size_t i = 0; i < stageCount; ++i)
	{
		stages[i].includeItr = stages[i].stageItr->value.FindMember("includes");
		if (stages[i].includeItr != stages[i].stageItr->value.MemberEnd())
		{
			if (LoadIncludes(stages[i].includeItr->value, includeFileCache, allocator) == false)
			{
				includeLoadSuccess = false;
			}
		}
	}

	// Process included files into complete source

	bool processSuccess = true;

	// TODO: Make this definition more robust
	Array<char> sourceBuffers[MaxStageCount] = {
		Array<char>(allocator),
		Array<char>(allocator)
	};

	if (includeLoadSuccess)
	{
		StringRef versionStr("#version 450\n");
		StringRef uniformBlock = shaderOut.uniformBlockDefinition;

		for (size_t i = 0; i < stageCount; ++i)
		{
			const char* stagePath = stages[i].mainItr->value.GetString();

			const rapidjson::Value* includeVal = nullptr;
			if (stages[i].includeItr != stages[i].stageItr->value.MemberEnd())
				includeVal = &stages[i].includeItr->value;

			if (ProcessSource(stagePath, versionStr, uniformBlock, includeVal, includeFileCache, allocator, sourceBuffers[i]) == false)
				processSuccess = false;
		}
	}

	bool success = false;
	StageSource stageSources[MaxStageCount];

	for (size_t i = 0; i < stageCount; ++i)
	{
		stageSources[i].stage = stages[i].stage;
		stageSources[i].source = StringRef(sourceBuffers[i].GetData(), sourceBuffers[i].GetCount());
	}

	ArrayView<const StageSource> stageSourceRef(stageSources, stageCount);

	if (processSuccess &&
		CompileAndLink(shaderOut, stageSourceRef, allocator, renderDevice, debugName))
	{
		UpdateTextureUniformLocations(shaderOut, renderDevice);

		success = true;
	}

	// Release loaded include files
	for (auto itr = includeFileCache.Begin(), end = includeFileCache.End(); itr != end; ++itr)
	{
		allocator->Deallocate(itr->second.string);
	}

	return success;
}


static bool FindShaderSections(
	StringRef shaderContents,
	StringRef& programSectionOut,
	ShaderLoader::StageSource stageSectionsOut[MaxStageCount],
	size_t& stageCountOut)
{
	KOKKO_PROFILE_FUNCTION();

	const StringRef stageDeclStr("#stage ");

	intptr_t sectionStart = 0;
	intptr_t sectionContentStart = 0;
	size_t stageCount = 0;
	RenderShaderStage currentStage = RenderShaderStage::VertexShader;

	for (;;)
	{
		sectionStart = shaderContents.FindFirst(stageDeclStr, sectionContentStart);

		if (sectionContentStart == 0)
		{
			if (sectionStart >= 0)
				programSectionOut = shaderContents.SubStrPos(0, sectionStart);
			else
				break;
		}
		else // Finish the previous stage
		{
			size_t sectionEnd = sectionStart >= 0 ? sectionStart : shaderContents.len;

			stageSectionsOut[stageCount].stage = currentStage;
			stageSectionsOut[stageCount].source = shaderContents.SubStrPos(sectionContentStart, sectionEnd);

			stageCount += 1;

			assert(stageCount <= MaxStageCount);
		}

		if (sectionStart < 0)
			break;

		intptr_t lineEnd = shaderContents.FindFirstOf(LineBreakChars, sectionStart + stageDeclStr.len);

		if (lineEnd < 0)
			break;

		StringRef sectionName = shaderContents.SubStrPos(sectionStart + stageDeclStr.len, lineEnd);

		if (sectionName == StringRef("vertex"))
			currentStage = RenderShaderStage::VertexShader;
		else if (sectionName == StringRef("fragment"))
			currentStage = RenderShaderStage::FragmentShader;
		else if (sectionName == StringRef("compute"))
			currentStage = RenderShaderStage::ComputeShader;

		sectionContentStart = shaderContents.FindFirstNotOf(LineBreakChars, lineEnd);

		if (sectionContentStart < 0)
			break;
	}

	if (stageCount > 0)
	{
		stageCountOut = stageCount;
		return true;
	}
	else
		return false;
}

static void ProcessProgramProperties(ShaderData& shaderOut, StringRef programSection, Allocator* allocator)
{
	KOKKO_PROFILE_FUNCTION();

	static const size_t MaxUniformCount = 32;
	ShaderLoader::AddUniforms_UniformData uniforms[MaxUniformCount];
	unsigned int uniformCount = 0;

	const StringRef propertyStr("#property ");

	size_t findStart = 0;

	for (;;)
	{
		intptr_t lineStart = programSection.FindFirst(propertyStr, findStart);
		
		if (lineStart < 0)
			break;

		intptr_t nameStart = lineStart + propertyStr.len;
		intptr_t lineEnd = programSection.FindFirstOf(LineBreakChars, nameStart);

		if (lineEnd < 0)
			lineEnd = programSection.len;

		findStart = lineEnd;

		StringRef nameAndType = programSection.SubStrPos(nameStart, lineEnd);
		intptr_t nameEnd = nameAndType.FindFirstOf(WhitespaceChars);

		if (nameEnd < 0 || nameAndType.str[nameEnd] == '\r' || nameAndType.str[nameEnd] == '\n')
			break;

		StringRef nameStr = nameAndType.SubStr(0, nameEnd);
		StringRef typeStr = nameAndType.SubStr(nameEnd + 1);

		ShaderLoader::AddUniforms_UniformData& uniform = uniforms[uniformCount];
		uniform.name = nameStr;

		uint32_t typeHash = Hash::FNV1a_32(typeStr.str, typeStr.len);

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
			// TODO: Implement array type size
			KK_LOG_ERROR("Failed to parse shader because array uniforms aren't implemented");
		}

		uniformCount += 1;
	}

	ArrayView<const ShaderLoader::AddUniforms_UniformData> uniformBufferRef(uniforms, uniformCount);
	AddUniforms(shaderOut, uniformBufferRef, allocator);

	// Set default value
	shaderOut.transparencyType = TransparencyType::Opaque;
	// TODO: Read value from shader declaration
}

static bool ProcessIncludes(
	StringRef sourceStr,
	uint32_t filePathHash,
	ShaderLoader::ProcessStore& store,
	Allocator* allocator,
	String& processedSourceOut)
{
	const StringRef includeDeclStr("#include ");

	store.filesIncludedInStage.Insert(filePathHash);

	size_t includeStatementEnd = 0;

	bool success = true;

	for (;;) // For each include statement
	{
		auto FindQuote = [sourceStr](intptr_t& quotePosOut, size_t startPos) -> bool
		{
			quotePosOut = sourceStr.FindFirstOf("\"\n\r", startPos);

			if (quotePosOut < 0 || sourceStr[quotePosOut] != '\"')
				return false;

			return true;
		};

		intptr_t lineStart = sourceStr.FindFirst(includeDeclStr, includeStatementEnd);

		if (lineStart < 0)
			break;

		intptr_t firstQuote, secondQuote;

		if (FindQuote(firstQuote, lineStart + includeDeclStr.len) == false)
		{
			success = false;
			break;
		}

		if (FindQuote(secondQuote, firstQuote + 1) == false)
		{
			success = false;
			break;
		}

		// Add the source that is between this and the previous include
		processedSourceOut.Append(sourceStr.SubStrPos(includeStatementEnd, lineStart));

		includeStatementEnd = secondQuote + 1;

		StringRef includePath = sourceStr.SubStrPos(firstQuote + 1, secondQuote);
		uint32_t pathHash = Hash::FNV1a_32(includePath.str, includePath.len);

		// Include file only if it hasn't been included yet in this shader stage
		if (store.filesIncludedInStage.Contains(pathHash) == false)
		{
			auto* file = store.includeFileCache.Lookup(pathHash);

			// File with this hash hasn't been read before, read it now
			if (file == nullptr)
			{
				store.pathString.Assign(includePath);

				char* stringBuffer;
				size_t stringLength;

				if (File::ReadText(store.pathString.GetCStr(), allocator, stringBuffer, stringLength))
				{
					ShaderLoader::FileString str;
					str.string = stringBuffer;
					str.length = stringLength;

					file = store.includeFileCache.Insert(pathHash);
					file->second = str;
				}
				else
				{
					KK_LOG_ERROR("Shader include couldn't be read from file: {}", store.pathString.GetCStr());
				}
			}

			if (file != nullptr) 
			{
				StringRef includeFileContent(file->second.string, file->second.length);

				if (ProcessIncludes(includeFileContent, pathHash, store, allocator, processedSourceOut) == false)
				{
					success = false;
					break;
				}
			}
			else // File hadn't been loaded before and couldn't be loaded now
			{
				success = false;
				break;
			}
		}
	}

	if (success)
	{
		// Add the source after the last include
		// or the whole source string if no includes were present
		processedSourceOut.Append(sourceStr.SubStr(includeStatementEnd));
	}

	return success;
}

static bool ProcessStage(
	StringRef versionStr,
	StringRef uniformBlockDefinition,
	StringRef mainFilePath,
	StringRef mainFileContent,
	ShaderLoader::ProcessStore& store,
	Allocator* allocator,
	String& processedSourceOut)
{
	processedSourceOut.Clear();
	processedSourceOut.Append(versionStr);

	if (uniformBlockDefinition.len > 0)
		processedSourceOut.Append(uniformBlockDefinition);

	uint32_t pathHash = Hash::FNV1a_32(mainFilePath.str, mainFilePath.len);

	if (ProcessIncludes(mainFileContent, pathHash, store, allocator, processedSourceOut) == false)
		return false;

	store.filesIncludedInStage.Clear();
	store.pathString.Clear();

	return true;
}

static bool ProcessShaderStages(
	ShaderData& shaderOut,
	StringRef shaderPath,
	ArrayView<const ShaderLoader::StageSource> stages,
	StringRef versionStr,
	Allocator* allocator,
	RenderDevice* renderDevice,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	HashMap<uint32_t, ShaderLoader::FileString> includeFileCache(allocator);
	SortedArray<uint32_t> filesIncludedInStage(allocator);
	String pathString(allocator);

	ShaderLoader::ProcessStore processStore{
		includeFileCache, filesIncludedInStage, pathString
	};

	String processedStageSources[MaxStageCount];
	for (size_t i = 0; i < MaxStageCount; ++i)
		processedStageSources[i] = String(allocator);

	bool processSuccess = true;

	for (size_t i = 0, count = stages.GetCount(); i < count; ++i)
	{
		if (ProcessStage(versionStr, shaderOut.uniformBlockDefinition, shaderPath, stages[i].source,
			processStore, allocator, processedStageSources[i]) == false)
			processSuccess = false;
	}

	bool compileSuccess = false;

	if (processSuccess)
	{
		ShaderLoader::StageSource stageSources[MaxStageCount];
		
		for (size_t i = 0, count = stages.GetCount(); i < count; ++i)
			stageSources[i] = ShaderLoader::StageSource{ stages[i].stage, processedStageSources[i].GetRef() };

		ArrayView<const ShaderLoader::StageSource> stageSourceRef(stageSources, stages.GetCount());

		if (CompileAndLink(shaderOut, stageSourceRef, allocator, renderDevice, debugName))
		{
			UpdateTextureUniformLocations(shaderOut, renderDevice);
			compileSuccess = true;
		}
	}

	// Release loaded include files
	for (auto itr = includeFileCache.Begin(), end = includeFileCache.End(); itr != end; ++itr)
	{
		allocator->Deallocate(itr->second.string);
	}

	return compileSuccess;
}

bool ShaderLoader::LoadFromShaderFile(
	ShaderData& shaderOut,
	StringRef shaderPath,
	StringRef shaderContent,
	Allocator* allocator,
	RenderDevice* renderDevice,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	StringRef programSection;
	StageSource stageSections[MaxStageCount];
	size_t stageCount;
	
	if (FindShaderSections(shaderContent, programSection, stageSections, stageCount) == false)
		return false;

	ProcessProgramProperties(shaderOut, programSection, allocator);

	StringRef versionStr("#version 450\n");
	ArrayView<const StageSource> stages(stageSections, stageCount);
	if (ProcessShaderStages(shaderOut, shaderPath, stages, versionStr, allocator, renderDevice, debugName) == false)
		return false;

	return true;
}
