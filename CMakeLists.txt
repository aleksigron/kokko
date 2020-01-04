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
	src/Array.hpp
	src/BitfieldVariable.hpp
	src/BitmapFont.cpp
	src/BitmapFont.hpp
	src/BitPack.hpp
	src/BoundingBox.hpp
	src/Buffer.hpp
	src/BufferRef.hpp
	src/Camera.hpp
	src/CameraController.cpp
	src/CameraController.hpp
	src/CascadedShadowMap.cpp
	src/CascadedShadowMap.hpp
	src/Collection.hpp
	src/Color.hpp
	src/ColorSrgb.hpp
	src/Debug.cpp
	src/Debug.hpp
	src/DebugConsole.cpp
	src/DebugConsole.hpp
	src/DebugCulling.cpp
	src/DebugCulling.hpp
	src/DebugGraph.cpp
	src/DebugGraph.hpp
	src/DebugLog.cpp
	src/DebugLog.hpp
	src/DebugTextRenderer.cpp
	src/DebugTextRenderer.hpp
	src/DebugVectorRenderer.cpp
	src/DebugVectorRenderer.hpp
	src/EncodingUtf8.cpp
	src/EncodingUtf8.hpp
	src/Engine.cpp
	src/Engine.hpp
	src/Entity.hpp
	src/EntityManager.hpp
	src/File.cpp
	src/File.hpp
	src/Hash.hpp
	src/HashMap.hpp
	src/ImageData.cpp
	src/ImageData.hpp
	src/ImmutableString.cpp
	src/ImmutableString.hpp
	src/IncludeOpenGL.hpp
	src/IncludeGLFW.hpp
	src/IndexedContainer.hpp
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
	src/Light.hpp
	src/LightManager.cpp
	src/LightManager.hpp
	src/Mat2x2.hpp
	src/Mat3x3.hpp
	src/Mat4x4.hpp
	src/MaterialManager.cpp
	src/MaterialManager.hpp
	src/MaterialData.hpp
	src/Math.hpp
	src/MeshData.hpp
	src/MeshLoader.hpp
	src/MeshManager.cpp
	src/MeshManager.hpp
	src/MultiArray.hpp
	src/Plane.hpp
	src/PointerInput.cpp
	src/PointerInput.hpp
	src/Projection.hpp
	src/Queue.hpp
	src/Rectangle.hpp
	src/RenderCommandData.hpp
	src/RenderCommandList.cpp
	src/RenderCommandList.hpp
	src/RenderCommandType.hpp
	src/Renderer.cpp
	src/Renderer.hpp
	src/RenderOrder.hpp
	src/RenderPipeline.cpp
	src/RenderPipeline.hpp
	src/ResourceManager.cpp
	src/ResourceManager.hpp
	src/Scene.cpp
	src/Scene.hpp
	src/SceneLayer.hpp
	src/SceneLoader.cpp
	src/SceneLoader.hpp
	src/SceneManager.cpp
	src/SceneManager.hpp
	src/Shader.cpp
	src/Shader.hpp
	src/Skybox.cpp
	src/Skybox.hpp
	src/Sort.hpp
	src/SortedArray.hpp
	src/String.cpp
	src/String.hpp
	src/StringRef.cpp
	src/StringRef.hpp
	src/TextInputHandler.hpp
	src/TextInput.cpp
	src/TextInput.hpp
	src/Texture.cpp
	src/Texture.hpp
	src/Time.cpp
	src/Time.hpp
	src/TransparencyType.hpp
	src/ValueSerialization.cpp
	src/ValueSerialization.hpp
	src/Vec2.hpp
	src/Vec3.hpp
	src/Vec4.hpp
	src/VertexFormat.cpp
	src/VertexFormat.hpp
	src/ViewFrustum.hpp
	src/Window.cpp
	src/Window.hpp
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
