#pragma once

#include "Core/Array.hpp"

#include "System/LogLevel.hpp"

class Allocator;

namespace kokko
{

class Logger
{
public:
	class Receiver
	{
	public:
		virtual ~Receiver() {}
		virtual void Log(const char* text, size_t length, LogLevel level) = 0;
	};

    Logger(Allocator* allocator);
	~Logger();

	void SetReceiver(Receiver* receiver);

	bool OpenLogFile(const char* filePath, bool append);

	void Log(const char* text, size_t length, LogLevel level);

	Array<char>& GetFormatBuffer() { return formatBuffer; }

private:
	void* fileHandle;
	Receiver* receiver;

	Array<char> formatBuffer;
};

}
