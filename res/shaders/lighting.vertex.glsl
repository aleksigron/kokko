#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fs_w_norm;

uniform mat4 _MVP;
uniform mat4 _MV;

void main()
{
	gl_Position = _MVP * vec4(position, 1.0);
	fs_w_norm = (_MV * vec4(normal, 0.0)).xyz;
}
