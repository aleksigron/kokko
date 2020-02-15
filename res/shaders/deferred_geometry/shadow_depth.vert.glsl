#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 _MVP;

void main()
{
 	gl_Position = _MVP * vec4(position, 1.0);
}
