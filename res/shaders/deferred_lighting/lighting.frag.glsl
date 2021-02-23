
in VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
} fs_in;

out vec3 color;

uniform sampler2D g_albedo;
uniform sampler2D g_normal;
uniform sampler2D g_material;
uniform sampler2D g_depth;
uniform sampler2D ssao_map;
uniform sampler2DShadow shadow_map;

const int DirLightIndex = 0;

vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_ggx(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
	
    float num = a2;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
	
    return num / denom;
}

float geo_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float geometry_smith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = geo_schlick_ggx(NdotV, roughness);
    float ggx1  = geo_schlick_ggx(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 calc_light(vec3 F0, vec3 N, vec3 V, vec3 L, vec3 albedo, vec3 light_col, float metalness, float roughness)
{
	vec3 H = normalize(V + L);
	float NdotV = abs(dot(N, V)) + 1e-5; // avoid artifact
	float LdotH = clamp(dot(L, H), 0.0, 1.0);
	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
	float HdotV = clamp(dot(H, V), 0.0, 1.0);

	float NDF = distribution_ggx(NdotH, roughness);
	float G = geometry_smith(NdotV, NdotL, roughness);
	vec3 F = fresnel_schlick(HdotV, F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * NdotV * NdotL;
	vec3 specular = numerator / max(denominator, 0.001);

	vec3 kD = (vec3(1.0) - F) * (1.0 - metalness);

	return (kD * albedo / M_PI + specular) * light_col * NdotL;
}

float offset_lookup(sampler2DShadow map, vec4 loc, vec2 offset)
{
    return textureProj(map, vec4(loc.xy + offset * shadow_map_scale * loc.w, loc.z, loc.w));
}

void main()
{
	// Read input buffers
	vec3 albedo = texture(g_albedo, fs_in.tex_coord).rgb;
	vec3 N = unpack_normal(texture(g_normal, fs_in.tex_coord).rg);

	vec3 material = texture(g_material, fs_in.tex_coord).rgb;
	float metalness = material.r;
	float roughness = material.g;

	float window_z = texture(g_depth, fs_in.tex_coord).r;
	vec3 surface_pos = view_pos_from_depth(window_z, perspective_mat, fs_in.eye_dir);
	float view_z = surface_pos.z;
	vec3 V = normalize(-surface_pos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metalness);

	vec3 Lo = vec3(0.0);

	{
		// Select correct shadow cascade
		int cascade_index = 0;
		for (int i = 0; i < shadow_casc_count; ++i)
			if (shadow_splits[i] <= -view_z && shadow_splits[i + 1] > -view_z)
				cascade_index = i;

        vec3 L = -light_dir[DirLightIndex].xyz;

		// Get shadow depth
		vec4 shadow_coord = shadow_mats[cascade_index] * vec4(surface_pos, 1.0);
		shadow_coord.x = (cascade_index + shadow_coord.x) / shadow_casc_count;
		float NdotL = max(dot(N, L), 0.0);
		shadow_coord.z -= shadow_bias_offset + clamp(shadow_bias_factor * tan(acos(NdotL)), 0.0, shadow_bias_clamp);

		float shadow_coeff = 0.0;
		for (float y = -1.5; y <= 1.5; y += 1.0)
			for (float x = -1.5; x <= 1.5; x += 1.0)
				shadow_coeff += offset_lookup(shadow_map, shadow_coord, vec2(x, y)); 
				
		shadow_coeff /= 16.0;
		
		Lo += calc_light(F0, N, V, L, albedo, light_col[DirLightIndex], metalness, roughness) * shadow_coeff;
	}

	int idx = 1;
	for (int end = point_count + 1; idx < end; ++idx)
	{
		vec3 surface_to_light = light_pos[idx] - surface_pos;
        vec3 L = normalize(surface_to_light);

		float distance = length(surface_to_light);
		float attenuation = 1.0 / (distance * distance);

		Lo += calc_light(F0, N, V, L, albedo, light_col[idx], metalness, roughness) * attenuation;
	}
	
	for (; idx < point_count + spot_count + 1; ++idx)
	{
		vec3 surface_to_light = light_pos[idx] - surface_pos;
        vec3 L = normalize(surface_to_light);

		float distance = length(surface_to_light);
		float direction_asin = asin(dot(L, light_dir[idx].xyz));
		float direction_att = clamp((direction_asin - (M_HPI) + light_dir[idx].w) * 20, 0.0, 1.0);
		float distance_att = 1.0 / (distance * distance);
		float attenuation = direction_att * distance_att;
		
		Lo += calc_light(F0, N, V, L, albedo, light_col[idx], metalness, roughness) * attenuation;
	}

	float ao = texture(ssao_map, fs_in.tex_coord).r;
	vec3 ambient = ambient_color * albedo * ao;

	color = ambient + Lo;
	color = color / (color + vec3(1.0));

	gl_FragDepth = window_z;
}
