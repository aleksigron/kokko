#pragma once

class JsonReader
{
public:
	struct String
	{
		const char* str = nullptr;
		unsigned int len = 0;

		bool operator == (const String& other)
		{
			if (len != other.len)
				return false;
			for (unsigned i = 0; i < len; ++i)
				if (str[i] != other.str[i])
					return false;
			return true;
		}

		inline bool operator != (const String& other)
		{ return operator == (other) == false; }
	};

	using ReadStringUnnamedFn = void(*)(const String&, void*);
	using ReadStringFn = void(*)(const String&, const String&, void*);
	using OpenCloseUnnamedFn = void(*)(void*);
	using OpenCloseFn = void(*)(const String&, void*);

	struct CallbackInfo
	{
		void* userData = nullptr;

		OpenCloseUnnamedFn openObjectUnnamed = nullptr;
		OpenCloseUnnamedFn closeObjectUnnamed = nullptr;

		OpenCloseFn openObject = nullptr;
		OpenCloseFn closeObject = nullptr;

		OpenCloseUnnamedFn openArrayUnnamed = nullptr;
		OpenCloseUnnamedFn closeArrayUnnamed = nullptr;

		OpenCloseFn openArray = nullptr;
		OpenCloseFn closeArray = nullptr;

		ReadStringUnnamedFn readStringUnnamed = nullptr;
		ReadStringFn readString = nullptr;
	};

private:
	String content;

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

	void Parse(const CallbackInfo& callback)
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

		String keyStack[maxDepth];

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
								String value;
								value.str = content.str + scopeStart;
								value.len = charIndex - scopeStart;

								callback.readStringUnnamed(value,
									callback.userData);
							}
							// A key has been defined in this scope
							else if (keyStack[currDepth].str != nullptr)
							{
								String value;
								value.str = content.str + scopeStart;
								value.len = charIndex - scopeStart;

								callback.readString(keyStack[currDepth],
									value, callback.userData);

								// Unset key
								keyStack[currDepth].str = nullptr;
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
						if (keyStack[currDepth].str != nullptr)
							callback.openObject(keyStack[currDepth], callback.userData);
						else
							callback.openObjectUnnamed(callback.userData);

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

						if (keyStack[currDepth].str != nullptr)
						{
							callback.closeObject(keyStack[currDepth], callback.userData);
							keyStack[currDepth].str = nullptr; // Unset key
						}
						else
							callback.closeObjectUnnamed(callback.userData);
					}
				}
					break;

				case '[':
				{
					// We're not within a string literal
					if (typeStack[currDepth] != St::String)
					{
						if (keyStack[currDepth].str != nullptr)
							callback.openArray(keyStack[currDepth], callback.userData);
						else
							callback.openArrayUnnamed(callback.userData);

						// Start object scope
						typeStack[++currDepth] = St::Array;
					}
				}
					break;

				case ']':
				{
					if (typeStack[currDepth] == St::Array)
					{
						// End object scope
						--currDepth;

						if (keyStack[currDepth].str != nullptr)
						{
							callback.closeArray(keyStack[currDepth], callback.userData);
							keyStack[currDepth].str = nullptr; // Unset key
						}
						else
							callback.closeArrayUnnamed(callback.userData);
					}
				}
					break;
			}

			prevChar = c;
			++charIndex;
		}
	}
};



































