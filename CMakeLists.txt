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

set (KOKKO_SOURCES
	deps/glad/src/glad.c
	src/main.cpp
	src/App.cpp
	src/App.hpp
	src/Array.hpp
	src/AsciiStringUtil.cpp
	src/AsciiStringUtil.hpp
	src/BitfieldVariable.hpp
	src/BitmapFont.cpp
	src/BitmapFont.hpp
	src/BoundingBox.hpp
	src/Buffer.hpp
	src/BufferRef.hpp
	src/Camera.cpp
	src/Camera.hpp
	src/CameraController.cpp
	src/CameraController.hpp
	src/Collection.hpp
	src/Color.hpp
	src/ColorSrgb.hpp
	src/Debug.cpp
	src/Debug.hpp
	src/DebugLog.cpp
	src/DebugLog.hpp
	src/DebugLogView.cpp
	src/DebugLogView.hpp
	src/DebugTextRenderer.cpp
	src/DebugTextRenderer.hpp
	src/DebugVectorRenderer.cpp
	src/DebugVectorRenderer.hpp
	src/Engine.cpp
	src/Engine.hpp
	src/File.cpp
	src/File.hpp
	src/FrustumCulling.cpp
	src/FrustumCulling.hpp
	src/Hash.hpp
	src/ImageData.cpp
	src/ImageData.hpp
	src/ImmutableString.hpp
	src/IncludeOpenGL.hpp
	src/IncludeGLFW.hpp
	src/IndexedContainer.hpp
	src/JsonReader.hpp
	src/KeyboardInput.cpp
	src/KeyboardInput.hpp
	src/Mat2x2.hpp
	src/Mat3x3.hpp
	src/Mat4x4.hpp
	src/Material.cpp
	src/Material.hpp
	src/Math.hpp
	src/MemoryAmount.hpp
	src/Mesh.cpp
	src/Mesh.hpp
	src/MultiArray.hpp
	src/ObjectId.hpp
	src/Plane.hpp
	src/PointerInput.cpp
	src/PointerInput.hpp
	src/Rectangle.hpp
	src/RenderData.hpp
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
	src/Shader.cpp
	src/Shader.hpp
	src/Skybox.cpp
	src/Skybox.hpp
	src/Sort.hpp
	src/StackAllocation.cpp
	src/StackAllocation.hpp
	src/StackAllocator.cpp
	src/StackAllocator.hpp
	src/StringRef.cpp
	src/StringRef.hpp
	src/Texture.cpp
	src/Texture.hpp
	src/Time.cpp
	src/Time.hpp
	src/Transform.hpp
	src/ValueSerialization.hpp
	src/Vec2.hpp
	src/Vec3.hpp
	src/Vec4.hpp
	src/VertexFormat.cpp
	src/VertexFormat.hpp
	src/ViewFrustum.cpp
	src/ViewFrustum.hpp
	src/VirtualMemory.cpp
	src/VirtualMemory.hpp
	src/Window.cpp
	src/Window.hpp
)

add_executable(${EXECUTABLE_NAME} ${KOKKO_SOURCES})

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
