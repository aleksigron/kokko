#version 330 core

struct DirectionalLight
{
	vec3 inverse_dir;
	vec3 color;
};

in vec2 tex_coord;
in vec3 view_dir;

out vec3 color;

uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;
uniform sampler2D g_depth;

uniform mat4 pers_mat;
uniform DirectionalLight light;

void main()
{
	// Read input buffers
	float window_z = texture(g_depth, tex_coord).r;
    vec3 norm = texture(g_norm, tex_coord).xyz;
	vec4 albSpec = texture(g_alb_spec, tex_coord);

	// Calculate position from window_z, pers_mat and view_dir

	//float ndc_z = (2.0 * window_z - (near - far)) / (far - near);
	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = pers_mat[3][2] / ((pers_mat[2][3] * ndc_z) - pers_mat[2][2]);
	vec3 pos = view_dir * view_z;

	// Diffuse lighting

    vec3 alb = albSpec.rgb;
	vec3 diffuse = max(dot(norm, light.inverse_dir), 0.0) * alb * light.color;

	// Specular lighting

	vec3 vert_to_eye = normalize(-pos);
	vec3 refl = normalize(reflect(light.inverse_dir, norm));
	float spec_power = 40;
	float spec_factor = pow(dot(vert_to_eye, refl), spec_power);
	float spec_int = albSpec.a;

	vec3 spec = light.color * spec_int * spec_factor;

	// Output
	
	gl_FragDepth = window_z;
	color = diffuse + spec;

	// Visualizations

    //color = vec3(pos * 0.1 + vec3(0.5));
	//color = vec3(norm * 0.5 + vec3(0.5));
}  