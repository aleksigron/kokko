#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

class Allocator;
class Filesystem;
class RenderDevice;

struct ShaderData;

namespace ShaderLoader
{
	bool LoadFromConfiguration(
		ShaderData& shaderOut,
		StringRef configuration,
		Allocator* allocator,
		Filesystem* filesystem,
		RenderDevice* renderDevice,
		StringRef debugName);
}

class ShaderFileLoader
{
public:
	ShaderFileLoader(Allocator* allocator,
		Filesystem* filesystem,
		RenderDevice* renderDevice);

	~ShaderFileLoader();

	bool LoadFromFile(
		ShaderData& shaderOut,
		StringRef shaderPath,
		StringRef shaderContent,
		StringRef debugName);

private:
	struct StageSource
	{
		RenderShaderStage stage;
		StringRef source;
	};

	static const size_t MaxStageCount = 2;

	static const char* const LineBreakChars;
	static const char* const WhitespaceChars;

	Allocator* allocator;
	Filesystem* filesystem;
	RenderDevice* renderDevice;

	HashMap<uint32_t, ArrayView<char>> includeFileCache;
	SortedArray<uint32_t> filesIncludedInStage;
	String pathString;
	String processedStageSources[MaxStageCount];

	bool FindShaderSections(
		StringRef shaderContents,
		StringRef& programSectionOut,
		StageSource stageSectionsOut[MaxStageCount],
		size_t& stageCountOut);

	void ProcessProgramProperties(
		ShaderData& shaderOut,
		StringRef programSection);

	bool ProcessShaderStages(
		ShaderData& shaderOut,
		StringRef shaderPath,
		ArrayView<const StageSource> stages,
		StringRef versionStr,
		StringRef debugName);

	bool ProcessStage(
		StringRef versionStr,
		StringRef uniformBlockDefinition,
		StringRef mainFilePath,
		StringRef mainFileContent,
		String& processedSourceOut);

	bool ProcessIncludes(
		StringRef sourceStr,
		uint32_t filePathHash,
		String& processedSourceOut);
};
