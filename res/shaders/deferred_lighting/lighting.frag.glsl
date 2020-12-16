
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
uniform sampler2DShadow shd_smp;

const int DirLightIndex = 0;

const int shadow_sample_count = 4;
const float shadow_dist_factor = 0.0015;
vec2 poisson_disk[shadow_sample_count] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

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

vec3 unpack_normal(vec2 packed_normal)
{
	float a = (packed_normal.r - 0.5) * M_TAU;
	float d = cos((packed_normal.g - 0.5) * M_PI);
	return vec3(cos(a) * d, sin(a) * d, cos(packed_normal.g * M_PI));
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

void main()
{
	// Read input buffers
	vec3 albedo = texture(g_albedo, fs_in.tex_coord).rgb;
	vec3 N = unpack_normal(texture(g_normal, fs_in.tex_coord).rg);

	vec3 material = texture(g_material, fs_in.tex_coord).rgb;
	float metalness = material.r;
	float roughness = material.g;

	float window_z = texture(g_depth, fs_in.tex_coord).r;

	// Calculate position from window_z, pers_mat and fs_in.eye_dir

	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = perspective_mat[3][2] / ((perspective_mat[2][3] * ndc_z) - perspective_mat[2][2]);
	vec3 surface_pos = fs_in.eye_dir * -view_z;
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
		float NdotL = max(dot(N, L), 0.0);
		float s_bias = shadow_bias_offset + clamp(shadow_bias_factor * tan(acos(NdotL)), 0.0, shadow_bias_clamp);
		float compare_depth = shadow_coord.z - s_bias;
		float shadow = 0.0;

		for (int i = 0; i < shadow_sample_count; i++) {
			vec2 xy_coord = shadow_coord.xy + poisson_disk[i] * shadow_dist_factor;
			xy_coord.x = (cascade_index + xy_coord.x) / shadow_casc_count;
			vec3 coord = vec3(xy_coord, compare_depth);
			shadow += texture(shd_smp, coord);
		}

		shadow /= shadow_sample_count;
		
		Lo += calc_light(F0, N, V, L, albedo, light_col[DirLightIndex], metalness, roughness) * shadow;
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

	float ao = 1.0;
	vec3 ambient = ambient_color * albedo * ao;

	color = ambient + Lo;
	color = color / (color + vec3(1.0));

	gl_FragDepth = window_z;
}
