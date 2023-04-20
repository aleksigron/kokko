#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

namespace kokko
{

namespace render
{

class CommandEncoder;

class CommandEncoderDebugScope
{
public:
	CommandEncoderDebugScope(CommandEncoder* commandEncoder, uint32_t id, kokko::ConstStringView message);
	CommandEncoderDebugScope(const CommandEncoderDebugScope&) = delete;
	CommandEncoderDebugScope(CommandEncoderDebugScope&& other) noexcept;
	~CommandEncoderDebugScope();

	CommandEncoderDebugScope& operator=(const CommandEncoderDebugScope&) = delete;
	CommandEncoderDebugScope& operator=(CommandEncoderDebugScope&& other) noexcept;

private:
	CommandEncoder* commandEncoder;
};

} // namespace render
} // namespace kokko
