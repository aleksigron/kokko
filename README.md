# Kokko Engine

This is a personal project that lets me test out new ideas and learn some OpenGL
programming in the process. I hope that little by little, this will become a
good, small game engine that might offer some new ways to solve old problems.

![Screenshot](https://aleksigron.blob.core.windows.net/public/kokko-20201118.jpg)

## Prerequisites
- OpenGL 4.4
- CMake for building the project
- C++14 compliant compiler
- Blender for exporting mesh files
- Glraw for encoding textures

### Creating a x64 Visual Studio solution on Windows
```
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ../
```

You can find a list of the available generators in the 
[CMake documentation](https://cmake.org/documentation/). 

### Working directory
You need to set a custom working directory in the IDE when running the project. 

In Visual Studio, go to project properties > _Configuration Properties_ >
_Debugging_ and set _Working Directory_ to the repository root.

## Features

### Graphics
- Deferred renderer
- Separated command list build, ordering, dispatch
- Diffuse, specular and emissive shading
- Directional, point and spot lights
- Cascaded shadow maps for directional lights
- Simple data-driven material system

### Resources
- Resource configuration files are JSON for human-readability
  - Scenes
  - Materials
  - Shaders
  - Texture metadata
- Mesh files are using a custom format

### Debugging
- Logging
- Memory statistics
- Vector rendering
- Frametime visualization
- Culling visualization

### Mesh format
I'm using a proprietary model export script for Blender. The motivation behind
using a custom format is to better understand the content pipeline. Also, I'm
not aware of a simple format for meshes that is optimized for runtime load
speed. Pretty much all simple formats I could find were text-based and that's
not good for efficiency. I'm hoping to develop the format and export enough to
release it as a separate repository with proper documentation and examples. You
can install it by copying the *blender_custom_format* folder to your Blender
installation's *scripts/addons_contrib* folder and enabling it in the add-ons
menu.

## Other tools
[rapidjson](https://github.com/Tencent/rapidjson) is used to read JSON
formatted resource files (scenes, shaders, materials, textures).

[GLFW](https://github.com/glfw/glfw) is used to manage OpenGL context, windows
and read input.

[Glad](https://github.com/Dav1dde/glad) is used to load the OpenGL profile and
extensions.

[glraw](https://github.com/cginternals/glraw) is used to process textures to a
runtime-friendly format. Example command to convert image to compressed texture:
```
glraw-cmd --compressed-format GL_COMPRESSED_RGB_S3TC_DXT1_EXT example.jpg
```

[Gohufont](https://github.com/hchargois/gohufont) is used for debug text
rendering.