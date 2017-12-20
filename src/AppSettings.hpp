#pragma once

#include <cstring>

#include "Array.hpp"
#include "ImmutableString.hpp"
#include "StringRef.hpp"

class AppSettings
{
private:
	struct Setting
	{
		char* keyValueString;
		unsigned int keyLength;
		unsigned int valueLength;
	};

	ImmutableString settingsFilename;

	Array<Setting> settings;

	Setting* FindSetting(StringRef key);

public:
	AppSettings();
	~AppSettings();

	void SetFilename(StringRef path);

	void LoadFromFile();
	void SaveToFile();

	void SetString(const char* key, StringRef value);
	void SetString(StringRef key, StringRef value);
	StringRef GetString(const char* key);
};
