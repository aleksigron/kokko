#include <filesystem>
#include <thread>

#include "Core/Core.hpp"

#include "Debug/Instrumentation.hpp"

#include "Memory/RootAllocator.hpp"

#include "System/Logger.hpp"

int main(int argc, char** argv)
{
	RootAllocator rootAllocator;
	Allocator* defaultAlloc = RootAllocator::GetDefaultAllocator();
	kokko::Logger logger(defaultAlloc);
	kokko::Log::SetLogInstance(&logger);

	Instrumentation& instr = Instrumentation::Get();
	instr.BeginSession("render_test_trace.json");

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

	fs::path testPath;

	for (const auto& entry : testItr)
	{
		if (entry.is_regular_file() == false)
			continue;

		const auto path = entry.path();

		if (path.filename() != testFilename)
			continue;

		std::error_code relativeError;
		testPath = fs::relative(path.parent_path(), testsRoot, relativeError);

		if (relativeError)
		{
			KK_LOG_ERROR("Test path couldn't be made relative to test root.");
			continue;
		}

		const std::string testNameStr = testPath.generic_u8string();

		KK_LOG_INFO("Run test: {}", testNameStr.c_str());

		{
			KOKKO_PROFILE_SCOPE(testNameStr.c_str());
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	instr.EndSession();

	return 0;
}
