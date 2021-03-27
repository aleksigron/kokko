#pragma once

#include "Core/StringRef.hpp"
#include "Core/String.hpp"

#include <cstdio>
#include <cstdint>

class Instrumentation
{
private:
	FILE* fileHandle;
	unsigned int profileCount;

public:
	Instrumentation();
	Instrumentation(const Instrumentation&) = delete;
	Instrumentation(Instrumentation&&) = delete;
	~Instrumentation();

	Instrumentation& operator==(const Instrumentation&) = delete;
	Instrumentation& operator==(Instrumentation&&) = delete;

	bool BeginSession(const char* filepath);
	void WriteProfile(const char* name, double start, double end, unsigned int threadId);
	void EndSession();

	static Instrumentation& Get()
	{
		// TODO: Find a better place to store this
		// Now the destructor isn't necessarily run and the file might not be closed
		static Instrumentation instance;
		return instance;
	}
};
