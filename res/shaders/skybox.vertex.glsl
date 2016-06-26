#version 330 core

layout(location = 0) in vec3 vert_pos;

out vec3 fs_w_direction;

uniform mat4 _MVP;
uniform mat4 _M;

void main()
{
	gl_Position = _MVP * vec4(vert_pos, 1.0);
	fs_w_direction = (_M * vec4(vert_pos, 0.0)).xyz;
}
