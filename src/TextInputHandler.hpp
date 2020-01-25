#pragma once

#include "Core/StringRef.hpp"

class TextInputHandler
{
public:
	virtual void OnTextInput(StringRef text) = 0;
};
