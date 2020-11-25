#pragma once

#include "Core/Array.hpp"
#include "Core/StringRef.hpp"

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

#include "Resources/MeshData.hpp"

class Allocator;
class BitmapFont;
class RenderDevice;
class ShaderManager;
class TextureManager;

class DebugTextRenderer
{
private:
	struct RenderData
	{
		unsigned int stringStart;
		unsigned int stringLength;
		Rectanglef area;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;

	BitmapFont* font;
	unsigned int stringCharCount;
	Array<char> stringData;
	Array<RenderData> renderData;

	Vec2f frameSize;
	Vec2f scaledFrameSize;
	float scaleFactor;

	MeshId meshId;
	Array<float> vertexData;
	Array<unsigned short> indexData;

	unsigned int materialBufferObjectId;

	void CreateAndUploadData();

public:
	DebugTextRenderer(Allocator* allocator, RenderDevice* renderDevice);
	~DebugTextRenderer();

	void Initialize(ShaderManager* shaderManager);

	bool LoadBitmapFont(TextureManager* textureManager, const char* filePath);
	bool HasValidFont() const { return font != nullptr; }

	void SetFrameSize(const Vec2f& size);
	void SetScaleFactor(float scale);

	Vec2f GetScaledFrameSize() const { return scaledFrameSize; }

	const BitmapFont* GetFont() const { return font; }

	int GetRowCountForTextLength(unsigned int characterCount) const;

	/**
	 * Add a text to be rendered this frame at a specified position.
	 */
	void AddText(StringRef str, Vec2f position);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(StringRef str, const Rectanglef& area);

	void Render();
};
