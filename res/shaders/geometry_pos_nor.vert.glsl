#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fs_w_pos;
out vec3 fs_w_norm;

uniform mat4 _MVP;
uniform mat4 _M;

void main()
{
	gl_Position = _MVP * vec4(position, 1.0);
	fs_w_pos = (_M * vec4(position, 1.0)).xyz;
	fs_w_norm = (_M * vec4(normal, 0.0)).xyz;
}
