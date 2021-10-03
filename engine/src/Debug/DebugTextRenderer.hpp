#pragma once

#include "Core/Array.hpp"
#include "Core/StringRef.hpp"

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

#include "Resources/MeshData.hpp"

class Allocator;
class RenderDevice;
class Filesystem;
class BitmapFont;
class ShaderManager;
class MeshManager;
class TextureManager;

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
	RenderDevice* renderDevice;
	Filesystem* filesystem;
	ShaderManager* shaderManager;
	MeshManager* meshManager;

	BitmapFont* font;
	size_t stringCharCount;
	Array<char> stringData;
	Array<DisplayData> displayData;

	Vec2f frameSize;
	Vec2f scaledFrameSize;
	float scaleFactor;

	MeshId meshId;
	Array<float> vertexData;
	Array<unsigned short> indexData;

	unsigned int bufferObjectId;

	void CreateAndUploadData();

	bool LoadBitmapFont(TextureManager* textureManager, const char* filePath);

public:
	DebugTextRenderer(Allocator* allocator, RenderDevice* renderDevice, Filesystem* filesystem);
	~DebugTextRenderer();

	void Initialize(ShaderManager* shaderManager,
		MeshManager* meshManager, TextureManager* textureManager);

	bool HasValidFont() const { return font != nullptr; }

	void SetFrameSize(const Vec2f& size);
	void SetScaleFactor(float scale);

	Vec2f GetScaledFrameSize() const { return scaledFrameSize; }

	const BitmapFont* GetFont() const { return font; }

	size_t GetRowCountForTextLength(size_t characterCount) const;

	/**
	 * Add a text to be rendered this frame at a specified normalized position.
	 */
	void AddTextNormalized(StringRef str, Vec2f position);

	/**
	 * Add a text to be rendered this frame at a specified pixel position.
	 */
	void AddText(StringRef str, Vec2f position);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(StringRef str, const Rectanglef& area);

	void Render();
};
