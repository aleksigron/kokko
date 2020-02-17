#version 330 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec2 vert_tex;

out vec2 fs_tex_coord;

void main()
{
	gl_Position = vec4(vert_pos, 1.0);
	fs_tex_coord = vert_tex;
}
