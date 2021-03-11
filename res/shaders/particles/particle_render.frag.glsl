
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec4 color;

void main()
{
	float alpha = smoothstep(0.5, 0.0, length(fs_in.tex_coord - vec2(0.5)));
	color = vec4(1.0, 1.0, 1.0, alpha);
}
