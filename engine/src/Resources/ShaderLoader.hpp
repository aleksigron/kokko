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
class RenderDevice;

struct ShaderData;

namespace kokko
{

class Filesystem;

}

class ShaderLoader
{
public:
	struct StageSource
	{
		RenderShaderStage stage;
		StringRef source;
	};

	ShaderLoader(Allocator* allocator,
		kokko::Filesystem* filesystem,
		RenderDevice* renderDevice);

	~ShaderLoader();

	bool LoadFromFile(
		ShaderData& shaderOut,
		StringRef shaderPath,
		StringRef shaderContent,
		StringRef debugName);

private:

	static const size_t MaxStageCount = 2;

	static const char* const LineBreakChars;
	static const char* const WhitespaceChars;

	Allocator* allocator;
	kokko::Filesystem* filesystem;
	RenderDevice* renderDevice;

	HashMap<uint32_t, kokko::String> includeFileCache;
	SortedArray<uint32_t> filesIncludedInStage;
	kokko::String pathString;
	kokko::String processedStageSources[MaxStageCount];

	bool FindShaderSections(
		StringRef shaderContents,
		StringRef& programSectionOut,
		StageSource stageSectionsOut[MaxStageCount],
		size_t& stageCountOut);

	void ProcessProgramProperties(
		ShaderData& shaderOut,
		StringRef programSection,
		StringRef shaderPath);

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
		kokko::String& processedSourceOut);

	bool ProcessIncludes(
		StringRef sourceStr,
		uint32_t filePathHash,
		kokko::String& processedSourceOut);
};
