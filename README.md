# Graphics Toolkit

This is a personal project that lets me test out new ideas and learn some OpenGL
programming in the process. I hope that little by little, this will become a
good, small game engine that might offer some new ways to solve old problems.

This project is currently available only as an Xcode project on OS X, but I
always try to consider cross-platform compatibility when writing new features.

![Screenshot](http://aleksigron.com/s/graphics-toolkit_2015-09-04_21.16.44.png)

## Running
You need to set a custom working directory in XCode when running the project.
From the menu bar, choose _Product_ > _Scheme_ > _Edit Scheme_. Select the _Run_
scheme and go to the _Options_ tab. Enable _Custom working directory_ and set
the working directory to the GraphicsToolkit folder next to the XCode project.

## Other tools
I'm currently using [glraw](https://github.com/cginternals/glraw) to process
textures to a runtime-friendly format.
