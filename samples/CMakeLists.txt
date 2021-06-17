cmake_minimum_required(VERSION 3.12)

project("Kokko Sample")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_NAME kokko-sample)
set(KOKKO_ENGINE_PATH ..)
set(KOKKO_LIB kokko)

include_directories(
	src
    ${KOKKO_ENGINE_PATH}/src
	${KOKKO_ENGINE_PATH}/deps/doctest
)

set (SOURCES
	src/main.cpp
	src/App.cpp
	src/App.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${SOURCES})

add_executable(${EXECUTABLE_NAME} $<TARGET_OBJECTS:${KOKKO_LIB}> ${SOURCES})

add_subdirectory(${KOKKO_ENGINE_PATH} kokko-build)

target_link_libraries(${EXECUTABLE_NAME} ${KOKKO_LIB})
