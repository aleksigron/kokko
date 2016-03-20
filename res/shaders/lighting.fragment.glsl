#version 330 core

in vec3 fs_w_norm;

out vec3 color;

void main()
{
	vec3 light_dir = vec3(0.577, 0.577, 0.577);
	float f = dot(fs_w_norm, light_dir);

	color = vec3(1.0, 1.0, 1.0) * f;
}
