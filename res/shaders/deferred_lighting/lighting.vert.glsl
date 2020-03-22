layout(location = 0) in vec3 ndc_pos;

out VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
}
vs_out;

void main()
{
	vs_out.tex_coord = ndc_pos.xy * 0.5 + vec2(0.5, 0.5);
	vs_out.eye_dir = vec3((2.0 * un.half_near_plane * vs_out.tex_coord) - un.half_near_plane, -1.0);
	gl_Position = vec4(ndc_pos, 1.0);
}
