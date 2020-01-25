#pragma once

#include "Core/StringRef.hpp"

class JsonReader
{
private:
	static void ReadStringNoOp(const StringRef&, const StringRef&, void*) {}
	static void OpenCloseNoOp(const StringRef&, void*) {}

public:
	using ReadStringFn = void(*)(const StringRef&, const StringRef&, void*);
	using OpenCloseFn = void(*)(const StringRef&, void*);

	struct CallbackInfo
	{
		void* userData = nullptr;

		OpenCloseFn openObject = OpenCloseNoOp;
		OpenCloseFn closeObject = OpenCloseNoOp;

		OpenCloseFn openArray = OpenCloseNoOp;
		OpenCloseFn closeArray = OpenCloseNoOp;

		ReadStringFn readString = ReadStringNoOp;
	};

private:
	StringRef content;

public:
	// Initialize JsonReader instance with a null-terminated string
	void SetContent(const char* string)
	{
		content.str = string;

		unsigned int i = 0;
		while (string[i] != '\0') ++i;
		content.len = i;
	}

	// Initialize JsonReader instance with a specific size string
	void SetContent(const char* buffer, unsigned int size)
	{
		content.str = buffer;
		content.len = size;
	}

	void Parse(const CallbackInfo& callback) const
	{
		enum class St : unsigned char
		{
			Root,
			Object,
			Array,
			String
		};

		unsigned int charIndex = 0;

		static const int maxDepth = 32;
		int currDepth = 0;

		St typeStack[maxDepth];
		typeStack[currDepth] = St::Root;

		StringRef keyStack[maxDepth];

		// Array of String should be value initialized already, but let's be safe
		keyStack[currDepth].str = nullptr;

		int scopeStart = 0;
		char prevChar = 0;

		while (charIndex < content.len)
		{
			char c = content.str[charIndex];

			switch (c)
			{
				case '"':
				{
					if (prevChar != '\\')
					{
						// Quotation mark wasn't escaped
						// This is a real quotation mark

						if (typeStack[currDepth] == St::String)
						{
							// End string literal
							--currDepth;

							// No key is needed in this scope
							if (typeStack[currDepth] == St::Array ||
								typeStack[currDepth] == St::Root)
							{
								StringRef value;
								value.str = content.str + scopeStart;
								value.len = charIndex - scopeStart;

								callback.readString(keyStack[currDepth], value,
									callback.userData);
							}
							// A key has been defined in this scope
							else if (keyStack[currDepth].str != nullptr)
							{
								StringRef value;
								value.str = content.str + scopeStart;
								value.len = charIndex - scopeStart;

								callback.readString(keyStack[currDepth],
									value, callback.userData);

								// Unset key
								keyStack[currDepth].Clear();
							}
							// Key has not been defined, use this as the key
							else
							{
								keyStack[currDepth].str = content.str + scopeStart;
								keyStack[currDepth].len = charIndex - scopeStart;
							}
						}
						else
						{
							// Start string literal
							typeStack[++currDepth] = St::String;
							scopeStart = charIndex + 1;
						}
					}
				}
					break;
/*
				case ':':
				{

				}
					break;

				case ',':

					break;
*/
				case '{':
				{
					// We're not within a string literal
					if (typeStack[currDepth] != St::String)
					{
						// If there's a key defined, the callee can detect it
						callback.openObject(keyStack[currDepth], callback.userData);

						// Start object scope
						typeStack[++currDepth] = St::Object;
					}
				}
					break;

				case '}':
				{
					if (typeStack[currDepth] == St::Object)
					{
						// End object scope
						--currDepth;

						// If there's a key defined, the callee can detect it
						callback.closeObject(keyStack[currDepth], callback.userData);

						// Unset key, if it existed anyway
						keyStack[currDepth].Clear();
					}
				}
					break;

				case '[':
				{
					// We're not within a string literal
					if (typeStack[currDepth] != St::String)
					{
						// If there's a key defined, the callee can detect it
						callback.openArray(keyStack[currDepth], callback.userData);

						// Start array scope
						typeStack[++currDepth] = St::Array;
					}
				}
					break;

				case ']':
				{
					if (typeStack[currDepth] == St::Array)
					{
						// End array scope
						--currDepth;

						// If there's a key defined, the callee can detect it
						callback.closeArray(keyStack[currDepth], callback.userData);

						// Unset key, if it existed anyway
						keyStack[currDepth].Clear(); // Unset key
					}
				}
					break;
			}

			prevChar = c;
			++charIndex;
		}
	}
};



































