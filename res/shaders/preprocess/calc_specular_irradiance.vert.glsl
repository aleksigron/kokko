layout (location = VERTEX_ATTR_INDEX_POS) in vec3 position;

out VS_TO_FS
{
	vec3 local_pos;
}
vs_out;

void main()
{
    gl_Position = viewport.VP * vec4(position, 1.0);
    vs_out.local_pos = position;
}
