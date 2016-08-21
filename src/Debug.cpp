#include "Debug.hpp"

#include <cstdio>

#include "DebugVectorRenderer.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugLogView.hpp"
#include "DebugLog.hpp"
#include "BitmapFont.hpp"

#include "KeyboardInput.hpp"

Debug::Debug(KeyboardInput* keyboardInput) :
	keyboardInput(keyboardInput),
	mode(DebugMode::None)
{
	vectorRenderer = new DebugVectorRenderer;
	textRenderer = new DebugTextRenderer;
	logView = new DebugLogView(textRenderer);
	log = new DebugLog(logView);
}

Debug::~Debug()
{
	delete log;
	delete logView;
	delete textRenderer;
}

void Debug::UpdateLogViewDrawArea()
{
	Vec2f size = textRenderer->GetScaledFrameSize();
	int lineHeight = textRenderer->GetFont()->GetLineHeight();

	Rectangle logArea;
	logArea.position.x = 0.0f;
	logArea.position.y = lineHeight;
	logArea.size.x = size.x;
	logArea.size.y = size.y - lineHeight;

	logView->SetDrawArea(logArea);
}

void Debug::Render(const Camera& camera)
{
	// Update mode
	if (keyboardInput->GetKeyDown(Key::N_1))
	{
		this->mode = DebugMode::None;
	}
	else if (keyboardInput->GetKeyDown(Key::N_2))
	{
		this->mode = DebugMode::LogView;
	}

	char modeNoneChar = (mode == DebugMode::None) ? '*' : ' ';
	char modeLogChar = (mode == DebugMode::LogView) ? '*' : ' ';

	// Draw debug mode guide
	char buffer[32];
	sprintf(buffer, "Debug mode: [1]None%c [2]Log%c", modeNoneChar, modeLogChar);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f), true);

	vectorRenderer->Render(camera);

	// Draw mode content
	switch (this->mode)
	{
		case DebugMode::None:
			break;

		case DebugMode::LogView:
			logView->DrawToTextRenderer();
			break;
	}

	// Draw debug texts
	textRenderer->Render();
}
