#pragma once

#include "Core/Array.hpp"
#include "Core/StringView.hpp"
#include "Core/UniquePtr.hpp"

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"

namespace kokko
{

class Allocator;
class BitmapFont;
class Filesystem;
class ModelManager;
class TextureManager;
class ShaderManager;

namespace render
{

class CommandEncoder;
class Device;

} // namespace render

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
	render::Device* renderDevice;
	Filesystem* filesystem;
	ShaderManager* shaderManager;
	ModelManager* modelManager;

	UniquePtr<BitmapFont> font;
	size_t stringCharCount;
	Array<char> stringData;
	Array<DisplayData> displayData;

	Vec2f frameSize;
	Vec2f scaledFrameSize;
	float scaleFactor;

	ModelId meshId;
	Array<float> vertexData;
	Array<unsigned short> indexData;

	render::BufferId bufferObjectId;

	void CreateAndUploadData();

	bool LoadBitmapFont(TextureManager* textureManager, const char* filePath);

public:
	DebugTextRenderer(Allocator* allocator, render::Device* renderDevice, Filesystem* filesystem);
	~DebugTextRenderer();

	bool Initialize(ShaderManager* shaderManager,
		ModelManager* modelManager, TextureManager* textureManager);

	bool HasValidFont() const { return font != nullptr; }

	void SetFrameSize(const Vec2f& size);
	void SetScaleFactor(float scale);

	Vec2f GetScaledFrameSize() const { return scaledFrameSize; }

	const BitmapFont* GetFont() const { return font.Get(); }

	size_t GetRowCountForTextLength(size_t characterCount) const;

	/**
	 * Add a text to be rendered this frame at a specified normalized position.
	 */
	void AddTextNormalized(ConstStringView str, Vec2f position);

	/**
	 * Add a text to be rendered this frame at a specified pixel position.
	 */
	void AddText(ConstStringView str, Vec2f position);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(ConstStringView str, const Rectanglef& area);

	void Render(render::CommandEncoder* encoder);
};

} // namespace kokko
