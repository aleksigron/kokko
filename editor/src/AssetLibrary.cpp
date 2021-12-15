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

const AssetLibrary::AssetInfo* AssetLibrary::FindAssetByUid(const Uid& uid)
{
	auto pair = uidToIndexMap.Lookup(uid);
	if (pair != nullptr)
	{
		return &assets[pair->second];
	}
	else
		return nullptr;
}

const AssetLibrary::AssetInfo* AssetLibrary::FindAssetByVirtualPath(const String& virtualPath)
{
	auto pair = pathToIndexMap.Lookup(virtualPath);
	if (pair != nullptr)
	{
		return &assets[pair->second];
	}
	else
		return nullptr;
}

void AssetLibrary::ScanEngineAssets()
{
	ScanAssets(false);
}

void AssetLibrary::SetProject(const EditorProject* project)
{
	// TODO: Remove current project assets when switching to new project

	editorProject = project;

	ScanAssets(true);
}

void AssetLibrary::ScanAssets(bool scanProject)
{
	namespace fs = std::filesystem;

	const fs::path materialExtension(".material");
	const fs::path textureJpgExt(".jpg");
	const fs::path textureJpegExt(".jpeg");
	const fs::path texturePngExt(".png");

	std::string assetPathStr;
	std::string metaPathStr;

	Array<uint8_t> fileContent(allocator);
	String metaContent(allocator);

	rapidjson::Document document;
	rapidjson::StringBuffer jsonStringBuffer;

	auto processEntry = [&](StringRef virtualPath, const fs::path& root, const fs::directory_entry& entry)
	{
		KOKKO_PROFILE_SCOPE("Scan file");

		const fs::path& currentPath = entry.path();
		std::filesystem::path currentExt = currentPath.extension();
		if (entry.is_regular_file() == false || currentExt == EditorConstants::MetadataExtension)
			return;

		bool assetTypeFound = true;
		AssetType assetType;
		uint32_t assetIndex = 0;

		assetPathStr = currentPath.generic_u8string();

		if (currentExt == materialExtension)
		{
			assetType = AssetType::Material;
			assetIndex = static_cast<uint32_t>(materials.GetCount());
			materials.PushBack(MaterialInfo{});
		}
		else
			assetTypeFound = false;

		if (assetTypeFound)
		{
			// Open asset file
			if (filesystem->ReadBinary(assetPathStr.c_str(), fileContent) == false)
			{
				KK_LOG_ERROR("Couldn't read asset file: {}", assetPathStr.c_str());
				return;
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

				document.SetObject();
				document.AddMember("hash", hashValue, alloc);
				document.AddMember("uid", uidValue, alloc);

				needsToWriteMetaFile = true;
			}

			if (needsToWriteMetaFile)
			{
				rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStringBuffer);
				document.Accept(writer);

				ArrayView<const char> jsonView(jsonStringBuffer.GetString(), jsonStringBuffer.GetLength());

				if (filesystem->WriteText(metaPathStr.c_str(), jsonView, false) == false)
					KK_LOG_ERROR("Couldn't write asset meta file: {}", metaPathStr.c_str());

				jsonStringBuffer.Clear();
			}

			std::error_code err;
			auto relative = std::filesystem::relative(currentPath, root, err);

			if (err)
			{
				KK_LOG_ERROR("Asset path could not be made relative, error: {}", err.message().c_str());
			}
			else
			{
				auto relativeStdStr = relative.generic_u8string();

				uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
				auto& assetInfo = assets.PushBack();
				assetInfo.virtualPath = virtualPath;
				assetInfo.filePath = String(allocator, StringRef(relativeStdStr.c_str(), relativeStdStr.length()));
				assetInfo.uid = assetUid;
				assetInfo.type = assetType;
				assetInfo.arrayIndex = assetIndex;

				auto uidPair = uidToIndexMap.Insert(assetUid);
				uidPair->second = assetRefIndex;

				auto pathPair = pathToIndexMap.Insert(assetInfo.GetVirtualPath());
				pathPair->second = assetRefIndex;
			}
		}
		else
		{
			KK_LOG_ERROR("File didn't match known asset types: {}", assetPathStr.c_str());
		}
	};

	if (scanProject)
	{
		const fs::path assetDir = editorProject->GetRootPath() / EditorConstants::AssetDirectoryName;
		const StringRef virtualPathAssets(EditorConstants::VirtualPathAssets);

		for (const auto& entry : fs::recursive_directory_iterator(assetDir))
			processEntry(virtualPathAssets, assetDir, entry);
	}
	else
	{
		const fs::path engineResDir = fs::absolute(EditorConstants::EngineResourcePath);
		const fs::path editorResDir = fs::absolute(EditorConstants::EditorResourcePath);
		const StringRef virtualPathEngine(EditorConstants::VirtualPathEngine);
		const StringRef virtualPathEditor(EditorConstants::VirtualPathEditor);

		for (const auto& entry : fs::recursive_directory_iterator(engineResDir))
			processEntry(virtualPathEngine, engineResDir, entry);

		for (const auto& entry : fs::recursive_directory_iterator(editorResDir))
			processEntry(virtualPathEditor, editorResDir, entry);
	}

}

String AssetLibrary::AssetInfo::GetVirtualPath() const
{
	return virtualPath + ('/' + filePath);
}

}
}
