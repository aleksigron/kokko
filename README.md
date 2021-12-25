# Kokko Engine

![Build status badge](https://github.com/aleksigron/kokko/actions/workflows/build.yml/badge.svg)

A simple cross-platform game engine using OpenGL. I hope that little by little, this can become a good, small game engine that might offer some new ways to solve old problems.

![Screenshot](https://aleksigron.blob.core.windows.net/public/kokko-20210925.jpg)

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

### Engine
- Entity component system (ECS)
- Editor UI implemented using Dear ImGui

### Resources
- Asset files are JSON or YAML for readability and mergeability
  - Scenes
  - Materials
- Mesh files are using a custom binary format

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
- C++17 compliant compiler
- Blender for exporting mesh files

## Getting started

### Create a Visual Studio solution
```
git submodule update --init --recursive
mkdir build
cmake -B build -G "Visual Studio 16 2019" -A x64
```

You need to have the repository root as the working directory when running the project.

The editor interface and workflow are in a very early state and are currently being worked on. There are known issues when it comes to creating projects and content files. Feel free to report issues on GitHub.

## Mesh format
I'm using a proprietary model export script for Blender. More information in docs/mesh_file_format.md. The motivation behind using a custom format is to better understand the content pipeline. At the time of making this decision, there was no widely used efficient format. I'm considering implementing support for glTF as a content format, as it is quite fit for the purpose.

## Tools
[GLFW](https://github.com/glfw/glfw) is used to manage OpenGL context, windows and read input.

[Glad](https://github.com/Dav1dde/glad) is used to load the OpenGL profile and extensions.

[Dear ImGui](https://github.com/ocornut/imgui) is used to draw the editor UI.

[rapidjson](https://github.com/Tencent/rapidjson) is used to read JSON formatted asset files.

[yaml-cpp](https://github.com/jbeder/yaml-cpp) is used to read and write YAML formatted asset files.

[fmt](https://github.com/fmtlib/fmt) is used for logging formatting.

[xxHash](https://github.com/Cyan4973/xxHash) is used for content hashing.

[Gohufont](https://github.com/hchargois/gohufont) is used for debug text rendering.
