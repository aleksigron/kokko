#include "Debug/DebugMemoryStats.hpp"

#include <cstdio>

#include "fmt/printf.h"

#include "Core/Color.hpp"
#include "Resources/BitmapFont.hpp"

#include "Memory/AllocatorManager.hpp"

#include "Debug/DebugTextRenderer.hpp"

DebugMemoryStats::DebugMemoryStats(AllocatorManager* allocatorManager, kokko::DebugTextRenderer* textRenderer) :
	allocatorManager(allocatorManager),
	textRenderer(textRenderer)
{
}

DebugMemoryStats::~DebugMemoryStats()
{
}

void DebugMemoryStats::SetDrawArea(const Rectanglef& area)
{
	drawArea = area;
}

void DebugMemoryStats::UpdateAndDraw()
{
	const Color white(1.0f, 1.0f, 1.0f);
	const unsigned int columnWidth0 = 24;
	const unsigned int columnWidth1 = 12;
	const unsigned int columnWidth2 = 12;

	const BitmapFont* font = textRenderer->GetFont();
	int lineHeight = font->GetLineHeight();
	int glyphWidth = font->GetGlyphWidth();
	Vec2f areaSize = this->drawArea.size;
	Vec2f areaPos = this->drawArea.position;

	{
		Rectanglef area;
		area.position.x = areaPos.x;
		area.position.y = areaPos.y;
		area.size.x = static_cast<float>(glyphWidth * columnWidth0);
		area.size.y = static_cast<float>(lineHeight);

		textRenderer->AddText(kokko::ConstStringView("Scope name"), area);
	}

	{
		Rectanglef area;
		area.position.x = areaPos.x + glyphWidth * columnWidth0;
		area.position.y = areaPos.y;
		area.size.x = static_cast<float>(glyphWidth * columnWidth1);
		area.size.y = static_cast<float>(lineHeight);

		textRenderer->AddText(kokko::ConstStringView("Count"), area);
	}

	{
		Rectanglef area;
		area.position.x = areaPos.x + glyphWidth * (columnWidth0 + columnWidth1);
		area.position.y = areaPos.y;
		area.size.x = static_cast<float>(glyphWidth * columnWidth2);
		area.size.y = static_cast<float>(lineHeight);

		textRenderer->AddText(kokko::ConstStringView("Size"), area);
	}

	char buffer[32];
	unsigned int scopeCount = allocatorManager->GetMemoryTrackingScopeCount();

	for (unsigned int i = 0; i < scopeCount; ++i)
	{
		const char* name = allocatorManager->GetNameForScopeIndex(i);
		std::size_t allocCount = allocatorManager->GetAllocationCountForScopeIndex(i);
		std::size_t allocSize = allocatorManager->GetAllocatedSizeForScopeIndex(i);

		{
			Rectanglef area;
			area.position.x = areaPos.x;
			area.position.y = areaPos.y + (lineHeight * (i + 1));
			area.size.x = static_cast<float>(glyphWidth * columnWidth0);
			area.size.y = static_cast<float>(lineHeight);

			textRenderer->AddText(kokko::ConstStringView(name), area);
		}

		{
			Rectanglef area;
			area.position.x = areaPos.x + glyphWidth * columnWidth0;
			area.position.y = areaPos.y + (lineHeight * (i + 1));
			area.size.x = static_cast<float>(glyphWidth * columnWidth1);
			area.size.y = static_cast<float>(lineHeight);

			fmt::format_to_n(buffer, sizeof(buffer), FMT_STRING("{}"), allocCount);
			textRenderer->AddText(kokko::ConstStringView(buffer), area);
		}

		{
			Rectanglef area;
			area.position.x = areaPos.x + glyphWidth * (columnWidth0 + columnWidth1);
			area.position.y = areaPos.y + (lineHeight * (i + 1));
			area.size.x = static_cast<float>(glyphWidth * columnWidth2);
			area.size.y = static_cast<float>(lineHeight);

			fmt::format_to_n(buffer, sizeof(buffer), FMT_STRING("{}"), allocSize);
			textRenderer->AddText(kokko::ConstStringView(buffer), area);
		}
	}
}
