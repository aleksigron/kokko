#include "Rendering/CommandEncoderDebugScope.hpp"

#include "Rendering/RenderCommandEncoder.hpp"

namespace kokko
{

namespace render
{

CommandEncoderDebugScope::CommandEncoderDebugScope(
	CommandEncoder* commandEncoder, uint32_t id, kokko::ConstStringView message) :
	commandEncoder(commandEncoder)
{
	commandEncoder->BeginDebugScope(id, message);
}

CommandEncoderDebugScope::CommandEncoderDebugScope(CommandEncoderDebugScope&& other) noexcept
{
	commandEncoder = other.commandEncoder;
	other.commandEncoder = nullptr;
}

CommandEncoderDebugScope::~CommandEncoderDebugScope()
{
	if (commandEncoder != nullptr)
		commandEncoder->EndDebugScope();
}

CommandEncoderDebugScope& CommandEncoderDebugScope::operator=(CommandEncoderDebugScope&& other) noexcept
{
	commandEncoder = other.commandEncoder;
	other.commandEncoder = nullptr;

	return *this;
}

} // namespace render
} // namespace kokko
