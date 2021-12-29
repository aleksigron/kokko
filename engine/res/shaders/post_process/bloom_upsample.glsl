#version 450
#property source_map tex2d

#stage vertex
layout(location = 0) in vec3 ndc_pos;

out VS_TO_FS
{
	vec2 tex_coord;
}
vs_out;

void main()
{
	vs_out.tex_coord = ndc_pos.xy * 0.5 + vec2(0.5, 0.5);
	gl_Position = vec4(ndc_pos, 1.0);
}

#stage fragment
#include "engine/shaders/common/constants.glsl"

in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform BloomUpsampleBlock
{
    float kernel[25];
	vec2 texture_scale;
    int kernel_extent;
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

    color = sum;
}
