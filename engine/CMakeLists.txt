cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
set(PLATFORM_MACOS 1)
endif()

# Define source files

set (CGLTF_SOURCES
	${PROJECT_ROOT}/deps/cgltf/include/cgltf/cgltf.h
	${PROJECT_ROOT}/deps/cgltf/src/cgltf.c
)

set (FMT_SOURCES
	${PROJECT_ROOT}/deps/fmt/src/format.cc
	${PROJECT_ROOT}/deps/fmt/src/os.cc
)

set (GLAD_SOURCES
	${PROJECT_ROOT}/deps/glad/include/glad/glad.h
	${PROJECT_ROOT}/deps/glad/src/glad.c
)

if(${PLATFORM_MACOS})
set (METAL_CPP_SOURCES
    ${PROJECT_ROOT}/deps/metal-cpp/MetalCpp.cpp
)
endif()

set (STB_IMAGE_SOURCES
	${PROJECT_ROOT}/deps/stb_image/include/stb_image/stb_image.h
	${PROJECT_ROOT}/deps/stb_image/src/stb_image.cpp
)

set (XXHASH_SOURCES
	${PROJECT_ROOT}/deps/xxhash/xxhash.c
	${PROJECT_ROOT}/deps/xxhash/xxhash.h
)

set (DEPS_SOURCES
	${CGLTF_SOURCES}
	${FMT_SOURCES}
	${GLAD_SOURCES}
    ${METAL_CPP_SOURCES}
	${STB_IMAGE_SOURCES}
	${XXHASH_SOURCES}
)

set (KOKKO_SOURCES
	src/Core/Array.cpp
	src/Core/Array.hpp
	src/Core/ArrayView.hpp
	src/Core/BitfieldVariable.cpp
	src/Core/BitfieldVariable.hpp
	src/Core/BitPack.hpp
	src/Core/Color.cpp
	src/Core/Color.hpp
	src/Core/ColorSrgb.hpp
	src/Core/Core.hpp
	src/Core/CString.cpp
	src/Core/CString.hpp
	src/Core/EncodingUtf8.cpp
	src/Core/EncodingUtf8.hpp
	src/Core/FixedArray.hpp
	src/Core/Hash.cpp
	src/Core/Hash.hpp
	src/Core/HashMap.cpp
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
	src/Core/StringUtil.cpp
	src/Core/StringUtil.hpp
	src/Core/StringView.cpp
	src/Core/StringView.hpp
	src/Core/TypeTraits.hpp
	src/Core/Uid.cpp
	src/Core/Uid.hpp
	src/Core/UniquePtr.hpp
	src/Debug/Debug.cpp
	src/Debug/Debug.hpp
	src/Debug/DebugCulling.cpp
	src/Debug/DebugCulling.hpp
	src/Debug/DebugGraph.cpp
	src/Debug/DebugGraph.hpp
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
	src/Debug/PerformanceTimer.hpp
	src/Engine/ComponentSerializer.hpp
	src/Engine/ComponentSystemDefaultImpl.hpp
	src/Engine/Engine.cpp
	src/Engine/Engine.hpp
	src/Engine/EngineConstants.cpp
	src/Engine/EngineConstants.hpp
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
	src/Graphics/EnvironmentSerializer.hpp
	src/Graphics/EnvironmentSystem.cpp
	src/Graphics/EnvironmentSystem.hpp
	src/Graphics/GraphicsFeature.hpp
	src/Graphics/GraphicsFeatureBloom.cpp
	src/Graphics/GraphicsFeatureBloom.hpp
	src/Graphics/GraphicsFeatureCommandList.cpp
	src/Graphics/GraphicsFeatureCommandList.hpp
	src/Graphics/GraphicsFeatureDeferredLighting.cpp
	src/Graphics/GraphicsFeatureDeferredLighting.hpp
	src/Graphics/GraphicsFeatureSsao.cpp
	src/Graphics/GraphicsFeatureSsao.hpp
	src/Graphics/GraphicsFeatureSkybox.cpp
	src/Graphics/GraphicsFeatureSkybox.hpp
	src/Graphics/GraphicsFeatureTonemapping.cpp
	src/Graphics/GraphicsFeatureTonemapping.hpp
	src/Graphics/ParticleSystem.cpp
	src/Graphics/ParticleSystem.hpp
	src/Graphics/ParticleEmitterSerializer.hpp
	src/Graphics/TerrainInstance.cpp
	src/Graphics/TerrainInstance.hpp
	src/Graphics/TerrainSerializer.hpp
	src/Graphics/TerrainSystem.cpp
	src/Graphics/TerrainSystem.hpp
	src/Graphics/TerrainQuadTree.cpp
	src/Graphics/TerrainQuadTree.hpp
	src/Graphics/TransformSerializer.hpp
	src/Graphics/TransformUpdateReceiver.hpp
	src/Graphics/Scene.cpp
	src/Graphics/Scene.hpp
	src/Math/AABB.cpp
	src/Math/AABB.hpp
	src/Math/Frustum.hpp
	src/Math/Intersect3D.cpp
	src/Math/Intersect3D.hpp
	src/Math/Mat2x2.hpp
	src/Math/Mat3x3.cpp
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
	src/Memory/Allocator.cpp
	src/Memory/Allocator.hpp
	src/Memory/AllocatorManager.cpp
	src/Memory/AllocatorManager.hpp
	src/Memory/DefaultAllocator.cpp
	src/Memory/DefaultAllocator.hpp
	src/Memory/MetricAllocator.cpp
	src/Memory/MetricAllocator.hpp
	src/Memory/RootAllocator.cpp
	src/Memory/RootAllocator.hpp
	src/Memory/TraceAllocator.cpp
	src/Memory/TraceAllocator.hpp
    src/Platform/Window.hpp
    src/Platform/Window.cpp
	src/Rendering/CameraParameters.hpp
	src/Rendering/CameraSerializer.hpp
	src/Rendering/CameraSystem.cpp
	src/Rendering/CameraSystem.hpp
	src/Rendering/CascadedShadowMap.cpp
	src/Rendering/CascadedShadowMap.hpp
    src/Rendering/CommandBuffer.cpp
    src/Rendering/CommandBuffer.hpp
	src/Rendering/CommandEncoder.cpp
	src/Rendering/CommandEncoder.hpp
    src/Rendering/CommandEncoderDebugScope.cpp
    src/Rendering/CommandEncoderDebugScope.hpp
	src/Rendering/CommandExecutor.cpp
	src/Rendering/CommandExecutor.hpp
	src/Rendering/CommandExecutorOpenGL.cpp
	src/Rendering/CommandExecutorOpenGL.hpp
	src/Rendering/Framebuffer.cpp
	src/Rendering/Framebuffer.hpp
	src/Rendering/Light.hpp
	src/Rendering/LightManager.cpp
	src/Rendering/LightManager.hpp
	src/Rendering/LightSerializer.hpp
	src/Rendering/MeshComponentSerializer.hpp
	src/Rendering/MeshComponentSystem.cpp
	src/Rendering/MeshComponentSystem.hpp
	src/Rendering/PostProcessRenderer.cpp
	src/Rendering/PostProcessRenderer.hpp
	src/Rendering/PostProcessRenderPass.hpp
	src/Rendering/RenderCommand.cpp
	src/Rendering/RenderCommand.hpp
	src/Rendering/RenderCommandBuffer.cpp
	src/Rendering/RenderCommandBuffer.hpp
	src/Rendering/RenderDebugFeature.hpp
	src/Rendering/RenderDebugSettings.cpp
	src/Rendering/RenderDebugSettings.hpp
    src/Rendering/RenderDevice.cpp
	src/Rendering/RenderDevice.hpp
    src/Rendering/RenderDeviceDebugScope.cpp
    src/Rendering/RenderDeviceDebugScope.hpp
    src/Rendering/RenderDeviceEnumsOpenGL.cpp
	src/Rendering/RenderDeviceEnumsOpenGL.hpp
	src/Rendering/Renderer.cpp
	src/Rendering/Renderer.hpp
	src/Rendering/RendererCommandList.cpp
	src/Rendering/RendererCommandList.hpp
	src/Rendering/RenderGraphResources.cpp
	src/Rendering/RenderGraphResources.hpp
	src/Rendering/RenderOrder.hpp
    src/Rendering/RenderPass.hpp
    src/Rendering/RenderPass.cpp
    src/Rendering/RenderPassDescriptor.hpp
	src/Rendering/RenderPassType.hpp
	src/Rendering/RenderResourceId.cpp
	src/Rendering/RenderResourceId.hpp
	src/Rendering/RenderTargetContainer.cpp
	src/Rendering/RenderTargetContainer.hpp
    src/Rendering/RenderTypes.cpp
    src/Rendering/RenderTypes.hpp
	src/Rendering/RenderViewport.hpp
	src/Rendering/StaticUniformBuffer.hpp
	src/Rendering/TransparencyType.hpp
	src/Rendering/Uniform.cpp
	src/Rendering/Uniform.hpp
	src/Rendering/UniformBuffer.hpp
	src/Rendering/UniformData.cpp
	src/Rendering/UniformData.hpp
	src/Rendering/UniformList.cpp
	src/Rendering/UniformList.hpp
	src/Rendering/VertexFormat.cpp
	src/Rendering/VertexFormat.hpp
	src/Resources/AssetLibrary.cpp
	src/Resources/AssetLibrary.hpp
	src/Resources/AssetLoader.hpp
	src/Resources/AssetType.hpp
	src/Resources/BitmapFont.cpp
	src/Resources/BitmapFont.hpp
	src/Resources/ImageData.cpp
	src/Resources/ImageData.hpp
	src/Resources/LevelSerializer.cpp
	src/Resources/LevelSerializer.hpp
	src/Resources/MaterialData.hpp
	src/Resources/MaterialManager.cpp
	src/Resources/MaterialManager.hpp
	src/Resources/MaterialSerializer.cpp
	src/Resources/MaterialSerializer.hpp
	src/Resources/MeshId.cpp
	src/Resources/MeshId.hpp
	src/Resources/MeshPresets.cpp
	src/Resources/MeshPresets.hpp
	src/Resources/ModelLoader.cpp
	src/Resources/ModelLoader.hpp
	src/Resources/ModelManager.cpp
	src/Resources/ModelManager.hpp
	src/Resources/ResourceManagers.hpp
	src/Resources/ShaderId.hpp
	src/Resources/ShaderLoader.cpp
	src/Resources/ShaderLoader.hpp
	src/Resources/ShaderManager.cpp
	src/Resources/ShaderManager.hpp
	src/Resources/TextureId.hpp
	src/Resources/TextureManager.cpp
	src/Resources/TextureManager.hpp
	src/Resources/UniformSerialization.cpp
	src/Resources/UniformSerialization.hpp
	src/Resources/YamlCustomTypes.cpp
	src/Resources/YamlCustomTypes.hpp
	src/Scripting/NativeScriptComponent.hpp
	src/Scripting/ScriptContext.hpp
	src/Scripting/ScriptSystem.cpp
	src/Scripting/ScriptSystem.hpp
	src/System/DoctestImpl.cpp
	src/System/EnablePerformanceGPU.cpp
	src/System/Filesystem.cpp
	src/System/Filesystem.hpp
	src/System/FilesystemResolver.hpp
	src/System/FilesystemResolverVirtual.cpp
	src/System/FilesystemResolverVirtual.hpp
	src/System/IncludeOpenGL.hpp
	src/System/IncludeGLFW.hpp
	src/System/InputManager.cpp
	src/System/InputManager.hpp
	src/System/InputSource.cpp
	src/System/InputSource.hpp
	src/System/InputView.cpp
	src/System/InputView.hpp
	src/System/KeyCode.hpp
	src/System/Log.cpp
	src/System/Log.hpp
	src/System/Logger.cpp
	src/System/Logger.hpp
	src/System/LogLevel.hpp
	src/System/Time.cpp
	src/System/Time.hpp
	src/System/WindowManager.cpp
	src/System/WindowManager.hpp
	src/System/WindowSettings.hpp
)

if (${PLATFORM_MACOS})
# Metal
list (APPEND KOKKO_SOURCES
    src/Platform/WindowMetal.hpp
    src/Platform/WindowMetal.mm
    src/Rendering/Metal/CommandBufferMetal.cpp
    src/Rendering/Metal/CommandBufferMetal.hpp
    src/Rendering/Metal/ConversionMetal.cpp
    src/Rendering/Metal/ConversionMetal.hpp
    src/Rendering/Metal/RenderDeviceMetal.hpp
    src/Rendering/Metal/RenderDeviceMetal.cpp
    src/Rendering/Metal/RenderPassMetal.hpp
    src/Rendering/Metal/RenderPassMetal.cpp
)
else()
# OpenGL
list (APPEND KOKKO_SOURCES
    src/Platform/WindowOpenGL.hpp
    src/Platform/WindowOpenGL.cpp
    src/Rendering/RenderDeviceOpenGL.cpp
    src/Rendering/RenderDeviceOpenGL.hpp
)
endif()

# Source groups for IDEs

source_group(Dependencies\\cgltf FILES ${CGLTF_SOURCES})
source_group(Dependencies\\fmt FILES ${FMT_SOURCES})
source_group(Dependencies\\glad FILES ${GLAD_SOURCES})
source_group(Dependencies\\stb_image FILES ${STB_IMAGE_SOURCES})
source_group(Dependencies\\xxhash FILES ${XXHASH_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${KOKKO_SOURCES})

# Create kokko library

add_library(${KOKKO_LIB} OBJECT ${DEPS_SOURCES} ${KOKKO_SOURCES})
set_target_properties(${KOKKO_LIB} PROPERTIES FOLDER "kokko")

# Add subdirectories

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_ROOT}/deps/glfw glfw-build)

set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_ROOT}/deps/rapidyaml rapidyaml-build)

# Set kokko library include directories and link libraries

target_include_directories(${KOKKO_LIB} PUBLIC
	src
	${PROJECT_ROOT}/deps/doctest
	${PROJECT_ROOT}/deps/cgltf/include
	${PROJECT_ROOT}/deps/fmt/include
	${PROJECT_ROOT}/deps/glad/include
	${PROJECT_ROOT}/deps/glfw/include
    ${PROJECT_ROOT}/deps/metal-cpp
	${PROJECT_ROOT}/deps/rapidjson/include
	${PROJECT_ROOT}/deps/rapidyaml/ext/c4core/src
	${PROJECT_ROOT}/deps/rapidyaml/src
	${PROJECT_ROOT}/deps/stb_image/include
	${PROJECT_ROOT}/deps/xxhash
	${PROJECT_ROOT}/deps/yaml-cpp/include
)

target_link_libraries(${KOKKO_LIB}
    PUBLIC glfw
    PUBLIC ryml::ryml
)

if(${PLATFORM_MACOS})
target_link_libraries(${KOKKO_LIB}
    INTERFACE "-framework Foundation"
    INTERFACE "-framework QuartzCore"
    INTERFACE "-framework Metal"
)
target_compile_definitions(${KOKKO_LIB} PUBLIC KOKKO_USE_METAL)
else()
find_package(OpenGL REQUIRED)
target_link_libraries(${KOKKO_LIB} PUBLIC OpenGL::GL)
target_compile_definitions(${KOKKO_LIB} PUBLIC KOKKO_USE_OPENGL)
endif()
