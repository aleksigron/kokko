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
			for (int i = 0; i < len; ++i)
				if (str[i] != other.str[i])
					return false;
			return true;
		}

		inline bool operator != (const String& other)
		{ return operator == (other) == false; }
	};

	using ReadStringCallbackFn = void(*)(const String&, const String&, void*);
	using OpenCloseCallbackFn = void(*)(void*);

	struct CallbackInfo
	{
		void* userData = nullptr;

		OpenCloseCallbackFn openObject = nullptr;
		OpenCloseCallbackFn closeObject = nullptr;

		OpenCloseCallbackFn openArray = nullptr;
		OpenCloseCallbackFn closeArray = nullptr;

		ReadStringCallbackFn readString = nullptr;
	};

private:
	String content;

public:
	// Initialize JsonReader instance with a null-terminated string
	JsonReader(const char* string)
	{
		content.str = string;

		unsigned int i = 0;
		while (string[i] != '\0') ++i;
		content.len = i;
	}

	// Initialize JsonReader instance with a specific size string
	JsonReader(const char* buffer, unsigned int size)
	{
		content.str = buffer;
		content.len = size;
	}

	void Parse(const CallbackInfo& callbackInfo)
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

		St stack[maxDepth];
		stack[currDepth] = St::Root;

		int scopeStart = 0;
		char prevChar = 0;

		String key;

		while (charIndex < content.len)
		{
			char c = content.str[charIndex];

			switch (c)
			{
				case '"':
				{
					if (prevChar != '\\')
					{
						// Double quotation mark wasn't escaped

						if (stack[currDepth] == St::String)
						{
							// A key has been defined in this scope
							if (key.str != nullptr)
							{
								String value;
								value.str = content.str + scopeStart;
								value.len = charIndex - scopeStart;

								callbackInfo.readString(key, value,
														callbackInfo.userData);

								key.str = nullptr;
							}
							else // Key has not been defined
							{
								key.str = content.str + scopeStart;
								key.len = charIndex - scopeStart;
							}

							// End string literal
							--currDepth;
						}
						else
						{
							// Start string literal
							stack[++currDepth] = St::String;
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
					if (stack[currDepth] != St::String)
					{
						callbackInfo.openObject(callbackInfo.userData);
						stack[++currDepth] = St::Object;
					}
				}
					break;

				case '}':
				{
					if (stack[currDepth] == St::Object)
					{
						callbackInfo.closeObject(callbackInfo.userData);
						--currDepth;
					}
				}
					break;
/*
				case '[':

					break;

				case ']':

					break;
 */
			}

			prevChar = c;
			++charIndex;
		}
	}
};



































