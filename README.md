# Kokko Engine

This is a personal project that lets me test out new ideas and learn some OpenGL programming in the process. I hope that little by little, this will become a good, small game engine that might offer some new ways to solve old problems.

![Screenshot](https://aleksigron.blob.core.windows.net/public/kokko-20210329.jpg)

## Features

### Graphics
- Deferred renderer
- Physically based rendering with image-based lighting
- HDR rendering pipeline
- Bloom and tonemapping post effects
- Directional, point and spot lights
- Cascaded shadow maps for directional lights
- Screen-space ambient occlusion
- Separated command list build, ordering, dispatch
- Simple data-driven material system

### Resources
- Resource configuration files are JSON for human-readability
  - Scenes
  - Materials
  - Shaders
- Mesh files are using a custom binary format
- Textures are processed to a runtime-friendly format with KTX

### Debugging
- Logging
- Profiling using Chrome tracing output
- Memory statistics
- Vector rendering
- Frametime visualization
- Culling visualization

## Prerequisites
- OpenGL 4.5
- CMake for building the project
- C++14 compliant compiler
- Blender for exporting mesh files
- KTX for encoding textures

### Creating a x64 Visual Studio solution on Windows
```
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ../
```

You can find a list of the available generators in the [CMake documentation](https://cmake.org/documentation/). 

### Working directory
You need to set a custom working directory in the IDE when running the project.

In Visual Studio, go to project properties > _Configuration Properties_ > _Debugging_ and set _Working Directory_ to the repository root or some other directory where you store the resource files.

## Mesh format
I'm using a proprietary model export script for Blender. More information in docs/mesh_file_format.md.

The motivation behind using a custom format is to better understand the content pipeline. Also, I'm not aware of a simple format for meshes that is optimized for runtime load speed. Pretty much all simple formats I could find were text-based and that's not good for efficiency. I'm hoping to develop the format and export enough to release it as a separate repository with proper documentation and examples.

## Tools
[rapidjson](https://github.com/Tencent/rapidjson) is used to read JSON formatted resource files (scenes, shaders, materials, textures).

[GLFW](https://github.com/glfw/glfw) is used to manage OpenGL context, windows and read input.

[Glad](https://github.com/Dav1dde/glad) is used to load the OpenGL profile and extensions.

[KTX](https://github.com/KhronosGroup/KTX-Software) is used to process textures to a runtime-friendly format.

[Gohufont](https://github.com/hchargois/gohufont) is used for debug text rendering.

## Assets
I've stopped storing assets in the code repository. While this makes it harder for people to test out the project, it removes the restrictions on asset sizes and licensing. It will make it easier to develop more advanced features and make sure performance issues don't arise when using more and bigger assets.