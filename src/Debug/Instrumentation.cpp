#include "Debug/Instrumentation.hpp"

Instrumentation::Instrumentation() : fileHandle(nullptr), profileCount(0)
{
}

Instrumentation::~Instrumentation()
{
	if (fileHandle != nullptr)
		std::fclose(fileHandle);
}

bool Instrumentation::BeginSession(const char* filepath)
{
	if (fileHandle == nullptr)
	{
		fileHandle = std::fopen(filepath, "wb");

		if (fileHandle != nullptr)
		{
			const char header[] = "{\"traceEvents\":[";
			std::fwrite(header, 1, sizeof(header) - 1, fileHandle);

			return true;
		}
	}

	return false;
}

void Instrumentation::WriteProfile(const char* name, double start, double end, unsigned int threadId)
{
	if (fileHandle != nullptr)
	{
		if (profileCount++ > 0)
		{
			char comma = ',';
			std::fwrite(&comma, 1, 1, fileHandle);
		}

		const char* format =
			"{\"cat\":\"function\","
			"\"dur\":%f,"
			"\"name\":\"%s\","
			"\"ph\":\"X\","
			"\"pid\":0,"
			"\"tid\":%d,"
			"\"ts\":%f}";

		std::fprintf(fileHandle, format, end - start, name, threadId, start);
		std::fflush(fileHandle);
	}
}

void Instrumentation::EndSession()
{
	if (fileHandle != nullptr)
	{
		const char footer[] = "]}";
		std::fwrite(footer, 1, sizeof(footer) - 1, fileHandle);

		std::fclose(fileHandle);
		fileHandle = nullptr;
		profileCount = 0;
	}
}
