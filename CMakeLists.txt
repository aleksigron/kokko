cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project("Kokko Engine")
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(KOKKO_LIB kokko)

add_subdirectory(engine kokko-engine-build)

set(ENGINE_PATH "${PROJECT_ROOT}/engine")

add_subdirectory(editor kokko-editor-build)
add_subdirectory(render-test kokko-render-test-build)
