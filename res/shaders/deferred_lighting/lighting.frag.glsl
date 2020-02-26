#version 330 core

#define PI 3.1415926538

#define MAX_LIGHT_COUNT 10
#define DIR_LIGHT_INDEX 0

#define MAX_CASCADE_COUNT 4

#define SPEC_POWER 40
#define ATT_CONST_FAC 1
#define ATT_LIN_FAC 0.5

in vec2 tex_coord;
in vec3 eye_dir;

out vec3 color;

uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;
uniform sampler2D g_emissive;
uniform sampler2D g_depth;

const float att_const_fac = 1.0;
const float att_lin_fac = 0.5;

uniform mat4x4 pers_mat;

uniform struct 
{
	int cascade_count;
	float splits[MAX_CASCADE_COUNT + 1];
	mat4x4 matrices[MAX_CASCADE_COUNT];
	sampler2DShadow samplers[MAX_CASCADE_COUNT];
}
shadow_params;

uniform struct
{
	vec3 col[MAX_LIGHT_COUNT];
	vec3 pos[MAX_LIGHT_COUNT];
	vec3 dir[MAX_LIGHT_COUNT];
	float angle[MAX_LIGHT_COUNT];
	int point_count;
	int spot_count;
}
lights;

const int shadow_sample_count = 4;
const float shadow_dist_factor = 0.001;
vec2 poisson_disk[shadow_sample_count] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float calc_spec_factor(vec3 eye_dir, vec3 light_dir, vec3 normal)
{
	float dir_dot = dot(eye_dir, reflect(-light_dir, normal));
	return pow(max(dir_dot, 0.0), SPEC_POWER);
}

vec3 unpack_normal(vec2 packed_normal)
{
	float a = (packed_normal.r - 0.5) * 2.0 * PI;
	float d = cos((packed_normal.g - 0.5) * PI);
	return vec3(cos(a) * d, sin(a) * d, cos(packed_normal.g * PI));
}

void main()
{
	// Read input buffers
	vec4 albSpec = texture(g_alb_spec, tex_coord);
	vec3 surface_norm = unpack_normal(texture(g_norm, tex_coord).rg);
	float surface_emissivity = texture(g_emissive, tex_coord).r;
	float window_z = texture(g_depth, tex_coord).r;

	vec3 surface_albedo = albSpec.rgb;
	float surface_spec_int = albSpec.a;

	// Calculate position from window_z, pers_mat and eye_dir

	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = pers_mat[3][2] / ((pers_mat[2][3] * ndc_z) - pers_mat[2][2]);
	vec3 surface_pos = eye_dir * -view_z;
	vec3 surface_to_eye = normalize(-surface_pos);

	color = vec3(0.0, 0.0, 0.0);

	{
		// Select correct shadow cascade
		int cascade_index = 0;
		for (int i = 0; i < shadow_params.cascade_count; ++i)
			if (shadow_params.splits[i] <= -view_z && shadow_params.splits[i + 1] > -view_z)
				cascade_index = i;

		// Get shadow depth
		vec4 shadow_coord = shadow_params.matrices[cascade_index] * vec4(surface_pos, 1.0);
		float normDotLightDir = max(dot(surface_norm, -lights.dir[DIR_LIGHT_INDEX]), 0.0);
		float compare_depth = shadow_coord.z - clamp(0.001875 * tan(acos(normDotLightDir)), 0, 0.005);
		float shadow = 0.0;

		for (int i = 0; i < shadow_sample_count; i++) {
			vec3 coord = vec3(shadow_coord.xy + poisson_disk[i] * shadow_dist_factor, compare_depth);
			shadow += texture(shadow_params.samplers[cascade_index], coord);
		}

		shadow /= shadow_sample_count;

		// Diffuse lighting

		vec3 diffuse = normDotLightDir * surface_albedo * lights.col[DIR_LIGHT_INDEX];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, -lights.dir[DIR_LIGHT_INDEX], surface_norm);
		vec3 spec = lights.col[DIR_LIGHT_INDEX] * surface_spec_int * spec_factor;

		color += (diffuse + spec) * shadow;
	}

	int idx = 1;

	for (; idx < lights.point_count + 1; ++idx)
	{
		vec3 surface_to_light = lights.pos[idx] - surface_pos;
		float distance = length(surface_to_light);
		float attenuation = 1.0 / (ATT_CONST_FAC + ATT_LIN_FAC * distance + distance * distance);

		vec3 light_dir = normalize(surface_to_light);
		float light_dot = max(dot(surface_norm, light_dir), 0.0);
		vec3 diffuse = attenuation * light_dot * surface_albedo * lights.col[idx];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, light_dir, surface_norm);
		vec3 spec = attenuation * surface_spec_int * spec_factor * lights.col[idx];

		color += diffuse + spec;
	}
	
	for (; idx < lights.point_count + lights.spot_count + 1; ++idx)
	{
		vec3 surface_to_light = lights.pos[idx] - surface_pos;
		float distance = length(surface_to_light);
		float attenuation = 1.0 / (ATT_CONST_FAC + ATT_LIN_FAC * distance + distance * distance);

		vec3 light_dir = normalize(surface_to_light);
		float light_dot = max(dot(surface_norm, light_dir), 0.0);
		vec3 diffuse = attenuation * light_dot * surface_albedo * lights.col[idx];

		// Specular lighting

		float spec_factor = calc_spec_factor(surface_to_eye, light_dir, surface_norm);
		vec3 spec = attenuation * surface_spec_int * spec_factor * lights.col[idx];

		float dir_att = clamp(cos(dot(light_dir, lights.dir[idx])) - lights.angle[idx] * 100, 0, 1);

		color += (diffuse + spec) * dir_att;
	}

	// Emissive lighting
	color += surface_emissivity * surface_albedo;

	gl_FragDepth = window_z;
}
