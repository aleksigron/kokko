cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project("Kokko Engine")

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(KOKKO_LIB kokko)

add_subdirectory(engine kokko-engine-build)
add_subdirectory(editor kokko-editor-build)
