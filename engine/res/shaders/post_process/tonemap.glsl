#version 450
#property light_acc_map tex2d

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

uniform sampler2D light_acc_map;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform TonemapBlock
{
	float exposure;
};

void main()
{
    vec3 linear = texture(light_acc_map, fs_in.tex_coord).rgb;
    color = vec3(1.0) - exp(-linear * exposure);
}
