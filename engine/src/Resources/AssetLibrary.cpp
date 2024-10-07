#include "AssetLibrary.hpp"

#include <filesystem>

#include "doctest/doctest.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "Core/Core.hpp"

#include "Engine/EngineConstants.hpp"

#include "System/Filesystem.hpp"

namespace kokko
{
namespace
{

void CreateBaseMetadataJson(rapidjson::Document& document, uint64_t hash, const kokko::Uid& uid)
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

void CreateTextureMetadataJson(rapidjson::Document& document, uint64_t hash, const kokko::Uid& uid,
	const TextureAssetMetadata& metadata)
{
	CreateBaseMetadataJson(document, hash, uid);

	rapidjson::Document::AllocatorType& alloc = document.GetAllocator();

	rapidjson::Value genMipsValue(metadata.generateMipmaps);
	document.AddMember("generate_mipmaps", genMipsValue, alloc);

	rapidjson::Value linearValue(metadata.preferLinear);
	document.AddMember("prefer_linear", linearValue, alloc);
}

int32_t LoadTextureMetadata(const rapidjson::Document& document, Array<TextureAssetMetadata>& metadataArray)
{
	TextureAssetMetadata metadata;

	auto genMipmapsItr = document.FindMember("generate_mipmaps");
	if (genMipmapsItr != document.MemberEnd() && genMipmapsItr->value.IsBool())
		metadata.generateMipmaps = genMipmapsItr->value.GetBool();

	auto preferLinearItr = document.FindMember("prefer_linear");
	if (preferLinearItr != document.MemberEnd() && preferLinearItr->value.IsBool())
		metadata.preferLinear = preferLinearItr->value.GetBool();

	int32_t index = static_cast<int32_t>(metadataArray.GetCount());
	metadataArray.PushBack(metadata);
	return index;
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

bool IsTextAsset(AssetType type)
{
	switch (type)
	{
	case AssetType::Level:
	case AssetType::Material:
	case AssetType::Shader:
		return true;
	case AssetType::Unknown:
	case AssetType::Model:
	case AssetType::Texture:
		return false;
	default:
		return false;
	}
}

void NormalizeLineEndings(ArrayView<const uint8_t> bytes, String& resultOut)
{
	static const ConstStringView crlf("\r\n");

	size_t count = bytes.GetCount();
	resultOut.Reserve(count);

	const char* chars = reinterpret_cast<const char*>(bytes.GetData());
	size_t charIdx = 0;
	while (charIdx < count)
	{
		bool match = false;

		if (charIdx + crlf.len <= count)
		{
			match = true;
			for (size_t i = 0; i < crlf.len; ++i)
				if (chars[charIdx + i] != crlf[i])
					match = false;
		}

		if (match)
		{
			resultOut.Append('\n');
			charIdx += crlf.len;
		}
		else
		{
			resultOut.Append(chars[charIdx]);
			charIdx += 1;
		}
	}
}

TEST_CASE("AssetLibrary.NormalizeLineEndings")
{
	String result(Allocator::GetDefault());

	const char source[] = "yes\r\nno\nmmaybe\r\n\tnever\r";
	const char expected[] = "yes\nno\nmmaybe\n\tnever\r";
	ArrayView<const uint8_t> view(reinterpret_cast<const uint8_t*>(source), sizeof(source) - 1);

	NormalizeLineEndings(view, result);
	CHECK(std::strcmp(result.GetCStr(), expected) == 0);
}

} // Anonymous namespace

AssetLibrary::AssetLibrary(Allocator* allocator, Filesystem* filesystem) :
	allocator(allocator),
	filesystem(filesystem),
	uidToIndexMap(allocator),
	pathToIndexMap(allocator),
	assets(allocator),
	textureMetadata(allocator),
	updatedAssets(allocator)
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

Optional<Uid> AssetLibrary::CreateAsset(
	AssetType type, ConstStringView pathRelativeToAssets, ArrayView<const uint8_t> content)
{
	Uid assetUid = Uid::Create();
	uint64_t calculatedHash = CalculateHash(type, content);
	int32_t metadataIndex = -1;
	uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
	ConstStringView mount = ConstStringView(EngineConstants::VirtualMountAssets);

	assets.PushBack(AssetInfo(allocator, mount, pathRelativeToAssets, assetUid, calculatedHash, metadataIndex, type));
	const String& virtualPath = assets.GetBack().GetVirtualPath();

	if (filesystem->Write(virtualPath.GetCStr(), content, false) == false)
	{
		assets.PopBack();
		KK_LOG_ERROR("New asset couldn't be written to path {}", virtualPath.GetCStr());
		return Optional<Uid>();
	}

	rapidjson::Document document;
	if (type == AssetType::Texture)
		CreateTextureMetadataJson(document, calculatedHash, assetUid, TextureAssetMetadata());
	else
		CreateBaseMetadataJson(document, calculatedHash, assetUid);

	String metaPath = virtualPath + EngineConstants::MetadataExtension;
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

bool AssetLibrary::RenameAsset(const Uid& uid, ConstStringView newFilename)
{
	auto uidMapPair = uidToIndexMap.Lookup(uid);
	if (uidMapPair == nullptr)
	{
		KK_LOG_ERROR("Asset was not found in uid map.");
		return false;
	}

	AssetInfo& asset = assets[uidMapPair->second];

	auto pathMapPair = pathToIndexMap.Lookup(asset.virtualPath);
	if (pathMapPair == nullptr)
	{
		KK_LOG_ERROR("Asset was not found in path map. Virtual path: {}", asset.virtualPath.GetCStr());
		return false;
	}

	namespace fs = std::filesystem;
	const fs::path& assetDir = projectConfig.assetFolderPath;
	const auto& filePath = asset.relativeFilePath;
	const auto& folderPath = asset.relativeFolderPath;
	fs::path oldAssetPath = assetDir / fs::path(filePath.str, filePath.str + filePath.len);
	fs::path newAssetPath = assetDir / fs::path(folderPath.str, folderPath.str + folderPath.len) /
		fs::path(newFilename.str, newFilename.str + newFilename.len);
	
	std::error_code renameError;
	std::filesystem::rename(oldAssetPath, newAssetPath, renameError);

	if (renameError)
	{	
		KK_LOG_ERROR("Asset file rename failed: {}", renameError.message().c_str());
		return false;
	}

	// Rename meta file as well

	fs::path oldMetaPath = oldAssetPath;
	oldMetaPath.concat(EngineConstants::MetadataExtension);
	fs::path newMetaPath = newAssetPath;
	newMetaPath.concat(EngineConstants::MetadataExtension);

	std::filesystem::rename(oldMetaPath, newMetaPath, renameError);

	if (renameError)
	{
		KK_LOG_ERROR("Meta file rename failed, reversing asset file rename. Error: {}", renameError.message().c_str());
		std::filesystem::rename(newAssetPath, oldAssetPath, renameError);

		if (renameError)
			KK_LOG_ERROR("Asset file rename reversal failed. Filesystem left in an inconsistent state. Error: {}",
				renameError.message().c_str());

		return false;
	}

	pathToIndexMap.Remove(pathMapPair);
	asset.UpdateFilename(newFilename);

	pathMapPair = pathToIndexMap.Insert(asset.GetVirtualPath());
	pathMapPair->second = uidMapPair->second;

	return true;
}

bool AssetLibrary::UpdateAssetContent(const Uid& uid, ArrayView<const uint8_t> content)
{
	auto pair = uidToIndexMap.Lookup(uid);
	if (pair == nullptr)
		return false;

	AssetInfo& asset = assets[pair->second];

	uint64_t calculatedHash = CalculateHash(asset.type, content);

	if (calculatedHash != asset.contentHash)
	{
		// Update in-memory data
		asset.contentHash = calculatedHash;

		// Update metadata file
		rapidjson::Document document;
		if (asset.type == AssetType::Texture)
		{
			TextureAssetMetadata metadata;

			if (asset.metadataIndex >= 0)
				metadata = textureMetadata[asset.metadataIndex];

			CreateTextureMetadataJson(document, calculatedHash, asset.uid, metadata);
		}
		else
			CreateBaseMetadataJson(document, calculatedHash, asset.uid);

		const String& assetVirtualPath = asset.GetVirtualPath();
		String metaVirtualPath = assetVirtualPath + EngineConstants::MetadataExtension;

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

	updatedAssets.InsertUnique(uid);

	return true;
}

const TextureAssetMetadata* AssetLibrary::GetTextureMetadata(const AssetInfo* asset) const
{
	assert(asset != nullptr);
	assert(asset->type == AssetType::Texture);

	if (asset == nullptr || asset->type != AssetType::Texture || asset->metadataIndex < 0)
		return nullptr;

	return &textureMetadata[asset->metadataIndex];
}

bool AssetLibrary::UpdateTextureMetadata(const Uid& uid, const TextureAssetMetadata& metadata)
{
	auto pair = uidToIndexMap.Lookup(uid);
	if (pair == nullptr)
		return false;

	AssetInfo& asset = assets[pair->second];

	assert(asset.type == AssetType::Texture);

	if (asset.metadataIndex < 0)
	{
		// Create metadata
		asset.metadataIndex = static_cast<int32_t>(textureMetadata.GetCount());
		textureMetadata.PushBack(metadata);
	}
	else
	{
		// Update existing metadata
		textureMetadata[asset.metadataIndex] = metadata;
	}

	rapidjson::Document document;
	CreateTextureMetadataJson(document, asset.contentHash, asset.uid, metadata);

	String metaVirtualPath = asset.GetVirtualPath() + EngineConstants::MetadataExtension;

	rapidjson::StringBuffer jsonStringBuffer;
	if (WriteDocumentToFile(filesystem, metaVirtualPath.GetCStr(), document, jsonStringBuffer) == false)
	{
		KK_LOG_ERROR("Couldn't write asset meta file: {}", metaVirtualPath.GetCStr());
		return false;
	}

	updatedAssets.InsertUnique(uid);

	return true;
}

void AssetLibrary::SetAppScopeConfig(const AssetScopeConfiguration& config)
{
	applicationConfig = config;
}

void AssetLibrary::SetProjectScopeConfig(const AssetScopeConfiguration& config)
{
	projectConfig = config;
}

bool AssetLibrary::ScanAssets(bool scanEngine, bool scanApp, bool scanProject)
{
	namespace fs = std::filesystem;

	const fs::path metadataExt(EngineConstants::MetadataExtension);
	const fs::path levelExt(".level");
	const fs::path materialExt(".material");
	const fs::path modelGltfExt(".gltf");
	const fs::path modelGlbExt(".glb");
	const fs::path modelMeshExt(".mesh");
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

	auto processEntry = [&](ConstStringView virtualMount, const fs::path& root, const fs::directory_entry& entry)
	{
		KOKKO_PROFILE_SCOPE("Scan file");

		if (entry.is_regular_file() == false)
			return;

		const fs::path& currentPath = entry.path();
		std::filesystem::path currentExt = currentPath.extension();
		if (currentExt == metadataExt)
			return;

		assetPathStr = currentPath.generic_u8string();

		// TODO: Make extension detection case-independent

		AssetType assetType = AssetType::Unknown;
		if (currentExt == levelExt)
			assetType = AssetType::Level;
		else if (currentExt == modelGltfExt ||
			currentExt == modelGlbExt ||
			currentExt == modelMeshExt)
			assetType = AssetType::Model;
		else if (currentExt == materialExt)
			assetType = AssetType::Material;
		else if (currentExt == shaderExt)
			assetType = AssetType::Shader;
		else if (currentExt == textureJpgExt ||
			currentExt == textureJpegExt ||
			currentExt == texturePngExt ||
			currentExt == textureHdrExt)
			assetType = AssetType::Texture;

		if (assetType == AssetType::Unknown)
		{
			KK_LOG_WARN("File didn't match known asset types: {}", assetPathStr.c_str());
			return;
		}

		fileContent.Clear();
		if (filesystem->ReadBinary(assetPathStr.c_str(), fileContent) == false)
		{
			KK_LOG_ERROR("Couldn't read asset file: {}", assetPathStr.c_str());
			return;
		}

		uint64_t calculatedHash = CalculateHash(assetType, fileContent.GetView());
		Uid assetUid;

		// Open meta file
		fs::path metaPath = currentPath;
		metaPath += metadataExt;
		metaPathStr = metaPath.u8string();

		bool needsToWriteMetaFile = false;

		metaContent.Clear();
		if (filesystem->ReadText(metaPathStr.c_str(), metaContent))
		{
			document.ParseInsitu(metaContent.GetData());

			if (document.GetParseError() != rapidjson::kParseErrorNone)
			{
				KK_LOG_ERROR("Error parsing meta file: {}. New file is generated if existing file is removed.",
					metaPathStr.c_str());
			}
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

			if (assetType == AssetType::Texture)
			{
				TextureAssetMetadata metadata;
				CreateTextureMetadataJson(document, calculatedHash, assetUid, metadata);
			}
			else
			{
				CreateBaseMetadataJson(document, calculatedHash, assetUid);
			}

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
			KK_LOG_ERROR("Asset path {} could not be made relative, error: {}", currentPath.u8string().c_str(),
				err.message().c_str());
			return;
		}

		auto relativeStdStr = relative.generic_u8string();

		uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
		int32_t metadataIndex = -1;
		if (assetType == AssetType::Texture)
			metadataIndex = LoadTextureMetadata(document, textureMetadata);

		assets.PushBack(AssetInfo(
			allocator, virtualMount, ConstStringView(relativeStdStr.c_str(), relativeStdStr.length()),
			assetUid, calculatedHash, metadataIndex, assetType));

		auto uidPair = uidToIndexMap.Insert(assetUid);
		uidPair->second = assetRefIndex;

		auto pathPair = pathToIndexMap.Insert(assets.GetBack().GetVirtualPath());
		pathPair->second = assetRefIndex;
	};

	auto scanScope = [&processEntry](const fs::path& resDir, ConstStringView virtualMount)
	{
		std::error_code itrError;
		auto dirItr = fs::recursive_directory_iterator(resDir, itrError);
		if (itrError)
		{
			KK_LOG_ERROR("Assets in {} couldn't be processed, check the current working directory.", resDir.string().c_str());
			return false;
		}

		for (const auto& entry : dirItr)
			processEntry(virtualMount, resDir, entry);

		return true;
	};

	if (scanEngine)
	{
		const fs::path engineResDir = fs::absolute(EngineConstants::EngineResourcePath);
		const ConstStringView virtualMountEngine(EngineConstants::VirtualMountEngine);

		if (scanScope(engineResDir, virtualMountEngine) == false)
			return false;
	}

	if (scanApp)
	{
		const fs::path& assetDir = fs::absolute(applicationConfig.assetFolderPath);

		if (scanScope(assetDir, applicationConfig.virtualMountName.GetRef()) == false)
			return false;
	}

	if (scanProject)
	{
		if (scanScope(projectConfig.assetFolderPath, projectConfig.virtualMountName.GetRef()) == false)
			return false;
	}

	return true;
}

bool AssetLibrary::GetNextUpdatedAssetUid(AssetType typeFilter, Uid& uid)
{
	size_t index = 0;
	while (index < updatedAssets.GetCount())
	{
		if (auto pair = uidToIndexMap.Lookup(updatedAssets[index]))
		{
			const AssetInfo& asset = assets[pair->second];
			if (asset.type == typeFilter)
			{
				uid = asset.uid;
				updatedAssets.Remove(index);
				return true;
			}
		}
		else
		{
			char uidStrBuf[Uid::StringLength + 1];
			updatedAssets[index].WriteTo(uidStrBuf);
			uidStrBuf[Uid::StringLength] = '\0';

			KK_LOG_WARN("Updated asset Uid {} not found in assets, removing.", uidStrBuf);
			updatedAssets.Remove(index);
			continue;
		}

		index += 1;
	}

	return false;
}

uint64_t AssetLibrary::CalculateHash(AssetType type, ArrayView<const uint8_t> content)
{
	uint64_t hash = 0;
	if (IsTextAsset(type))
	{
		String normalized(allocator);
		NormalizeLineEndings(content, normalized);
		hash = HashValue64(normalized.GetData(), normalized.GetLength(), 0);
	}
	else
	{
		hash = HashValue64(content.GetData(), content.GetCount(), 0);
	}
	return hash;
}

AssetInfo::AssetInfo(
	Allocator* allocator,
	ConstStringView virtualMount,
	ConstStringView relativePath,
	Uid uid,
	uint64_t contentHash,
	int32_t metadataIndex,
	AssetType type) :
	virtualPath(allocator),
	uid(uid),
	contentHash(contentHash),
	metadataIndex(metadataIndex),
	type(type)
{
	virtualPath.Reserve(virtualMount.len + 1 + relativePath.len);
	virtualPath.Append(virtualMount);
	virtualPath.Append('/');
	virtualPath.Append(relativePath);

	const ConstStringView virtualPathView = virtualPath.GetRef();
	intptr_t lastSlash = virtualPathView.FindLast(ConstStringView("/", 1));

	this->virtualMount = virtualPathView.SubStr(0, virtualMount.len);
	this->relativeFolderPath = virtualPathView.SubStrPos(virtualMount.len + 1, lastSlash);
	this->relativeFilePath = virtualPathView.SubStr(virtualMount.len + 1);
	this->filename = virtualPathView.SubStr(lastSlash + 1);
}

void AssetInfo::UpdateFilename(ConstStringView newFilename)
{
	size_t lenWithoutFilename = virtualPath.GetLength() - filename.len;
	virtualPath.Resize(lenWithoutFilename);
	virtualPath.Reserve(lenWithoutFilename + newFilename.len);
	virtualPath.Append(newFilename);

	// If virtualPath got re-allocated, we need to update string views pointing into it
	const ConstStringView virtualPathView = virtualPath.GetRef();
	virtualMount = virtualPathView.SubStr(0, virtualMount.len);
	relativeFolderPath = virtualPathView.SubStrPos(virtualMount.len + 1, lenWithoutFilename - 1);
	relativeFilePath = virtualPathView.SubStr(virtualMount.len + 1);
	filename = virtualPathView.SubStr(lenWithoutFilename);
}

TEST_CASE("AssetInfo.VirtualPathParts")
{
	Uid uid;
	uid.raw[0] = 877228993468580528ull;
	uid.raw[1] = 6433944024937364386ull;

	AssetInfo assetInfo(
		Allocator::GetDefault(),
		ConstStringView("engine"),
		ConstStringView("materials/deferred_geometry/fallback.material"),
		uid,
		12570451739923486631ull,
		-1,
		AssetType::Material);

	CHECK(assetInfo.GetVirtualPath() == ConstStringView("engine/materials/deferred_geometry/fallback.material"));
	CHECK(assetInfo.GetRelativeFolderPath() == ConstStringView("materials/deferred_geometry"));
	CHECK(assetInfo.GetRelativeFilePath() == ConstStringView("materials/deferred_geometry/fallback.material"));
	CHECK(assetInfo.GetFilename() == ConstStringView("fallback.material"));
	CHECK(assetInfo.GetUid() == uid);
	CHECK(assetInfo.GetType() == AssetType::Material);

	assetInfo.UpdateFilename(ConstStringView("testing_new_filename_for_test_test.material"));

	CHECK(assetInfo.GetVirtualPath() ==
		ConstStringView("engine/materials/deferred_geometry/testing_new_filename_for_test_test.material"));
	CHECK(assetInfo.GetRelativeFolderPath() == ConstStringView("materials/deferred_geometry"));
	CHECK(assetInfo.GetRelativeFilePath() ==
		ConstStringView("materials/deferred_geometry/testing_new_filename_for_test_test.material"));
	CHECK(assetInfo.GetFilename() == ConstStringView("testing_new_filename_for_test_test.material"));
}

}
