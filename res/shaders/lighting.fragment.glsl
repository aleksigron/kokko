#version 330 core

in vec3 fs_w_norm;

out vec3 color;

uniform vec3 base_color;

void main()
{
	vec3 light_0_dir = vec3(0.577, 0.577, 0.577);
	vec3 light_0_col = vec3(1.0, 1.0, 1.0);
	float intensity_0 = clamp(dot(fs_w_norm, light_0_dir), 0.0, 1.0);

	vec3 light_1_dir = vec3(-0.577, -0.577, -0.577);
	vec3 light_1_col = vec3(0.45, 0.45, 0.5);
	float intensity_1 = clamp(dot(fs_w_norm, light_1_dir), 0.0, 1.0);

	vec3 lit_color = light_0_col * intensity_0 + light_1_col * intensity_1;
	color = base_color * lit_color;
}
