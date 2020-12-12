
in VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
} fs_in;

out vec3 color;

uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;
uniform sampler2D g_emissive;
uniform sampler2D g_depth;
uniform sampler2DShadow shd_smp;

const float SpecPower = 40.0;

const int DirLightIndex = 0;

const int shadow_sample_count = 4;
const float shadow_dist_factor = 0.0015;
vec2 poisson_disk[shadow_sample_count] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float calc_spec_factor(vec3 eye_dir, vec3 light_dir, vec3 normal)
{
	float dir_dot = dot(eye_dir, reflect(-light_dir, normal));
	return pow(max(dir_dot, 0.0), SpecPower);
}

vec3 unpack_normal(vec2 packed_normal)
{
	float a = (packed_normal.r - 0.5) * M_TAU;
	float d = cos((packed_normal.g - 0.5) * M_PI);
	return vec3(cos(a) * d, sin(a) * d, cos(packed_normal.g * M_PI));
}

void main()
{
	// Read input buffers
	vec4 albSpec = texture(g_alb_spec, fs_in.tex_coord);
	vec3 surface_norm = unpack_normal(texture(g_norm, fs_in.tex_coord).rg);
	float surface_emissivity = texture(g_emissive, fs_in.tex_coord).r;
	float window_z = texture(g_depth, fs_in.tex_coord).r;

	vec3 surface_albedo = albSpec.rgb;
	float surface_spec_int = albSpec.a;

	// Calculate position from window_z, pers_mat and fs_in.eye_dir

	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = perspective_mat[3][2] / ((perspective_mat[2][3] * ndc_z) - perspective_mat[2][2]);
	vec3 surface_pos = fs_in.eye_dir * -view_z;
	vec3 surface_to_eye = normalize(-surface_pos);

	color = ambient_color * surface_albedo;

	{
		// Select correct shadow cascade
		int cascade_index = 0;
		for (int i = 0; i < shadow_casc_count; ++i)
			if (shadow_splits[i] <= -view_z && shadow_splits[i + 1] > -view_z)
				cascade_index = i;

		vec3 dir_to_light = -light_dir[DirLightIndex].xyz;

		// Get shadow depth
		vec4 shadow_coord = shadow_mats[cascade_index] * vec4(surface_pos, 1.0);
		float normDotLightDir = max(dot(surface_norm, dir_to_light), 0.0);
		float s_bias = shadow_bias_offset + clamp(shadow_bias_factor * tan(acos(normDotLightDir)), 0.0, shadow_bias_clamp);
		float compare_depth = shadow_coord.z - s_bias;
		float shadow = 0.0;

		for (int i = 0; i < shadow_sample_count; i++) {
			vec2 xy_coord = shadow_coord.xy + poisson_disk[i] * shadow_dist_factor;
			xy_coord.x = (cascade_index + xy_coord.x) / shadow_casc_count;
			vec3 coord = vec3(xy_coord, compare_depth);
			shadow += texture(shd_smp, coord);
		}

		shadow /= shadow_sample_count;

		// Diffuse lighting

		vec3 diffuse = normDotLightDir * surface_albedo * light_col[DirLightIndex];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, dir_to_light, surface_norm);
		vec3 spec = light_col[DirLightIndex] * surface_spec_int * spec_factor;

		color += (diffuse + spec) * shadow;
	}

	int idx = 1;

	for (; idx < point_count + 1; ++idx)
	{
		vec3 surface_to_light = light_pos[idx] - surface_pos;
		float distance = length(surface_to_light);
		float attenuation = 1.0 / (1.0 + distance * distance);

		vec3 dir_to_light = normalize(surface_to_light);
		float light_dot = max(dot(surface_norm, dir_to_light), 0.0);
		vec3 diffuse = attenuation * light_dot * surface_albedo * light_col[idx];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, dir_to_light, surface_norm);
		vec3 spec = attenuation * surface_spec_int * spec_factor * light_col[idx];

		color += diffuse + spec;
	}
	
	for (; idx < point_count + spot_count + 1; ++idx)
	{
		vec3 surface_to_light = light_pos[idx] - surface_pos;
		float distance = length(surface_to_light);
		float attenuation = 1.0 / (1.0 + distance * distance);

		vec3 dir_to_light = normalize(surface_to_light);
		float light_dot = max(dot(surface_norm, dir_to_light), 0.0);
		vec3 diffuse = attenuation * light_dot * surface_albedo * light_col[idx];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, dir_to_light, surface_norm);
		vec3 spec = attenuation * surface_spec_int * spec_factor * light_col[idx];

		float dir_asin = asin(dot(dir_to_light, light_dir[idx].xyz));
		float dir_att = clamp((dir_asin - (M_HPI) + light_dir[idx].w) * 20, 0.0, 1.0);

		color += (diffuse + spec) * dir_att;
	}

	// Emissive lighting
	color += surface_emissivity * surface_albedo;

	gl_FragDepth = window_z;
}
