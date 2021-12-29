#version 450
#property occlusion_map tex2d

#stage vertex
#include "engine/shaders/common/constants.glsl"

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
