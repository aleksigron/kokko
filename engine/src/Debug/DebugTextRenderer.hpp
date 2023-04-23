#pragma once

#include "Core/Array.hpp"
#include "Core/StringView.hpp"

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"

class Allocator;
class BitmapFont;

namespace kokko
{

class Filesystem;
class MeshManager;
class TextureManager;
class ShaderManager;

namespace render
{

class CommandEncoder;
class Device;

} // namespace render
} // namespace kokko

class DebugTextRenderer
{
private:
	struct DisplayData
	{
		size_t stringStart;
		size_t stringLength;
		Rectanglef area;
	};

	Allocator* allocator;
	kokko::render::Device* renderDevice;
	kokko::Filesystem* filesystem;
	kokko::ShaderManager* shaderManager;
	kokko::MeshManager* meshManager;

	BitmapFont* font;
	size_t stringCharCount;
	Array<char> stringData;
	Array<DisplayData> displayData;

	Vec2f frameSize;
	Vec2f scaledFrameSize;
	float scaleFactor;

	kokko::MeshId meshId;
	Array<float> vertexData;
	Array<unsigned short> indexData;

	kokko::render::BufferId bufferObjectId;

	void CreateAndUploadData();

	bool LoadBitmapFont(kokko::TextureManager* textureManager, const char* filePath);

public:
	DebugTextRenderer(Allocator* allocator, kokko::render::Device* renderDevice, kokko::Filesystem* filesystem);
	~DebugTextRenderer();

	bool Initialize(kokko::ShaderManager* shaderManager,
		kokko::MeshManager* meshManager, kokko::TextureManager* textureManager);

	bool HasValidFont() const { return font != nullptr; }

	void SetFrameSize(const Vec2f& size);
	void SetScaleFactor(float scale);

	Vec2f GetScaledFrameSize() const { return scaledFrameSize; }

	const BitmapFont* GetFont() const { return font; }

	size_t GetRowCountForTextLength(size_t characterCount) const;

	/**
	 * Add a text to be rendered this frame at a specified normalized position.
	 */
	void AddTextNormalized(kokko::ConstStringView str, Vec2f position);

	/**
	 * Add a text to be rendered this frame at a specified pixel position.
	 */
	void AddText(kokko::ConstStringView str, Vec2f position);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(kokko::ConstStringView str, const Rectanglef& area);

	void Render(kokko::render::CommandEncoder* encoder);
};
