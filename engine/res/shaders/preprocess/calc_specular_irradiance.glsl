#version 450
#property environment_map texCube

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/viewport_block.glsl"

layout (location = VERTEX_ATTR_INDEX_POS) in vec3 position;

out VS_TO_FS
{
	vec3 local_pos;
}
vs_out;

void main()
{
    gl_Position = viewport.VP * vec4(position, 1.0);
    vs_out.local_pos = position;
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/preprocess/importance_sample.glsl"

out vec3 out_color;

in VS_TO_FS
{
	vec3 local_pos;
}
fs_in;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform CalcSpecularBlock
{
	float roughness;
}
uniforms;

uniform samplerCube environment_map;

void main()
{
	vec3 N = normalize(fs_in.local_pos);
	vec3 V = N;

	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 H  = importance_sample_ggx(Xi, N, uniforms.roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			prefilteredColor += texture(environment_map, L).rgb * NdotL;
			totalWeight      += NdotL;
		}
	}

	out_color = prefilteredColor / totalWeight;
}