
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out float color;

uniform sampler2D occlusion_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform SsaoBlurBlock
{
	vec2 texture_scale;
};

void main()
{
	float acc = 0.0;

	for (int x = -2; x < 2; ++x)
		for (int y = -2; y < 2; ++y)
			acc += texture(occlusion_map, vec2(x, y) * texture_scale + fs_in.tex_coord).r;

	color = acc / 16.0;
}
