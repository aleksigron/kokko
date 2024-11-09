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
	${PROJECT_ROOT}/deps/stb_image_write/include
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
)

set (IMGUIZMO_SOURCES
	${PROJECT_ROOT}/deps/imguizmo/ImGuizmo.cpp
	${PROJECT_ROOT}/deps/imguizmo/ImGuizmo.h
)

set (STB_IMAGE_WRITE_SOURCES
	${PROJECT_ROOT}/deps/stb_image_write/include/stb_image_write/stb_image_write.h
	${PROJECT_ROOT}/deps/stb_image_write/src/stb_image_write.cpp
)

set (EDITOR_SOURCES
	src/App/main.cpp
	src/App/EditorApp.cpp
	src/App/EditorApp.hpp
	src/App/EditorCamera.cpp
	src/App/EditorCamera.hpp
	src/App/EditorConstants.cpp
	src/App/EditorConstants.hpp
	src/App/EditorContext.hpp
	src/App/EditorCore.cpp
	src/App/EditorCore.hpp
	src/App/EditorImages.cpp
	src/App/EditorImages.hpp
	src/App/EditorProject.cpp
	src/App/EditorProject.hpp
	src/App/EditorUserSettings.cpp
	src/App/EditorUserSettings.hpp
	src/System/ConsoleLogger.cpp
	src/System/ConsoleLogger.hpp
	src/System/EditorAssetLoader.cpp
	src/System/EditorAssetLoader.hpp
	src/System/Hash_StdFilesystem.hpp
	src/System/ImguiImplOpenGL.cpp
	src/System/ImguiImplOpenGL.hpp
	src/Views/AssetBrowserView.cpp
	src/Views/AssetBrowserView.hpp
	src/Views/AssetView.cpp
	src/Views/AssetView.hpp
	src/Views/ConsoleView.cpp
	src/Views/ConsoleView.hpp
	src/Views/DebugView.cpp
	src/Views/DebugView.hpp
	src/Views/EditorWindow.hpp
	src/Views/EntityListView.cpp
	src/Views/EntityListView.hpp
	src/Views/EntityView.cpp
	src/Views/EntityView.hpp
	src/Views/FilePickerDialog.cpp
	src/Views/FilePickerDialog.hpp
	src/Views/SceneView.cpp
	src/Views/SceneView.hpp
)

source_group(Dependencies\\imgui FILES ${IMGUI_SOURCES})
source_group(Dependencies\\imguizmo FILES ${IMGUIZMO_SOURCES})
source_group(Dependencies\\stb_image_write FILES ${STB_IMAGE_WRITE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${EDITOR_SOURCES})

set(SOURCES
	${IMGUI_SOURCES}
	${IMGUIZMO_SOURCES}
	${STB_IMAGE_WRITE_SOURCES}
	${EDITOR_SOURCES}
)

add_executable(${EXECUTABLE_NAME} $<TARGET_OBJECTS:${KOKKO_LIB}> ${SOURCES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "kokko")

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_ROOT}/build/bin/Release"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_ROOT}/build/bin/RelWithDebInfo"
	RUNTIME_OUTPUT_DIRECTORY_RELMINSIZE "${PROJECT_ROOT}/build/bin/RelMinSize"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/bin/Debug"
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
	IMGUI_IMPL_OPENGL_LOADER_GLAD
)

if(KOKKO_USE_SANITIZER)
target_compile_options(${EXECUTABLE_NAME} INTERFACE -fsanitize=${KOKKO_USE_SANITIZER})
target_link_options(${EXECUTABLE_NAME} INTERFACE -fsanitize=${KOKKO_USE_SANITIZER})
endif()

target_link_libraries(${EXECUTABLE_NAME} ${KOKKO_LIB})
