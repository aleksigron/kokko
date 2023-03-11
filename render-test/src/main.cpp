#include <filesystem>
#include <thread>

#include "webp/encode.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EngineConstants.hpp"
#include "Engine/World.hpp"

#include "Memory/RootAllocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Framebuffer.hpp"

#include "Resources/AssetLibrary.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemResolverVirtual.hpp"
#include "System/Logger.hpp"
#include "System/WindowSettings.hpp"

#include "TestRunnerAssetLoader.hpp"

int main(int argc, char** argv)
{
	// Setup RootAllocator and logging

	RootAllocator rootAllocator;
	Allocator* defaultAlloc = RootAllocator::GetDefaultAllocator();
	kokko::Logger logger(defaultAlloc);
	kokko::Log::SetLogInstance(&logger);

	Instrumentation& instr = Instrumentation::Get();
	instr.BeginSession("render_test_trace.json");

	// Setup other engine systems

	kokko::FilesystemResolverVirtual::MountPoint mounts[] = {
		kokko::FilesystemResolverVirtual::MountPoint{
			kokko::ConstStringView(kokko::EngineConstants::VirtualMountEngine),
			kokko::ConstStringView(kokko::EngineConstants::EngineResourcePath)
		},
		kokko::FilesystemResolverVirtual::MountPoint{ 
			kokko::ConstStringView(kokko::EngineConstants::VirtualMountAssets),
			kokko::ConstStringView("render-test/res")
		}
	};
	kokko::FilesystemResolverVirtual resolver(defaultAlloc);
	resolver.SetMountPoints(ArrayView(mounts));

	kokko::Filesystem filesystem(defaultAlloc, &resolver);
	kokko::AssetLibrary assetLibrary(defaultAlloc, &filesystem);

	auto assetConfig = kokko::AssetScopeConfiguration{
		"render-test/res",
		kokko::String(defaultAlloc, kokko::EngineConstants::VirtualMountAssets)
	};
	assetLibrary.SetAppScopeConfig(assetConfig);

	AllocatorManager allocManager(defaultAlloc);
	kokko::TestRunnerAssetLoader assetLoader(defaultAlloc, &filesystem, &assetLibrary);
	Engine engine(&allocManager, &filesystem, &assetLoader);
	engine.GetSettings()->enableDebugTools = false;

	if (assetLibrary.ScanAssets(true, true, false) == false)
	{
		instr.EndSession();
		return -1;
	}
	
	kokko::WindowSettings windowSettings;
	windowSettings.verticalSync = false;
	windowSettings.visible = false;
	windowSettings.maximized = false;
	windowSettings.width = 200;
	windowSettings.height = 200;
	windowSettings.title = "kokko-render-test-runner";

	if (engine.Initialize(windowSettings) == false)
	{
		instr.EndSession();
		return -1;
	}

	// Setup tests

	namespace fs = std::filesystem;
	const auto testsRoot = fs::absolute("render-test/tests");
	const auto testFilename = fs::path("test.level");

	std::error_code testItrError;
	auto testItr = fs::recursive_directory_iterator(testsRoot, testItrError);
	if (testItrError)
	{
		KK_LOG_ERROR("Render tests couldn't be found, please check the current working directory.");
		return -1;
	}

	kokko::String testLevelContent(defaultAlloc);

	Framebuffer framebuffer;
	RenderTextureSizedFormat colorFormat[] = { RenderTextureSizedFormat::SRGB8 };
	constexpr int width = 512;
	constexpr int height = 512;
	constexpr int bytesPerPixel = 3;
	constexpr int stride = width * bytesPerPixel;
	constexpr int totalBytes = stride * height;
	framebuffer.SetRenderDevice(engine.GetRenderDevice());
	framebuffer.Create(width, height, Optional<RenderTextureSizedFormat>(), ArrayView(colorFormat));

	Array<unsigned char> pixelDataArray(defaultAlloc);
	pixelDataArray.Resize(totalBytes);
	unsigned char* pixelData = pixelDataArray.GetData();

	for (const auto& entry : testItr)
	{
		if (entry.is_regular_file() == false)
			continue;

		const auto& path = entry.path();

		if (path.filename() != testFilename)
			continue;

		const auto testFolderPath = path.parent_path();

		std::error_code relativeError;
		fs::path testPath = fs::relative(testFolderPath, testsRoot, relativeError);

		if (relativeError)
		{
			KK_LOG_ERROR("Test path couldn't be made relative to test root.");
			continue;
		}

		const std::string testNameStr = testPath.generic_u8string();

		KK_LOG_INFO("Run test: {}", testNameStr.c_str());

		{
			KOKKO_PROFILE_SCOPE(testNameStr.c_str());

			World* world = engine.GetWorld();

			{
				KOKKO_PROFILE_SCOPE("Load test level content");

				if (filesystem.ReadText(path.u8string().c_str(), testLevelContent) == false)
				{
					KK_LOG_ERROR("Test level content couldn't be loaded.");
					continue;
				}

				world->GetSerializer()->DeserializeFromString(testLevelContent.GetCStr());
			}

			{
				KOKKO_PROFILE_SCOPE("Run render test");

				engine.StartFrame();
				engine.UpdateWorld();
				engine.Render(Optional<CameraParameters>(), framebuffer);
				engine.EndFrame();

				engine.GetRenderDevice()->BindFramebuffer(RenderFramebufferTarget::Framebuffer, framebuffer.GetFramebufferId());
				engine.GetRenderDevice()->ReadFramebufferPixels(0, 0, width, height,
					RenderTextureBaseFormat::RGB, RenderTextureDataType::UnsignedByte, pixelData);

				uint8_t* webpBuffer;
				size_t webpSize = WebPEncodeRGB(pixelData, width, height, stride, 95.0f, &webpBuffer);
				if (webpSize != 0)
				{
					auto resultPath = testFolderPath / "actual.webp";

					ArrayView<uint8_t> content(webpBuffer, webpSize);
					filesystem.Write(resultPath.u8string().c_str(), content, false);

					WebPFree(webpBuffer);
				}
			}

			world->ClearAllEntities();
		}
	}

	instr.EndSession();

	return 0;
}
