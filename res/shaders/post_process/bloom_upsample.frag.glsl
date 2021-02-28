
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = 0) uniform BloomUpsampleBlock
{
	vec2 texture_scale;
};

void main()
{
	color = texture(source_map, fs_in.tex_coord).rgb;
}
