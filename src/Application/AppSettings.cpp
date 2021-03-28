#include "Application/AppSettings.hpp"

#include <cstdio>
#include <cstdlib>

#include "Core/BufferRef.hpp"
#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "System/File.hpp"

AppSettings::AppSettings(Allocator* allocator) :
	allocator(allocator),
	settingsFilename(allocator),
	settings(allocator)
{
}

AppSettings::~AppSettings()
{
	for (unsigned int i = 0, count = settings.GetCount(); i < count; ++i)
	{
		allocator->Deallocate(settings[i].keyValueString);
	}
}

void AppSettings::SetFilename(StringRef path)
{
	settings.Clear();

	settingsFilename = String(allocator, path);
}

void AppSettings::LoadFromFile()
{
	KOKKO_PROFILE_FUNCTION();

	if (settingsFilename.GetLength() > 0)
	{
		Buffer<char> content(allocator);

		if (File::ReadText(settingsFilename.GetCStr(), content))
		{
			bool keyRead = false;
			size_t keyStart = 0;
			size_t valueStart = 0;

			for (size_t i = 0, count = content.Count(); i < count; ++i)
			{
				char c = content[i];

				if (keyRead == false && c == ' ')
				{
					valueStart = i + 1;
					keyRead = true;
				}
				else if (keyRead == true && c == '\n')
				{
					StringRef key(content.Data() + keyStart, valueStart - keyStart - 1);
					StringRef value(content.Data() + valueStart, i - valueStart);

					this->SetString(key, value);

					keyStart = i + 1;
					keyRead = false;
				}
			}

			if (keyRead == true)
			{
				StringRef key(content.Data() + keyStart, valueStart - keyStart - 1);
				StringRef value(content.Data() + valueStart, content.Count() - valueStart);

				this->SetString(key, value);
			}
		}
	}
}

void AppSettings::SaveToFile()
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int totalLength = 0;

	for (unsigned int i = 0, count = settings.GetCount(); i < count; ++i)
	{
		const Setting& s = settings.At(i);
		totalLength += s.keyLength + s.valueLength + 2;
	}

	Buffer<char> content(allocator);
	content.Allocate(totalLength);

	char* data = content.Data();
	for (unsigned int i = 0, count = settings.GetCount(); i < count; ++i)
	{
		const Setting& s = settings.At(i);

		std::memcpy(data, s.keyValueString, s.keyLength);
		data += s.keyLength;

		*data = ' ';
		data += 1;

		std::memcpy(data, s.keyValueString + s.keyLength, s.valueLength);
		data += s.valueLength;

		*data = '\n';
		data += 1;
	}

	File::Write(settingsFilename.GetCStr(), content.GetRef(), false);
}

bool AppSettings::TryGetString(StringRef key, StringRef& valueOut)
{
	StringRef value = this->GetString(key);

	if (value.IsNonNull())
	{
		valueOut = value;
		return true;
	}
	else
		return false;
}

bool AppSettings::TryGetString(const char* key, StringRef& valueOut)
{
	return this->TryGetString(StringRef(key), valueOut);
}

void AppSettings::SetString(const char* key, StringRef value)
{
	this->SetString(StringRef(key), value);
}

void AppSettings::SetString(StringRef key, StringRef value)
{
	if (key.len > 0 && value.len > 0)
	{
		Setting* s = this->FindSetting(key);

		if (s != nullptr)
			allocator->Deallocate(s->keyValueString);
		else
			s = &(settings.PushBack());

		s->keyLength = key.len;
		s->valueLength = value.len;

		unsigned int totalSize = s->keyLength + s->valueLength;
		s->keyValueString = static_cast<char*>(allocator->Allocate(totalSize));

		std::memcpy(s->keyValueString, key.str, s->keyLength);
		std::memcpy(s->keyValueString + s->keyLength, value.str, s->valueLength);
	}
}

StringRef AppSettings::GetString(StringRef key)
{
	Setting* s = this->FindSetting(key);

	if (s != nullptr)
		return StringRef(s->keyValueString + s->keyLength, s->valueLength);
	else
		return StringRef();
}

bool AppSettings::TryGetDouble(const char* key, double& valueOut)
{
	return this->TryGetDouble(StringRef(key), valueOut);
}

bool AppSettings::TryGetDouble(StringRef key, double& valueOut)
{
	StringRef value = this->GetString(key);

	if (value.IsNonNull())
	{
		valueOut = std::strtod(value.str, nullptr);
		return true;
	}
	else
		return false;
}

StringRef AppSettings::GetString(const char* key)
{
	return this->GetString(StringRef(key));
}

void AppSettings::SetDouble(const char* key, double value)
{
	this->SetDouble(StringRef(key), value);
}

void AppSettings::SetDouble(StringRef key, double value)
{
	char buffer[32];

	std::snprintf(buffer, sizeof(buffer), "%f", value);

	this->SetString(key, StringRef(buffer));
}

double AppSettings::GetDouble(const char* key)
{
	return this->GetDouble(StringRef(key));
}

double AppSettings::GetDouble(StringRef key)
{
	StringRef value = this->GetString(key);

	if (value.IsNonNull())
		return std::strtod(value.str, nullptr);
	else
		return 0.0;
}

AppSettings::Setting* AppSettings::FindSetting(StringRef key)
{
	for (unsigned int i = 0, count = settings.GetCount(); i < count; ++i)
	{
		Setting& s = settings.At(i);

		StringRef kr(s.keyValueString, s.keyLength);
		
		if (kr == key)
			return &s;
	}

	return nullptr;
}
