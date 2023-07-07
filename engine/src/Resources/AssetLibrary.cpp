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

bool IsTextAsset(AssetType type)
{
	switch (type)
	{
	case AssetType::Level:
	case AssetType::Material:
	case AssetType::Shader:
		return true;
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

Optional<Uid> AssetLibrary::CreateAsset(AssetType type, ConstStringView pathRelativeToAssets, ArrayView<const uint8_t> content)
{
	Uid assetUid = Uid::Create();
	uint64_t calculatedHash = CalculateHash(type, content);
	uint32_t assetRefIndex = static_cast<uint32_t>(assets.GetCount());
	ConstStringView mount = ConstStringView(EngineConstants::VirtualMountAssets);

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
		CreateMetadataJson(document, calculatedHash, asset.uid);

		String assetVirtualPath = asset.GetVirtualPath();
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

		Optional<AssetType> assetType;
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

		uint64_t calculatedHash = CalculateHash(assetType.GetValue(), fileContent.GetView());
		Uid assetUid;

		// Open meta file
		fs::path metaPath = currentPath;
		metaPath += metadataExt;
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
			allocator, virtualMount, ConstStringView(relativeStdStr.c_str(), relativeStdStr.length()),
			assetUid, calculatedHash, assetType.GetValue()));

		auto uidPair = uidToIndexMap.Insert(assetUid);
		uidPair->second = assetRefIndex;

		auto pathPair = pathToIndexMap.Insert(assets.GetBack().GetVirtualPath());
		pathPair->second = assetRefIndex;
	};

	if (scanEngine)
	{
		const fs::path engineResDir = fs::absolute(EngineConstants::EngineResourcePath);
		const ConstStringView virtualMountEngine(EngineConstants::VirtualMountEngine);

		std::error_code engineItrError;
		auto engineItr = fs::recursive_directory_iterator(engineResDir, engineItrError);
		if (engineItrError)
		{
			KK_LOG_ERROR("Engine assets couldn't be processed, please check the current working directory.");
			return false;
		}

		for (const auto& entry : engineItr)
			processEntry(virtualMountEngine, engineResDir, entry);
	}

	if (scanApp)
	{
		const fs::path& assetDir = fs::absolute(applicationConfig.assetFolderPath);

		std::error_code appItrError;
		auto appItr = fs::recursive_directory_iterator(assetDir, appItrError);
		if (appItrError)
		{
			KK_LOG_ERROR("Application assets couldn't be processed, please check the current working directory.");
			return false;
		}

		for (const auto& entry : appItr)
			processEntry(applicationConfig.virtualMountName.GetRef(), assetDir, entry);
	}

	if (scanProject)
	{
		const fs::path& assetDir = projectConfig.assetFolderPath;

		std::error_code projectItrError;
		auto projectItr = fs::recursive_directory_iterator(assetDir, projectItrError);
		if (projectItrError)
		{
			KK_LOG_ERROR("Project assets couldn't be processed: {}", projectItrError.message().c_str());
			return false;
		}

		for (const auto& entry : projectItr)
			processEntry(projectConfig.virtualMountName.GetRef(), assetDir, entry);
	}

	return true;
}

uint64_t AssetLibrary::CalculateHash(AssetType type, ArrayView<const uint8_t> content)
{
	uint64_t hash = 0;
	if (IsTextAsset(type))
	{
		String normalized(allocator);
		NormalizeLineEndings(content, normalized);
		hash = Hash64(normalized.GetData(), normalized.GetLength(), 0);
	}
	else
	{
		hash = Hash64(content.GetData(), content.GetCount(), 0);
	}
	return hash;
}

AssetInfo::AssetInfo(Allocator* allocator, ConstStringView virtualMount, ConstStringView relativePath,
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

	const ConstStringView virtualPathView = virtualPath.GetRef();

	this->virtualMount = virtualPathView.SubStr(0, virtualMount.len);
	this->pathRelativeToMount = virtualPathView.SubStr(virtualMount.len + 1);
	
	intptr_t lastSlash = virtualPathView.FindLast(ConstStringView("/", 1));
	this->filename = virtualPathView.SubStr(lastSlash + 1);
}

TEST_CASE("AssetInfo.VirtualPathParts")
{
	Uid uid;
	uid.raw[0] = 877228993468580528;
	uid.raw[1] = 6433944024937364386;

	AssetInfo assetInfo(
		Allocator::GetDefault(),
		ConstStringView("engine"),
		ConstStringView("materials/deferred_geometry/fallback.material"),
		uid,
		12570451739923486631,
		AssetType::Material);

	CHECK(assetInfo.GetVirtualPath().GetRef() == ConstStringView("engine/materials/deferred_geometry/fallback.material"));
	CHECK(assetInfo.GetFilename() == ConstStringView("fallback.material"));
	CHECK(assetInfo.GetUid() == uid);
	CHECK(assetInfo.GetType() == AssetType::Material);
}

}
