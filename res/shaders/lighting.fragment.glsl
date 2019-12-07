#version 330 core

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
};

in vec2 tex_coord;
in vec3 eye_dir;

out vec3 color;

uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;
uniform sampler2D g_depth;

const int max_cascade_count = 4;

struct ShadowParameters
{
	int cascade_count;
	float splits[max_cascade_count + 1];
	mat4x4 matrices[max_cascade_count];
	sampler2DShadow samplers[max_cascade_count];
};

uniform ShadowParameters shadow_params;

uniform mat4x4 pers_mat;
uniform DirectionalLight light;

const int shadow_sample_count = 4;
const float shadow_dist_factor = 0.001;
vec2 poisson_disk[shadow_sample_count] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

void main()
{
	// Read input buffers
	float window_z = texture(g_depth, tex_coord).r;
    vec3 norm = texture(g_norm, tex_coord).xyz;
	vec4 albSpec = texture(g_alb_spec, tex_coord);

	// Calculate position from window_z, pers_mat and eye_dir

	//float ndc_z = (2.0 * window_z - near - far) / (far - near);
	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = pers_mat[3][2] / ((pers_mat[2][3] * ndc_z) - pers_mat[2][2]);
	vec3 view_pos = eye_dir * -view_z;

	// Select correct shadow cascade
	int cascade_index = 0;
	for (int i = 0; i < shadow_params.cascade_count; ++i)
		if (shadow_params.splits[i] <= -view_z && shadow_params.splits[i + 1] > -view_z)
			cascade_index = i;

	// Get shadow depth
	vec4 shadow_coord = shadow_params.matrices[cascade_index] * vec4(view_pos, 1.0);
	float normDotLightDir = max(dot(norm, -light.direction), 0.0);
	float compare_depth = shadow_coord.z - clamp(0.002 * tan(acos(normDotLightDir)), 0, 0.005);
	float shadow = 0.0;

	for (int i = 0; i < shadow_sample_count; i++) {
		vec3 coord = vec3(shadow_coord.xy + poisson_disk[i] * shadow_dist_factor, compare_depth);
		shadow += texture(shadow_params.samplers[cascade_index], coord);
	}

	shadow /= shadow_sample_count;

	// Diffuse lighting

    vec3 alb = albSpec.rgb;
	vec3 diffuse = normDotLightDir * alb * light.color;

	// Specular lighting

	vec3 eyeDir = normalize(-view_pos);
	vec3 refl = reflect(light.direction, norm);
	float eyeDirDotRefl = max(dot(eyeDir, refl), 0.0);
	float spec_power = 40;
	float spec_factor = pow(eyeDirDotRefl, spec_power);
	float spec_int = albSpec.a;

	vec3 spec = light.color * spec_int * spec_factor;

	// Output
	
	gl_FragDepth = window_z;
	color = (diffuse + spec) * shadow;

	// Visualizations

    //color = view_pos * 0.05 + vec3(0.5, 0.5, 1.0);
	//color = vec3(view_pos.x * 0.05 + 0.5); // View-space position X
	//color = vec3(view_pos.y * 0.05 + 0.5); // View-space position Y
	//color = vec3(view_pos.z * 0.05 + 1.0); // View-space position Z

	//color = vec3(shadow_coord.x); // Light-space position X
	//color = vec3(shadow_coord.y); // Light-space position Y
	//color = vec3(shadow_coord.z); // Light-space position Z
	//color = vec3(shadow_z);

	//color = vec3(norm * 0.5 + vec3(0.5));
	//color = vec3(window_z);
	//color = vec3(view_z * -0.02);
}
