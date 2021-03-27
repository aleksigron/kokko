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
	Instrumentation() : fileHandle(nullptr), profileCount(0)
	{
	}

	bool BeginSession(const char* filepath);
	void WriteProfile(const char* name, double start, double end, unsigned int threadId);
	void EndSession();

	static Instrumentation& Get()
	{
		static Instrumentation instance;
		return instance;
	}
};
