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

	ArrayView<const uint8_t> jsonView(reinterpret_cast<const uint8_t*>(stringBuffer.GetString()), stringBuffer.GetLength());

	bool result = filesystem->Write(path, jsonView, false);

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

Optional<Uid> AssetLibrary::CreateAsset(AssetType type, StringRef pathRelativeToAssets, ArrayView<const uint8_t> content)
{
	Uid assetUid = Uid::Create();
	uint64_t calculatedHash = Hash64(content.GetData(), content.GetCount(), 0);
	uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
	StringRef mount = StringRef(EditorConstants::VirtualMountAssets);

	assets.PushBack(AssetInfo(allocator, mount, pathRelativeToAssets, assetUid, calculatedHash, type));
	const String& virtualPath = assets.GetBack().GetVirtualPath();

	if (filesystem->Write(virtualPath.GetCStr(), content, false) == false)
	{
		assets.PopBack();
		KK_LOG_ERROR("New asset couldn't be written to path {}", virtualPath.GetCStr());
		return Optional<Uid>();
	}

	rapidjson::Document document;
	CreateMetadataJson(document, calculatedHash, assetUid);

	String metaPath = virtualPath + EditorConstants::MetadataExtensionStr;
	rapidjson::StringBuffer jsonStringBuffer;
	if (WriteDocumentToFile(filesystem, metaPath.GetCStr(), document, jsonStringBuffer) == false)
	{
		KK_LOG_ERROR("Couldn't write asset meta file: {}", metaPath.GetCStr());
	}

	auto uidPair = uidToIndexMap.Insert(assetUid);
	uidPair->second = assetRefIndex;

	auto pathPair = pathToIndexMap.Insert(assets.GetBack().GetVirtualPath());
	pathPair->second = assetRefIndex;

	return assetUid;
}

bool AssetLibrary::UpdateAssetContent(const Uid& uid, ArrayView<const uint8_t> content)
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

		if (filesystem->Write(assetVirtualPath.GetCStr(), content, false) == false)
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

	const fs::path levelExt(".level");
	const fs::path materialExt(".material");
	const fs::path shaderExt(".glsl");
	const fs::path textureJpgExt(".jpg");
	const fs::path textureJpegExt(".jpeg");
	const fs::path texturePngExt(".png");
	const fs::path textureHdrExt(".hdr");

	std::string assetPathStr;
	std::string metaPathStr;

	Array<uint8_t> fileContent(allocator);
	String metaContent(allocator);

	rapidjson::Document document;
	rapidjson::StringBuffer jsonStringBuffer;

	auto processEntry = [&](StringRef virtualMount, const fs::path& root, const fs::directory_entry& entry)
	{
		KOKKO_PROFILE_SCOPE("Scan file");

		const fs::path& currentPath = entry.path();
		std::filesystem::path currentExt = currentPath.extension();
		if (entry.is_regular_file() == false || currentExt == EditorConstants::MetadataExtension)
			return;

		assetPathStr = currentPath.generic_u8string();

		Optional<AssetType> assetType;
		if (currentExt == levelExt)
		{
			assetType = AssetType::Level;
		}
		else if (currentExt == materialExt)
		{
			assetType = AssetType::Material;
		}
		else if (currentExt == shaderExt)
		{
			assetType = AssetType::Shader;
		}
		else if (currentExt == textureJpgExt ||
			currentExt == textureJpegExt ||
			currentExt == texturePngExt ||
			currentExt == textureHdrExt)
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

		assets.PushBack(AssetInfo(
			allocator, virtualMount, StringRef(relativeStdStr.c_str(), relativeStdStr.length()),
			assetUid, calculatedHash, assetType.GetValue()));

		auto uidPair = uidToIndexMap.Insert(assetUid);
		uidPair->second = assetRefIndex;

		auto pathPair = pathToIndexMap.Insert(assets.GetBack().GetVirtualPath());
		pathPair->second = assetRefIndex;
	};

	if (scanProject)
	{
		const fs::path& assetDir = editorProject->GetAssetPath();
		const StringRef virtualMountAssets(EditorConstants::VirtualMountAssets);

		for (const auto& entry : fs::recursive_directory_iterator(assetDir))
			processEntry(virtualMountAssets, assetDir, entry);
	}
	else
	{
		const fs::path engineResDir = fs::absolute(EditorConstants::EngineResourcePath);
		const fs::path editorResDir = fs::absolute(EditorConstants::EditorResourcePath);
		const StringRef virtualMountEngine(EditorConstants::VirtualMountEngine);
		const StringRef virtualMountEditor(EditorConstants::VirtualMountEditor);

		for (const auto& entry : fs::recursive_directory_iterator(engineResDir))
			processEntry(virtualMountEngine, engineResDir, entry);

		for (const auto& entry : fs::recursive_directory_iterator(editorResDir))
			processEntry(virtualMountEditor, editorResDir, entry);
	}

}

AssetInfo::AssetInfo(Allocator* allocator, StringRef virtualMount, StringRef relativePath,
	Uid uid, uint64_t contentHash, AssetType type) :
	virtualPath(allocator),
	uid(uid),
	contentHash(contentHash),
	type(type)
{
	virtualPath.Reserve(virtualMount.len + 1 + relativePath.len);
	virtualPath.Append(virtualMount);
	virtualPath.Append('/');
	virtualPath.Append(relativePath);

	this->virtualMount = virtualPath.GetRef().SubStr(0, virtualMount.len);
	this->pathRelativeToMount = virtualPath.GetRef().SubStr(virtualMount.len + 1);
	
	intptr_t lastSlash = this->pathRelativeToMount.FindLast(StringRef("/", 1));
	this->filename = virtualPath.GetRef().SubStr(lastSlash + 1);
}

}
}
