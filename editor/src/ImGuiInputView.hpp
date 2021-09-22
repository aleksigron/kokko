#pragma once

#include "System/InputView.hpp"

class ImGuiInputView final : public FilterInputView
{
public:
	ImGuiInputView(InputView* source);

	virtual bool WantsMouseInput() override;
	virtual bool WantsKeyboardInput() override;
	virtual bool WantsTextInput() override;
};

