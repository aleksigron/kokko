#include "AppSettings.hpp"

#include "File.hpp"

AppSettings::AppSettings()
{
}

AppSettings::~AppSettings()
{
}

void AppSettings::SetFilename(StringRef path)
{
	settings.Clear();

	settingsFilename = ImmutableString(path.str, path.len);
}

void AppSettings::LoadFromFile()
{
	if (settingsFilename.GetLength() > 0)
	{
		Buffer<char> content = File::ReadText(settingsFilename.GetRef());

		if (content.IsValid())
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
}

void AppSettings::SetString(const char* key, StringRef value)
{
	this->SetString(StringRef(key), value);
}

void AppSettings::SetString(StringRef key, StringRef value)
{
	if (key.len > 0 && value.len)
	{
		Setting* s = this->FindSetting(key);

		if (s != nullptr)
			delete[] s->keyValueString;
		else
			s = &(settings.PushBack());

		s->keyLength = key.len;
		s->valueLength = value.len;
		s->keyValueString = new char[s->keyLength + s->valueLength];

		std::memcpy(s->keyValueString, key.str, s->keyLength);
		std::memcpy(s->keyValueString + s->keyLength, value.str, s->valueLength);
	}
}

StringRef AppSettings::GetString(const char* key)
{
	Setting* s = this->FindSetting(StringRef(key));

	if (s != nullptr)
		return StringRef(s->keyValueString + s->keyLength, s->valueLength);
	else
		return StringRef();
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

	return false;
}
