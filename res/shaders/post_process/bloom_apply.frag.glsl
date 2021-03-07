
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform BloomApplyBlock
{
    float kernel[25];
	vec2 texture_scale;
    int kernel_extent;
	float intensity;
};

void main()
{
    int kernel_width = 2 * kernel_extent + 1;
    vec3 sum = vec3(0.0);
    for (int y = -kernel_extent; y <= kernel_extent; ++y)
    {
        for (int x = -kernel_extent; x <= kernel_extent; ++x)
        {
            int index = (y + kernel_extent) * kernel_width + (x + kernel_extent);
            vec2 coordinates = fs_in.tex_coord + vec2(x, y) * texture_scale;
            sum += texture(source_map, coordinates).rgb * kernel[index];
        }
    }

    color = sum * intensity;
}
