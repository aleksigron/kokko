#version 330 core

in vec3 fs_v_norm;

out vec3 color;

uniform mat4x4 _V;

uniform vec3 base_color;

void main()
{
	vec3 nor = normalize(fs_v_norm);

	vec3 light_0_dir = (_V * vec4(0.577, 0.577, 0.577, 0.0)).xyz;
	vec3 light_0_col = vec3(1.0, 1.0, 1.0);
	float intensity_0 = clamp(dot(nor, light_0_dir), 0.0, 1.0);

	vec3 light_1_dir = (_V * vec4(-0.577, 0.577, -0.577, 0.0)).xyz;
	vec3 light_1_col = vec3(0.45, 0.45, 0.55);
	float intensity_1 = clamp(dot(nor, light_1_dir), 0.0, 1.0);

	vec3 lit_color = light_0_col * intensity_0 + light_1_col * intensity_1;
	color = base_color * lit_color;
}
