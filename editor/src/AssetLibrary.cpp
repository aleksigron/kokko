#include "AssetLibrary.hpp"

#include <filesystem>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "Core/Core.hpp"

#include "System/Filesystem.hpp"

#include "EditorConstants.hpp"
#include "EditorProject.hpp"

namespace
{

void CreateMetadataJson(rapidjson::Document& document, uint64_t hash, const kokko::Uid& uid)
{
	char uidStrBuf[kokko::Uid::StringLength];
	uid.WriteTo(uidStrBuf);

	rapidjson::Document::AllocatorType& alloc = document.GetAllocator();
	document.SetObject();

	rapidjson::Value hashValue(hash);
	document.AddMember("hash", hashValue, alloc);

	rapidjson::Value uidValue(uidStrBuf, kokko::Uid::StringLength, alloc);
	document.AddMember("uid", uidValue, alloc);
}

bool WriteDocumentToFile(
	Filesystem* filesystem,
	const char* path,
	const rapidjson::Document& document,
	rapidjson::StringBuffer& stringBuffer)
{
	rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
	document.Accept(writer);

	ArrayView<const char> jsonView(stringBuffer.GetString(), stringBuffer.GetLength());

	bool result = filesystem->WriteText(path, jsonView, false);

	stringBuffer.Clear();

	return result;
}

} // Anonymous namespace

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
	assets(allocator)
{
}

AssetLibrary::~AssetLibrary()
{
}

const AssetInfo* AssetLibrary::FindAssetByUid(const Uid& uid)
{
	auto pair = uidToIndexMap.Lookup(uid);
	if (pair != nullptr)
	{
		return &assets[pair->second];
	}
	else
		return nullptr;
}

const AssetInfo* AssetLibrary::FindAssetByVirtualPath(const String& virtualPath)
{
	auto pair = pathToIndexMap.Lookup(virtualPath);
	if (pair != nullptr)
	{
		return &assets[pair->second];
	}
	else
		return nullptr;
}

bool AssetLibrary::UpdateAssetContent(const Uid& uid, ArrayView<const char> content)
{
	auto pair = uidToIndexMap.Lookup(uid);
	if (pair == nullptr)
		return false;

	AssetInfo& asset = assets[pair->second];

	uint64_t calculatedHash = Hash64(content.GetData(), content.GetCount(), 0);

	if (calculatedHash != asset.contentHash)
	{
		// Update in-memory data
		asset.contentHash = calculatedHash;

		// Update metadata file
		rapidjson::Document document;
		CreateMetadataJson(document, calculatedHash, asset.uid);

		String assetVirtualPath = asset.GetVirtualPath();
		String metaVirtualPath = assetVirtualPath + EditorConstants::MetadataExtensionStr;

		rapidjson::StringBuffer jsonStringBuffer;
		if (WriteDocumentToFile(filesystem, metaVirtualPath.GetCStr(), document, jsonStringBuffer) == false)
		{
			KK_LOG_ERROR("Couldn't write asset meta file: {}", metaVirtualPath.GetCStr());
			return false;
		}

		// Update asset file

		if (filesystem->WriteText(assetVirtualPath.GetCStr(), content, false) == false)
		{
			KK_LOG_ERROR("Couldn't write asset file: {}", assetVirtualPath.GetCStr());
			return false;
		}
	}

	return true;
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
	const fs::path shaderExt(".glsl");
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

		assetPathStr = currentPath.generic_u8string();

		Optional<AssetType> assetType;
		if (currentExt == materialExtension)
		{
			assetType = AssetType::Material;
		}
		else if (currentExt == shaderExt)
		{
			assetType = AssetType::Shader;
		}
		else if (currentExt == textureJpgExt ||
			currentExt == textureJpegExt ||
			currentExt == texturePngExt)
		{
			assetType = AssetType::Texture;
		}

		if (assetType.HasValue() == false)
		{
			KK_LOG_WARN("File didn't match known asset types: {}", assetPathStr.c_str());
			return;
		}

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

			if (document.GetParseError() != rapidjson::kParseErrorNone)
				KK_LOG_ERROR("Error parsing meta file: {}", metaPathStr.c_str());
			else
			{
				auto hashItr = document.FindMember("hash");
				auto uidItr = document.FindMember("uid");
				if (hashItr == document.MemberEnd() && !hashItr->value.IsUint64() &&
					uidItr == document.MemberEnd() && !uidItr->value.IsString())
				{
					KK_LOG_ERROR("Invalid meta file: {}", metaPathStr.c_str());
				}
				else
				{
					ArrayView<const char> uidStr(uidItr->value.GetString(), uidItr->value.GetStringLength());
					auto uidParseResult = Uid::FromString(uidStr);

					if (uidParseResult.HasValue() == false)
						KK_LOG_ERROR("Invalid UID format in file: {}", metaPathStr.c_str());
					else
					{
						assetUid = uidParseResult.GetValue();
						uint64_t storedHash = hashItr->value.GetUint64();

						if (storedHash != calculatedHash)
						{
							hashItr->value.SetUint64(calculatedHash);

							needsToWriteMetaFile = true;
						}
					}
				}
			}
		}
		else // Meta file couldn't be read
		{
			assetUid = Uid::Create();

			CreateMetadataJson(document, calculatedHash, assetUid);

			needsToWriteMetaFile = true;
		}

		if (auto existingPair = uidToIndexMap.Lookup(assetUid))
		{
			auto& existing = assets[existingPair->second];
			KK_LOG_ERROR("Asset with duplicate UID found: {}\nExisting asset: {}",
				assetPathStr.c_str(), existing.GetVirtualPath().GetCStr());
			return;
		}

		if (needsToWriteMetaFile)
		{
			if (WriteDocumentToFile(filesystem, metaPathStr.c_str(), document, jsonStringBuffer) == false)
				KK_LOG_ERROR("Couldn't write asset meta file: {}", metaPathStr.c_str());
		}

		std::error_code err;
		auto relative = std::filesystem::relative(currentPath, root, err);

		if (err)
		{
			KK_LOG_ERROR("Asset path could not be made relative, error: {}", err.message().c_str());
			return;
		}

		auto relativeStdStr = relative.generic_u8string();

		uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());

		auto& assetInfo = assets.PushBack();
		assetInfo.virtualPath = virtualPath;
		assetInfo.filePath = String(allocator, StringRef(relativeStdStr.c_str(), relativeStdStr.length()));
		assetInfo.uid = assetUid;
		assetInfo.contentHash = calculatedHash;
		assetInfo.type = assetType.GetValue();

		auto uidPair = uidToIndexMap.Insert(assetUid);
		uidPair->second = assetRefIndex;

		auto pathPair = pathToIndexMap.Insert(assetInfo.GetVirtualPath());
		pathPair->second = assetRefIndex;
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

String AssetInfo::GetVirtualPath() const
{
	return virtualPath + ('/' + filePath);
}

}
}
