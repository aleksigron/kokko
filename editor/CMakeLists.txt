cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project("Kokko Editor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_NAME kokko-editor)
set(KOKKO_LIB kokko)

set(ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(ENGINE_PATH "${ROOT_PATH}/engine")

include_directories(
	src
	${ENGINE_PATH}/src
	${ROOT_PATH}/deps/doctest
	${ROOT_PATH}/deps/fmt/include
	${ROOT_PATH}/deps/glad/include
	${ROOT_PATH}/deps/rapidjson/include
	${ROOT_PATH}/deps/imgui
	${ROOT_PATH}/deps/imguizmo
)

set (IMGUI_SOURCES
	${ROOT_PATH}/deps/imgui/imconfig.h
	${ROOT_PATH}/deps/imgui/imgui.cpp
	${ROOT_PATH}/deps/imgui/imgui.h
	${ROOT_PATH}/deps/imgui/imgui_demo.cpp
	${ROOT_PATH}/deps/imgui/imgui_draw.cpp
	${ROOT_PATH}/deps/imgui/imgui_tables.cpp
	${ROOT_PATH}/deps/imgui/imgui_widgets.cpp
	${ROOT_PATH}/deps/imgui/backends/imgui_impl_glfw.cpp
	${ROOT_PATH}/deps/imgui/backends/imgui_impl_glfw.h
	${ROOT_PATH}/deps/imgui/backends/imgui_impl_opengl3.cpp
	${ROOT_PATH}/deps/imgui/backends/imgui_impl_opengl3.h
)

set (IMGUIZMO_SOURCES
	${ROOT_PATH}/deps/imguizmo/ImGuizmo.cpp
	${ROOT_PATH}/deps/imguizmo/ImGuizmo.h
)

set (EDITOR_SOURCES
	src/main.cpp
	src/AssetBrowserView.cpp
	src/AssetBrowserView.hpp
	src/AssetLibrary.cpp
	src/AssetLibrary.hpp
	src/AssetView.cpp
	src/AssetView.hpp
	src/DebugView.cpp
	src/DebugView.hpp
	src/EditorApp.cpp
	src/EditorApp.hpp
	src/EditorAssetLoader.cpp
	src/EditorAssetLoader.hpp
	src/EditorCamera.cpp
	src/EditorCamera.hpp
	src/EditorConstants.cpp
	src/EditorConstants.hpp
	src/EditorContext.hpp
	src/EditorCore.cpp
	src/EditorCore.hpp
	src/EditorImages.cpp
	src/EditorImages.hpp
	src/EditorProject.cpp
	src/EditorProject.hpp
	src/EditorUserSettings.cpp
	src/EditorUserSettings.hpp
	src/EditorWindow.hpp
	src/EntityListView.cpp
	src/EntityListView.hpp
	src/EntityView.cpp
	src/EntityView.hpp
	src/FilePickerDialog.cpp
	src/FilePickerDialog.hpp
	src/Hash_StdFilesystem.hpp
	src/SceneView.cpp
	src/SceneView.hpp
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

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/release"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/relwithdebinfo"
	RUNTIME_OUTPUT_DIRECTORY_RELMINSIZE "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/relminsize"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/debug"
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
	IMGUI_IMPL_OPENGL_LOADER_GLAD
)

add_subdirectory(${ENGINE_PATH} kokko-build)

target_link_libraries(${EXECUTABLE_NAME} ${KOKKO_LIB})
