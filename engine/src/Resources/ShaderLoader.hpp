#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "Rendering/RenderTypes.hpp"

class Allocator;
class RenderDevice;

struct ShaderData;

namespace kokko
{

class Filesystem;

class ShaderLoader
{
public:
	struct StageSource
	{
		RenderShaderStage stage;
		ConstStringView source;
	};

	ShaderLoader(Allocator* allocator,
		Filesystem* filesystem,
		RenderDevice* renderDevice);

	~ShaderLoader();

	bool LoadFromFile(
		ShaderData& shaderOut,
		ConstStringView shaderPath,
		ConstStringView shaderContent,
		ConstStringView debugName);

	static constexpr size_t MaxStageCount = 3;

private:

	static const char* const LineBreakChars;
	static const char* const WhitespaceChars;

	Allocator* allocator;
	Filesystem* filesystem;
	RenderDevice* renderDevice;

	HashMap<uint32_t, kokko::String> includeFileCache;
	SortedArray<uint32_t> filesIncludedInStage;
	String pathString;
	String processedStageSources[MaxStageCount];

	bool FindShaderSections(
		ConstStringView shaderContents,
		ConstStringView& programSectionOut,
		StageSource stageSectionsOut[MaxStageCount],
		size_t& stageCountOut);

	void ProcessProgramProperties(
		ShaderData& shaderOut,
		ConstStringView programSection,
		ConstStringView shaderPath);

	bool ProcessShaderStages(
		ShaderData& shaderOut,
		ConstStringView shaderPath,
		ArrayView<const StageSource> stages,
		ConstStringView versionStr,
		ConstStringView debugName);

	bool ProcessStage(
		ConstStringView versionStr,
		ConstStringView uniformBlockDefinition,
		ConstStringView mainFilePath,
		ConstStringView mainFileContent,
		String& processedSourceOut);

	bool ProcessIncludes(
		ConstStringView sourceStr,
		uint32_t filePathHash,
		String& processedSourceOut);
};

}
