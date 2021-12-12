#include "AssetLibrary.hpp"

#include <filesystem>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "Core/Core.hpp"

#include "System/Filesystem.hpp"

#include "EditorConstants.hpp"
#include "EditorProject.hpp"

namespace kokko
{
namespace editor
{

AssetLibrary::AssetLibrary(Allocator* allocator, Filesystem* filesystem) :
	allocator(allocator),
	filesystem(filesystem),
	editorProject(nullptr),
	uidToIndexMap(allocator),
	pathToIndexMap(allocator),
	assets(allocator),
	materials(allocator)
{
}

AssetLibrary::~AssetLibrary()
{
}

void AssetLibrary::Initialize(const EditorProject* project)
{
	uidToIndexMap.Clear();
	pathToIndexMap.Clear();
	assets.Clear();
	materials.Clear();

	editorProject = project;

	ScanAssets();
}

void AssetLibrary::ScanAssets()
{
	namespace fs = std::filesystem;

	const fs::path assetDir = editorProject->GetRootPath() / EditorConstants::AssetDirectoryName;
	const fs::path materialExtension(".material");

	std::string assetPathStr;
	std::string metaPathStr;

	Array<uint8_t> fileContent(allocator);
	String metaContent(allocator);

	rapidjson::Document document;
	rapidjson::StringBuffer jsonStringBuffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStringBuffer);

	for (const auto& entry : fs::recursive_directory_iterator(assetDir))
	{
		KOKKO_PROFILE_SCOPE("Scan file");

		const fs::path& currentPath = entry.path();
		if (entry.is_regular_file() == false || currentPath.extension() == EditorConstants::MetadataExtension)
			continue;
		
		// Open asset file
		assetPathStr = currentPath.u8string();
		if (filesystem->ReadBinary(assetPathStr.c_str(), fileContent) == false)
		{
			KK_LOG_ERROR("Couldn't read asset file: {}", assetPathStr.c_str());
			continue;
		}

		uint64_t calculatedHash = Hash64(fileContent.GetData(), fileContent.GetCount(), 0);
		Uid assetUid;

		// Open meta file
		fs::path metaPath = currentPath;
		metaPath += EditorConstants::MetadataExtension;
		metaPathStr = metaPath.u8string();

		bool needsToWriteMetaFile = false;

		if (filesystem->ReadText(metaPathStr.c_str(), metaContent))
		{
			document.ParseInsitu(metaContent.GetData());

			if (document.GetParseError() == rapidjson::kParseErrorNone)
			{
				auto hashItr = document.FindMember("hash");
				auto uidItr = document.FindMember("uid");
				if (hashItr != document.MemberEnd() && hashItr->value.IsUint64() &&
					uidItr != document.MemberEnd() && uidItr->value.IsString())
				{
					ArrayView<const char> uidStr(uidItr->value.GetString(), uidItr->value.GetStringLength());
					auto uidParseResult = Uid::FromString(uidStr);
					if (uidParseResult.HasValue())
					{
						assetUid = uidParseResult.GetValue();
						uint64_t storedHash = hashItr->value.GetUint64();

						if (storedHash != calculatedHash)
						{
							hashItr->value.SetUint64(calculatedHash);

							needsToWriteMetaFile = true;
						}
					}
					else
						KK_LOG_ERROR("Invalid UID format in file: {}", metaPathStr.c_str());
				}
				else
					KK_LOG_ERROR("Invalid meta file: {}", metaPathStr.c_str());
			}
			else
				KK_LOG_ERROR("Invalid meta file: {}", metaPathStr.c_str());
		}
		else // Meta file couldn't be read
		{
			char uidStrBuf[Uid::StringLength];

			assetUid = Uid::Create();
			assetUid.WriteTo(uidStrBuf);

			rapidjson::Document::AllocatorType& alloc = document.GetAllocator();

			auto hashValue = rapidjson::Value();
			hashValue.SetUint64(calculatedHash);

			auto uidValue = rapidjson::Value();
			uidValue.SetString(uidStrBuf, Uid::StringLength);

			document.Clear();
			document.SetObject();
			document.AddMember("hash", hashValue, document.GetAllocator());
			document.AddMember("uid", uidValue, document.GetAllocator());

			needsToWriteMetaFile = true;
		}

		if (needsToWriteMetaFile)
		{
			document.Accept(writer);

			ArrayView<const char> jsonView(jsonStringBuffer.GetString(), jsonStringBuffer.GetLength());

			if (filesystem->WriteText(metaPathStr.c_str(), jsonView, false) == false)
				KK_LOG_ERROR("Couldn't write asset meta file: {}", metaPathStr.c_str());

			jsonStringBuffer.Clear();
		}

		bool assetTypeFound = true;
		AssetType assetType;
		uint32_t assetIndex = 0;
		
		if (currentPath.extension() == materialExtension)
		{
			assetType = AssetType::Material;
			assetIndex = static_cast<uint32_t>(materials.GetCount());
			materials.PushBack(MaterialInfo{});
		}
		else
			assetTypeFound = false;

		if (assetTypeFound)
		{
			std::error_code err;
			auto relative = std::filesystem::relative(currentPath, assetDir, err);

			if (err)
			{
				KK_LOG_ERROR("Asset path could not be made relative, error: {}", err.message().c_str());
			}
			else
			{
				uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
				auto& assetInfo = assets.PushBack();
				assetInfo.path = relative;
				assetInfo.uid = assetUid;
				assetInfo.type = assetType;
				assetInfo.arrayIndex = assetIndex;

				auto uidPair = uidToIndexMap.Insert(assetUid);
				uidPair->second = assetRefIndex;

				auto pathPair = pathToIndexMap.Insert(relative);
				pathPair->second = assetRefIndex;
			}
		}
		else
		{
			KK_LOG_ERROR("File didn't match known asset types: {}", assetPathStr.c_str());
		}
	}
}

const AssetLibrary::AssetInfo* AssetLibrary::FindAssetByPath(const std::filesystem::path& path)
{
	auto pair = pathToIndexMap.Lookup(path);
	if (pair != nullptr)
	{
		return &assets[pair->second];
	}
	else
		return nullptr;
}

}
}
