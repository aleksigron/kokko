#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fragColor;

uniform mat4 _MVP;
uniform vec3 base_color;

void main()
{
	gl_Position = _MVP * vec4(position, 1);
	fragColor = base_color;
}
