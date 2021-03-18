
in VS_TO_FS
{
	vec2 tex_coord;
	float age;
} fs_in;

out vec4 color;

void main()
{
	float age_alpha = 1.0 - (fs_in.age * fs_in.age * fs_in.age);
	float pos_alpha = smoothstep(0.5, 0.0, length(fs_in.tex_coord - vec2(0.5)));
	color = vec4(1.5, 1.5, 1.5, pos_alpha * age_alpha);
}
