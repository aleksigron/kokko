#version 330 core

struct DirectionalLight
{
	vec3 inverse_dir;
	vec3 color;
};

in vec2 tex_coord;
in vec3 eye_dir;

out vec3 color;

uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;
uniform sampler2D g_depth;

uniform mat4x4 shadow_mat;
uniform sampler2D shadow_depth;

uniform mat4x4 pers_mat;
uniform DirectionalLight light;

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

	// Get shadow depth
	vec4 shadow_coord = shadow_mat * vec4(view_pos, 1.0);
	float shadow_z = texture(shadow_depth, shadow_coord.xy).r;

	// Diffuse lighting

    vec3 alb = albSpec.rgb;
	float normDotLightDir = max(dot(norm, light.inverse_dir), 0.0);
	vec3 diffuse = normDotLightDir * alb * light.color;

	// Specular lighting

	vec3 eyeDir = normalize(-view_pos);
	vec3 refl = reflect(light.inverse_dir, norm);
	float eyeDirDotRefl = max(dot(eyeDir, refl), 0.0);
	float spec_power = 40;
	float spec_factor = pow(eyeDirDotRefl, spec_power);
	float spec_int = albSpec.a;

	vec3 spec = light.color * spec_int * spec_factor;
	
	const float shadow_bias = 0.002;
	float shadow = max(0.0, sign(shadow_z - shadow_coord.z + shadow_bias));

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
}
