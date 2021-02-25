
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D light_acc_map;

layout(std140, binding = 0) uniform TonemapBlock
{
	float exposure;
};

void main()
{
    vec3 linear = texture(light_acc_map, fs_in.tex_coord).rgb;
    color = vec3(1.0) - exp(-linear * exposure);
}
