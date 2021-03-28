
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
	//vec3 R = N;
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