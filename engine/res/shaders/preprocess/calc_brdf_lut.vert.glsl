layout (location = VERTEX_ATTR_INDEX_POS) in vec3 position;

out VS_TO_FS
{
	vec2 tex_coord;
}
vs_out;

void main()
{
	vs_out.tex_coord = position.xy * 0.5 + vec2(0.5, 0.5);
	gl_Position = vec4(position, 1.0);
}
