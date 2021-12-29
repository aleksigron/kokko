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
#include "engine/shaders/post_process/bloom_common.glsl"

in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform BloomExtractBlock
{
	vec2 texture_scale;
	float threshold;
    float soft_threshold;
};

vec3 prefilter(vec3 c)
{
    float brightness = max(c.r, max(c.g, c.b));
    float knee = threshold * soft_threshold;
    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0, 2 * knee);
    soft = soft * soft / (4 * knee + 0.00001);
    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 0.00001);
    return c * contribution;
}

void main()
{
    color = prefilter(sample_box(source_map, texture_scale, fs_in.tex_coord, 1.0));
}
