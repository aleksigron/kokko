#include "Debug/Instrumentation.hpp"

#include <cstdio>

#include "fmt/format.h"

namespace kokko
{

Instrumentation::Instrumentation() : fileHandle(nullptr), profileCount(0)
{
}

Instrumentation::~Instrumentation()
{
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);
		std::fclose(file);
	}
}

bool Instrumentation::BeginSession(const char* filepath)
{
	if (fileHandle == nullptr)
	{
		fileHandle = std::fopen(filepath, "wb");

		if (fileHandle != nullptr)
		{
			FILE* file = static_cast<FILE*>(fileHandle);
			const char header[] = "{\"traceEvents\":[";
			std::fwrite(header, 1, sizeof(header) - 1, file);

			return true;
		}
	}

	return false;
}

void Instrumentation::WriteProfile(const char* name, double start, double end, size_t threadId)
{
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);

		char comma = profileCount > 0 ? ',' : ' ';

		profileCount += 1;

		fmt::print(
			file,
			FMT_STRING("{:c}{{"
				"\"cat\":\"function\","
				"\"dur\":{:f},"
				"\"name\":\"{}\","
				"\"ph\":\"X\","
				"\"pid\":0,"
				"\"tid\":{:d},"
				"\"ts\":{:f}"
				"}}"),
			comma, end - start, name, threadId, start);
	}
}

void Instrumentation::EndSession()
{
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);

		const char footer[] = "]}";
		std::fwrite(footer, 1, sizeof(footer) - 1, file);

		std::fclose(file);
		fileHandle = nullptr;
		profileCount = 0;
	}
}

} // namespace kokko
