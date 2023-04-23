#include <filesystem>
#include <thread>

#include "webp/decode.h"
#include "webp/encode.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EngineConstants.hpp"
#include "Engine/World.hpp"

#include "Memory/RootAllocator.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Framebuffer.hpp"

#include "Resources/AssetLibrary.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemResolverVirtual.hpp"
#include "System/Logger.hpp"
#include "System/WindowManager.hpp"
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

	constexpr int width = 512;
	constexpr int height = 512;
	constexpr int bytesPerPixel = 3;
	constexpr int stride = width * bytesPerPixel;
	constexpr int imageBytes = stride * height;
	constexpr int totalBytes = imageBytes + stride; // Reserve one row for swapping row order

	kokko::render::Framebuffer framebuffer;
	RenderTextureSizedFormat colorFormat[] = { RenderTextureSizedFormat::SRGB8 };
	kokko::render::Device* renderDevice = engine.GetRenderDevice();
	kokko::render::CommandEncoder* encoder = engine.GetCommandEncoder();
	framebuffer.SetRenderDevice(renderDevice);
	framebuffer.Create(width, height, Optional<RenderTextureSizedFormat>(), ArrayView(colorFormat));

	Array<uint8_t> resultPixelArray(defaultAlloc);
	resultPixelArray.Resize(totalBytes);
	uint8_t* resultPixels = resultPixelArray.GetData();
	uint8_t* swapBuffer = &resultPixels[imageBytes];

	Array<uint8_t> expectationPixelArray(defaultAlloc);
	expectationPixelArray.Resize(imageBytes);

	Array<uint8_t> expectationFileArray(defaultAlloc);
	kokko::String testLevelContent(defaultAlloc);

	int failedTests = 0;
	int erroredTests = 0;

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

			kokko::World* world = engine.GetWorld();
			world->ClearAllEntities();

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
			}

			{
				KOKKO_PROFILE_SCOPE("Read render result");

				encoder->BindFramebuffer(framebuffer.GetFramebufferId());
				// TODO: Start using pixel buffer to read pixels
				// When supporting Vulkan, we will need to use vkCmdBlitImage or
				// vkCmdCopyImage to copy to host-visible memory
				renderDevice->ReadFramebufferPixels(0, 0, width, height,
					RenderTextureBaseFormat::RGB, RenderTextureDataType::UnsignedByte, resultPixels);
			}

			{
				KOKKO_PROFILE_SCOPE("Reorder image rows");

				for (int rowIndex = 0, numSwaps = height / 2; rowIndex < numSwaps; ++rowIndex)
				{
					uint8_t* forwardRow = resultPixels + (stride * rowIndex);
					uint8_t* reverseRow = resultPixels + (stride * (height - rowIndex - 1));
					std::memcpy(swapBuffer, forwardRow, stride);
					std::memcpy(forwardRow, reverseRow, stride);
					std::memcpy(reverseRow, swapBuffer, stride);
				}
			}

			uint8_t* resultWebpBuffer;
			size_t resultWebpSize = 0;

			{
				KOKKO_PROFILE_SCOPE("Encode result to WebP");

				resultWebpSize = WebPEncodeLosslessRGB(resultPixels, width, height, stride, &resultWebpBuffer);
			}

			if (resultWebpSize != 0)
			{
				KOKKO_PROFILE_SCOPE("Write result to file");

				const auto resultPath = testFolderPath / "actual.webp";
				const auto resultPathStr = resultPath.u8string();

				ArrayView<uint8_t> content(resultWebpBuffer, resultWebpSize);
				if (filesystem.Write(resultPathStr.c_str(), content, false) == false)
				{
					KK_LOG_ERROR("Writing test result failed: {}", resultPathStr.c_str());
				}

				WebPFree(resultWebpBuffer);
			}

			uint8_t* expectWebpData = nullptr;
			size_t expectWebpSize = 0;

			{
				KOKKO_PROFILE_SCOPE("Read expectation from file");

				const auto expectationPath = testFolderPath / "expected.webp";
				const auto expectationPathStr = expectationPath.u8string();

				if (filesystem.ReadBinary(expectationPathStr.c_str(), expectationFileArray))
				{
					expectWebpData = expectationFileArray.GetData();
					expectWebpSize = expectationFileArray.GetCount();
				}
				else
				{
					KK_LOG_ERROR("Reading test expectation failed: {}", expectationPathStr.c_str());
				}
			}

			uint8_t* expectationPixels = nullptr;

			if (expectWebpData != nullptr)
			{
				KOKKO_PROFILE_SCOPE("Decode expectation from WebP");

				uint8_t* output = expectationPixelArray.GetData();
				expectationPixels = WebPDecodeRGBInto(expectWebpData, expectWebpSize, output, imageBytes, stride);
			}

			if (expectationPixels != nullptr)
			{
				KOKKO_PROFILE_SCOPE("Compare result to expectation");

				double diff = 0.0;

				for (int y = 0; y < height; ++y)
				{
					for (int x = 0; x < width; ++x)
					{
						size_t index = y * stride + x * bytesPerPixel;
						double diff0 = std::abs(resultPixels[index + 0] - expectationPixels[index + 0]) / 255.0;
						double diff1 = std::abs(resultPixels[index + 1] - expectationPixels[index + 1]) / 255.0;
						double diff2 = std::abs(resultPixels[index + 2] - expectationPixels[index + 2]) / 255.0;
						diff += (diff0 + diff1 + diff2) / 3.0;
					}
				}

				diff = diff / (width * height);
				constexpr double allowed = 0.0002;

				if (diff > allowed)
				{
					KK_LOG_INFO("Test failed, diff {} is over allowed value {}", diff, allowed);
					failedTests += 1;
				}
				else
				{
					KK_LOG_INFO("Test passed, diff {}", diff);
				}
			}
		}

		// Swap window buffers to allow better frame capture
		engine.GetWindowManager()->GetWindow()->Swap();
	}

	instr.EndSession();

	if (failedTests != 0 || erroredTests != 0)
	{
		KK_LOG_ERROR("Failed tests: {}, errored tests: {}", failedTests, erroredTests);
		return failedTests + erroredTests;
	}

	return 0;
}
