cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project("Kokko Engine")

set(CMAKE_CXX_STANDARD 17)

# Don't use language extensions
set(CMAKE_CXX_EXTENSIONS OFF)

set(IMGUI_LIB imgui)
set(KOKKO_LIB kokko)

include_directories(
	src
	deps/glad/include
	deps/glfw/include
	deps/imgui
	deps/rapidjson/include
	deps/stb_image/include
	deps/yaml-cpp/include
)

set (IMGUI_SOURCES
	deps/imgui/imconfig.h
	deps/imgui/imgui.cpp
	deps/imgui/imgui.h
	deps/imgui/imgui_demo.cpp
	deps/imgui/imgui_draw.cpp
	deps/imgui/imgui_tables.cpp
	deps/imgui/imgui_widgets.cpp
)

add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(${IMGUI_LIB} STATIC ${IMGUI_SOURCES})

set (GLAD_SOURCES
	deps/glad/include/glad/glad.h
	deps/glad/src/glad.c
)

set (STB_IMAGE_SOURCES
	deps/stb_image/include/stb_image/stb_image.h
	deps/stb_image/src/stb_image.cpp
)

set (DEPS_SOURCES
	${GLAD_SOURCES}
	${STB_IMAGE_SOURCES}
)

set (KOKKO_SOURCES
	src/Core/Array.hpp
	src/Core/BitfieldVariable.hpp
	src/Core/BitPack.hpp
	src/Core/Buffer.hpp
	src/Core/BufferRef.hpp
	src/Core/Color.hpp
	src/Core/ColorSrgb.hpp
	src/Core/Core.hpp
	src/Core/EncodingUtf8.cpp
	src/Core/EncodingUtf8.hpp
	src/Core/FixedArray.hpp
	src/Core/Hash.hpp
	src/Core/HashMap.hpp
	src/Core/Optional.hpp
	src/Core/Pair.hpp
	src/Core/Queue.hpp
	src/Core/Sort.hpp
	src/Core/SortedArray.hpp
	src/Core/String.cpp
	src/Core/String.hpp
	src/Core/StringRef.cpp
	src/Core/StringRef.hpp
	src/Debug/Debug.cpp
	src/Debug/Debug.hpp
	src/Debug/DebugConsole.cpp
	src/Debug/DebugConsole.hpp
	src/Debug/DebugCulling.cpp
	src/Debug/DebugCulling.hpp
	src/Debug/DebugGraph.cpp
	src/Debug/DebugGraph.hpp
	src/Debug/DebugLog.cpp
	src/Debug/DebugLog.hpp
	src/Debug/DebugMemoryStats.cpp
	src/Debug/DebugMemoryStats.hpp
	src/Debug/DebugTextRenderer.cpp
	src/Debug/DebugTextRenderer.hpp
	src/Debug/DebugUtil.hpp
	src/Debug/DebugVectorRenderer.cpp
	src/Debug/DebugVectorRenderer.hpp
	src/Debug/Instrumentation.cpp
	src/Debug/Instrumentation.hpp
	src/Debug/InstrumentationTimer.hpp
	src/Debug/LogHelper.cpp
	src/Debug/LogHelper.hpp
	src/Debug/LogLevel.hpp
	src/Debug/PerformanceTimer.hpp
	src/Editor/EditorCamera.cpp
	src/Editor/EditorCamera.hpp
	src/Editor/EditorUI.cpp
	src/Editor/EditorUI.hpp
	src/Editor/EditorViews.hpp
	src/Editor/EntityView.cpp
	src/Editor/EntityView.hpp
	src/Editor/FilePickerDialog.cpp
	src/Editor/FilePickerDialog.hpp
	src/Engine/ComponentSystemDefaultImpl.hpp
	src/Engine/Engine.cpp
	src/Engine/Engine.hpp
	src/Engine/InstanceAllocatorPair.hpp
	src/Engine/World.cpp
	src/Engine/World.hpp
	src/Entity/Entity.hpp
	src/Entity/EntityManager.cpp
	src/Entity/EntityManager.hpp
	src/Graphics/BloomEffect.cpp
	src/Graphics/BloomEffect.hpp
	src/Graphics/EnvironmentManager.cpp
	src/Graphics/EnvironmentManager.hpp
	src/Graphics/ParticleSystem.cpp
	src/Graphics/ParticleSystem.hpp
	src/Graphics/ScreenSpaceAmbientOcclusion.cpp
	src/Graphics/ScreenSpaceAmbientOcclusion.hpp
	src/Graphics/TerrainInstance.cpp
	src/Graphics/TerrainInstance.hpp
	src/Graphics/TerrainManager.cpp
	src/Graphics/TerrainManager.hpp
	src/Graphics/TransformUpdateReceiver.hpp
	src/Graphics/Scene.cpp
	src/Graphics/Scene.hpp
	src/Math/BoundingBox.hpp
	src/Math/Frustum.hpp
	src/Math/Intersect3D.cpp
	src/Math/Intersect3D.hpp
	src/Math/Mat2x2.hpp
	src/Math/Mat3x3.hpp
	src/Math/Mat4x4.cpp
	src/Math/Mat4x4.hpp
	src/Math/Math.hpp
	src/Math/Plane.hpp
	src/Math/Projection.hpp
	src/Math/Random.cpp
	src/Math/Random.hpp
	src/Math/Rectangle.hpp
	src/Math/Vec2.hpp
	src/Math/Vec3.hpp
	src/Math/Vec4.hpp
	src/Memory/Memory.cpp
	src/Memory/Memory.hpp
	src/Memory/Allocator.hpp
	src/Memory/AllocatorManager.cpp
	src/Memory/AllocatorManager.hpp
	src/Memory/DefaultAllocator.cpp
	src/Memory/DefaultAllocator.hpp
	src/Memory/ProxyAllocator.cpp
	src/Memory/ProxyAllocator.hpp
	src/Memory/TraceAllocator.cpp
	src/Memory/TraceAllocator.hpp
	src/Rendering/CameraId.hpp
	src/Rendering/CameraParameters.hpp
	src/Rendering/CameraSystem.cpp
	src/Rendering/CameraSystem.hpp
	src/Rendering/CascadedShadowMap.cpp
	src/Rendering/CascadedShadowMap.hpp
	src/Rendering/CustomRenderer.hpp
	src/Rendering/Light.hpp
	src/Rendering/LightManager.cpp
	src/Rendering/LightManager.hpp
	src/Rendering/PostProcessRenderer.cpp
	src/Rendering/PostProcessRenderer.hpp
	src/Rendering/PostProcessRenderPass.hpp
	src/Rendering/RenderCommandData.hpp
	src/Rendering/RenderCommandList.cpp
	src/Rendering/RenderCommandList.hpp
	src/Rendering/RenderCommandType.hpp
	src/Rendering/RenderDevice.hpp
	src/Rendering/RenderDeviceEnums.hpp
	src/Rendering/RenderDeviceOpenGL.cpp
	src/Rendering/RenderDeviceOpenGL.hpp
	src/Rendering/Renderer.cpp
	src/Rendering/Renderer.hpp
	src/Rendering/RendererData.hpp
	src/Rendering/RenderOrder.hpp
	src/Rendering/RenderTargetContainer.cpp
	src/Rendering/RenderTargetContainer.hpp
	src/Rendering/RenderViewport.hpp
	src/Rendering/StaticUniformBuffer.hpp
	src/Rendering/TransparencyType.hpp
	src/Rendering/Uniform.cpp
	src/Rendering/Uniform.hpp
	src/Rendering/UniformBuffer.hpp
	src/Rendering/VertexFormat.cpp
	src/Rendering/VertexFormat.hpp
	src/Resources/BitmapFont.cpp
	src/Resources/BitmapFont.hpp
	src/Resources/ImageData.cpp
	src/Resources/ImageData.hpp
	src/Resources/LevelLoader.cpp
	src/Resources/LevelLoader.hpp
	src/Resources/LevelWriter.cpp
	src/Resources/LevelWriter.hpp
	src/Resources/MaterialManager.cpp
	src/Resources/MaterialManager.hpp
	src/Resources/MaterialData.hpp
	src/Resources/MeshData.hpp
	src/Resources/MeshLoader.hpp
	src/Resources/MeshManager.cpp
	src/Resources/MeshManager.hpp
	src/Resources/MeshPresets.cpp
	src/Resources/MeshPresets.hpp
	src/Resources/ResourceManagers.hpp
	src/Resources/ShaderId.hpp
	src/Resources/ShaderLoader.cpp
	src/Resources/ShaderLoader.hpp
	src/Resources/ShaderManager.cpp
	src/Resources/ShaderManager.hpp
	src/Resources/TextureId.hpp
	src/Resources/TextureManager.cpp
	src/Resources/TextureManager.hpp
	src/Resources/ValueSerialization.cpp
	src/Resources/ValueSerialization.hpp
	src/Resources/YamlCustomTypes.cpp
	src/Resources/YamlCustomTypes.hpp
	src/Scripting/NativeScriptComponent.hpp
	src/Scripting/ScriptContext.hpp
	src/Scripting/ScriptSystem.cpp
	src/Scripting/ScriptSystem.hpp
	src/System/File.cpp
	src/System/File.hpp
	src/System/ImGuiInputView.cpp
	src/System/ImGuiInputView.hpp
	src/System/ImGuiPlatformBackend.cpp
	src/System/ImGuiPlatformBackend.hpp
	src/System/ImGuiRenderBackend.cpp
	src/System/ImGuiRenderBackend.hpp
	src/System/IncludeOpenGL.hpp
	src/System/IncludeGLFW.hpp
	src/System/InputManager.cpp
	src/System/InputManager.hpp
	src/System/InputSource.cpp
	src/System/InputSource.hpp
	src/System/InputView.cpp
	src/System/InputView.hpp
	src/System/KeyCode.hpp
	src/System/Time.cpp
	src/System/Time.hpp
	src/System/Window.cpp
	src/System/Window.hpp
)

source_group(Dependencies\\glad FILES ${GLAD_SOURCES})
source_group(Dependencies\\stb_image FILES ${STB_IMAGE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${KOKKO_SOURCES})

add_library(${KOKKO_LIB} STATIC ${DEPS_SOURCES} ${KOKKO_SOURCES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)

set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/ktx)

set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/yaml-cpp)

find_package(OpenGL REQUIRED)

target_link_libraries(${KOKKO_LIB} ${IMGUI_LIB})
target_link_libraries(${KOKKO_LIB} glfw)
target_link_libraries(${KOKKO_LIB} ktx_read)
target_link_libraries(${KOKKO_LIB} yaml-cpp)
target_link_libraries(${KOKKO_LIB} OpenGL::GL)
