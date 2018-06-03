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

	bool TryGetString(StringRef key, StringRef& valueOut);
	bool TryGetString(const char* key, StringRef& valueOut);

	void SetString(const char* key, StringRef value);
	void SetString(StringRef key, StringRef value);
	StringRef GetString(const char* key);
	StringRef GetString(StringRef key);

	bool TryGetDouble(const char* key, double& valueOut);
	bool TryGetDouble(StringRef key, double& valueOut);

	void SetDouble(const char* key, double value);
	void SetDouble(StringRef key, double value);
	double GetDouble(const char* key);
	double GetDouble(StringRef key);
};
