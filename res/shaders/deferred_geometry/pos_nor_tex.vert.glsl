#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec3 fs_v_norm;
out vec2 fs_texcoord;

uniform mat4 _MVP;
uniform mat4 _MV;

void main()
{
	gl_Position = _MVP * vec4(position, 1.0);
	fs_v_norm = (_MV * vec4(normal, 0.0)).xyz;
	fs_texcoord = texcoord;
}
