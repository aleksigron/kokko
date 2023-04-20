#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Rendering/UniformList.hpp"
#include "Rendering/RenderResourceId.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;

namespace kokko
{

class AssetLoader;
class Filesystem;

namespace render
{
class Device;
}
}

struct ShaderData
{
	kokko::Uid uid;

	void* buffer;
	kokko::ConstStringView uniformBlockDefinition;
	kokko::ConstStringView path;

	TransparencyType transparencyType;

	kokko::render::ShaderId driverId;

	kokko::UniformList uniforms;
};

class ShaderManager
{
private:
	Allocator* allocator;
	kokko::Filesystem* filesystem;
	kokko::AssetLoader* assetLoader;
	kokko::render::Device* renderDevice;

	struct InstanceData
	{
		size_t count;
		size_t allocated;
		void* buffer;

		unsigned int* freeList;
		ShaderData* shader;
	}
	data;

	unsigned int freeListFirst;
	HashMap<kokko::Uid, ShaderId> uidMap;

	void Reallocate(size_t required);

public:
	ShaderManager(
		Allocator* allocator,
		kokko::Filesystem* filesystem,
		kokko::AssetLoader* assetLoader,
		kokko::render::Device* renderDevice);
	~ShaderManager();

	ShaderId CreateShader();
	void RemoveShader(ShaderId id);

	ShaderId FindShaderByUid(const kokko::Uid& uid);
	ShaderId FindShaderByPath(kokko::ConstStringView path);

	const ShaderData& GetShaderData(ShaderId id) const
	{
		return data.shader[id.i];
	}
};
