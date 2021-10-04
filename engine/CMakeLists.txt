cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project("Kokko Engine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(KOKKO_LIB kokko)

set(ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

include_directories(
	src
	${ROOT_PATH}/deps/doctest
	${ROOT_PATH}/deps/fmt/include
	${ROOT_PATH}/deps/glad/include
	${ROOT_PATH}/deps/glfw/include
	${ROOT_PATH}/deps/rapidjson/include
	${ROOT_PATH}/deps/stb_image/include
	${ROOT_PATH}/deps/yaml-cpp/include
)

set (FMT_SOURCES
	${ROOT_PATH}/deps/fmt/src/format.cc
	${ROOT_PATH}/deps/fmt/src/os.cc
)

set (GLAD_SOURCES
	${ROOT_PATH}/deps/glad/include/glad/glad.h
	${ROOT_PATH}/deps/glad/src/glad.c
)

set (STB_IMAGE_SOURCES
	${ROOT_PATH}/deps/stb_image/include/stb_image/stb_image.h
	${ROOT_PATH}/deps/stb_image/src/stb_image.cpp
)

set (DEPS_SOURCES
	${FMT_SOURCES}
	${GLAD_SOURCES}
	${STB_IMAGE_SOURCES}
)

set (KOKKO_SOURCES
	src/Core/Array.cpp
	src/Core/Array.hpp
	src/Core/ArrayView.hpp
	src/Core/BitfieldVariable.hpp
	src/Core/BitPack.hpp
	src/Core/Color.hpp
	src/Core/ColorSrgb.hpp
	src/Core/Core.hpp
	src/Core/CString.cpp
	src/Core/CString.hpp
	src/Core/EncodingUtf8.cpp
	src/Core/EncodingUtf8.hpp
	src/Core/FixedArray.hpp
	src/Core/Hash.hpp
	src/Core/HashMap.hpp
	src/Core/Optional.hpp
	src/Core/Pair.hpp
	src/Core/Queue.cpp
	src/Core/Queue.hpp
	src/Core/Range.hpp
	src/Core/Sort.hpp
	src/Core/SortedArray.cpp
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
	src/Debug/InstrumentationTimer.cpp
	src/Debug/InstrumentationTimer.hpp
	src/Debug/Log.cpp
	src/Debug/Log.hpp
	src/Debug/LogLevel.hpp
	src/Debug/PerformanceTimer.hpp
	src/Engine/ComponentSerializer.hpp
	src/Engine/ComponentSystemDefaultImpl.hpp
	src/Engine/Engine.cpp
	src/Engine/Engine.hpp
	src/Engine/EngineSettings.hpp
	src/Engine/Entity.hpp
	src/Engine/EntityFactory.cpp
	src/Engine/EntityFactory.hpp
	src/Engine/EntityManager.cpp
	src/Engine/EntityManager.hpp
	src/Engine/InstanceAllocatorPair.hpp
	src/Engine/Job.hpp
	src/Engine/JobAllocator.cpp
	src/Engine/JobAllocator.hpp
	src/Engine/JobFunction.hpp
	src/Engine/JobHelpers.hpp
	src/Engine/JobQueue.cpp
	src/Engine/JobQueue.hpp
	src/Engine/JobSystem.cpp
	src/Engine/JobSystem.hpp
	src/Engine/JobWorker.cpp
	src/Engine/JobWorker.hpp
	src/Engine/World.cpp
	src/Engine/World.hpp
	src/Graphics/BloomEffect.cpp
	src/Graphics/BloomEffect.hpp
	src/Graphics/EnvironmentManager.cpp
	src/Graphics/EnvironmentManager.hpp
	src/Graphics/ParticleSystem.cpp
	src/Graphics/ParticleSystem.hpp
	src/Graphics/ParticleEmitterSerializer.hpp
	src/Graphics/ScreenSpaceAmbientOcclusion.cpp
	src/Graphics/ScreenSpaceAmbientOcclusion.hpp
	src/Graphics/TerrainInstance.cpp
	src/Graphics/TerrainInstance.hpp
	src/Graphics/TerrainSystem.cpp
	src/Graphics/TerrainSystem.hpp
	src/Graphics/TransformSerializer.hpp
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
	src/Math/Math.cpp
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
	src/Memory/Allocator.cpp
	src/Memory/Allocator.hpp
	src/Memory/AllocatorManager.cpp
	src/Memory/AllocatorManager.hpp
	src/Memory/DefaultAllocator.cpp
	src/Memory/DefaultAllocator.hpp
	src/Memory/MetricAllocator.cpp
	src/Memory/MetricAllocator.hpp
	src/Memory/TraceAllocator.cpp
	src/Memory/TraceAllocator.hpp
	src/Rendering/CameraParameters.hpp
	src/Rendering/CameraSerializer.hpp
	src/Rendering/CameraSystem.cpp
	src/Rendering/CameraSystem.hpp
	src/Rendering/CascadedShadowMap.cpp
	src/Rendering/CascadedShadowMap.hpp
	src/Rendering/CustomRenderer.hpp
	src/Rendering/Framebuffer.cpp
	src/Rendering/Framebuffer.hpp
	src/Rendering/Light.hpp
	src/Rendering/LightManager.cpp
	src/Rendering/LightManager.hpp
	src/Rendering/LightSerializer.hpp
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
	src/Rendering/RenderObjectSerializer.hpp
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
	src/Resources/LevelSerializer.cpp
	src/Resources/LevelSerializer.hpp
	src/Resources/MaterialManager.cpp
	src/Resources/MaterialManager.hpp
	src/Resources/MaterialData.hpp
	src/Resources/MeshData.hpp
	src/Resources/MeshLoader.cpp
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
	src/System/DoctestImpl.cpp
	src/System/Filesystem.hpp
	src/System/FilesystemDefault.cpp
	src/System/FilesystemDefault.hpp
	src/System/FilesystemRelative.cpp
	src/System/FilesystemRelative.hpp
	src/System/FilesystemVirtual.cpp
	src/System/FilesystemVirtual.hpp
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

source_group(Dependencies\\fmt FILES ${FMT_SOURCES})
source_group(Dependencies\\glad FILES ${GLAD_SOURCES})
source_group(Dependencies\\stb_image FILES ${STB_IMAGE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${KOKKO_SOURCES})

add_library(${KOKKO_LIB} OBJECT ${DEPS_SOURCES} ${KOKKO_SOURCES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${ROOT_PATH}/deps/glfw glfw-build)

set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(${ROOT_PATH}/deps/yaml-cpp yaml-cpp-build)

find_package(OpenGL REQUIRED)

target_link_libraries(${KOKKO_LIB} glfw)
target_link_libraries(${KOKKO_LIB} yaml-cpp)
target_link_libraries(${KOKKO_LIB} OpenGL::GL)