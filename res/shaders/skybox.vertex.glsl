#version 330 core

layout(location = 0) in vec3 vert_pos;

out vec3 fs_w_direction;

uniform mat4 _VP;

void main()
{
	gl_Position = _VP * vec4(vert_pos, 1.0);
	fs_w_direction = vert_pos;
}
