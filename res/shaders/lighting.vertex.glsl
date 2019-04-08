#version 330 core

layout(location = 0) in vec3 ndc_pos;

out vec2 tex_coord;

void main()
{
	gl_Position = vec4(ndc_pos, 1.0);
	tex_coord = ndc_pos.xy * 0.5 + vec2(0.5, 0.5);
}
