#pragma once

#include "Array.hpp"
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
		unsigned int stringStart;
		unsigned int stringLength;
		Rectangle area;
	};

	BitmapFont* font;
	unsigned int stringCharCount;
	Array<char> stringData;
	Array<RenderData> renderData;

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

	const BitmapFont* GetFont() const { return font; }

	int GetRowCountForTextLength(unsigned int characterCount) const;

	/**
	 * Add a text to be rendered this frame at a specified position.
	 */
	void AddText(StringRef str, Vec2f position);

	/**
	 * Add a text to be rendered this frame in a specified area.
	 */
	void AddText(StringRef str, Rectangle area);

	void Render();
};
