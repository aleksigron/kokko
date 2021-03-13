cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project("Kokko Engine")

# Use C++14 standard
set(CMAKE_CXX_STANDARD 14)

# Don't use language extensions
set(CMAKE_CXX_EXTENSIONS OFF)

set (EXECUTABLE_NAME kokko)

include_directories(
	src
	include
	deps/glfw/include
	deps/glad/include
)

set (DEPS_SOURCES
	deps/glad/src/glad.c
)

set (KOKKO_SOURCES
	src/Application/main.cpp
	src/Application/App.cpp
	src/Application/App.hpp
	src/Application/AppSettings.cpp
	src/Application/AppSettings.hpp
	src/Application/CameraController.cpp
	src/Application/CameraController.hpp
	src/Core/Array.hpp
	src/Core/BitfieldVariable.hpp
	src/Core/BitPack.hpp
	src/Core/Buffer.hpp
	src/Core/BufferRef.hpp
	src/Core/Color.hpp
	src/Core/ColorSrgb.hpp
	src/Core/EncodingUtf8.cpp
	src/Core/EncodingUtf8.hpp
	src/Core/Hash.hpp
	src/Core/HashMap.hpp
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
	src/Debug/LogHelper.cpp
	src/Debug/LogHelper.hpp
	src/Debug/LogLevel.hpp
	src/Debug/PerformanceTimer.hpp
	src/Engine/Engine.cpp
	src/Engine/Engine.hpp
	src/Entity/Entity.hpp
	src/Entity/EntityManager.hpp
	src/Graphics/BloomEffect.cpp
	src/Graphics/BloomEffect.hpp
	src/Graphics/ParticleSystem.cpp
	src/Graphics/ParticleSystem.hpp
	src/Graphics/ScreenSpaceAmbientOcclusion.cpp
	src/Graphics/ScreenSpaceAmbientOcclusion.hpp
	src/Graphics/TerrainInstance.cpp
	src/Graphics/TerrainInstance.hpp
	src/Graphics/TerrainManager.cpp
	src/Graphics/TerrainManager.hpp
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
	src/Rendering/Camera.hpp
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
	src/Resources/MaterialManager.cpp
	src/Resources/MaterialManager.hpp
	src/Resources/MaterialData.hpp
	src/Resources/MeshData.hpp
	src/Resources/MeshLoader.hpp
	src/Resources/MeshManager.cpp
	src/Resources/MeshManager.hpp
	src/Resources/MeshPresets.cpp
	src/Resources/MeshPresets.hpp
	src/Resources/ShaderId.hpp
	src/Resources/ShaderLoader.cpp
	src/Resources/ShaderLoader.hpp
	src/Resources/ShaderManager.cpp
	src/Resources/ShaderManager.hpp
	src/Resources/TextureManager.cpp
	src/Resources/TextureManager.hpp
	src/Resources/ValueSerialization.cpp
	src/Resources/ValueSerialization.hpp
	src/Scene/ITransformUpdateReceiver.hpp
	src/Scene/Scene.cpp
	src/Scene/Scene.hpp
	src/Scene/SceneLayer.hpp
	src/Scene/SceneLoader.cpp
	src/Scene/SceneLoader.hpp
	src/Scene/SceneManager.cpp
	src/Scene/SceneManager.hpp
	src/System/File.cpp
	src/System/File.hpp
	src/System/IncludeOpenGL.hpp
	src/System/IncludeGLFW.hpp
	src/System/InputManager.cpp
	src/System/InputManager.hpp
	src/System/KeyboardInput.cpp
	src/System/KeyboardInput.hpp
	src/System/KeyboardInputView.cpp
	src/System/KeyboardInputView.hpp
	src/System/PointerInput.cpp
	src/System/PointerInput.hpp
	src/System/TextInputHandler.hpp
	src/System/TextInput.cpp
	src/System/TextInput.hpp
	src/System/Time.cpp
	src/System/Time.hpp
	src/System/Window.cpp
	src/System/Window.hpp
)

source_group(Dependencies FILES ${DEPS_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${KOKKO_SOURCES})

add_executable(${EXECUTABLE_NAME} ${DEPS_SOURCES} ${KOKKO_SOURCES})

# Build GLFW with the project

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)

set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/ktx)

find_package(OpenGL REQUIRED)

target_link_libraries(${EXECUTABLE_NAME} glfw)
target_link_libraries(${EXECUTABLE_NAME} ktx_read)
target_link_libraries(${EXECUTABLE_NAME} OpenGL::GL)
