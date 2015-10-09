#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

out vec3 vert_color;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(position, 1);
	vec4 w_normal = MVP * vec4(normal, 0);
	float d = dot(w_normal.xyz, vec3(0.0, 1.0, 0.0));
	vert_color = color * d;
}
