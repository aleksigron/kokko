# Kokko Engine

This is a personal project that lets me test out new ideas and learn some OpenGL
programming in the process. I hope that little by little, this will become a
good, small game engine that might offer some new ways to solve old problems.

For more information on different areas of the engine, look in the docs folder.

![Screenshot](http://aleksigron.com/s/kokko_2017-07-19_22.11.18.jpg)

## Prerequisites
- CMake for building the project
- C++14 compliant compiler
- Blender for exporting mesh files
- Glraw for encoding textures

## Running
Use CMake to generate IDE project files or directly build the project.

### Creating a Visual Studio solution on Windows
```
mkdir build
cd build
cmake -G "Visual Studio 15 2017" ../
```

You can find a list of the available generators in the 
[CMake documentation](https://cmake.org/documentation/). 

### Working directory
You need to set a custom working directory in the IDE when running the project. 

In Visual Studio, go to project properties > _Configuration Properties_ >
_Debugging_ and set _Working Directory_ to the repository root.

In Xcode, choose _Product_ > _Scheme_ > _Edit Scheme_. Select the _Run_ scheme
and go to the _Options_ tab. Enable _Custom working directory_ and set the it
to the repository root.

## Other tools
I'm using a proprietary model export script for Blender. The motivation behind
using a custom format is to better understand the content pipeline. Also, I'm
not aware of a simple format for meshes that is optimized for runtime load
speed. Pretty much all simple formats I could find were text-based and that's
not good for efficiency. I'm hoping to develop the format and export enough to
release it as a separate repository with proper documentation and examples.

[Glad](https://github.com/Dav1dde/glad) is used to load the OpenGL profile and
extensions.

[glraw](https://github.com/cginternals/glraw) is used to process textures to a
runtime-friendly format. Example command to convert image to compressed texture:
```
glraw-cmd --compressed-format GL_COMPRESSED_RGB_S3TC_DXT1_EXT example.jpg
```
