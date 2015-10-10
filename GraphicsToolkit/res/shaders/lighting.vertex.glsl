#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

out vec3 fs_w_norm;
out vec3 fs_vert_color;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(position, 1);
	fs_w_norm = (MVP * vec4(normal, 0)).xyz;
	fs_vert_color = color;
}
