#pragma once

#include "StringRef.hpp"
#include "Rectangle.hpp"
#include "Vec2.hpp"

class BitmapFont;
struct Mesh;

class DebugTextRenderer
{
private:
	struct RenderData
	{
		StringRef string;
		Rectangle area;
	};

	BitmapFont* font;

	char* stringData;
	unsigned int stringDataUsed;
	unsigned int stringDataAllocated;

	RenderData* renderData;
	unsigned int renderDataCount;
	unsigned int renderDataAllocated;

	Vec2f frameSize;
	Vec2f scaledFrameSize;
	float scaleFactor;

	void CreateAndUploadData(Mesh& mesh);

public:
	DebugTextRenderer();
	~DebugTextRenderer();

	bool LoadBitmapFont(const char* filePath);
	bool HasValidFont() const { return font != nullptr; }

	void SetFrameSize(const Vec2f& size);
	void SetScaleFactor(float scale);

	Vec2f GetScaledFrameSize() const { return scaledFrameSize; }

	/**
	 * Add a text to be rendered this frame at a specified position.
	 */
	void AddText(StringRef str, Vec2f position, bool copyString);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(StringRef str, Rectangle area, bool copyString);

	void Render();
};
