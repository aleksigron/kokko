
#version 330 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_nor;

out vec3 fs_v_norm;
out vec2 fs_tex_coord;

uniform mat4 _MVP;
uniform mat4 _MV;

void main()
{
	gl_Position = _MVP * vec4(vert_pos, 1.0);
	fs_v_norm = (_MV * vec4(vert_nor, 0.0)).xyz;
	fs_tex_coord = vert_pos.xy;
}
