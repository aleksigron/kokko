cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_NAME kokko-render-test-runner)

include_directories(
	src
	${ENGINE_PATH}/src
	${PROJECT_ROOT}/deps/doctest
	${PROJECT_ROOT}/deps/fmt/include
	${PROJECT_ROOT}/deps/glad/include
	${PROJECT_ROOT}/deps/rapidjson/include
	${PROJECT_ROOT}/deps/libwebp/src
)

set (RENDER_TEST_SOURCES
	src/main.cpp
	src/TestRunnerAssetLoader.cpp
	src/TestRunnerAssetLoader.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${RENDER_TEST_SOURCES})

set(SOURCES
	${RENDER_TEST_SOURCES}
)

add_executable(${EXECUTABLE_NAME} ${SOURCES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "kokko")
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${KOKKO_LIB})

set(CMAKE_FOLDER "webp") # Group webp targets into a folder
add_subdirectory(${PROJECT_ROOT}/deps/libwebp libwebp-build)
set(CMAKE_FOLDER) # Disable folder grouping
target_link_libraries(${EXECUTABLE_NAME} PUBLIC webp)

