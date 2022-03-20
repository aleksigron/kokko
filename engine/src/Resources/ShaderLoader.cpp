#include "Resources/ShaderLoader.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>

#include "fmt/format.h"

#include "rapidjson/document.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/CString.hpp"
#include "Core/Hash.hpp"
#include "Core/Range.hpp"
#include "Core/String.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/ShaderManager.hpp"

#include "System/Filesystem.hpp"

namespace
{
const size_t MaxStageCount = 2;

struct AddUniforms_UniformData
{
	StringRef name;
	unsigned int arraySize;
	kokko::UniformDataType type;
};

bool BufferUniformSortPredicate(const kokko::BufferUniform& a, const kokko::BufferUniform& b)
{
	const kokko::UniformTypeInfo& aType = kokko::UniformTypeInfo::Types[static_cast<unsigned int>(a.type)];
	const kokko::UniformTypeInfo& bType = kokko::UniformTypeInfo::Types[static_cast<unsigned int>(b.type)];
	return aType.size > bType.size;
}

void AddUniformsAndShaderPath(
	ShaderData& shaderOut,
	ArrayView<const AddUniforms_UniformData> uniforms,
	StringRef shaderPath,
	Allocator* allocator)
{
	KOKKO_PROFILE_FUNCTION();

	static_assert(UniformBlockBinding::Material < 10, "Material uniform block binding point must be less than 10");
	const char* blockStartFormat = "layout(std140, binding = {}) uniform MaterialBlock {{\n";
	const size_t blockStartPlaceholdersLen = 3;
	const size_t blockStartLen = std::strlen(blockStartFormat) - blockStartPlaceholdersLen + 1;
	const char* blockRowFormat = "{} {};\n";
	const size_t blockRowPlaceholdersLen = 4;
	const size_t blockRowFixedMaxLen = std::strlen(blockRowFormat) - blockRowPlaceholdersLen;
	const char* blockEnd = "};\n";
	const size_t blockEndLen = std::strlen(blockEnd);

	// Calculate how much memory we need to store:
	// - Uniform names
	// - Uniform block definition
	// - Buffer uniform definitions
	// - Texture uniform definitions
	// - Shader path string

	size_t nameBytes = 0;
	size_t uniformBlockBytes = blockStartLen + blockEndLen + 1;

	size_t textureUniformCount = 0;
	size_t bufferUniformCount = 0;

	for (size_t uIndex = 0, uCount = uniforms.GetCount(); uIndex < uCount; ++uIndex)
	{
		const AddUniforms_UniformData& uniform = uniforms[uIndex];
		kokko::UniformTypeInfo type = kokko::UniformTypeInfo::FromType(uniform.type);

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
	nameBytes = Math::RoundUpToMultiple(nameBytes, shaderDataAlignment);
	uniformBlockBytes = Math::RoundUpToMultiple(uniformBlockBytes, shaderDataAlignment);
	size_t shaderPathBytes = Math::RoundUpToMultiple(shaderPath.len + 1, shaderDataAlignment);

	size_t bufferUniformBytes = sizeof(kokko::BufferUniform) * bufferUniformCount;
	size_t textureUniformBytes = sizeof(kokko::TextureUniform) * textureUniformCount;

	size_t shaderDataBytes =
		nameBytes + uniformBlockBytes + shaderPathBytes + bufferUniformBytes + textureUniformBytes;

	// Allocate memory

	shaderOut.buffer = allocator->Allocate(shaderDataBytes);

	char* namePtr = static_cast<char*>(shaderOut.buffer);
	char* shaderDataEnd = namePtr + shaderDataBytes;
	char* uniformBlockPtr = namePtr + nameBytes;
	char* shaderPathPtr = uniformBlockPtr + uniformBlockBytes;
	kokko::BufferUniform* bufferUniformPtr = reinterpret_cast<kokko::BufferUniform*>(shaderPathPtr + shaderPathBytes);
	kokko::TextureUniform* textureUniformPtr = reinterpret_cast<kokko::TextureUniform*>(bufferUniformPtr + bufferUniformCount);

	shaderOut.uniforms.bufferUniforms = bufferUniformPtr;
	shaderOut.uniforms.textureUniforms = textureUniformPtr;

	// Copy uniform definitions to allocated memory
	
	unsigned int textureUniformsCopied = 0;
	unsigned int bufferUniformsCopied = 0;

	for (size_t uIndex = 0, uCount = uniforms.GetCount(); uIndex < uCount; ++uIndex)
	{
		kokko::ShaderUniform* baseUniform = nullptr;

		kokko::UniformDataType dataType = uniforms[uIndex].type;

		if (kokko::UniformTypeInfo::FromType(dataType).isTexture)
		{
			kokko::TextureUniform& uniform = shaderOut.uniforms.textureUniforms[textureUniformsCopied];

			// Since shader is not compiled at this point, we can't know the uniform location
			uniform.uniformLocation = -1;
			uniform.textureObject = 0;

			switch (dataType)
			{
			case kokko::UniformDataType::Tex2D:
				uniform.textureTarget = RenderTextureTarget::Texture2d;
				break;
			case kokko::UniformDataType::TexCube:
				uniform.textureTarget = RenderTextureTarget::TextureCubeMap;
				break;
			default:
				uniform.textureTarget = RenderTextureTarget::Texture2d;
				break;
			}

			++textureUniformsCopied;

			baseUniform = static_cast<kokko::ShaderUniform*>(&uniform);
		}
		else
		{
			kokko::BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[bufferUniformsCopied];
			uniform.dataOffset = 0;
			uniform.bufferObjectOffset = 0;
			uniform.arraySize = uniforms[uIndex].arraySize;

			++bufferUniformsCopied;

			baseUniform = static_cast<kokko::ShaderUniform*>(&uniform);
		}

		// Copy uniform name
		const StringRef& uniformName = uniforms[uIndex].name;

		std::memcpy(namePtr, uniformName.str, uniformName.len);
		namePtr[uniformName.len] = '\0';

		baseUniform->name = StringRef(namePtr, uniformName.len);
		namePtr += uniformName.len + 1;

		// Compute uniform name hash
		baseUniform->nameHash = kokko::HashString(baseUniform->name.str, baseUniform->name.len);
		baseUniform->type = dataType;
	}

	// Copy shader path
	std::memcpy(shaderPathPtr, shaderPath.str, shaderPath.len);
	shaderPathPtr[shaderPath.len] = '\0';
	shaderOut.path = StringRef(shaderPathPtr, shaderPath.len);

	shaderOut.uniforms.bufferUniformCount = bufferUniformCount;
	shaderOut.uniforms.textureUniformCount = textureUniformCount;

	// Order buffer uniforms based on size
	kokko::BufferUniform* begin = shaderOut.uniforms.bufferUniforms;
	kokko::BufferUniform* end = begin + bufferUniformCount;
	std::sort(begin, end, BufferUniformSortPredicate);

	// Calculate CPU and GPU buffer offsets for buffer uniforms

	unsigned int dataBufferOffset = 0;
	unsigned int bufferObjectOffset = 0;

	for (size_t i = 0, count = bufferUniformCount; i < count; ++i)
	{
		kokko::BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
		const kokko::UniformTypeInfo& type = kokko::UniformTypeInfo::FromType(uniform.type);
		
		// Round up the offset to match type aligment
		bufferObjectOffset = Math::RoundUpToMultiple(bufferObjectOffset, type.alignment);

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
	shaderOut.uniforms.uniformBufferSize = Math::RoundUpToMultiple(bufferObjectOffset, bufferSizeUnit);

	// Generate uniform block definition

	if (shaderOut.uniforms.bufferUniformCount == 0)
	{
		shaderOut.uniformBlockDefinition = StringRef();
	}
	else
	{
		{
			shaderOut.uniformBlockDefinition.str = uniformBlockPtr;

			auto bufLeft = shaderDataEnd - uniformBlockPtr;
			uint32_t materialBind = UniformBlockBinding::Material;

			auto formatRes = fmt::format_to_n(uniformBlockPtr, bufLeft, blockStartFormat, materialBind);
			assert(static_cast<ptrdiff_t>(formatRes.size) <= bufLeft);
			uniformBlockPtr += formatRes.size;
		}

		for (size_t i = 0, count = shaderOut.uniforms.bufferUniformCount; i < count; ++i)
		{
			const kokko::BufferUniform& uniform = shaderOut.uniforms.bufferUniforms[i];
			const kokko::UniformTypeInfo& typeInfo = kokko::UniformTypeInfo::FromType(uniform.type);

			auto bufLeft = shaderDataEnd - uniformBlockPtr;

			// TODO: StringRef doesn't always refer to a null-terminated string, so fix this
			auto formatRes = fmt::format_to_n(
				uniformBlockPtr, bufLeft, blockRowFormat, typeInfo.typeName, uniform.name.str);
			assert(static_cast<ptrdiff_t>(formatRes.size) <= bufLeft);
			uniformBlockPtr += formatRes.size;
		}

		StringCopyN(uniformBlockPtr, blockEnd, blockEndLen + 1);
		uniformBlockPtr += blockEndLen;

		shaderOut.uniformBlockDefinition.len = uniformBlockPtr - shaderOut.uniformBlockDefinition.str;
	}
}

void UpdateTextureUniformLocations(
	ShaderData& shaderInOut,
	RenderDevice* renderDevice)
{
	KOKKO_PROFILE_FUNCTION();

	for (size_t idx = 0, count = shaderInOut.uniforms.textureUniformCount; idx < count; ++idx)
	{
		kokko::TextureUniform& u = shaderInOut.uniforms.textureUniforms[idx];
		u.uniformLocation = renderDevice->GetUniformLocation(shaderInOut.driverId, u.name.str);
	}
}

bool Compile(
	ShaderData& shaderOut,
	Allocator* allocator,
	RenderDevice* renderDevice,
	RenderShaderStage stage,
	StringRef source,
	unsigned int& shaderIdOut)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int shaderId = renderDevice->CreateShaderStage(stage);

	renderDevice->SetShaderStageSource(shaderId, source.str, static_cast<int>(source.len));
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
			kokko::String infoLog(allocator);
			infoLog.Resize(infoLogLength);

			// Print out info log
			renderDevice->GetShaderStageInfoLog(shaderId, infoLogLength, infoLog.Begin());

			KK_LOG_ERROR("Shader stage compilation failed:\n{}\nSource:\n{}", infoLog.GetCStr(), source.str);
		}
	}

	return false;
}

bool CompileAndLink(
	ShaderData& shaderOut,
	ArrayView<const ShaderLoader::StageSource> stages,
	Allocator* allocator,
	RenderDevice* renderDevice,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int stageObjects[MaxStageCount] = { 0 };

	for (size_t i = 0, count = stages.GetCount(); i < stages.GetCount(); ++i)
	{
		RenderShaderStage stageType = stages[i].stage;
		StringRef source = stages[i].source;

		unsigned int stageObject = 0;
		bool compiled = Compile(shaderOut, allocator, renderDevice, stageType, source, stageObject);

		if (compiled == false)
		{
			for (size_t j = 0; j < i; ++j)
				renderDevice->DestroyShaderStage(stageObjects[j]);

			return false;
		}

		stageObjects[i] = stageObject;
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
			kokko::String infoLog(allocator);
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

} // Anonymous namespace

// ========================
// ===== ShaderLoader =====
// ========================

const char* const ShaderLoader::LineBreakChars = "\r\n";
const char* const ShaderLoader::WhitespaceChars = " \t\r\n";

ShaderLoader::ShaderLoader(Allocator* allocator, kokko::Filesystem* filesystem, RenderDevice* renderDevice) :
	allocator(allocator),
	filesystem(filesystem),
	renderDevice(renderDevice),
	includeFileCache(allocator),
	filesIncludedInStage(allocator),
	pathString(allocator)
{
	for (size_t i = 0; i < MaxStageCount; ++i)
		processedStageSources[i] = kokko::String(allocator);
}

ShaderLoader::~ShaderLoader()
{
}

bool ShaderLoader::LoadFromFile(
	ShaderData& shaderOut,
	StringRef shaderPath,
	StringRef shaderContent,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	StringRef programSection;
	StageSource stageSections[MaxStageCount];
	size_t stageCount;
	
	if (FindShaderSections(shaderContent, programSection, stageSections, stageCount) == false)
		return false;

	ProcessProgramProperties(shaderOut, programSection, shaderPath);

	StringRef versionStr("#version 450\n");
	ArrayView<const StageSource> stages(stageSections, stageCount);
	if (ProcessShaderStages(shaderOut, shaderPath, stages, versionStr, debugName) == false)
		return false;

	return true;
}

bool ShaderLoader::FindShaderSections(
	StringRef shaderContents,
	StringRef& programSectionOut,
	StageSource stageSectionsOut[MaxStageCount],
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

void ShaderLoader::ProcessProgramProperties(ShaderData& shaderOut, StringRef programSection, StringRef shaderPath)
{
	KOKKO_PROFILE_FUNCTION();

	static const size_t MaxUniformCount = 32;
	AddUniforms_UniformData uniforms[MaxUniformCount];
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

		AddUniforms_UniformData& uniform = uniforms[uniformCount];
		uniform.name = nameStr;

		uint32_t typeHash = kokko::HashString(typeStr.str, typeStr.len);

		switch (typeHash)
		{
		case "tex2d"_hash: uniform.type = kokko::UniformDataType::Tex2D; break;
		case "texCube"_hash: uniform.type = kokko::UniformDataType::TexCube; break;
		case "mat4x4"_hash: uniform.type = kokko::UniformDataType::Mat4x4; break;
		case "mat4x4Array"_hash: uniform.type = kokko::UniformDataType::Mat4x4Array; break;
		case "vec4"_hash: uniform.type = kokko::UniformDataType::Vec4; break;
		case "vec4Array"_hash: uniform.type = kokko::UniformDataType::Vec4Array; break;
		case "vec3"_hash: uniform.type = kokko::UniformDataType::Vec3; break;
		case "vec3Array"_hash: uniform.type = kokko::UniformDataType::Vec3Array; break;
		case "vec2"_hash: uniform.type = kokko::UniformDataType::Vec2; break;
		case "vec2Array"_hash: uniform.type = kokko::UniformDataType::Vec2Array; break;
		case "float"_hash: uniform.type = kokko::UniformDataType::Float; break;
		case "floatArray"_hash: uniform.type = kokko::UniformDataType::FloatArray; break;
		case "int"_hash: uniform.type = kokko::UniformDataType::Int; break;
		case "intArray"_hash: uniform.type = kokko::UniformDataType::IntArray; break;

		default:
			break;
		}

		uniform.arraySize = 0;

		if (kokko::UniformTypeInfo::FromType(uniform.type).isArray)
		{
			// TODO: Implement array type size
			KK_LOG_ERROR("Failed to parse shader because array uniforms aren't implemented");
		}

		uniformCount += 1;
	}

	ArrayView<const AddUniforms_UniformData> uniformBufferRef(uniforms, uniformCount);
	AddUniformsAndShaderPath(shaderOut, uniformBufferRef, shaderPath, allocator);

	// Set default value
	shaderOut.transparencyType = TransparencyType::Opaque;
	// TODO: Read value from shader declaration
}

bool ShaderLoader::ProcessShaderStages(
	ShaderData& shaderOut,
	StringRef shaderPath,
	ArrayView<const StageSource> stages,
	StringRef versionStr,
	StringRef debugName)
{
	KOKKO_PROFILE_FUNCTION();

	bool processSuccess = true;

	for (size_t i = 0, count = stages.GetCount(); i < count; ++i)
	{
		if (ProcessStage(versionStr, shaderOut.uniformBlockDefinition, shaderPath,
			stages[i].source, processedStageSources[i]) == false)
			processSuccess = false;
	}

	bool compileSuccess = false;

	if (processSuccess)
	{
		StageSource stageSources[MaxStageCount];

		for (size_t i = 0, count = stages.GetCount(); i < count; ++i)
			stageSources[i] = StageSource{ stages[i].stage, processedStageSources[i].GetRef() };

		ArrayView<const StageSource> stageSourceRef(stageSources, stages.GetCount());

		if (CompileAndLink(shaderOut, stageSourceRef, allocator, renderDevice, debugName))
		{
			UpdateTextureUniformLocations(shaderOut, renderDevice);
			compileSuccess = true;
		}
	}

	return compileSuccess;
}

bool ShaderLoader::ProcessStage(
	StringRef versionStr,
	StringRef uniformBlockDefinition,
	StringRef mainFilePath,
	StringRef mainFileContent,
	kokko::String& processedSourceOut)
{
	processedSourceOut.Clear();
	processedSourceOut.Append(versionStr);

	if (uniformBlockDefinition.len > 0)
		processedSourceOut.Append(uniformBlockDefinition);

	uint32_t pathHash = kokko::HashString(mainFilePath.str, mainFilePath.len);

	if (ProcessIncludes(mainFileContent, pathHash, processedSourceOut) == false)
		return false;

	filesIncludedInStage.Clear();
	pathString.Clear();

	return true;
}

bool ShaderLoader::ProcessIncludes(
	StringRef sourceStr,
	uint32_t filePathHash,
	kokko::String& processedSourceOut)
{
	const StringRef includeDeclStr("#include ");

	filesIncludedInStage.Insert(filePathHash);

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
		uint32_t pathHash = kokko::HashString(includePath.str, includePath.len);

		// Include file only if it hasn't been included yet in this shader stage
		if (filesIncludedInStage.Contains(pathHash) == false)
		{
			auto* file = includeFileCache.Lookup(pathHash);

			// File with this hash hasn't been read before, read it now
			if (file == nullptr)
			{
				pathString.Assign(includePath);

				kokko::String fileContent(allocator);

				if (filesystem->ReadText(pathString.GetCStr(), fileContent))
				{
					file = includeFileCache.Insert(pathHash);
					file->second = std::move(fileContent);
				}
				else
				{
					KK_LOG_ERROR("Shader include couldn't be read from file: {}", pathString.GetCStr());
				}
			}

			if (file != nullptr)
			{
				if (ProcessIncludes(file->second.GetRef(), pathHash, processedSourceOut) == false)
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
