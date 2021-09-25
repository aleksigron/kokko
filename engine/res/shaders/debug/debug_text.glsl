#version 450
#property glyph_tex tex2d

#stage vertex
#include "res/shaders/common/constants.glsl"
layout(location = VERTEX_ATTR_INDEX_POS) in vec3 vert_pos;
layout(location = VERTEX_ATTR_INDEX_UV0) in vec2 vert_tex;

out VS_TO_FS
{
	vec2 tex_coord;
}
vs_out;

void main()
{
	gl_Position = vec4(vert_pos, 1.0);
	vs_out.tex_coord = vert_tex;
}

#stage fragment
in VS_TO_FS
{
	vec2 tex_coord;
}
fs_in;

out vec4 color;

uniform sampler2D glyph_tex;

void main()
{
	float val = texture(glyph_tex, fs_in.tex_coord).r;
	float alpha = 0.4 + val * 0.6;

	color = vec4(val, val, val, alpha);
}
