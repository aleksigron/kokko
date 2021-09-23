cmake_minimum_required(VERSION 3.12)

project("Kokko Editor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_NAME kokko-editor)
set(KOKKO_ENGINE_PATH ..)
set(KOKKO_LIB kokko)

include_directories(
	src
	${KOKKO_ENGINE_PATH}/src
	${KOKKO_ENGINE_PATH}/deps/doctest
	${KOKKO_ENGINE_PATH}/deps/fmt/include
	${KOKKO_ENGINE_PATH}/deps/glad/include
	${KOKKO_ENGINE_PATH}/deps/imgui
	${KOKKO_ENGINE_PATH}/deps/imguizmo
)

set (IMGUI_SOURCES
	${KOKKO_ENGINE_PATH}/deps/imgui/imconfig.h
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui.h
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui_demo.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui_draw.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui_tables.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/imgui_widgets.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/backends/imgui_impl_glfw.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/backends/imgui_impl_glfw.h
	${KOKKO_ENGINE_PATH}/deps/imgui/backends/imgui_impl_opengl3.cpp
	${KOKKO_ENGINE_PATH}/deps/imgui/backends/imgui_impl_opengl3.h
)

set (IMGUIZMO_SOURCES
	${KOKKO_ENGINE_PATH}/deps/imguizmo/ImGuizmo.cpp
	${KOKKO_ENGINE_PATH}/deps/imguizmo/ImGuizmo.h
)

set (EDITOR_SOURCES
	src/main.cpp
	src/AssetBrowserView.cpp
	src/AssetBrowserView.hpp
	src/DebugView.cpp
	src/DebugView.hpp
	src/EditorApp.cpp
	src/EditorApp.hpp
	src/EditorCamera.cpp
	src/EditorCamera.hpp
	src/EditorConstants.cpp
	src/EditorConstants.hpp
	src/EditorCore.cpp
	src/EditorCore.hpp
	src/EditorImages.cpp
	src/EditorImages.hpp
	src/EditorWindowInfo.hpp
	src/EntityListView.cpp
	src/EntityListView.hpp
	src/EntityView.cpp
	src/EntityView.hpp
	src/FilePickerDialog.cpp
	src/FilePickerDialog.hpp
	src/SceneView.cpp
	src/SceneView.hpp
	src/SelectionContext.hpp
)

source_group(Dependencies\\imgui FILES ${IMGUI_SOURCES})
source_group(Dependencies\\imguizmo FILES ${IMGUIZMO_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${EDITOR_SOURCES})

set(SOURCES
	${IMGUI_SOURCES}
	${IMGUIZMO_SOURCES}
	${EDITOR_SOURCES}
)

add_executable(${EXECUTABLE_NAME} $<TARGET_OBJECTS:${KOKKO_LIB}> ${SOURCES})

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
	IMGUI_IMPL_OPENGL_LOADER_GLAD
)

add_subdirectory(${KOKKO_ENGINE_PATH} kokko-build)

target_link_libraries(${EXECUTABLE_NAME} ${KOKKO_LIB})