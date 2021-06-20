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
