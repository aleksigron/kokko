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
	src/main.cpp
	src/App.cpp
	src/App.hpp
	src/AppSettings.cpp
	src/AppSettings.hpp
	src/BitfieldVariable.hpp
	src/BitmapFont.cpp
	src/BitmapFont.hpp
	src/BitPack.hpp
	src/CameraController.cpp
	src/CameraController.hpp
	src/Color.hpp
	src/ColorSrgb.hpp
	src/EncodingUtf8.cpp
	src/EncodingUtf8.hpp
	src/Engine.cpp
	src/Engine.hpp
	src/Entity.hpp
	src/EntityManager.hpp
	src/File.cpp
	src/File.hpp
	src/ImageData.cpp
	src/ImageData.hpp
	src/IncludeOpenGL.hpp
	src/IncludeGLFW.hpp
	src/InputManager.cpp
	src/InputManager.hpp
	src/Intersect3D.cpp
	src/Intersect3D.hpp
	src/ITransformUpdateReceiver.hpp
	src/JsonReader.hpp
	src/KeyboardInput.cpp
	src/KeyboardInput.hpp
	src/KeyboardInputView.cpp
	src/KeyboardInputView.hpp
	src/MaterialManager.cpp
	src/MaterialManager.hpp
	src/MaterialData.hpp
	src/MeshData.hpp
	src/MeshLoader.hpp
	src/MeshManager.cpp
	src/MeshManager.hpp
	src/PointerInput.cpp
	src/PointerInput.hpp
	src/ResourceManager.cpp
	src/ResourceManager.hpp
	src/Scene.cpp
	src/Scene.hpp
	src/SceneLayer.hpp
	src/SceneLoader.cpp
	src/SceneLoader.hpp
	src/SceneManager.cpp
	src/SceneManager.hpp
	src/Skybox.cpp
	src/Skybox.hpp
	src/TextInputHandler.hpp
	src/TextInput.cpp
	src/TextInput.hpp
	src/Texture.cpp
	src/Texture.hpp
	src/Time.cpp
	src/Time.hpp
	src/ValueSerialization.cpp
	src/ValueSerialization.hpp
	src/Window.cpp
	src/Window.hpp
	src/Core/Array.hpp
	src/Core/Buffer.hpp
	src/Core/BufferRef.hpp
	src/Core/Collection.hpp
	src/Core/Hash.hpp
	src/Core/HashMap.hpp
	src/Core/ImmutableString.cpp
	src/Core/ImmutableString.hpp
	src/Core/IndexedContainer.hpp
	src/Core/MultiArray.hpp
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
	src/Debug/DebugVectorRenderer.cpp
	src/Debug/DebugVectorRenderer.hpp
	src/Math/BoundingBox.hpp
	src/Math/Frustum.hpp
	src/Math/Mat2x2.hpp
	src/Math/Mat3x3.hpp
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
	src/Memory/StackAllocation.cpp
	src/Memory/StackAllocation.hpp
	src/Memory/StackAllocator.cpp
	src/Memory/StackAllocator.hpp
	src/Memory/VirtualMemory.cpp
	src/Memory/VirtualMemory.hpp
	src/Rendering/Camera.hpp
	src/Rendering/CascadedShadowMap.cpp
	src/Rendering/CascadedShadowMap.hpp
	src/Rendering/Light.hpp
	src/Rendering/LightManager.cpp
	src/Rendering/LightManager.hpp
	src/Rendering/RenderCommandData.hpp
	src/Rendering/RenderCommandList.cpp
	src/Rendering/RenderCommandList.hpp
	src/Rendering/RenderCommandType.hpp
	src/Rendering/Renderer.cpp
	src/Rendering/Renderer.hpp
	src/Rendering/RenderOrder.hpp
	src/Rendering/RenderPipeline.cpp
	src/Rendering/RenderPipeline.hpp
	src/Rendering/Shader.cpp
	src/Rendering/Shader.hpp
	src/Rendering/TransparencyType.hpp
	src/Rendering/VertexFormat.cpp
	src/Rendering/VertexFormat.hpp
)

source_group(Dependencies FILES ${DEPS_SOURCES})
source_group(TREE ../src FILES ${KOKKO_SOURCES})

add_executable(${EXECUTABLE_NAME} ${DEPS_SOURCES} ${KOKKO_SOURCES})

# Build GLFW with the project

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)

find_package(OpenGL REQUIRED)

# message(${OPENGL_INCLUDE_DIR})

# target_include_directories(${EXECUTABLE_NAME} ${OPENGL_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} glfw)
target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_gl_LIBRARY})
