#include "Debug/LogHelper.hpp"

#include "Core/Array.hpp"

#include "Debug/DebugLog.hpp"

static DebugLog* DebugLogInstance = nullptr;

static const size_t FormatBufferSize = 1024;

static const size_t LevelStringLength = 8;
static const char LevelStrings[][LevelStringLength] =
{
	"[DEBUG]",
	"[INFO ]",
	"[WARN ]",
	"[ERROR]"
};

void Log::SetLogInstance(DebugLog* instance)
{
	DebugLogInstance = instance;
}

void Log::_DebugVarLog(const char* file, int line, fmt::string_view format, fmt::format_args args)
{
	Array<char>& formatBuffer = DebugLogInstance->GetFormatBuffer();
	formatBuffer.Resize(FormatBufferSize);
	size_t n = formatBuffer.GetCount() - 1;
	char* buffer = formatBuffer.GetData();
	char* itr = buffer;

	auto result = fmt::format_to_n(itr, n, "[DEBUG] {}:{} ", file, line);

	n = &formatBuffer.GetBack() - result.out;
	result = fmt::vformat_to_n(result.out, n, format, args);

	size_t length = result.out - buffer;

	DebugLogInstance->Log(buffer, length, LogLevel::Debug);
}

void Log::_VarLog(LogLevel level, fmt::string_view format, fmt::format_args args)
{
	Array<char>& formatBuffer = DebugLogInstance->GetFormatBuffer();
	formatBuffer.Resize(FormatBufferSize);
	size_t n = formatBuffer.GetCount() - 1;
	char* buffer = formatBuffer.GetData();
	char* itr = buffer;

	const char* levelStr = LevelStrings[static_cast<size_t>(level)];

	auto result = fmt::format_to_n(itr, n, "{} ", levelStr);

	n = &formatBuffer.GetBack() - result.out;
	result = fmt::vformat_to_n(result.out, n, format, args);

	size_t length = result.out - buffer;

	DebugLogInstance->Log(buffer, length, level);
}
