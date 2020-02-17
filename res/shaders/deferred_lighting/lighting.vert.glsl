#version 330 core

layout(location = 0) in vec3 ndc_pos;

uniform vec2 half_near_plane;

out vec2 tex_coord;
out vec3 eye_dir;

void main()
{
	tex_coord = ndc_pos.xy * 0.5 + vec2(0.5, 0.5);
	eye_dir = vec3((2.0 * half_near_plane * tex_coord) - half_near_plane, -1.0);
	gl_Position = vec4(ndc_pos, 1.0);
}
