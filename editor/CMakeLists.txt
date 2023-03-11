cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_NAME kokko-editor)

include_directories(
	src
	${ENGINE_PATH}/src
	${PROJECT_ROOT}/deps/doctest
	${PROJECT_ROOT}/deps/fmt/include
	${PROJECT_ROOT}/deps/glad/include
	${PROJECT_ROOT}/deps/rapidjson/include
	${PROJECT_ROOT}/deps/imgui
	${PROJECT_ROOT}/deps/imguizmo
)

set (IMGUI_SOURCES
	${PROJECT_ROOT}/deps/imgui/imconfig.h
	${PROJECT_ROOT}/deps/imgui/imgui.cpp
	${PROJECT_ROOT}/deps/imgui/imgui.h
	${PROJECT_ROOT}/deps/imgui/imgui_demo.cpp
	${PROJECT_ROOT}/deps/imgui/imgui_draw.cpp
	${PROJECT_ROOT}/deps/imgui/imgui_tables.cpp
	${PROJECT_ROOT}/deps/imgui/imgui_widgets.cpp
	${PROJECT_ROOT}/deps/imgui/backends/imgui_impl_glfw.cpp
	${PROJECT_ROOT}/deps/imgui/backends/imgui_impl_glfw.h
	${PROJECT_ROOT}/deps/imgui/backends/imgui_impl_opengl3.cpp
	${PROJECT_ROOT}/deps/imgui/backends/imgui_impl_opengl3.h
)

set (IMGUIZMO_SOURCES
	${PROJECT_ROOT}/deps/imguizmo/ImGuizmo.cpp
	${PROJECT_ROOT}/deps/imguizmo/ImGuizmo.h
)

set (EDITOR_SOURCES
	src/main.cpp
	src/AssetBrowserView.cpp
	src/AssetBrowserView.hpp
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
set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "kokko")

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_ROOT}/build/bin/release"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_ROOT}/build/bin/relwithdebinfo"
	RUNTIME_OUTPUT_DIRECTORY_RELMINSIZE "${PROJECT_ROOT}/build/bin/relminsize"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/bin/debug"
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
	IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(${EXECUTABLE_NAME} ${KOKKO_LIB})
