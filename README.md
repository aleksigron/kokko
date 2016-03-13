# Kokko Engine

This is a personal project that lets me test out new ideas and learn some OpenGL
programming in the process. I hope that little by little, this will become a
good, small game engine that might offer some new ways to solve old problems.

This project is currently available only as an Xcode project on OS X, but I
always try to consider cross-platform compatibility when writing new features.

![Screenshot](http://aleksigron.com/s/graphics-toolkit_2015-10-09_18.49.34.png)

## Running
You need to set a custom working directory in Xcode when running the project.
From the menu bar, choose _Product_ > _Scheme_ > _Edit Scheme_. Select the _Run_
scheme and go to the _Options_ tab. Enable _Custom working directory_ and set
the working directory to the repository root.

## Other tools
I'm using a proprietary model export script for Blender. The motivation behind
using a custom format is to better understand the content pipeline. Also, I'm
not aware of a simple format for meshes that is optimized for runtime load
speed. Pretty much all simple formats I could find were text-based and that's
not good for efficiency. I'm hoping to develop the format and export enough to
release it as a separate repository with proper documentation and examples.

[glraw](https://github.com/cginternals/glraw) is used to process textures to a
runtime-friendly format.
