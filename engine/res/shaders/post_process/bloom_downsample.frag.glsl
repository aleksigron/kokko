
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform BloomDownsampleBlock
{
	vec2 texture_scale;
};

void main()
{
	color = sample_box(source_map, texture_scale, fs_in.tex_coord, 1.0);
}
